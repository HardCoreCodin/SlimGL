#pragma once

#include "./scene.h"
#include "./mesh_tracer.h"

struct SceneTracer {
    SphereTracer sphere_tracer{};
    MeshTracer mesh_tracer{nullptr};
    u32 *stack{nullptr};
    Ray aux_ray;
    RayHit aux_hit;

    INLINE_XPU SceneTracer(u32 *stack, u32 *mesh_stack) : mesh_tracer{mesh_stack}, stack{stack} {}

    explicit SceneTracer(u32 stack_size, u32 mesh_stack_size, memory::MonotonicAllocator *memory_allocator = nullptr) {
        memory::MonotonicAllocator temp_allocator;
        if (!memory_allocator) {
            temp_allocator = memory::MonotonicAllocator{sizeof(u32) * (mesh_stack_size + stack_size)};
            memory_allocator = &temp_allocator;
        }

        stack = (u32*)memory_allocator->allocate(sizeof(u32) * stack_size);
        mesh_tracer = MeshTracer{mesh_stack_size, memory_allocator};
    }

    XPU Geometry* trace(Ray &ray, RayHit &hit, const Scene &scene, bool any_hit = false, f32 max_distance = INFINITY) {
        ray.reset(ray.direction.scaleAdd(TRACE_OFFSET, ray.origin), ray.direction);
        hit.distance = max_distance;

        bool hit_left, hit_right;
        f32 left_near_distance, right_near_distance, left_far_distance, right_far_distance;
        if (!(ray.hitsAABB(scene.bvh.nodes->aabb, left_near_distance, left_far_distance) && left_near_distance < hit.distance))
            return nullptr;

        u32 *indices = scene.bvh_leaf_geometry_indices;
        if (unlikely(scene.bvh.nodes->leaf_count))
            return hitGeometries(indices, scene.counts.geometries, scene, left_far_distance, ray, hit, any_hit);

        BVHNode *left_node = scene.bvh.nodes + scene.bvh.nodes->first_index;
        BVHNode *right_node, *tmp_node;
        Geometry *hit_geo, *closest_hit_geo = nullptr;
        u32 top = 0;

        while (true) {
            right_node = left_node + 1;

            hit_left  = ray.hitsAABB(left_node->aabb, left_near_distance, left_far_distance) && left_near_distance < hit.distance;
            hit_right = ray.hitsAABB(right_node->aabb, right_near_distance, right_far_distance) && right_near_distance < hit.distance;

            if (hit_left) {
                if (unlikely(left_node->leaf_count)) {
                    hit_geo = hitGeometries(indices + left_node->first_index, left_node->leaf_count,
                                            scene, left_far_distance, ray, hit, any_hit);
                    if (hit_geo) {
                        closest_hit_geo = hit_geo;
                        if (any_hit)
                            break;
                    }

                    left_node = nullptr;
                }
            } else
                left_node = nullptr;

            if (hit_right) {
                if (unlikely(right_node->leaf_count)) {
                    hit_geo = hitGeometries(indices + right_node->first_index, right_node->leaf_count,
                                            scene, right_far_distance, ray, hit, any_hit);
                    if (hit_geo) {
                        closest_hit_geo = hit_geo;
                        if (any_hit)
                            break;
                    }
                    right_node = nullptr;
                }
            } else
                right_node = nullptr;

            if (left_node) {
                if (right_node) {
                    if (!any_hit && left_near_distance > right_near_distance) {
                        tmp_node = left_node;
                        left_node = right_node;
                        right_node = tmp_node;
                    }
                    stack[top++] = right_node->first_index;
                }
                left_node = scene.bvh.nodes + left_node->first_index;
            } else if (right_node) {
                left_node = scene.bvh.nodes + right_node->first_index;
            } else {
                if (top == 0) break;
                left_node = scene.bvh.nodes + stack[--top];
            }
        }

        return closest_hit_geo;
    }
    
    XPU bool hitLight(const BaseLight *light, Ray &ray, RayHit &hit) {
        return sphere_tracer.hit(
            light->position,
            1.0f,
            ray.origin,
            ray.direction,
            hit.distance
        );
    }

    XPU Geometry* hitGeometries(const u32 *geometry_indices, u32 geo_count, const Scene &scene, f32 closest_distance, const Ray &ray, RayHit &hit, bool any_hit) {
        Geometry *geo, *hit_geo = nullptr;
        u8 visibility_flag = any_hit ? GEOMETRY_IS_SHADOWING : GEOMETRY_IS_VISIBLE;

        aux_hit.distance = Min(closest_distance + EPS, hit.distance);

        for (u32 i = 0; i < geo_count; i++) {
            geo = scene.geometries + geometry_indices[i];

            if (!(geo->flags & visibility_flag))
                continue;

            if (hitGeometryInLocalSpace(*geo, scene.meshes, ray, aux_hit, any_hit)) {
                if (any_hit)
                    return geo;

                if (aux_hit.distance < hit.distance) {
                    hit_geo = geo;
                    hit = aux_hit;
                    hit.NdotRd = -(hit.normal.dot(aux_ray.direction));
                }
            }
        }

        return hit_geo;
    }

    INLINE_XPU bool hitGeometryInLocalSpace(const Geometry &geo, const Mesh *meshes, const Ray &ray, RayHit &hit, bool any_hit = false) {
        aux_ray.localize(ray, geo.transform);
        aux_ray.pixel_coords = ray.pixel_coords;
        aux_ray.depth = ray.depth;
        f32 n, f;
        AABB aabb;

        if (geo.type == GeometryType_Mesh) {
            aabb = meshes[geo.id].aabb;
        } else {
            aabb.max = geo.type == GeometryType_Tet ? TET_MAX : 1.0f;
            aabb.min = -aabb.max.x;
            if (geo.type == GeometryType_Quad) {
                aabb.min.y = -EPS;
                aabb.max.y = EPS;
            }
        }
        if (!aux_ray.hitsAABB(aabb, n, f)) return false;

        switch (geo.type) {
            case GeometryType_Quad: return aux_ray.hitsDefaultQuad(hit, geo.flags & GEOMETRY_IS_TRANSPARENT);
            case GeometryType_Box: return aux_ray.hitsDefaultBox(hit, geo.flags & GEOMETRY_IS_TRANSPARENT);
            case GeometryType_Sphere: return aux_ray.hitsDefaultSphere(hit, geo.flags & GEOMETRY_IS_TRANSPARENT);
            case GeometryType_Tet   : return aux_ray.hitsDefaultTetrahedron(hit, geo.flags & GEOMETRY_IS_TRANSPARENT);
            case GeometryType_Mesh  : return mesh_tracer.trace(meshes[geo.id], aux_ray, hit, any_hit);
            default: return false;
        }
    }
};