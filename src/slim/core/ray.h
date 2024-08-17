#pragma once

#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/mat3.h"
#include "../math/quat.h"
#include "./transform.h"


struct RayHit {
    vec3 position, normal;
    UV uv;
    f32 distance, uv_coverage, cone_width, NdotRd;
    f32 scaling_factor;
    u32 id;
    bool from_behind = false;
};

struct Ray {
    vec3 origin, scaled_origin, direction, direction_reciprocal;
    vec2i pixel_coords;
    Sides faces;
    OctantShifts octant_shifts;
    u8 depth;

    INLINE_XPU vec3 at(f32 t) const { return direction.scaleAdd(t, origin); }
    INLINE_XPU vec3 operator [](f32 t) const { return at(t); }

    INLINE_XPU void localize(const vec3 &ray_origin, const vec3 &ray_direction, const Transform &transform) {
        vec3 inv_scale = 1.0f / transform.scale;
        quat inv_rotation = transform.orientation.conjugate();
        reset(inv_scale * (inv_rotation * (ray_origin - transform.position)),
              inv_scale * (inv_rotation * ray_direction));
    }

    INLINE_XPU void localize(const Ray &ray, const Transform &transform) {
        localize(ray.origin, ray.direction, transform);
    }

    INLINE_XPU void reset(const vec3 &new_origin, const vec3 &new_direction) {
        origin = new_origin;
        direction = new_direction;

        direction_reciprocal = 1.0f / direction;
        scaled_origin = -origin * direction_reciprocal;

        faces = direction.facing();
        octant_shifts = faces;
    }

    INLINE_XPU bool hitsAABB(const AABB &aabb, f32 &near_distance, f32 &far_distance) const {
        vec3 min_t{*(&aabb.min.x + octant_shifts.x), *(&aabb.min.y + octant_shifts.y), *(&aabb.min.z + octant_shifts.z)};
        vec3 max_t{*(&aabb.max.x - octant_shifts.x), *(&aabb.max.y - octant_shifts.y), *(&aabb.max.z - octant_shifts.z)};
        min_t = min_t.mulAdd(direction_reciprocal, scaled_origin);
        max_t = max_t.mulAdd(direction_reciprocal, scaled_origin);
        near_distance = Max(0, min_t.maximum());
        far_distance = max_t.minimum();
        return near_distance <= far_distance;
    }

    INLINE_XPU bool hitsPlane(const vec3 &plane_origin, const vec3 &plane_normal, RayHit &hit) const {
        f32 NdotRd = plane_normal.dot(direction);
        if (NdotRd == 0) // The ray is parallel to the plane
            return false;

        f32 NdotRoP = plane_normal.dot(plane_origin - origin);
        if (NdotRoP == 0) // The ray originated within the plane
            return false;

        bool ray_is_facing_the_plane = NdotRd < 0;
        hit.from_behind = NdotRoP > 0;
        if (hit.from_behind == ray_is_facing_the_plane) // The ray can't hit the plane
            return false;

        f32 t =  NdotRoP / NdotRd;

        hit.distance = t;
        hit.position = at(t);
        hit.normal = plane_normal;

        return true;
    }

    INLINE_XPU bool hitsDefaultQuad(RayHit &hit, bool is_transparent = false) const {
        if (direction.y == 0) // The ray is parallel to the plane
            return false;

        if (origin.y == 0) // The ray start in the plane
            return false;

        hit.from_behind = origin.y < 0;
        if (hit.from_behind == direction.y < 0) // The ray can't hit the plane
            return false;

        f32 t = fabsf(origin.y * direction_reciprocal.y);
        if (t > hit.distance)
            return false;

        hit.position = at(t);
        if (hit.position.x < -1 ||
            hit.position.x > +1 ||
            hit.position.z < -1 ||
            hit.position.z > +1) {
            return false;
        }

        hit.uv.x = hit.position.x;
        hit.uv.y = hit.position.z;
        hit.uv.shiftToNormalized();

        if (is_transparent && hit.uv.onCheckerboard())
            return false;

        hit.distance = t;
        hit.normal.x = hit.normal.z = 0.0f;
        hit.normal.y = 1.0f;
        hit.uv_coverage = 0.25f;

        return true;
    }

