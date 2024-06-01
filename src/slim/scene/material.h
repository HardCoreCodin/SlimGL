#pragma once

#include "./mesh.h"
#include "./grid.h"
#include "./box.h"
#include "./camera.h"
#include "../core/ray.h"
#include "../core/texture.h"
#include "../core/transform.h"
#include "../serialization/mesh.h"
#include "../serialization/texture.h"


#define LAMBERT 1
#define PHONG 2
#define BLINN 4

enum BRDFType {
    phong,
    ggx
};

struct Rasterizer;
struct Shaded;
struct Scene;
struct Mesh;
typedef void (*PixelShader)(Shaded &shaded, const Scene &scene);
typedef u8 (  *MeshShader )(const Mesh &mesh, const Rasterizer &rasterizer);

struct Material {
    PixelShader pixel_shader;
    MeshShader mesh_shader;
    BRDFType brdf{phong};
    u8 flags{PHONG | LAMBERT};
    u8 texture_count;
    f32 roughness, shininess, normal_magnitude;
    vec3 diffuse, specular;
    u8 texture_ids[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    Material(PixelShader pixel_shader,
             MeshShader mesh_shader,
             u8 texture_count = 0,
             vec3 diffuse = vec3{1.0f},
             vec3 specular = vec3{1.0f},
             f32 roughness = 1.0f,
             f32 shininess = 1.0f,
             f32 normal_magnitude = 1.0f,
             BRDFType brdf = phong,
             u8 flags = PHONG | LAMBERT) :
             pixel_shader{pixel_shader},
             mesh_shader{mesh_shader},
             brdf{brdf},
             flags{flags},
             texture_count{texture_count},
             roughness{roughness},
             shininess{shininess},
             normal_magnitude{normal_magnitude},
             diffuse{diffuse},
             specular{specular}
             {}
};

struct MaterialHas {
    bool diffuse{false};
    bool specular{false};

    explicit MaterialHas(u8 flags) : diffuse{(bool)(flags & (u8)LAMBERT)}, specular{(bool)(flags & (u8)PHONG | flags & (u8)BLINN)} {};
};

struct MaterialUses {
    bool blinn{false};
    bool phong{false};

    explicit MaterialUses(u8 flags) : blinn{(bool)(flags & (u8)BLINN)}, phong{(bool)(flags & (u8)PHONG)} {};
};

struct Shaded {
    Color color;
    vec3 position, normal, viewing_direction, viewing_origin, reflected_direction, light_direction, diffuse;
    vec2i coords;
    f32 opacity, u, v, uv_area;
    f64 depth;
    Material *material;
    Geometry *geometry;
};


struct AmbientLight{ Color color; };
struct Light {
    vec3 position_or_direction, color;
    f32 intensity = 1.0f;
    bool is_directional;

    Light(const vec3 &position_or_direction, const vec3 &color, f32 intensity = 1.0f, bool is_directional = false) :
            position_or_direction{position_or_direction},
            color{color},
            intensity{intensity},
            is_directional{is_directional}
    {}
};
