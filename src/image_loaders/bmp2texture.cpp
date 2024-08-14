#include "./slim/platforms/win32_bitmap.h"
#include "./slim/serialization/texture.h"


struct PixelQuad {
    Pixel TL, TR, BL, BR;

    Color getAverageColor() const {
        return {
                0.25f * (TL.color.r + TR.color.r + BL.color.r + BR.color.r),
                0.25f * (TL.color.g + TR.color.g + BL.color.g + BR.color.g),
                0.25f * (TL.color.b + TR.color.b + BL.color.b + BR.color.b)
        };
    }
};

enum CubeMapLoaderMode {
    CubeMapLoaderMode_None,
    CubeMapLoaderMode_Main,
    CubeMapLoaderMode_Top,
    CubeMapLoaderMode_Bottom
};

struct TextureMipLoader {
    u32 width, height;
    Pixel *texels;
    PixelQuad *texel_quads;

    void init(u32 Width, u32 Height) {
        width = Width;
        height = Height;
        texels = new Pixel[width * height];
        texel_quads = new PixelQuad[(width + 1) * (height + 1)];
    }

    void load(bool wrap,
              CubeMapLoaderMode cube_map_loader_mode = CubeMapLoaderMode_None,
              Pixel *main_faces_texels = nullptr,
              Pixel *top_face_texels = nullptr,
              Pixel *bottom_face_texels = nullptr) {
        PixelQuad *TL, *TR, *BL, *BR;
        bool L, R, T, B;
        const u32 stride = width + 1;
        const u32 last_x = width - 1;
        const u32 last_y = height - 1;
        const u32 l = 0;
        const u32 r = width;
        Pixel *texel = texels;
        PixelQuad *top_line = texel_quads;
        PixelQuad *bottom_line = top_line + height * stride;
        PixelQuad *current_line = top_line, *next_line = top_line + stride;
        for (u32 y = 0; y < height; y++, current_line += stride, next_line += stride) {
            T = (y == 0);
            B = (y == last_y);

            for (u32 x = 0; x < width; x++, texel++) {
                TL = current_line + x;
                TR = current_line + x + 1;
                BL = next_line + x;
                BR = next_line + x + 1;

                L = (x == 0);
                R = (x == last_x);

                TL->BR = TR->BL = BL->TR = BR->TL = *texel;
                if (     L) TL->BL = BL->TL = *texel;
                else if (R) TR->BR = BR->TR = *texel;
                if (wrap) {
                    if (     L) current_line[r].BR = next_line[r].TR = *texel;
                    else if (R) current_line[l].BL = next_line[l].TL = *texel;
                    if (T) {
                        bottom_line[x].BR = bottom_line[x + 1].BL = *texel;
                        if (     L) bottom_line[r].BR = *texel;
                        else if (R) bottom_line[l].BL = *texel;
                    } else if (B) {
                        top_line[x].TR = top_line[x + 1].TL = *texel;
                        if (     L) top_line[r].TR = *texel;
                        else if (R) top_line[l].TL = *texel;
                    }
                } else {
                    if (T) {
                        TL->TR = TR->TL = *texel;

                        if (     L) TL->TL = *texel;
                        else if (R) TR->TR = *texel;

                    } else if (B) {
                        BL->BR = BR->BL = *texel;

                        if (     L) BL->BL = *texel;
                        else if (R) BR->BR = *texel;
                    }
                }
            }
        }

        if (cube_map_loader_mode) {
            u32 h = height;
            u32 w = h;
            u32 quad_w = (w + 1) * 4;
            u32 s = w * h;
            u32 last        = w - 1;
            u32 last_row    = w * last;
            u32 last_texel  = s - 1;
            u32 Lo = w * 0;
            u32 Fo = w * 1;
            u32 Ro = w * 2;
            u32 Bo = w * 3;

            switch (cube_map_loader_mode) {
                case CubeMapLoaderMode_Main: {
                    for (u32 i = 0; i < h; i++, Lo++, Ro++, Fo++, Bo++) {
                        top_line[1 + Lo].TL = top_line[Lo].TR = top_face_texels[w * i];
                        top_line[1 + Fo].TL = top_line[Fo].TR = top_face_texels[last_row + i];
                        top_line[1 + Ro].TL = top_line[Ro].TR = top_face_texels[last_texel - (w * i)];
                        top_line[1 + Bo].TL = top_line[Bo].TR = top_face_texels[w - i];

                        bottom_line[1 + Lo].BL = bottom_line[Lo].BR = bottom_face_texels[w * (last - i)];
                        bottom_line[1 + Fo].BL = bottom_line[Fo].BR = bottom_face_texels[i];
                        bottom_line[1 + Ro].BL = bottom_line[Ro].BR = bottom_face_texels[w * i + last];
                        bottom_line[1 + Bo].BL = bottom_line[Bo].BR = bottom_face_texels[last_row + (w - i)];
                    }
                    top_line[            0].TL = top_face_texels[0].lerpTo(texels[width - 1], 0.5f);
                    top_line[   quad_w - 1].TR = top_face_texels[0].lerpTo(texels[0        ], 0.5f);
                    bottom_line[         0].BL = bottom_face_texels[last_row].lerpTo(texels[width * last], 0.5f);
                    bottom_line[quad_w - 1].BR = bottom_face_texels[last_row].lerpTo(texels[0           ], 0.5f);
                } break;
                case CubeMapLoaderMode_Top: {
                    Pixel *left_face_top_texel  = main_faces_texels + Lo;
                    Pixel *front_face_top_texel = main_faces_texels + Fo;
                    Pixel *right_face_top_texel = main_faces_texels + Ro;
                    Pixel *back_face_top_texel  = main_faces_texels + Bo;

                    top_line[   0].TL = left_face_top_texel[ 0].lerpTo(back_face_top_texel[last],  0.5f);
                    top_line[   w].TR = back_face_top_texel[ 0].lerpTo(right_face_top_texel[last], 0.5f);
                    bottom_line[0].BL = front_face_top_texel[0].lerpTo(left_face_top_texel[last],  0.5f);
                    bottom_line[w].BR = right_face_top_texel[0].lerpTo(front_face_top_texel[last], 0.5f);

                    right_face_top_texel += last;
                    back_face_top_texel  += last;
                    PixelQuad *left_column  = texel_quads;
                    PixelQuad *right_column = texel_quads + w;

                    quad_w = w + 1;
                    for (u32 i = 0; i < h; i++,
                        left_column += quad_w,
                        right_column += quad_w,
                        left_face_top_texel++,
                        front_face_top_texel++,
                        right_face_top_texel--,
                        back_face_top_texel--) {
                        top_line[   1 + i].TL = top_line[         i].TR = *back_face_top_texel;
                        bottom_line[1 + i].BL = bottom_line[      i].BR = *front_face_top_texel;
                        left_column[    0].BL = left_column[ quad_w].TL = *left_face_top_texel;
                        right_column[   0].BR = right_column[quad_w].TR = *right_face_top_texel;
                    }
                } break;
                case CubeMapLoaderMode_Bottom: {
                    u32 main_faces_stride = w * 4;
                    u32 main_faces_last_row = main_faces_stride * last;
                    Lo += main_faces_last_row;
                    Ro += main_faces_last_row;
                    Fo += main_faces_last_row;
                    Bo += main_faces_last_row;

                    Pixel *left_face_bottom_texel  = main_faces_texels + Lo;
                    Pixel *front_face_bottom_texel = main_faces_texels + Fo;
                    Pixel *right_face_bottom_texel = main_faces_texels + Ro;
                    Pixel *back_face_bottom_texel  = main_faces_texels + Bo;

                    top_line[   0].TL = front_face_bottom_texel[0].lerpTo(left_face_bottom_texel[last],  0.5f);
                    top_line[   w].TR = right_face_bottom_texel[0].lerpTo(front_face_bottom_texel[last], 0.5f);
                    bottom_line[0].BL = left_face_bottom_texel[ 0].lerpTo(back_face_bottom_texel[last], 0.5f);
                    bottom_line[w].BR = back_face_bottom_texel[ 0].lerpTo(right_face_bottom_texel[last], 0.5f);

                    left_face_bottom_texel += last;
                    back_face_bottom_texel += last;
                    PixelQuad *left_column  = texel_quads;
                    PixelQuad *right_column = texel_quads + w;

                    quad_w = w + 1;
                    for (u32 i = 0; i < h; i++,
                        left_column += quad_w,
                        right_column += quad_w,
                        left_face_bottom_texel--,
                        front_face_bottom_texel++,
                        right_face_bottom_texel++,
                        back_face_bottom_texel--) {
                        top_line[   1 + i].TL = top_line[   i].TR = *front_face_bottom_texel;
                        bottom_line[1 + i].BL = bottom_line[i].BR = *back_face_bottom_texel;
                        left_column[ 0].BL = left_column[ quad_w].TL = *left_face_bottom_texel;
                        right_column[0].BR = right_column[quad_w].TR = *right_face_bottom_texel;
                    }
                } break;
                default: break;
            }
        }
    }
};

