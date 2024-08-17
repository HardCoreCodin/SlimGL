#pragma once

#include "./text.h"
#include "./rectangle.h"
#include "../core/hud.h"

void drawHUD(const HUD &hud, const Canvas &canvas, const RectI *viewport_bounds = nullptr) {
    u32 max_length = 0;

    HUDLine *line = hud.lines;
    bool alt;
    for (u32 i = 0; i < hud.settings.line_count; i++, line++) {
        alt = line->use_alternate && *line->use_alternate;
        const char *text = alt ? line->alternate_value.char_ptr : line->value.string.char_ptr;
        max_length = Max(max_length, (line->title.length + String::getLength(text)));
    }
    i32 hud_right  = (i32)(hud.left + max_length * FONT_WIDTH);
    i32 hut_bottom = (i32)(hud.top + hud.settings.line_count * (u32)(hud.settings.line_height * (f32)FONT_HEIGHT));
    RectI rect{
        hud.left - FONT_WIDTH / 2,
        hud_right + FONT_WIDTH / 2,
        hud.top - FONT_HEIGHT / 2,
        hut_bottom + FONT_HEIGHT / 2
    };
    _fillRect(rect, canvas, DarkGrey, 0.75f, viewport_bounds);

    u32 x = hud.left;
    u32 y = hud.top;
    line = hud.lines;
    for (u32 i = 0; i < hud.settings.line_count; i++, line++) {
        alt = line->use_alternate && *line->use_alternate;
        ColorID color = alt ? line->alternate_value_color : line->value_color;
        char *text = alt ? line->alternate_value.char_ptr : line->value.string.char_ptr;
        _drawText(line->title.char_ptr, x, y, canvas, line->title_color, 1.0f, viewport_bounds);
        _drawText(text, x + (i32)line->title.length * FONT_WIDTH, y, canvas, color, 1.0f, viewport_bounds);
        y += (i32)(hud.settings.line_height * (f32)FONT_HEIGHT);
    }
}