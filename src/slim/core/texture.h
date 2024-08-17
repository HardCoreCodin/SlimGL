#pragma once

#include "./base.h"

struct TexelQuadComponent {
    u8 TL, TR, BL, BR;
};

struct TexelQuad {
    TexelQuadComponent R, G, B;
};

struct TextureMip {
    u32 width, height;
    TexelQuad *texel_quads;

    INLINE_XPU Pixel sample(f32 u, f32 v) const {
        if (u > 1) u -= (f32)((u32)u);
        if (v > 1) v -= (f32)((u32)v);

        const f32 U = u * (f32)width  + 0.5f;
        const f32 V = v * (f32)height + 0.5f;
        const u32 x = (u32)U;
        const u32 y = (u32)V;
        const f32 r = U - (f32)x;
        const f32 b = V - (f32)y;
        const f32 l = 1 - r;
        const f32 t = 1 - b;
        const f32 tl = t * l * COLOR_COMPONENT_TO_FLOAT;
        const f32 tr = t * r * COLOR_COMPONENT_TO_FLOAT;
        const f32 bl = b * l * COLOR_COMPONENT_TO_FLOAT;
        const f32 br = b * r * COLOR_COMPONENT_TO_FLOAT;

        const TexelQuad texel_quad = texel_quads[y * (width + 1) + x];
        return {
                fast_mul_add((f32)texel_quad.R.BR, br, fast_mul_add((f32)texel_quad.R.BL, bl, fast_mul_add((f32)texel_quad.R.TR, tr, (f32)texel_quad.R.TL * tl))),
                fast_mul_add((f32)texel_quad.G.BR, br, fast_mul_add((f32)texel_quad.G.BL, bl, fast_mul_add((f32)texel_quad.G.TR, tr, (f32)texel_quad.G.TL * tl))),
                fast_mul_add((f32)texel_quad.B.BR, br, fast_mul_add((f32)texel_quad.B.BL, bl, fast_mul_add((f32)texel_quad.B.TR, tr, (f32)texel_quad.B.TL * tl))),
                1.0f
        };
    }
};

struct Texture : ImageInfo {
    TextureMip *mips = nullptr;

    XPU static u32 GetMipLevel(f32 texel_area, u32 mip_count) {
        u32 mip_level = 0;
        while (texel_area > 1 && ++mip_level < mip_count) texel_area *= 0.25f;
        if (mip_level >= mip_count)
            mip_level = mip_count - 1;

        return mip_level;
    }

    XPU static u32 GetMipLevel(u32 width, u32 height, u32 mip_count, f32 uv_coverage) {
        return GetMipLevel(uv_coverage * (f32)(width * height), mip_count);
    }

    XPU static u32 GetMipLevel(const Texture &texture, f32 uv_coverage) {
        return GetMipLevel(uv_coverage * (f32)(texture.width * texture.height), texture.mip_count);
    }

    INLINE_XPU u32 mipLevel(f32 uv_coverage) const {
        return GetMipLevel(uv_coverage * (f32)(width * height), mip_count);
    }

    INLINE_XPU Pixel sample(f32 u, f32 v, f32 uv_coverage) const {
        return mips[flags.mipmap ? GetMipLevel(uv_coverage * (f32)(width * height), mip_count) : 0].sample(u, v);
    }

    INLINE_XPU Pixel sampleCube(f32 X, f32 Y, f32 Z) const {
        f32 u, v;
        u8 mip = 0;

        Sides sides{X, Y, Z};
////        left{signbit(x)},
////            bottom{signbit(y)},
////            back{signbit(z)},
////            right{!signbit(x)},
////            top{!signbit(y)},
////            front{!signbit(z)}
////        {}
//
//        f32 ax = abs(X);
//        f32 ay = abs(Y);
//        f32 az = abs(Z);
//
//        if (ax < Z)

        f32 z_over_x = X ? (Z / X) : 2;
        f32 y_over_x = X ? (Y / X) : 2;
        if (z_over_x <=  1 &&
            z_over_x >= -1 &&
            y_over_x <=  1 &&
            y_over_x >= -1) { // Right or Left
            u = fast_mul_add(z_over_x, -0.125f, fast_mul_add(0.5f, sides.right, 0.125f));
            v = fast_mul_add(y_over_x, sides.left - 0.5f, 0.5f);
        } else {
            f32 x_over_z = Z ? (X / Z) : 2;
            f32 y_over_z = Z ? (Y / Z) : 2;
            if (x_over_z <=  1 &&
                x_over_z >= -1 &&
                y_over_z <=  1 &&
                y_over_z >= -1) { // Front or Back:
                v = fast_mul_add(y_over_z, signbit(Z) - 0.5f, + 0.5f);
                u = fast_mul_add(x_over_z, 0.125f, fast_mul_add(signbit(Z), 0.5f, 0.375f));
            } else { // Top or Bottom
                u = fast_mul_add(X / abs(Y), 0.5f, 0.5f);
                v = fast_mul_add(Z / Y, 0.5f, 0.5f);
                mip = 2 - !signbit(Y);
            }
        }

        return mips[mip].sample(u, v);
    }

//    INLINE_XPU Pixel sampleCube(f32 x, f32 y, f32 z) const {
//        f32 x_rcp = 1.0f / x;
//        f32 y_rcp = 1.0f / y;
//        f32 z_rcp = 1.0f / z;
//
//        OctantShifts octant_shifts{Sides{x, y, z}};
//        f32 box[6]{-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
////        vec3 min_t{
////            x_rcp * box[octant_shifts.x    ],
////            y_rcp * box[octant_shifts.y + 1],
////            z_rcp * box[octant_shifts.z + 2]
////        };
//        vec3 max_t{
//            x_rcp * box[3 - octant_shifts.x],
//            y_rcp * box[4 - octant_shifts.y],
//            z_rcp * box[5 - octant_shifts.z]
//        };
////        f32 near_distance = Max(0, min_t.maximum());
//        f32 far_distance = max_t.minimum();
//
//
//
//
//        u8 mip = ((u8)side & (u8)BoxSide_Bottom) | (((u8)side & (u8)BoxSide_Top) >> 3);
//        switch (side) {
//            case
//        }
//        f32 z_over_x = X ? (Z / X) : 2;
//        f32 y_over_x = X ? (Y / X) : 2;
//        if (z_over_x <=  1 &&
//            z_over_x >= -1 &&
//            y_over_x <=  1 &&
//            y_over_x >= -1) { // Right or Left
//            u = fast_mul_add(z_over_x, -0.125f, fast_mul_add(0.5f, !signbit(X), 0.125f));
//            v = fast_mul_add(y_over_x, signbit(X) - 0.5f, 0.5f);
//        } else {
//            f32 x_over_z = Z ? (X / Z) : 2;
//            f32 y_over_z = Z ? (Y / Z) : 2;
//            if (x_over_z <=  1 &&
//                x_over_z >= -1 &&
//                y_over_z <=  1 &&
//                y_over_z >= -1) { // Front or Back:
//                v = fast_mul_add(y_over_z, signbit(Z) - 0.5f, + 0.5f);
//                u = fast_mul_add(x_over_z, 0.125f, fast_mul_add(signbit(Z), 0.5f, 0.375f));
//            } else { // Top or Bottom
//                u = fast_mul_add(X / abs(Y), 0.5f, 0.5f);
//                v = fast_mul_add(Z / Y, 0.5f, 0.5f);
//                mip = 2 - !signbit(Y);
//            }
//        }
//
//        return mips[mip].sample(u, v);
//    }

};