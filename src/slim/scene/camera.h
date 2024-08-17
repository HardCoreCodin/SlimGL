#pragma once

#include "../math/mat3.h"


struct Camera {
    OrientationUsing3x3Matrix orientation{};
    vec3 position{0};
    vec3 current_velocity{0};
    f32 focal_length{  CAMERA_DEFAULT__FOCAL_LENGTH};
    f32 zoom_amount{   CAMERA_DEFAULT__FOCAL_LENGTH};
    f32 target_distance{CAMERA_DEFAULT__TARGET_DISTANCE};
    f32 dolly_amount{0};

    void zoom(f32 amount) {
        f32 new_amount = zoom_amount + amount;
        focal_length = new_amount > 1 ? new_amount : (new_amount < -1.0f ? (-1.0f / new_amount) : 1.0f);
        zoom_amount = new_amount;
    }

    void dolly(f32 amount) {
        vec3 target_position = orientation.forward.scaleAdd(target_distance, position);

        // Compute new target distance:
        dolly_amount += amount;
        target_distance = powf(2.0f, dolly_amount / -200.0f) * CAMERA_DEFAULT__TARGET_DISTANCE;

        // Back-track from target position_x to new current position_x:
        position = target_position - (orientation.forward * target_distance);
    }

    void orbit(f32 azimuth, f32 altitude) {
        // Move the camera forward to the position_x of its target:
        position += orientation.forward * target_distance;

        // Reorient the camera while it is at the position_x of its target:
        orientation.rotate(altitude, azimuth);

        // Back the camera away from its target position_x using the updated forward direction:
        position -= orientation.forward * target_distance;
    }

    void pan(f32 right_amount, f32 up_amount) {
        position += orientation.up * up_amount + orientation.right * right_amount;
    }

    INLINE_XPU vec3 internPos(const vec3 &pos) const { return _unrotate(_untranslate(pos)); }
    INLINE_XPU vec3 internDir(const vec3 &dir) const { return _unrotate(dir); }
    INLINE_XPU vec3 externPos(const vec3 &pos) const { return _translate(_rotate(pos)); }
    INLINE_XPU vec3 externDir(const vec3 &dir) const { return _rotate(dir); }

private:
    INLINE_XPU vec3 _rotate(const vec3 &pos) const { return orientation * pos; }
    INLINE_XPU vec3 _unrotate(const vec3 &pos) const { return orientation.transposed() * pos; }
    INLINE_XPU vec3 _translate(const vec3 &pos) const { return pos + position; }
    INLINE_XPU vec3 _untranslate(const vec3 &pos) const { return pos - position; }
};

struct CameraRayProjection {
    mat3 inverted_camera_rotation;
    vec3 start, right, down, camera_position;
    vec2 C_start;
    f32 squared_distance_to_projection_plane;
    f32 sample_size;

    INLINE_XPU f32 getDepthAt(vec3 &position) const { return (inverted_camera_rotation * (position - camera_position)).z; }
    INLINE_XPU vec3 getRayDirectionAt(i32 x, i32 y) const { return start + down*y + right*x; }

    void reset(const Camera &camera, const Dimensions &dim, bool antialias) {
        sample_size = antialias ? 0.5f : 1.0f;
        squared_distance_to_projection_plane = dim.h_height * camera.focal_length;
        C_start.x = (sample_size * 0.5f) - dim.h_width;
        C_start.y = dim.h_height - (sample_size * 0.5f);

        inverted_camera_rotation = camera.orientation.inverted();
        camera_position = camera.position;
        down = -camera.orientation.up      * sample_size;
        right = camera.orientation.right   * sample_size;
        start = camera.orientation.right   * C_start.x +
                camera.orientation.up      * C_start.y +
                camera.orientation.forward * squared_distance_to_projection_plane;
        squared_distance_to_projection_plane *= squared_distance_to_projection_plane;
    }
};