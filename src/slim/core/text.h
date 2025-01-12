#pragma once

#include "./base.h"
#include "./string.h"
#include "./transform2d.h"


struct Font
{
    struct GlyphMetric {
        f32 ax; // advance.x
        f32 ay; // advance.y

        f32 bw; // bitmap.width;
        f32 bh; // bitmap.rows;

        f32 bl; // bitmap_left;
        f32 bt; // bitmap_top;

        f32 tx; // x offset of glyph in texture coordinates
    };

    u32 atlas_width;
    u32 atlas_height;
    GlyphMetric metrics[128];
};


struct Text2D {
    const Font& font;
    String string;
    Color color = White;
    Transform2D transform = {};

    Text2D(const Font& font, const String &string, Color color = White, const Transform2D &transform = {})
    :
        font{font},
        string{string},
        color{color},
        transform{transform}
    {}
};



#define FREE_GLYPH_BUFFER_CAP (640 * 1000)