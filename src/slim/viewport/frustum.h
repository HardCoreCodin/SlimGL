#pragma once

#include "./projection.h"


struct Frustum {
    Projection projection{
            CAMERA_DEFAULT__FOCAL_LENGTH,
            (f32)DEFAULT_HEIGHT / (f32)DEFAULT_WIDTH,
            VIEWPORT_DEFAULT__NEAR_CLIPPING_PLANE_DISTANCE,
            VIEWPORT_DEFAULT__FAR_CLIPPING_PLANE_DISTANCE
    };

    f32 near_clipping_plane_distance{VIEWPORT_DEFAULT__NEAR_CLIPPING_PLANE_DISTANCE};
    f32 far_clipping_plane_distance{ VIEWPORT_DEFAULT__FAR_CLIPPING_PLANE_DISTANCE};
    bool flip_z{false}, cull_back_faces{true};

    void updateProjection(f32 focal_length, f32 height_over_width) {
        projection.update(focal_length,
                          height_over_width,
                          near_clipping_plane_distance,
                          far_clipping_plane_distance);
    }

    bool cullAndClipEdge(Edge &edge, f32 focal_length, f32 aspect_ratio) const {
        f32 distance = near_clipping_plane_distance;

        vec3 A{edge.from};
        vec3 B{edge.to};

        u8 out = (A.z < distance) | ((B.z < distance) << 1);
        if (out) {
            if (out == 3) return false;
            if (out & 1) A = A.lerpTo(B, (distance - A.z) / (B.z - A.z));
            else         B = B.lerpTo(A, (distance - B.z) / (A.z - B.z));
        }

        distance = far_clipping_plane_distance;
        out = (A.z > distance) | ((B.z > distance) << 1);
        if (out) {
            if (out == 3) return false;
            if (out & 1) A = A.lerpTo(B, (A.z - distance) / (A.z - B.z));
            else         B = B.lerpTo(A, (B.z - distance) / (B.z - A.z));
        }

        // Left plane (facing to the right):
        vec3 N{focal_length, 0, aspect_ratio};
        f32 NdotA = N.dot(A);
        f32 NdotB = N.dot(B);

        out = (NdotA < 0) | ((NdotB < 0) << 1);
        if (out) {
            if (out == 3) return false;
            if (out & 1) A = A.lerpTo(B, NdotA / (NdotA - NdotB));
            else         B = B.lerpTo(A, NdotB / (NdotB - NdotA));
        }

        // Right plane (facing to the left):
        N.x = -N.x;
        NdotA = N.dot(A);
        NdotB = N.dot(B);

        out = (NdotA < 0) | ((NdotB < 0) << 1);
        if (out) {
            if (out == 3) return false;
            if (out & 1) A = A.lerpTo(B, NdotA / (NdotA - NdotB));
            else         B = B.lerpTo(A, NdotB / (NdotB - NdotA));
        }

        // Bottom plane (facing up):
        N = {0, focal_length, 1};
        NdotA = N.dot(A);
        NdotB = N.dot(B);

        out = (NdotA < 0) | ((NdotB < 0) << 1);
        if (out) {
            if (out == 3) return false;
            if (out & 1) A = A.lerpTo(B, NdotA / (NdotA - NdotB));
            else         B = B.lerpTo(A, NdotB / (NdotB - NdotA));
        }

        // Top plane (facing down):
        N.y = -N.y;
        NdotA = N.dot(A);
        NdotB = N.dot(B);

        out = (NdotA < 0) | ((NdotB < 0) << 1);
        if (out) {
            if (out == 3) return false;
            if (out & 1) A = A.lerpTo(B, NdotA / (NdotA - NdotB));
            else         B = B.lerpTo(A, NdotB / (NdotB - NdotA));
        }

        edge.from = A;
        edge.to   = B;

        return true;
    }

    void checkEdge(Edge &edge, f32 focal_length, f32 aspect_ratio, Sides &from_sides, Sides &to_sides) const {
        from_sides.mask = 0;
        to_sides.mask = 0;

        from_sides.back = edge.from.z < near_clipping_plane_distance;
        to_sides.back = edge.to.z < near_clipping_plane_distance;
        from_sides.front = edge.from.z > far_clipping_plane_distance;
        to_sides.front = edge.to.z > far_clipping_plane_distance;

        // Left plane (facing to the right):
        vec3 N{focal_length, 0, aspect_ratio};
        f32 NdotA = N.dot(edge.from);
        f32 NdotB = N.dot(edge.to);
        from_sides.left = NdotA < 0;
        to_sides.left = NdotB < 0;

        // Right plane (facing to the left):
        N.x = -N.x;
        NdotA = N.dot(edge.from);
        NdotB = N.dot(edge.to);
        from_sides.right = NdotA < 0;
        to_sides.right = NdotB < 0;

        // Bottom plane (facing up):
        N = {0, focal_length, 1};
        NdotA = N.dot(edge.from);
        NdotB = N.dot(edge.to);
        from_sides.bottom = NdotA < 0;
        to_sides.bottom = NdotB < 0;

        // Top plane (facing down):
        N.y = -N.y;
        NdotA = N.dot(edge.from);
        NdotB = N.dot(edge.to);
        from_sides.top = NdotA < 0;
        to_sides.top = NdotB < 0;
    }

    INLINE void projectPoint(vec3 &point, const Dimensions &dimensions) const {
        point.x = ((projection.scale.x * point.x / point.z) + 1) * dimensions.h_width;
        point.y = ((projection.scale.y * point.y / point.z) + 1) * dimensions.h_height;
        point.y = dimensions.f_height - point.y;
    }

    void projectEdge(Edge &edge, const Dimensions &dimensions) const {
        projectPoint(edge.from, dimensions);
        projectPoint(edge.to, dimensions);
    }
};