    INLINE_XPU BoxSide hitsDefaultBox(RayHit &hit, bool is_transparent = false) const {
        vec3 signed_rcp{faces};
        signed_rcp *= direction_reciprocal;
        vec3 near_t{scaled_origin - signed_rcp};
        vec3 far_t{scaled_origin + signed_rcp};

        BoxSide side;
        Axis near_axis, far_axis;
        f32 far_hit_t = far_t.minimum(far_axis);
        if (far_hit_t < 0) // Further-away hit is behind the ray - tracers can not occur.
            return BoxSide_None;

        f32 near_hit_t = near_t.maximum(near_axis);
        if (near_hit_t > hit.distance || far_hit_t < (near_hit_t > 0 ? near_hit_t : 0))
            return BoxSide_None;

        f32 t = near_hit_t;
        hit.from_behind = near_hit_t < 0; // Near hit is behind the ray, far hit is in front of it: hit is from behind
        if (hit.from_behind) {
            if (far_hit_t > hit.distance)
                return BoxSide_None;

            t = far_hit_t;
            side = OctantShifts{faces.flipped()}.getBoxSide(far_axis);
        } else
            side = octant_shifts.getBoxSide(near_axis);

        hit.position = at(t);
        hit.uv.setByBoxSide(side, hit.position.x, hit.position.y, hit.position.z);

        if (is_transparent && hit.uv.onCheckerboard()) {
            if (hit.from_behind || far_hit_t > hit.distance)
                return BoxSide_None;

            side = OctantShifts{faces.flipped()}.getBoxSide(far_axis);;
            hit.from_behind = true;
            t = far_hit_t;
            hit.position = at(t);
            hit.uv.setByBoxSide(side, hit.position.x, hit.position.y, hit.position.z);
            if (hit.uv.onCheckerboard())
                return BoxSide_None;

        }

        hit.distance = t;
        hit.normal.x = (f32)((i32)(side == BoxSide_Right) - (i32)(side == BoxSide_Left));
        hit.normal.y = (f32)((i32)(side == BoxSide_Top) - (i32)(side == BoxSide_Bottom));
        hit.normal.z = (f32)((i32)(side == BoxSide_Front) - (i32)(side == BoxSide_Back));
//        if (hit.from_behind) hit.normal = -hit.normal;

        hit.uv_coverage = 0.25f;

        return side;
    }

    INLINE_XPU bool hitsDefaultSphere(RayHit &hit, bool is_transparent = false, f32 *out_squared_distance_to_center = nullptr) const {
        f32 t_to_closest = -(origin.dot(direction));
        if (t_to_closest <= 0) // Ray is aiming away from the sphere
            return false;

        f32 direction_squared_length = direction.squaredLength();
        f32 squared_distance_to_center = origin.squaredLength() * direction_squared_length - t_to_closest*t_to_closest;
        if (squared_distance_to_center > 1.0f) // Ray missed the sphere
            return false;

        f32 delta = sqrtf( direction_squared_length - squared_distance_to_center);
        direction_squared_length = 1.0f / direction_squared_length;
        f32 t = (t_to_closest - delta) * direction_squared_length;
        if (t > hit.distance)
            return false;

        hit.normal = at(t);
        hit.uv.setBySphere(hit.normal.x, hit.normal.y, hit.normal.z);

        hit.from_behind = t <= 0 || (is_transparent && hit.uv.onCheckerboard());
        if (hit.from_behind) {
            t = (t_to_closest + delta) * direction_squared_length;
            if (t <= 0 || t > hit.distance)
                return false;

            hit.normal = at(t);
            hit.uv.setBySphere(hit.normal.x, hit.normal.y, hit.normal.z);
            if (is_transparent && hit.uv.onCheckerboard())
                return false;
        }

        hit.distance = t;
        hit.uv_coverage = 1.0f / UNIT_SPHERE_AREA_OVER_SIX;

        if (out_squared_distance_to_center)
            *out_squared_distance_to_center = squared_distance_to_center * direction_squared_length;

        return true;
    }

