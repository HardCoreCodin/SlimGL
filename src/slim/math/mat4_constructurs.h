#pragma once

#include "./utils.h"
#include "../core/transform.h"
#include "../scene/camera.h"
#include "../viewport/projection.h"

mat4 Mat4(const Camera &camera) { return Mat4(camera.orientation, camera.position); }
mat4 Mat4(const Transform &transform) { return Mat4(transform.orientation, transform.scale, transform.position); }
mat4 Mat4(const Projection &projection) {
    return {
        projection.scale.x, 0, 0, 0,
        0, projection.scale.y, 0, 0,
        0, 0, projection.scale.z, projection.params.is_perspective ? 1.0f : 0.0f,
        0, 0, projection.shear, projection.params.is_perspective ? 0.0f : 1.0f
    };
}
