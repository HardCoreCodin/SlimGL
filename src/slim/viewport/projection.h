#pragma once

#include "../math/vec3.h"

struct ProjectionParams
{
    float near_distance;
    float far_distance;
    union {
        float width;
        float focal_length;
    };
    union {
        float height;
        float height_over_width;
    };
    bool is_perspective;
    bool to_cube;

    static ProjectionParams makePerspective(
        float focal_length, 
        float height_over_width,
        float near_distance,
        float far_distance,
        bool to_cube = true) {
        return {near_distance, far_distance, focal_length, height_over_width, true, to_cube};
    }

    static ProjectionParams makeOrthographic(
        float width, 
        float height,
        float near_distance,
        float far_distance,
        bool to_cube = true) {
        return {near_distance, far_distance, width, height, false, to_cube};
    }
};

struct Projection {
    ProjectionParams params;
    vec3 scale;
    f32 shear;

    Projection(const ProjectionParams &params) {
        update(params);
    }

    Projection(f32 focal_length, f32 height_over_width, f32 near_distance, f32 far_distance, bool is_perspective = true, bool to_cube = true) : 
        Projection{ProjectionParams{near_distance, far_distance, focal_length, height_over_width, is_perspective, to_cube}} {}

    Projection(const Projection &other) : params{params}, scale{other.scale}, shear{other.shear} {}

    void update(const ProjectionParams &new_params) {
        params = new_params;
        const float f = params.far_distance;
        const float n = params.near_distance;
        if (params.is_perspective)
        {
            scale.x = params.focal_length * params.height_over_width;
            scale.y = params.focal_length;
            scale.z = 1.0f / (f - n);
            shear = scale.z * -n;
            if (params.to_cube) {
                shear *= 2 * f;
                scale.z *= f + n;
            } else {
                shear *= f;
                scale.z *= f;
            }
        }
        else
        {
            scale.x = 2.0f / params.width;
            scale.y = 2.0f / params.height;
            scale.z = 2.0f / (f - n);
            shear = (f + n) / (n - f);
            if (!params.to_cube) {
                scale.z *= 0.5f;
                shear *= 0.5f;
            }
        }
    }

    void update(f32 focal_length, f32 height_over_width, f32 near_distance, f32 far_distance) {
        params.focal_length = focal_length;
        params.height_over_width = height_over_width;
        params.near_distance = near_distance;
        params.far_distance = far_distance;
        update(params);
    }

    vec3 project(const vec3 &position) const {
        vec3 projected_position{
            position.x * scale.x,
            position.y * scale.y,
            position.z * scale.z + shear
        };
        return projected_position / position.z;
    }
};
