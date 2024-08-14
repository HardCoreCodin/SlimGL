#pragma once

#include "./base.h"


u8* componentsToByteColor(u8 *component, ByteColor &byte_color, ImageInfo &info) {
    byte_color.B = *(component++);
    byte_color.G = *(component++);
    byte_color.R = *(component++);
    if (info.flags.alpha)
        byte_color.A = *(component++);
    else
        byte_color.A = 0;

    return component;
}

u8* componentsToPixel(u8 *component, Pixel *pixel, ImageInfo &info, f32 gamma = 2.2f) {
    ByteColor byte_color;
    component = componentsToByteColor(component, byte_color, info);

    *pixel = byte_color;
    if (info.flags.normal) {
        f32 r = pixel->color.r * 2.0f - 1.0f;
        f32 g = pixel->color.g * 2.0f - 1.0f;
        f32 l_rcp = 1.0f / sqrtf(r*r + g*g + 1.0f);
        r *= l_rcp;
        g *= l_rcp;
        f32 b = sqrtf(1.0f - r*r - g*g);
        pixel->color.r = r * 0.5f + 0.5f;
        pixel->color.g = g * 0.5f + 0.5f;
        pixel->color.b = b * 0.5f + 0.5f;
    }
    if (!info.flags.linear) pixel->color.applyGamma(gamma);

    return component;
}

void componentsToPixels(u8 *components, ImageInfo &info, Pixel *pixels, f32 gamma = 2.2f) {
    Pixel* pixel = pixels;
    u8 *component = components;
    u32 count = info.width * info.height;
    for (u32 i = 0; i < count; i++, pixel++)
        component = componentsToPixel(component, pixel, info, gamma);
}

void componentsToByteColors(u8 *components, ImageInfo &info, ByteColor *byte_colors, f32 gamma = 2.2f) {
    ByteColor* byte_color = byte_colors;
    u8 *component = components;
    u32 count = info.size;
    Pixel pixel;
    if (info.flags.linear && !info.flags.normal)
        for (u32 i = 0; i < count; i++, byte_color++)
            component = componentsToByteColor(component, *byte_color, info);
    else
        for (u32 i = 0; i < count; i++, byte_color++) {
            component = componentsToPixel(component, &pixel, info, gamma);
            *byte_color = pixel.color.toByteColor();
        }
}

void componentsToChannels(u8 *components, ImageInfo &info, f32 *channels, f32 gamma = 2.2f) {
    f32* channel = channels;
    u8 *component = components;
    u32 component_count = info.flags.alpha ? 4 : 3;
    u32 count = component_count * info.size;
    Pixel pixel;
    for (u32 i = 0; i < count; i++) {
        component = componentsToPixel(component, &pixel, info, gamma);

        *(channel++) = pixel.color.red;
        *(channel++) = pixel.color.green;
        *(channel++) = pixel.color.blue;
        if (info.flags.alpha)
            *(channel++) = pixel.opacity;
    }
}

void flipImage(const u8 *components, ImageInfo &info, u8 *flipped) {
    u32 component_count = info.flags.alpha ? 4 : 3;
    u32 component_stride = component_count * info.width;
    u32 trg_offset = component_stride * (info.height - 1);
    u32 src_offset = 0;
    for (u32 y = 0; y < info.height; y++) {
        for (u32 x = 0; x < info.width; x++) {
            flipped[trg_offset++] = components[src_offset++];
            flipped[trg_offset++] = components[src_offset++];
            flipped[trg_offset++] = components[src_offset++];
            if (info.flags.alpha)
                flipped[trg_offset++] = components[src_offset++];
        }
        trg_offset -= component_stride * 2;
    }
}

void tileImage(u8 *components, ImageInfo &image_info, u8 *tiled) {
    u32 component_count = image_info.flags.alpha ? 4 : 3;
    u8 *component, *tiled_component = tiled;
    TiledGridInfo grid{image_info};
    u32 X, Y = 0;
    for (grid.row = 0; grid.row < grid.rows; grid.row++) {
        X = 0;
        u32 tile_height = grid.row == grid.bottom_row ? grid.bottom_row_tile_height : image_info.tile_height;
        for (grid.column = 0; grid.column < grid.columns; grid.column++) {
            u32 tile_width = grid.column == grid.right_column ? grid.right_column_tile_stride : image_info.tile_width;

            component = components + component_count * (image_info.stride * Y + X);
            for (u32 y = 0; y < tile_height; y++) {
                for (u32 x = 0; x < tile_width; x++) {
                    *(tiled_component++) = *(component++);
                    *(tiled_component++) = *(component++);
                    *(tiled_component++) = *(component++);
                    if (image_info.flags.alpha)
                        *(tiled_component++) = *(component++);
                }

                component += component_count * (image_info.stride - tile_width);
            }

            X += image_info.tile_width;
        }
        Y += image_info.tile_height;
    }
}