    INLINE_XPU bool hitsDefaultTetrahedron(RayHit &hit, bool is_transparent = false) const {
        mat3 tangent_matrix;
        vec3 tangent_pos;
        vec3 face_normal;
        bool found_triangle = false;

        RayHit current_hit;
        current_hit.uv_coverage = SQRT3 / 4.0f;

        for (u8 t = 0; t < 4; t++) {
            tangent_pos = face_normal = vec3{t == 3 ? TET_MAX : -TET_MAX};
            switch (t) {
                case 0: face_normal.y = TET_MAX; break;
                case 1: face_normal.x = TET_MAX; break;
                case 2: face_normal.z = TET_MAX; break;
                case 3: tangent_pos.y = -TET_MAX; break;
            }

            current_hit.distance = hit.distance;
            if (!hitsPlane(tangent_pos, face_normal, current_hit))
                continue;

            switch (t) {
                case 0:
                    tangent_matrix.X.x = TET_MAX;
                    tangent_matrix.X.y = -TET_MIN;
                    tangent_matrix.X.z = -TET_MAX;

                    tangent_matrix.Y.x = TET_MIN;
                    tangent_matrix.Y.y = TET_MIN;
                    tangent_matrix.Y.z = TET_MAX;

                    tangent_matrix.Z.x = -TET_MIN;
                    tangent_matrix.Z.y = TET_MAX;
                    tangent_matrix.Z.z = -TET_MAX;
                    break;
                case 1:
                    tangent_matrix.X.x = TET_MIN;
                    tangent_matrix.X.y = TET_MIN;
                    tangent_matrix.X.z = TET_MAX;

                    tangent_matrix.Y.x = -TET_MIN;
                    tangent_matrix.Y.y = TET_MAX;
                    tangent_matrix.Y.z = -TET_MAX;

                    tangent_matrix.Z.x = TET_MAX;
                    tangent_matrix.Z.y = -TET_MIN;
                    tangent_matrix.Z.z = -TET_MAX;
                    break;
                case 2:
                    tangent_matrix.X.x = -TET_MIN;
                    tangent_matrix.X.y = TET_MAX;
                    tangent_matrix.X.z = -TET_MAX;

                    tangent_matrix.Y.x = TET_MAX;
                    tangent_matrix.Y.y = -TET_MIN;
                    tangent_matrix.Y.z = -TET_MAX;

                    tangent_matrix.Z.x = TET_MIN;
                    tangent_matrix.Z.y = TET_MIN;
                    tangent_matrix.Z.z = TET_MAX;
                    break;
                case 3:
                    tangent_matrix.X.x = -TET_MAX;
                    tangent_matrix.X.y = TET_MIN;
                    tangent_matrix.X.z = TET_MAX;

                    tangent_matrix.Y.x = TET_MIN;
                    tangent_matrix.Y.y = TET_MIN;
                    tangent_matrix.Y.z = TET_MAX;

                    tangent_matrix.Z.x = TET_MIN;
                    tangent_matrix.Z.y = -TET_MAX;
                    tangent_matrix.Z.z = TET_MAX;
                    break;
            }

            tangent_pos = tangent_matrix * (current_hit.position - tangent_pos);
            if (tangent_pos.x < 0 || tangent_pos.y < 0 || tangent_pos.y + tangent_pos.x > 1)
                continue;

            current_hit.uv.x = tangent_pos.x;
            current_hit.uv.y = tangent_pos.y;

            if (is_transparent && current_hit.uv.onCheckerboard())
                continue;

            if (current_hit.distance < hit.distance) {
                hit = current_hit;
                found_triangle = true;
            }
        }

        return found_triangle;
    }
};

struct SphereTracer {
    f32 b, c, t_near, t_far, t_max;

    INLINE_XPU bool hit(const vec3 &center, f32 radius, f32 one_over_radius, const vec3 &Ro, const vec3 &Rd, f32 &max_distance) {
        t_max = max_distance * one_over_radius;
        vec3 rc{(center - Ro) * one_over_radius};

        b = Rd.dot(rc);
        c = rc.squaredLength() - 1;
        f32 h = b*b - c;

        if (h >= 0) {
            h = sqrtf(h);
            t_near = b - h;
            t_far  = b + h;
            if (t_far > 0 && t_near < t_max) {
                max_distance = t_max * radius;
                return true;
            }
        }

        return false;
    }

    INLINE_XPU f32 integrateDensity() {
        f32 tn = Max(t_near, 0);
        f32 tf = Min(t_far, t_max);
        f32 tn2 = tn*tn;
        f32 tf2 = tf*tf;
        return (c*tn - b*tn2 + tn*tn2/3.0f - (
                c*tf - b*tf2 + tf*tf2/3.0f   ))
                * (3.0f / 4.0f);
    }
};
