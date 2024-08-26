#pragma once

#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/quat.h"


INLINE_XPU f32 ggxTrowbridgeReitz_D(f32 roughness, f32 NdotH) { // NDF
    // http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    f32 a = roughness * roughness;
    f32 denom = NdotH * NdotH * (a - 1.0f) + 1.0f;
    return (
        a
        /
        (pi * denom * denom)
    );
}

INLINE_XPU f32 ggxSchlickSmith_G(f32 roughness, f32 NdotL, f32 NdotV, bool IBL = false) {
// https://learnopengl.com/PBR/Theory
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
    f32 a = roughness * roughness;
    f32 k = a * 0.5f; // Approximation from Karis (UE4)
//    if (IBL) {
//        k *= k * 0.5f;
//    } else { // direct
//        k += 1.0f;
//        k *= k * 0.125f;
//    }
    f32 one_minus_k = 1.0f - k;
    f32 denom = fast_mul_add(NdotV, one_minus_k, k);
    f32 result = NdotV / fmaxf(denom, EPS);
    denom = fast_mul_add(NdotL, one_minus_k, k);
    result *= NdotL / fmaxf(denom, EPS);
    return result;
}

INLINE_XPU Color schlickFresnel(f32 HdotL, const Color &F0) {
    return F0 + (1.0f - F0) * powf(1.0f - HdotL, 5.0f);
}

INLINE_XPU Color cookTorrance(f32 roughness, f32 NdotL, f32 NdotV, f32 HdotL, f32 NdotH, const Color &F0, Color &F) {
    F = schlickFresnel(HdotL, F0);
    f32 D = ggxTrowbridgeReitz_D(roughness, NdotH);
    f32 G = ggxSchlickSmith_G(roughness, NdotL, NdotV);
    Color Ks = F * (D * G
              /
              (4.0f * NdotL * NdotV)
    );
    return Ks;
}

INLINE_XPU f32 classicSpecularFactor(f32 shininess, f32 specular_factor, f32 exponent) {
    return powf(specular_factor, exponent * shininess) * shininess;
}

INLINE_XPU f32  distanceToColor(f32 distance                                              ) { return 4.0f / distance; }
INLINE_XPU vec3 directionAndDistanceToColor(const vec3 &direction, f32 distance           ) { return (direction + 1.0f) * distanceToColor(distance); }


// axis      = up ^ normal = [0, 1, 0] ^ [x, y, z] = [1*z - 0*y, 0*x - 0*z, 0*y - 1*x] = [z, 0, -x]
// cos_angle = up . normal = [0, 1, 0] . [x, y, z] = 0*x + 1*y + 0*z = y
// (Pre)Swizzle N.z <-> N.y
INLINE_XPU quat getNormalRotation(const vec3 &N, f32 magnitude = 1.0f                     ) { return quat::AxisAngle(vec3{N.z, 0, -N.x}.normalized(),  acosf(N.y) * magnitude); }
INLINE_XPU vec3 decodeNormal(const Color &color                                           ) { return vec3{color.r, color.b, color.g}.scaleAdd(2.0f, -1.0f).normalized(); }
INLINE_XPU quat convertNormalSampleToRotation(const Color &normal_sample, f32 magnitude   ) { return getNormalRotation(decodeNormal(normal_sample), magnitude); }
INLINE_XPU vec3 rotateNormal(const vec3 &normal, const Color &normal_sample, f32 magnitude) { return convertNormalSampleToRotation(normal_sample, magnitude) * normal; }

INLINE_XPU Color getColorByDirection(const vec3 &direction) { return directionToColor(direction); }
INLINE_XPU Color getColorByDirectionAndDistance(const vec3 &direction, f32 distance) { return directionAndDistanceToColor(direction, distance).toColor(); }
INLINE_XPU Color getColorByDistance(f32 distance) { return distanceToColor(distance); }
INLINE_XPU Color getColorByUV(vec2 uv) { return {uv.u, uv.v, 1.0f}; }

struct Material {
    Color albedo = 1.0f;
    f32 roughness = 1.0f;

    u8 flags = 0;
    u8 texture_count = 0;
    u8 texture_ids[2] = {0, 0};
    f32 metalness = 0.0f;
    Color reflectivity{F0_Default};
    BRDFType brdf{BRDF_CookTorrance};

    f32 normal_magnitude = 1.0f;
    UV uv_repeat = 1.0f;

    Color emission = 0.0f;
    f32 IOR = 1.0f;

    INLINE_XPU bool isEmissive() const { return flags & MATERIAL_IS_EMISSIVE; }
    INLINE_XPU bool isReflective() const { return flags & MATERIAL_IS_REFLECTIVE; }
    INLINE_XPU bool isRefractive() const { return flags & MATERIAL_IS_REFRACTIVE; }
    INLINE_XPU bool isTextured() const { return texture_count && (flags & (MATERIAL_HAS_ALBEDO_MAP | MATERIAL_HAS_NORMAL_MAP)); }
    INLINE_XPU bool hasAlbedoMap() const { return texture_count && (flags & MATERIAL_HAS_ALBEDO_MAP); }
    INLINE_XPU bool hasNormalMap() const { return texture_count && (flags & MATERIAL_HAS_NORMAL_MAP); }
};