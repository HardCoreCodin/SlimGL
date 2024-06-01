#pragma once

#include "./common.h"
#include "../math/utils.h"
#include "../scene/scene.h"

INLINE vec3 sampleNormal(Texture &texture, f32 u, f32 v, f32 uv_area) {
    Pixel pixel = texture.sample(u, v, uv_area);
    vec3 normal = pixel.color;
    f32 y = normal.z;
    normal.z = normal.y;
    normal.y = y;
    return normal.scaleAdd(2.0f, vec3{-1.0f}).normalized();
}
INLINE quat getNormalRotation(vec3 normal, f32 magnitude) {
    // axis      = up ^ normal = [0, 1, 0] ^ [x, y, z] = [1*z - 0*y, 0*x - 0*z, 0*y - 1*x] = [z, 0, -x]
    // cos_angle = up . normal = [0, 1, 0] . [x, y, z] = 0*x + 1*y + 0*z = y
    vec3 axis{normal.z, 0, -normal.x};
    f32 angle = acosf(normal.y);
    return quat::AxisAngle(axis.normalized(),  angle * magnitude);
//    vec3 up{0, 1, 0};
//    quat q{up.cross(normal), sqrtf(1.0f + up.dot(normal))};
//    return quat{{normal.z * magnitude, 0.0f, -normal.x * magnitude}, (1.0f + normal.y) * magnitude}.normalized();
}

INLINE bool isChequerboard(f32 u, f32 v, f32 half_step_count) {
    f32 s = u * half_step_count;
    f32 t = v * half_step_count;
    s -= floorf(s);
    t -= floorf(t);
    return (s > 0.5f) ^ (t < 0.5f);
}

void shadePixelTextured(Shaded &shaded, const Scene &scene) {
    Pixel pixel = scene.textures[shaded.material->texture_ids[0]].sample(shaded.u, shaded.v, shaded.uv_area);
    shaded.color = pixel.color;
}

void shadePixelDepth(Shaded &shaded, const Scene &scene) {
    shaded.color = shaded.depth > 10.0f ? 1.0f : (f32)(shaded.depth * 0.1);
}

void shadePixelUV(Shaded &shaded, const Scene &scene) {
    shaded.color.r = shaded.u;
    shaded.color.g = shaded.v;
    shaded.color.b = 0;
}

void shadePixelPosition(Shaded &shaded, const Scene &scene) {
    shaded.color = ((shaded.position + 1.0f) * 0.5f).toColor();
}

void shadePixelNormal(Shaded &shaded, const Scene &scene) {
    if (shaded.material->normal_magnitude && shaded.material->texture_count > 1)
        shaded.normal = getNormalRotation(
                sampleNormal(scene.textures[shaded.material->texture_ids[1]], shaded.u, shaded.v, shaded.uv_area),
                shaded.material->normal_magnitude
        ) * shaded.normal;

    shaded.color = shaded.normal.scaleAdd(0.5f, vec3{0.5f}).toColor();
}

void shadePixelCheckerboard(Shaded &shaded, const Scene &scene) {
    shaded.color = isChequerboard(shaded.u, shaded.v, 4) ? 1.0f : 0.0f;
}
void shadePixelLighting(Shaded &shaded, const Scene &scene) {
    if (shaded.material->normal_magnitude && shaded.material->texture_count > 1)
        shaded.normal = getNormalRotation(
                sampleNormal(scene.textures[shaded.material->texture_ids[1]], shaded.u, shaded.v, shaded.uv_area),
                shaded.material->normal_magnitude
        ) * shaded.normal;

    shaded.color = scene.ambient_light.color;
    vec3 lighting;
    Light *light = scene.lights;
    for (u32 i = 0; i < scene.counts.lights; i++, light++) {
        shaded.light_direction = light->position_or_direction - shaded.position;
        f32 squared_distance = shaded.light_direction.squaredLength();
        shaded.light_direction = shaded.light_direction / sqrtf(squared_distance);
        f32 NdotL = shaded.normal.dot(shaded.light_direction);
        if (NdotL > 0) lighting += light->color * NdotL * light->intensity / squared_distance;
    }
    shaded.color += lighting.toColor();
}
void shadePixelClassic(Shaded &shaded, const Scene &scene) {
    MaterialUses material_uses{shaded.material->flags};
    f32 NdotL, NdotRd, squared_distance;

    shaded.diffuse = shaded.material->diffuse;
    if (shaded.material->texture_count) {
        shaded.diffuse = shaded.diffuse * scene.textures[shaded.material->texture_ids[0]].sample(shaded.u, shaded.v, shaded.uv_area).color;
        if (shaded.material->normal_magnitude && shaded.material->texture_count > 1)
            shaded.normal = getNormalRotation(
                    sampleNormal(scene.textures[shaded.material->texture_ids[1]], shaded.u, shaded.v, shaded.uv_area),
                    shaded.material->normal_magnitude
            ) * shaded.normal;
    }

    shaded.color = scene.ambient_light.color;
    shaded.viewing_direction = (shaded.position - shaded.viewing_origin).normalized();
    if (material_uses.phong) {
        NdotRd = clampedValue(shaded.normal.dot(shaded.viewing_direction));
        shaded.reflected_direction = reflectWithDot(shaded.viewing_direction, shaded.normal, NdotRd);
    }
    f32 one_over_distance;
    Light *light = scene.lights;
    for (u32 i = 0; i < scene.counts.lights; i++, light++) {
        shaded.light_direction = light->position_or_direction - shaded.position;
        NdotL = shaded.normal.dot(shaded.light_direction);
        if (NdotL > 0) {
            squared_distance = shaded.light_direction.squaredLength();
            one_over_distance = 1.0f / sqrtf(squared_distance);
            shaded.light_direction *= one_over_distance;
            NdotL *= one_over_distance;
            shaded.color = shadePointOnSurface(shaded, NdotL).mulAdd(light->color * (light->intensity / squared_distance), shaded.color).toColor();
        }
    }

    shaded.color.r = toneMappedBaked(shaded.color.r);
    shaded.color.g = toneMappedBaked(shaded.color.g);
    shaded.color.b = toneMappedBaked(shaded.color.b);
}

void shadePixelClassicChequerboard(Shaded &shaded, const Scene &scene) {
    shadePixelClassic(shaded, scene);

    if (isChequerboard(shaded.u, shaded.v, 4)) shaded.color *= 0.5f;
}