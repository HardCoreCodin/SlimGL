#pragma once

#include "math/vec2.h"

struct Transform2D {
    vec2 position{0.0f};
    f32 rotation{0.0f};
    vec2 scale{1.0f};

    INLINE_XPU vec2 externPos(const vec2 &pos) const { return _translate(_rotate(_scale(pos))); }
    INLINE_XPU vec2 internPos(const vec2 &pos) const { return _unscale(_unrotate(_untranslate(pos))); }

    INLINE_XPU vec2 externDir(const vec2 &dir) const { return _rotate(_scale(dir)).normalized(); }
    INLINE_XPU vec2 internDir(const vec2 &dir) const { return _unscale(_unrotate(dir)).normalized(); }

private:
    INLINE_XPU vec2 _scale(const vec2 &pos) const { return scale * pos; }
    INLINE_XPU vec2 _rotate(const vec2 &pos) const { return pos.rotated_by(rotation); }
    INLINE_XPU vec2 _translate(const vec2 &pos) const { return pos + position; }
    INLINE_XPU vec2 _unscale(const vec2 &pos) const { return pos / scale; }
    INLINE_XPU vec2 _unrotate(const vec2 &pos) const { return pos.rotated_by(-rotation); }
    INLINE_XPU vec2 _untranslate(const vec2 &pos) const { return pos - position; }
};