void loadMips(Texture &texture, TextureMipLoader *mips) {
    TextureMipLoader *current_mip = mips;
    TextureMipLoader *next_mip = current_mip + 1;

    u32 mip_width  = texture.width;
    u32 mip_height = texture.height;

    PixelQuad *colors_quad;
    u32 stride, offset, start;

    while (mip_width > 4 && mip_height > 4) {
        start = mip_width + 1;
        stride = start * 2;

        mip_width  /= 2;
        mip_height /= 2;

        next_mip->init(mip_width, mip_height);

        for (u32 y = 0; y < mip_height; y++) {
            offset = start + 1;

            for (u32 x = 0; x < mip_width; x++) {
                colors_quad = current_mip->texel_quads + offset;
                next_mip->texels[mip_width * y + x].color = colors_quad->getAverageColor();
                offset += 2;
            }

            start += stride;
        }

        next_mip->load(texture.flags.wrap);

        current_mip++;
        next_mip++;
    }
}

int main(int argc, char *argv[]) {
    Texture texture;

    char* bitmap_file_path = argv[1];
    char* texture_file_path = argv[2];
    for (u8 i = 3; i < (u8)argc; i++) {
        if (     argv[i][0] == '-' && argv[i][1] == 'f') texture.flags.flip = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'l') texture.flags.linear = true;
        else if (argv[i][0] == '-' && argv[i][1] == 't') texture.flags.tile = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'm') texture.flags.mipmap = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'w') texture.flags.wrap = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'n') texture.flags.normal = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'c') texture.flags.cubemap = true;
        else return 0;
    }

    u8* components = loadBitmap(bitmap_file_path, texture);
    TextureMipLoader *loader_mips = nullptr;

    texture.flags.channel = false;
    if (texture.flags.cubemap) {
        texture.flags.wrap = false;
        texture.flags.normal = false;
        texture.flags.mipmap = false;
        texture.flags.tile = false;
        texture.mip_count = 3;

        u32 face_width = texture.height;
        u32 main_width = face_width * 4;

        loader_mips = new TextureMipLoader[3];
        loader_mips[0].init(main_width, texture.height);
        loader_mips[1].init(face_width, texture.height);
        loader_mips[2].init(face_width, texture.height);

        Pixel *all_texels = new Pixel[texture.width * texture.height];
        componentsToPixels(components, texture, all_texels);

        Pixel *texel = all_texels;
        for (u32 y = 0; y < texture.height; y++)
            for (u32 x = 0; x < texture.width; x++, texel++)
                if (x < main_width)
                    loader_mips[0].texels[(main_width * y) + x] = *texel;
                else if (x < (main_width + face_width))
                    loader_mips[1].texels[(texture.height * y) + (x - main_width)] = *texel;
                else
                    loader_mips[2].texels[(texture.height * y) + (x - (main_width + face_width))] = *texel;

        delete[] all_texels;
        loader_mips[0].load(texture.flags.wrap, CubeMapLoaderMode_Main, nullptr, loader_mips[1].texels, loader_mips[2].texels);
        loader_mips[1].load(texture.flags.wrap, CubeMapLoaderMode_Top, loader_mips[0].texels);
        loader_mips[2].load(texture.flags.wrap, CubeMapLoaderMode_Bottom, loader_mips[0].texels);
    } else {
        texture.mip_count = 1;
        if (texture.flags.mipmap) {
            u32 mip_width  = texture.width;
            u32 mip_height = texture.height;
            while (mip_width > 4 && mip_height > 4) {
                mip_width /= 2;
                mip_height /= 2;
                texture.mip_count++;
            }
        }

        auto *mips = new TextureMipLoader[texture.mip_count];
        mips->init(texture.width, texture.height);
        componentsToPixels(components, texture, mips->texels);

        mips->load(texture.flags.wrap);
        if (texture.flags.mipmap) loadMips(texture, mips);
    }

    // Create final mips with 8-bit per channel from the float channels in the mip loaders:
    texture.mips = new TextureMip[texture.mip_count];
    TextureMip *mip = texture.mips;
    TextureMipLoader *loader_mip = loader_mips;
    for (u16 i = 0; i < texture.mip_count; i++, mip++, loader_mip++) {
        mip->width  = loader_mip->width;
        mip->height = loader_mip->height;
        mip->texel_quads = new TexelQuad[(mip->width + 1) * (mip->height + 1)];

        TexelQuad *texel_quad = mip->texel_quads;
        PixelQuad *loader_texel_quad = loader_mip->texel_quads;
        u32 texel_quads_count = (u32)(mip->width + 1) * (u32)(mip->height + 1);
        for (u32 t = 0; t < texel_quads_count; t++, texel_quad++, loader_texel_quad++) {
            texel_quad->R.TL = (u8)(loader_texel_quad->TL.color.r * FLOAT_TO_COLOR_COMPONENT);
            texel_quad->G.TL = (u8)(loader_texel_quad->TL.color.g * FLOAT_TO_COLOR_COMPONENT);
            texel_quad->B.TL = (u8)(loader_texel_quad->TL.color.b * FLOAT_TO_COLOR_COMPONENT);

            texel_quad->R.TR = (u8)(loader_texel_quad->TR.color.r * FLOAT_TO_COLOR_COMPONENT);
            texel_quad->G.TR = (u8)(loader_texel_quad->TR.color.g * FLOAT_TO_COLOR_COMPONENT);
            texel_quad->B.TR = (u8)(loader_texel_quad->TR.color.b * FLOAT_TO_COLOR_COMPONENT);

            texel_quad->R.BL = (u8)(loader_texel_quad->BL.color.r * FLOAT_TO_COLOR_COMPONENT);
            texel_quad->G.BL = (u8)(loader_texel_quad->BL.color.g * FLOAT_TO_COLOR_COMPONENT);
            texel_quad->B.BL = (u8)(loader_texel_quad->BL.color.b * FLOAT_TO_COLOR_COMPONENT);

            texel_quad->R.BR = (u8)(loader_texel_quad->BR.color.r * FLOAT_TO_COLOR_COMPONENT);
            texel_quad->G.BR = (u8)(loader_texel_quad->BR.color.g * FLOAT_TO_COLOR_COMPONENT);
            texel_quad->B.BR = (u8)(loader_texel_quad->BR.color.b * FLOAT_TO_COLOR_COMPONENT);

        }
    }

    save(texture, texture_file_path);

    return 0;
}