#define NOMINMAX

#include "../slim/core/image.h"
#include "../slim/serialization/image.h"
#include "../slim/serialization/font.h"
#include "../slim/platforms/win32_base.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#undef INFINITE
#include <msdfgen/msdfgen.h>
#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdf-atlas-gen.h>


// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02

#define FREE_GLYPH_FONT_SIZE 64

using namespace msdfgen;


using namespace msdf_atlas;

bool generateAtlas(const char *fontFilename) {
    bool success = false;
    // Initialize instance of FreeType library
    if (msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype()) {
        // Load font file
        if (msdfgen::FontHandle *fh = msdfgen::loadFont(ft, fontFilename)) {
            // Storage for glyph geometry and their coordinates in the atlas
            std::vector<GlyphGeometry> glyphs;
            // FontGeometry is a helper class that loads a set of glyphs from a single font.
            // It can also be used to get additional font metrics, kerning information, etc.
            FontGeometry fontGeometry(&glyphs);
            // Load a set of character glyphs:
            // The second argument can be ignored unless you mix different font sizes in one atlas.
            // In the last argument, you can specify a charset other than ASCII.
            // To load specific glyph indices, use loadGlyphs instead.
            msdf_atlas::Charset charset;
            for (uint32_t c = 32; c <= 126; c++) charset.add(c);
            fontGeometry.loadCharset(fh, 1.0, charset);
            // Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
            const double maxCornerAngle = 3.0;
            for (GlyphGeometry &glyph : glyphs)
                glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
            // TightAtlasPacker class computes the layout of the atlas.
            TightAtlasPacker packer;
            // Set atlas parameters:
            // setDimensions or setDimensionsConstraint to find the best value
            packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::SQUARE);
            // setScale for a fixed size or setMinimumScale to use the largest that fits
            packer.setScale(40.0);
            // setPixelRange or setUnitRange
            packer.setPixelRange(2.0);
            packer.setMiterLimit(1.0);
            packer.setPadding(0);
            // Compute atlas layout - pack glyphs
            packer.pack(glyphs.data(), glyphs.size());
            // Get final atlas dimensions
            int width = 0, height = 0;
            packer.getDimensions(width, height);
            // The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
            ImmediateAtlasGenerator<
                float, // pixel type of buffer for individual glyphs depends on generator function
                3, // number of atlas color channels
                msdfGenerator, // function to generate bitmaps for individual glyphs
                BitmapAtlasStorage<byte, 3> // class that stores the atlas bitmap
                // For example, a custom atlas storage class that stores it in VRAM can be used.
            > generator(width, height);
            // GeneratorAttributes can be modified to change the generator's default settings.
            GeneratorAttributes attributes;
            generator.setAttributes(attributes);
            generator.setThreadCount(4);
            // Generate atlas bitmap
            generator.generate(glyphs.data(), glyphs.size());
            // The atlas bitmap can now be retrieved via atlasStorage as a BitmapConstRef.
            // The glyphs array (or fontGeometry) contains positioning data for typesetting text.
            //success = my_project::submitAtlasBitmapAndLayout(generator.atlasStorage(), glyphs);

            msdfgen::BitmapConstRef<byte, 3> bitmap = (msdfgen::BitmapConstRef<byte, 3>)generator.atlasStorage();

    
            vec2i leftover{(i32)(width % 4), (i32)(height % 4)};
            vec2i padding{leftover.x ? 4 - leftover.x : 0, leftover.y ? 4 - leftover.y : 0};
            width += padding.x;
            height += padding.y;
                char* image_file_path = "arialbd.raw_image";//argv[3];

            RawImage image;
            image.updateDimensions(width, height);
            //image.width = image.stride = width;
            //image.height = height;
            //image.size = width * height;
            image.content = new u8[image.size * 3];
            image.flags.flags = 0;
            image.flags.channel = 1;
            image.flags.linear = 1;
            //image.mip_count = 0;
            for (u32 i = 0; i < image.size*3; i++) image.content[i] = 0;//bitmap.pixels[i];
            
            u32 horizontal_offset = 0;
            u32 trg_offset = horizontal_offset;
            u32 src_offset = 0;
            for (u32 y = 0; y < bitmap.height; y++) {
                for (u32 x = 0; x < bitmap.width; x++) {
                    for (u32 i = 0; i < 3; i++) {
                        image.content[trg_offset + x * 3 + i] = bitmap.pixels[src_offset + x * 3 + i];
                    }
                }
                trg_offset += width * 3;
                src_offset += bitmap.width * 3;
            }
            save(image, image_file_path);

            const auto& metrics = fontGeometry.getMetrics();
            
            const f32 world_scale = 1.0f / (f32)(metrics.ascenderY - metrics.descenderY);
            const vec2 uv_scale = 1.0f / vec2((f32)width, height);

            Font font{width, height};
            
            for (uint32_t c = 32; c <= 126; c++) {
                Font::GlyphMetric &glyph_metric = font.metrics[c];
                auto glyph = fontGeometry.getGlyph(c);
                
                double al, ab, ar, at;
                double pl, pb, pr, pt;
                glyph->getQuadAtlasBounds(al, ab, ar, at);
                glyph->getQuadPlaneBounds(pl, pb, pr, pt);
                glyph_metric.pos = vec2((f32)pl, -pt) * world_scale;
                glyph_metric.size = vec2((f32)(pr - pl), pt - pb) * world_scale;
                glyph_metric.uv_pos = vec2((f32)al, ab) * uv_scale;
                glyph_metric.uv_size = vec2((f32)(ar - al), at - ab) * uv_scale;
                glyph_metric.advance = vec2((f32)(glyph->getAdvance()), 0.0f);
            }
            save(font, "arialbd.font");
            // Cleanup
            msdfgen::destroyFont(fh);
        }
        msdfgen::deinitializeFreetype(ft);
    }
    return success;
}


