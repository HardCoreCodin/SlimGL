#pragma once

#include "./vec3.h"

struct quat {
    vec3 axis;
    f32 amount;

    INLINE_XPU quat() noexcept : quat{vec3{0}, 1.0f} {}
    INLINE_XPU quat(f32 axis_x, f32 axis_y, f32 axis_z, f32 amount) noexcept : axis{axis_x, axis_y, axis_z}, amount{amount} {}
    INLINE_XPU quat(vec3 axis, f32 amount) noexcept : axis{axis}, amount{amount} {}
    INLINE_XPU quat(const quat &other) noexcept : quat{other.axis, other.amount} {}

    INLINE_XPU void setToIdentity() {
        *this = {};
    }

    INLINE_XPU f32 length() const {
        return sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z + amount * amount);
    }

    INLINE_XPU quat normalized() const {
        return *this / length();
    }

    INLINE_XPU quat operator * (f32 rhs) const {
        return {
                axis * rhs,
                amount * rhs
        };
    }

    INLINE_XPU quat operator / (f32 rhs) const {
        return *this * (1.0f / rhs);
    }

    INLINE_XPU vec3 operator * (const vec3 &rhs) const {
        vec3 out{axis.cross(rhs)};
        vec3 qqv{axis.cross(out)};
        out = out.scaleAdd(amount, qqv);
        out = out.scaleAdd(2, rhs);
        return out;
    }

    INLINE_XPU quat operator * (const quat &rhs) const {
        return {
                {
                        amount * rhs.axis.x + axis.x * rhs.amount + axis.y * rhs.axis.z - axis.z * rhs.axis.y,
                        amount * rhs.axis.y - axis.x * rhs.axis.z + axis.y * rhs.amount + axis.z * rhs.axis.x,
                        amount * rhs.axis.z + axis.x * rhs.axis.y - axis.y * rhs.axis.x + axis.z * rhs.amount
                },
                amount * rhs.amount - axis.x * rhs.axis.x - axis.y * rhs.axis.y - axis.z * rhs.axis.z
        };
    }

    INLINE_XPU void operator *= (const quat &rhs) {
        f32 x = axis.x; f32 X = rhs.axis.x;
        f32 y = axis.y; f32 Y = rhs.axis.y;
        f32 z = axis.z; f32 Z = rhs.axis.z;
        f32 w = amount; f32 W = rhs.amount;
        axis.x = X*w + W*x + Z*y - Y*z;
        axis.y = Y*w - Z*x + W*y + X*z;
        axis.z = Z*w + Y*x - X*y + W*z;
        amount = W*w - X*x - Y*y - Z*z;
    }

    INLINE_XPU quat conjugate() const {
        return {
                -axis,
                amount
        };
    }

    INLINE_XPU quat rotated(const vec3 &Axis, f32 radians) const {
        return *this * AxisAngle(Axis, radians);
    }

    static INLINE_XPU quat AxisAngle(const vec3 &axis, f32 radians) {
        radians *= 0.5f;
        return quat{
                axis * sinf(radians),
                cosf(radians)
        }.normalized();
    }

    static INLINE_XPU quat RotationBetweenNormalized(const vec3 &from, const vec3 to)  { return quat{from.cross(to), from.dot(to)}.normalized(); }
    static INLINE_XPU quat RotationAroundX(f32 radians) { return AxisAngle(vec3{1.0f, 0.0f, 0.0f}, -radians); }
    static INLINE_XPU quat RotationAroundY(f32 radians) { return AxisAngle(vec3{0.0f, 1.0f, 0.0f}, -radians); }
    static INLINE_XPU quat RotationAroundZ(f32 radians) { return AxisAngle(vec3{0.0f, 0.0f, 1.0f}, -radians); }
    INLINE_XPU void setToRotationAroundX(f32 radians) { *this = RotationAroundX(radians); }
    INLINE_XPU void setToRotationAroundY(f32 radians) { *this = RotationAroundY(radians); }
    INLINE_XPU void setToRotationAroundZ(f32 radians) { *this = RotationAroundZ(radians); }

    INLINE_XPU void setXYZ(vec3 &X, vec3 &Y, vec3 &Z) const {
        /*
        f32 x = axis.x;
        f32 y = axis.y;
        f32 z = axis.z;
        f32 w = axis.w;
        f32 qxx = x * x;
		f32 qyy = y * y;
		f32 qzz = z * z;
		f32 qxz = x * z;
		f32 qxy = x * y;
		f32 qyz = y * z;
		f32 qwx = w * x;
		f32 qwy = w * y;
		f32 qwz = w * z;

        X.x = 1.0f - 2.0f * (qyy + qzz);
		X.y = 2.0f * (qxy + qwz);
		X.z = 2.0f * (qxz - qwy);

		Y.x = 2.0f * (qxy - qwz);
		Y.y = 1.0f - 2.0f * (qxx +  qzz);
		Y.z = 2.0f * (qyz + qwx);

		Z.x = 2.0f * (qxz + qwy);
		Z.y = 2.0f * (qyz - qwx);
		Z.z = 1.0f - 2.0f* (qxx +  qyy);
        return;*/
        f32 q0 = amount;
        f32 q1 = -axis.x;
        f32 q2 = -axis.y;
        f32 q3 = -axis.z;

        X.x = 2 * (q0 * q0 + q1 * q1) - 1;
        X.y = 2 * (q1 * q2 - q0 * q3);
        X.z = 2 * (q1 * q3 + q0 * q2);

        Y.x = 2 * (q1 * q2 + q0 * q3);
        Y.y = 2 * (q0 * q0 + q2 * q2) - 1;
        Y.z = 2 * (q2 * q3 - q0 * q1);

        Z.x = 2 * (q1 * q3 - q0 * q2);
        Z.y = 2 * (q2 * q3 + q0 * q1);
        Z.z = 2 * (q0 * q0 + q3 * q3) - 1;
    }
};

struct OrientationUsingQuaternion : Orientation<quat> {
    INLINE_XPU OrientationUsingQuaternion(f32 x_radians = 0.0f, f32 y_radians = 0.0f, f32 z_radians = 0.0f) : Orientation<quat>{x_radians, y_radians, z_radians} {}

    INLINE_XPU OrientationUsingQuaternion& operator = (const quat &q) {
        axis = q.axis;
        amount = q.amount;

        return *this;
    }
};