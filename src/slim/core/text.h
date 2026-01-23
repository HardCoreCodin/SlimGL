#pragma once

#include "./base.h"
#include "./string.h"
#include "./transform.h"
#include "./transform2d.h"


struct Font {
    struct GlyphMetric {
        vec2 pos;
        vec2 size;
        vec2 uv_pos;
        vec2 uv_size;
        vec2 advance;
    };

    u32 atlas_width;
    u32 atlas_height;
    GlyphMetric metrics[256];
};


struct Text {
    const Font& font;
    String string;
    Color color = White;
    Transform2D transform2D = {};
    Transform transform3D = {};
    bool is2D = true;

    Text(const Font& font, const String &string, Color color = White, const Transform2D &transform = {})
    :
        font{font},
        string{string},
        color{color},
        transform2D{transform},
        transform3D{},
        is2D{true}
    {}

    Text(const Font& font, const String &string, Color color = White, const Transform &transform = {})
    :
        font{font},
        string{string},
        color{color},
        transform2D{},
        transform3D{transform},
        is2D{false}
    {}
};



#define FREE_GLYPH_BUFFER_CAP (640 * 1000)