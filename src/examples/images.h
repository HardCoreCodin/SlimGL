#pragma once

#include "../slim/serialization/image.h"

enum ImageID {
    Floor_Albedo,
    Floor_Normal,

    Dog_Albedo,
    Dog_Normal,

    ImageCount
};

RawImage images[4];
const char *image_file_names[ImageCount] {
    "floor_albedo.raw_image",
    "floor_normal.raw_image",
    "dog_albedo.raw_image",
    "dog_normal.raw_image"
};
ImagePack<u8> image_pack{ImageCount, images, image_file_names, __FILE__};

constexpr u8 CUBE_MAP_SETS_COUNT = 2;

union CubeMapSets {
    struct {
        CubeMapSet cathedral;
        CubeMapSet bolonga;
    };
    CubeMapSet array[CUBE_MAP_SETS_COUNT];

    CubeMapSets() {}
};
CubeMapSets cube_map_sets;

const char *bolonga_akybox[6] {
    "bolonga_color_full_px.raw_image",
    "bolonga_color_full_nx.raw_image",
    "bolonga_color_full_py.raw_image",
    "bolonga_color_full_ny.raw_image",
    "bolonga_color_full_pz.raw_image",
    "bolonga_color_full_nz.raw_image"
};
ImagePack<u8> bolonga_skybox_image_pack{6, cube_map_sets.bolonga.skybox.array, bolonga_akybox, __FILE__};

const char *bolonga_radiance[6] {
    "bolonga_radiance_px.raw_image",
    "bolonga_radiance_nx.raw_image",
    "bolonga_radiance_py.raw_image",
    "bolonga_radiance_ny.raw_image",
    "bolonga_radiance_pz.raw_image",
    "bolonga_radiance_nz.raw_image"
};
ImagePack<u8> bolonga_radiance_image_pack{6, cube_map_sets.bolonga.radiance.array, bolonga_radiance, __FILE__};

const char *bolonga_irradiance[6] {
    "bolonga_irradiance_px.raw_image",
    "bolonga_irradiance_nx.raw_image",
    "bolonga_irradiance_py.raw_image",
    "bolonga_irradiance_ny.raw_image",
    "bolonga_irradiance_pz.raw_image",
    "bolonga_irradiance_nz.raw_image"
};
ImagePack<u8> bolonga_irradiance_image_pack{6, cube_map_sets.bolonga.irradiance.array, bolonga_irradiance, __FILE__};


const char *cathedral_akybox[6] {
    "cathedral_color_full_px.raw_image",
    "cathedral_color_full_nx.raw_image",
    "cathedral_color_full_py.raw_image",
    "cathedral_color_full_ny.raw_image",
    "cathedral_color_full_pz.raw_image",
    "cathedral_color_full_nz.raw_image"
};
ImagePack<u8> cathedral_skybox_image_pack{6, cube_map_sets.cathedral.skybox.array, cathedral_akybox, __FILE__};

const char *cathedral_radiance[6] {
    "cathedral_radiance_px.raw_image",
    "cathedral_radiance_nx.raw_image",
    "cathedral_radiance_py.raw_image",
    "cathedral_radiance_ny.raw_image",
    "cathedral_radiance_pz.raw_image",
    "cathedral_radiance_nz.raw_image"
};
ImagePack<u8> cathedral_radiance_image_pack{6, cube_map_sets.cathedral.radiance.array, cathedral_radiance, __FILE__};

const char *cathedral_irradiance[6] {
    "cathedral_irradiance_px.raw_image",
    "cathedral_irradiance_nx.raw_image",
    "cathedral_irradiance_py.raw_image",
    "cathedral_irradiance_ny.raw_image",
    "cathedral_irradiance_pz.raw_image",
    "cathedral_irradiance_nz.raw_image"
};
ImagePack<u8> cathedral_irradiance_image_pack{6, cube_map_sets.cathedral.irradiance.array, cathedral_irradiance, __FILE__};