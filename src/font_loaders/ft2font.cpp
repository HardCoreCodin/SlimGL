#include "../slim/core/image.h"
#include "../slim/serialization/image.h"
#include "../slim/serialization/font.h"
#include "../slim/platforms/win32_base.h"

#include <ft2build.h>
#include FT_FREETYPE_H

// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02

#define FREE_GLYPH_FONT_SIZE 64

int main(int argc, char *argv[]) {
    char* font_src_file_path = argv[1];
    char* font_trg_file_path = argv[2];
    char* image_file_path = argv[3];

    RawImage image;
    Font font{0, 0};


    FT_Library library = {0};

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "ERROR: could initialize FreeType2 library\n");
        exit(1);
    }

    // const char *const font_file_path = "./VictorMono-Regular.ttf";

    FT_Face face;
    error = FT_New_Face(library, font_src_file_path, 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "ERROR: `%s` has an unknown format\n", font_src_file_path);
        exit(1);
    } else if (error) {
        fprintf(stderr, "ERROR: could not load file `%s`\n", font_src_file_path);
        exit(1);
    }

    FT_UInt pixel_size = FREE_GLYPH_FONT_SIZE;
    error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
    if (error) {
        fprintf(stderr, "ERROR: could not set pixel size to %u\n", pixel_size);
        exit(1);
    }

    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    for (int i = 32; i < 128; ++i) {
        if (FT_Load_Char(face, i, load_flags)) {
            fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
            exit(1);
        }

        font.atlas_width += face->glyph->bitmap.width;
        if (font.atlas_height < face->glyph->bitmap.rows) {
            font.atlas_height = face->glyph->bitmap.rows;
        }
    }

    vec2i leftover{(i32)(font.atlas_width % 4), (i32)(font.atlas_height % 4)};
    vec2i padding{leftover.x ? 4 - leftover.x : 0, leftover.y ? 4 - leftover.y : 0};
    font.atlas_width += padding.x;
    font.atlas_height += padding.y;

    image.width = image.stride = font.atlas_width;
    image.height = font.atlas_height;
    image.size = image.width * image.height;
    image.content = new u8[image.size];
    image.flags.flags = 0;

    for (u32 i = 0; i < image.size; i++) image.content[i] = 0;

    u32 horizontal_offset = 0;
    for (int i = 32; i < 128; ++i) {
        if (FT_Load_Char(face, i, load_flags)) {
            fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
            exit(1);
        }

        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
            fprintf(stderr, "ERROR: could not render glyph of a character with code %d\n", i);
            exit(1);
        }

        font.metrics[i].ax = (f32)(face->glyph->advance.x >> 6);
        font.metrics[i].ay = (f32)(face->glyph->advance.y >> 6);
        font.metrics[i].bw = (f32)face->glyph->bitmap.width;
        font.metrics[i].bh = (f32)face->glyph->bitmap.rows;
        font.metrics[i].bl = (f32)face->glyph->bitmap_left;
        font.metrics[i].bt = (f32)face->glyph->bitmap_top;
        font.metrics[i].tx = (f32)horizontal_offset / (f32)font.atlas_width;

        if (face->glyph->bitmap.rows && face->glyph->bitmap.width) {
            u32 trg_offset = horizontal_offset;
            u32 src_offset = 0;
            for (u32 y = 0; y < face->glyph->bitmap.rows; y++) {
                for (u32 x = 0; x < face->glyph->bitmap.width; x++) {
                    image.content[trg_offset + x] = face->glyph->bitmap.buffer[src_offset + x];
                }

                trg_offset += font.atlas_width;
                src_offset += face->glyph->bitmap.width;
            }

            horizontal_offset += face->glyph->bitmap.width;
        }
    }

    save(image, image_file_path);
    save(font, font_trg_file_path);

    return 0;
}