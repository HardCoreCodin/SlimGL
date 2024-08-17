#pragma once

#include "./scene.h"
#include "./scene_tracer.h"
#include "../core/ray.h"
#include "../core/transform.h"
#include "../viewport/viewport.h"


struct Selection {
    Scene &scene;
    SceneTracer &scene_tracer;
    CameraRayProjection &projection;
    Ray ray, local_ray;
    RayHit hit, local_ray_hit;

    Transform xform;
    quat object_rotation;
    vec3 transformation_plane_origin,
         transformation_plane_normal,
         transformation_plane_center,
         object_scale,
         world_offset,
         *world_position{nullptr};
    Geometry *geometry{nullptr};
    Light *light{nullptr};
    f32 object_distance = 0;

    BoxSide box_side = BoxSide_None;
    bool changed = false;
    bool left_mouse_button_was_pressed = false;

    Selection(Scene &scene, SceneTracer &scene_tracer, CameraRayProjection &projection) :
        scene{scene}, scene_tracer{scene_tracer}, projection{projection} {}

    void manipulate(const Viewport &viewport) {
        const Dimensions &dimensions = viewport.dimensions;
        Camera &camera = *viewport.camera;
        i32 x = mouse::pos_x - viewport.bounds.left;
        i32 y = mouse::pos_y - viewport.bounds.top;
        vec3 new_origin{camera.position};
        vec3 new_direction{projection.getRayDirectionAt(x, y).normalized()};

        if (mouse::left_button.is_pressed && !left_mouse_button_was_pressed && !controls::is_pressed::alt) {
            // This is the first frame after the left mouse button went down:
            // Cast a ray onto the scene to find the closest object behind the hovered pixel:
            ray.reset(new_origin, new_direction);
            Geometry *hit_geo = scene_tracer.trace(ray, hit, scene);
            Light *hit_light = nullptr;
            Light *current_light = scene.lights;
            if (scene.lights)
                for (u32 i = 0; i < scene.counts.lights; i++, current_light++)
                    if (current_light->isPoint() &&
                        scene_tracer.light_tracer.hit(
                            current_light->position_or_direction,
                            current_light->intensity * 0.5f * LIGHT_INTENSITY_RADIUS_FACTOR,
                            LIGHT_RADIUS_INTENSITY_FACTOR / (current_light->intensity * 0.5f),
                            ray.origin,
                            ray.direction,
                            hit.distance
                        ))
                        hit_light = current_light;

            if (hit_light) {
                light = hit_light;
                geometry = nullptr;

                // Capture a pointer to the selected object's position for later use in transformations:
                world_position = &light->position_or_direction;

                xform.position = light->position_or_direction;
                xform.scale = light->intensity * 0.5f * LIGHT_INTENSITY_RADIUS_FACTOR;
                xform.orientation.reset();
            } else {
                light = nullptr;
                if (hit_geo) {
                    geometry = hit_geo;

                    xform = geometry->transform;
                    if (geometry->type == GeometryType_Mesh)
                        xform.scale *= scene.meshes[geometry->id].aabb.max;

                    // Capture a pointer to the selected object's position for later use in transformations:
                    world_position = &geometry->transform.position;
                } else
                    geometry = nullptr;
            }

            changed = geometry != hit_geo || light != hit_light;

            if (hit_geo || hit_light) {
                local_ray_hit.distance = INFINITY;
                local_ray.localize(ray, xform);
                local_ray.hitsDefaultBox(local_ray_hit);
                transformation_plane_origin = xform.externPos(local_ray_hit.position);
                world_offset = transformation_plane_origin - *world_position;

                // Track how far away the hit position is from the camera along the depth axis:
                object_distance = (camera.orientation.transposed() * (transformation_plane_origin - ray.origin)).z;
            }
        }
        left_mouse_button_was_pressed = mouse::left_button.is_pressed;
        if (geometry || light) {
            if (controls::is_pressed::alt) {
                bool any_mouse_button_is_pressed = (
                        mouse::left_button.is_pressed ||
                        mouse::middle_button.is_pressed ||
                        mouse::right_button.is_pressed);
                if (!any_mouse_button_is_pressed) {
                    // Cast a ray onto the bounding box of the currently selected object:
                    if (geometry) {
                        xform = geometry->transform;
                        if (geometry->type == GeometryType_Mesh)
                            xform.scale *= scene.meshes[geometry->id].aabb.max;
                    } else {
                        xform.position = light->position_or_direction;
                        xform.scale = light->intensity * 0.5f * LIGHT_INTENSITY_RADIUS_FACTOR;
                        xform.orientation.reset();
                    }

                    ray.reset(new_origin, new_direction);
                    local_ray.localize(ray, xform);
                    local_ray_hit.distance = INFINITY;
                    box_side = local_ray.hitsDefaultBox(local_ray_hit);
                    if (box_side) {
                        transformation_plane_origin = xform.externPos(local_ray_hit.position);
                        transformation_plane_center = xform.externPos(local_ray_hit.normal);
                        transformation_plane_normal = xform.externDir(local_ray_hit.normal).normalized();
                        world_offset = transformation_plane_origin - *world_position;
                        object_scale    = geometry ? geometry->transform.scale : xform.scale;
                        object_rotation = xform.orientation;
                    }
                }

                if (any_mouse_button_is_pressed && box_side) {
                    ray.reset(new_origin, new_direction);
                    if (ray.hitsPlane(transformation_plane_origin, transformation_plane_normal, hit)) {
                        if (geometry) {
                            xform = geometry->transform;
                            if (geometry->type == GeometryType_Mesh)
                                xform.scale *= scene.meshes[geometry->id].aabb.max;
                        } else {
                            xform.position = light->position_or_direction;
                            xform.scale = light->intensity * 0.5f * LIGHT_INTENSITY_RADIUS_FACTOR;
                            xform.orientation.reset();
                        }

                        if (mouse::left_button.is_pressed) {
                            *world_position = hit.position - world_offset;
                        } else if (mouse::middle_button.is_pressed) {
                            vec3 abs_pos{absolute(xform.internPos(hit.position))};
                            vec3 abs_org{absolute(xform.internPos(transformation_plane_origin))};
                            vec3 scale_diff{abs_pos - abs_org};
                            switch (box_side) {
                                case BoxSide_Left:
                                case BoxSide_Right: scale_diff.x = abs_pos.x = abs_org.x = 0.0f; break;
                                case BoxSide_Bottom:
                                case BoxSide_Top: scale_diff.y = abs_pos.y = abs_org.y = 0.0f; break;
                                default: scale_diff.z = abs_pos.z = abs_org.z =  0.0f;
                            }
                            if (geometry) {
                                geometry->transform.scale = object_scale + scale_diff * geometry->transform.scale;
                                geometry->transform.scale.x = abs(geometry->transform.scale.x);
                                geometry->transform.scale.y = abs(geometry->transform.scale.y);
                                geometry->transform.scale.z = abs(geometry->transform.scale.z);
                            } else if (light) {
                                light->intensity = abs(
                                    LIGHT_RADIUS_INTENSITY_FACTOR * 2.0f * (
                                        object_scale.x + (
                                            (abs_pos.length() - abs_org.length()) * (light->intensity * LIGHT_INTENSITY_RADIUS_FACTOR * 0.5f)
                                        )
                                    )
                                );
                            }

                        } else if (mouse::right_button.is_pressed && geometry) {
                            vec3 v1{ hit.position - transformation_plane_center };
                            vec3 v2{ transformation_plane_origin - transformation_plane_center };
                            quat rotation = quat{v2.cross(v1), (v1.dot(v2)) + sqrtf(v1.squaredLength() * v2.squaredLength())};
                            geometry->transform.orientation = (rotation.normalized() * object_rotation).normalized();
                        }
                    }
                }
            } else {
                box_side = BoxSide_None;
                if (mouse::left_button.is_pressed && mouse::moved) {
                    // BoxSide_Back-project the new mouse position onto a quad at a distance of the selected-object away from the camera

                    // Screen -> NDC:
                    f32 X = ((f32)x + 0.5f) / dimensions.h_width  - 1;
                    f32 Y = ((f32)y + 0.5f) / dimensions.h_height - 1;

                    // NDC -> View:
                    X *= object_distance / (camera.focal_length * dimensions.height_over_width);
                    Y *= object_distance / camera.focal_length;

                    // View -> World (BoxSide_Back-track by the world offset from the hit position back to the selected-object's center):
                    *world_position = camera.orientation * vec3{X, -Y, object_distance} + camera.position - world_offset;
                }
            }
        }
    }
};