int main(int argc, char *argv[]) {
    generateAtlas("C:\\Windows\\Fonts\\arialbd.ttf");
    return 0;
    char* font_src_file_path = "C:\\Windows\\Fonts\\arialbd.ttf";// "VictorMono-Regular.ttf";//argv[1];
    //char* font_trg_file_path = argv[2];
    char* image_file_path = "arialbd.raw_image";//argv[3];

    RawImage image;
    Font font{0, 0};

    /*
    FT_Library library = {0};

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "ERROR: could initialize FreeType2 library\n");
        exit(1);
    }
    */
    FreetypeHandle *ft = initializeFreetype();
    
    if (!ft) {
        fprintf(stderr, "ERROR: could initialize FreeType2 library\n");
        exit(1);
    }

    FontHandle *font_handle = loadFont(ft, font_src_file_path);
    if (!font_handle) {
        fprintf(stderr, "ERROR: could initialize FreeType2 library\n");
        exit(1);
    }

    msdf_atlas::Charset charset;
    for (uint32_t c = 0x0020; c <= 0x00FF; c++) charset.add(c);

    double font_scale = 1.0;
    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    msdf_atlas::FontGeometry font_geometry = msdf_atlas::FontGeometry(&glyphs); 
    font_geometry.loadCharset(font_handle, font_scale, charset);

    double font_size = 40.0;
    msdf_atlas::TightAtlasPacker atlas_packer;
    atlas_packer.setPixelRange(2.0);
    atlas_packer.setMiterLimit(1.0);
    atlas_packer.setPadding(0);
    atlas_packer.setScale(font_size);
    atlas_packer.pack(glyphs.data(), (int)glyphs.size());
    int width, height;
    atlas_packer.getDimensions(width, height);
    double scale = atlas_packer.getScale();

    /*
    uint64_t coloringSeed = 0;
    unsigned long long glyph_seed = 0;
    for (msdf_atlas::GlyphGeometry& glyph : glyphs)
    {
        glyph_seed *= 6364136223846793005ull;
        glyph.edgeColoring(msdfgen::edgeColoringInkTrap, 3.0, glyph_seed);
    }
*/
    //GenerateAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)scale, glyphs, font_geometry, width, height);
    
    
    /*
    FT_Face &face = fontHandle->face;

    FT_UInt pixel_size = FREE_GLYPH_FONT_SIZE;
    FT_Error error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
    if (error) {
        fprintf(stderr, "ERROR: could not set pixel size to %u\n", pixel_size);
    } else {
        Shape shape;
        if (loadGlyph(shape, fontHandle, 'A', FONT_SCALING_EM_NORMALIZED)) {
            shape.normalize();
            //                      max. angle
            edgeColoringSimple(shape, 3.0);
            //          output width, height
            Bitmap<float, 3> msdf(32, 32);
            //                            scale, translation (in em's)
            msdfgen::SDFTransformation t(Projection(32.0, Vector2(0.125, 0.125)), msdfgen::Range(0.125));
            msdfgen::generateMSDF(msdf, shape, t);
            //savePng(msdf, "output.png");
        }            
    }
    

    destroyFont(fontHandle);
      deinitializeFreetype(ft);
    */

    // const char *const font_file_path = "./VictorMono-Regular.ttf";
    /*
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
    */

    /*
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
    */

    /*
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
    */
    //save(font, font_trg_file_path);

    return 0;
}