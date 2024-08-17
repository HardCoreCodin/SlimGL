#pragma once

#include "../math/quat.h"

struct Transform {
    OrientationUsingQuaternion orientation{};
    vec3 position{0.0f};
    vec3 scale{1.0f};

    INLINE_XPU vec3 externPos(const vec3 &pos) const { return _translate(_rotate(_scale(pos))); }
    INLINE_XPU vec3 internPos(const vec3 &pos) const { return _unscale(_unrotate(_untranslate(pos))); }

    INLINE_XPU vec3 externDir(const vec3 &dir) const { return _rotate(_scale(dir)).normalized(); }
    INLINE_XPU vec3 internDir(const vec3 &dir) const { return _unscale(_unrotate(dir)).normalized(); }

    INLINE_XPU AABB internAABB(const AABB &aabb) const {
        vec3 pos, min{+INFINITY}, max{-INFINITY};

        for (u8 i = 0; i < 2; i++)
            for (u8 j = 0; j < 2; j++)
                for (u8 k = 0; k < 2; k++) {
                    pos.x = (&aabb.min + i)->x;
                    pos.y = (&aabb.min + j)->y;
                    pos.z = (&aabb.min + k)->z;
                    pos = internPos(pos);
                    min = minimum(min, pos);
                    max = maximum(max, pos);
                }

        return {min, max};
    }

    INLINE_XPU AABB externAABB(const AABB &aabb) const {
        vec3 pos, min{+INFINITY}, max{-INFINITY};

        for (u8 i = 0; i < 2; i++)
            for (u8 j = 0; j < 2; j++)
                for (u8 k = 0; k < 2; k++) {
                    pos.x = (&aabb.min + i)->x;
                    pos.y = (&aabb.min + j)->y;
                    pos.z = (&aabb.min + k)->z;
                    pos = externPos(pos);
                    min = minimum(min, pos);
                    max = maximum(max, pos);
                }

        return {min, max};
    }

private:
    INLINE_XPU vec3 _scale(const vec3 &pos) const { return scale * pos; }
    INLINE_XPU vec3 _rotate(const vec3 &pos) const { return orientation * pos; }
    INLINE_XPU vec3 _translate(const vec3 &pos) const { return pos + position; }
    INLINE_XPU vec3 _unscale(const vec3 &pos) const { return pos / scale; }
    INLINE_XPU vec3 _unrotate(const vec3 &pos) const { return orientation.conjugate() * pos; }
    INLINE_XPU vec3 _untranslate(const vec3 &pos) const { return pos - position; }
};

struct Geometry {
    Transform transform{};
    GeometryType type{GeometryType_None};
    u32 material_id = 0, id = 0;
    u8 flags = GEOMETRY_IS_VISIBLE | GEOMETRY_IS_SHADOWING;
    ColorID color{White};
};