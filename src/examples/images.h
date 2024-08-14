#pragma once

#include "../slim/serialization/image.h"
#include "../slim/serialization/texture.h"

enum ImageID {
    Floor_Albedo,
    Floor_Normal,

    Dog_Albedo,
    Dog_Normal,
//
//    Cathedral_SkyboxColor,
//    Cathedral_SkyboxRadiance,
//    Cathedral_SkyboxIrradiance,
//
//    Bolonga_SkyboxColor,
//    Bolonga_SkyboxRadiance,
//    Bolonga_SkyboxIrradiance,

    ImageCount
};

ByteColorImage floor_albedo_image;
ByteColorImage floor_normal_image;
ByteColorImage dog_albedo_image;
ByteColorImage dog_normal_image;
ByteColorImage *images = &floor_albedo_image;
const char *image_file_names[ImageCount] {
    "floor_albedo.image",
    "floor_normal.image",
    "dog_albedo.image",
    "dog_normal.image"
};

ImagePack<ByteColor> image_pack{ImageCount, images, image_file_names, __FILE__};

Texture floor_albedo_texture;
Texture floor_normal_texture;
Texture dog_albedo_texture;
Texture dog_normal_texture;
Texture *textures = &floor_albedo_texture;
const char *texture_file_names[ImageCount] {
    "floor_albedo.texture",
    "floor_normal.texture",
    "dog_albedo.texture",
    "dog_normal.texture"
};

TexturePack texture_pack{ImageCount, textures, texture_file_names, __FILE__};