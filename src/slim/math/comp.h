#pragma once

#include "../core/base.h"

struct comp {
    f32 real, imag;

    INLINE_XPU comp(f32 real = 0.0f, f32 imag = 0.0f) : real{real}, imag{imag} {}

    INLINE_XPU f32 length() const {
        return sqrtf(real*real + imag*imag);
    }

    INLINE_XPU comp conjugate() const {
        return {real, -imag};
    }

    INLINE_XPU comp operator + (f32 rhs) const {
        return {real + rhs, imag};
    }

    INLINE_XPU comp& operator += (f32 rhs) {
        real += rhs;
        return *this;
    }

    INLINE_XPU comp operator - (f32 rhs) const {
        return {real - rhs, imag};
    }

    INLINE_XPU comp& operator -= (f32 rhs) {
        real -= rhs;
        return *this;
    }

    INLINE_XPU comp operator * (f32 rhs) const {
        comp result{*this};
        result *= rhs;
        return result;
    }

    INLINE_XPU comp& operator *= (f32 rhs) {
        real *= rhs;
        imag *= rhs;
        return *this;
    }

    INLINE_XPU comp operator + (const comp &rhs) const {
        comp result{*this};
        result += rhs;
        return result;
    }

    INLINE_XPU comp& operator += (const comp &rhs) {
        real += rhs.real;
        imag += rhs.imag;
        return *this;
    }

    INLINE_XPU comp operator - (const comp &rhs) const {
        comp result{*this};
        result -= rhs;
        return result;
    }

    INLINE_XPU comp& operator -= (const comp &rhs) {
        real -= rhs.real;
        imag -= rhs.imag;
        return *this;
    }

    INLINE_XPU comp operator * (const comp &rhs) const {
        comp result{*this};
        result *= rhs;
        return result;
    }

    INLINE_XPU comp& operator *= (const comp &rhs) {
        f32 x = real;
        f32 y = imag;
        real = fast_mul_add(x, rhs.real, y * -rhs.imag);
        imag = fast_mul_add(x, rhs.imag, y * rhs.real);
        return *this;
    }
};


INLINE_XPU void generateOrbit(u32 N, comp *array, bool full = true, bool clockwise = false, bool reuse_rotation = false) {
    if (N == 0) return;

    array[0] = {1, 0};
    if (N == 1) return;

    f64 angle_step = TAU / (f64)N;
    f32 direction = clockwise ? -1.0f : 1.0f;
    if (!full) N >>= 1;

    if (reuse_rotation) {
        comp step{cosf(angle_step), direction * sinf(angle_step)};

        array[1] = step;
        if (N == 2) return;

        comp current{step};
        for (u32 i = 2; i < N; i++)
            array[i] = current *= step;
    } else {
        f64 angle = angle_step;
        for (u32 i = 1; i < N; i++, angle += angle_step)
            array[i] = {(f32)(cos(angle)), direction * (f32)(sin(angle))};
    }
}