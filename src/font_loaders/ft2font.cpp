#define NOMINMAX

#include "../slim/core/image.h"
#include "../slim/serialization/image.h"
#include "../slim/serialization/font.h"
#include "../slim/platforms/win32_base.h"

#undef INFINITE
#include <msdfgen/msdfgen.h>
#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdf-atlas-gen.h>

using namespace msdfgen;
using namespace msdf_atlas;


int main(int argc, char *argv[]) {
    char* font_src_file_path = argv[1];
    char* font_trg_file_path = argv[2];
    char* image_file_path = argv[3];

    font_src_file_path = "C:\\Windows\\Fonts\\arialbd.ttf";
    font_trg_file_path = "arialbd.font";
    image_file_path    = "arialbd.raw_image";
    // "VictorMono-Regular.ttf";//argv[1];


    if (FreetypeHandle *ft = initializeFreetype()) {
        if (FontHandle *fh = loadFont(ft, font_src_file_path)) {
            std::vector<GlyphGeometry> glyphs;
            FontGeometry fontGeometry(&glyphs);
            
            Charset charset;
            for (uint32_t c = 32; c <= 126; c++) charset.add(c);
            fontGeometry.loadCharset(fh, 1.0, charset);
            
            for (GlyphGeometry &glyph : glyphs)
                glyph.edgeColoring(&edgeColoringInkTrap, 3.0, 0);
            
            TightAtlasPacker packer;
            packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::SQUARE);
            packer.setScale(40.0);
            packer.setPixelRange(2.0);
            packer.setMiterLimit(1.0);
            packer.setPadding(0);
            packer.pack(glyphs.data(), glyphs.size());
            
            int width = 0, height = 0;
            packer.getDimensions(width, height);

            ImmediateAtlasGenerator<float, 3, msdfGenerator, BitmapAtlasStorage<byte, 3>> generator(width, height);

            GeneratorAttributes attributes;
            generator.setAttributes(attributes);
            generator.setThreadCount(4);
            generator.generate(glyphs.data(), glyphs.size());
            
            BitmapConstRef<byte, 3> bitmap = (BitmapConstRef<byte, 3>)generator.atlasStorage();

            vec2i leftover{
                (i32)(width % 4), 
                (i32)(height % 4)
            };
            vec2i padding{
                leftover.x ? 4 - leftover.x : 0, 
                leftover.y ? 4 - leftover.y : 0
            };
            width += padding.x;
            height += padding.y;

            RawImage image;
            image.updateDimensions(width, height);
            image.content = new u8[image.size * 3];
            image.flags.flags = 0;
            image.flags.channel = 1;
            image.flags.linear = 1;
            for (u32 i = 0; i < image.size*3; i++) image.content[i] = 0;
            
            u32 horizontal_offset = 0;
            u32 trg_offset = horizontal_offset;
            u32 src_offset = 0;
            for (u32 y = 0; y < bitmap.height; y++) {
                for (u32 x = 0; x < bitmap.width; x++)
                    for (u32 i = 0; i < 3; i++)
                        image.content[trg_offset + x * 3 + i] = bitmap.pixels[src_offset + x * 3 + i];

                trg_offset += width * 3;
                src_offset += bitmap.width * 3;
            }
            save(image, image_file_path);

            const auto& metrics = fontGeometry.getMetrics();
            
            const f32 world_scale = 1.0f / (f32)(metrics.ascenderY - metrics.descenderY);
            const vec2 uv_scale = 1.0f / vec2((f32)width, height);
                
            double al, ab, ar, at;
            double pl, pb, pr, pt;

            Font font{width, height};
            
            for (uint32_t c = 32; c <= 126; c++) {
                Font::GlyphMetric &glyph_metric = font.metrics[c];
                auto glyph = fontGeometry.getGlyph(c);
                glyph->getQuadAtlasBounds(al, ab, ar, at);
                glyph->getQuadPlaneBounds(pl, pb, pr, pt);
                glyph_metric.pos = vec2((f32)pl, -pt) * world_scale;
                glyph_metric.size = vec2((f32)(pr - pl), pt - pb) * world_scale;
                glyph_metric.uv_pos = vec2((f32)al, ab) * uv_scale;
                glyph_metric.uv_size = vec2((f32)(ar - al), at - ab) * uv_scale;
                glyph_metric.advance = vec2((f32)(glyph->getAdvance()), 0.0f);
            }
            save(font, font_trg_file_path);

            destroyFont(fh);
        }
        deinitializeFreetype(ft);
    }
        
    return 0;
}