#pragma once

#include "../core/string.h"

struct HUDLine {
    String title, alternate_value;
    NumberString value;
    enum ColorID title_color = BrightGrey;
    enum ColorID value_color = BrightGrey;
    enum ColorID alternate_value_color = Grey;
    bool *use_alternate = nullptr;

    HUDLine(enum ColorID default_color = BrightGrey) :
            title{},
            alternate_value{},
            value{},
            title_color{default_color},
            value_color{default_color} {}
    HUDLine(const char* title,
            enum ColorID default_color = BrightGrey) :
            title{title},
            alternate_value{},
            value{},
            title_color{default_color},
            value_color{default_color} {}
    HUDLine(const char* title,
            const char* value,
            enum ColorID default_color = BrightGrey) :
                    title{title},
                    alternate_value{},
                    value{value},
                    title_color{default_color},
                    alternate_value_color{default_color}
                    {}
    HUDLine(const char* title,
            const char* value,
            const char* alt_value,
            bool *alt = nullptr,
            enum ColorID value_color = BrightGrey,
            enum ColorID alt_color = Grey,
            enum ColorID title_color = BrightGrey) :
            title{title},
            alternate_value{alt_value},
            value{value},
            title_color{title_color},
            value_color{          ((alt && value_color == BrightGrey && alt_color == Grey) ? DarkYellow  : value_color)},
            alternate_value_color{((alt && value_color == BrightGrey && alt_color == Grey) ? BrightGreen : value_color)},
            use_alternate{alt}
    {}
};

struct HUDSettings {
    u32 line_count = 0;
    f32 line_height = 1.2f;
    enum ColorID default_color = BrightGrey;

    HUDSettings(u32 line_count = 0,
                f32 line_height = 1.2f,
                ColorID default_color = BrightGrey) : line_count{line_count}, line_height{line_height}, default_color{default_color} {}
};
struct HUD {
    HUDSettings settings;
    HUDLine *lines = nullptr;
    i32 left = 10, top = 10;
    bool enabled = true;

    HUD() = default;
    HUD(HUDSettings settings,
        HUDLine *lines,
        i32 left = 10,
        i32 top = 10) :
        settings{settings}, lines{lines}, left{left}, top{top} {
        if (settings.default_color != BrightGrey) for (u32 i = 0; i < settings.line_count; i++)
            lines[i].title_color = lines[i].alternate_value_color = lines[i].value_color = settings.default_color;
    }
    HUD(HUDSettings settings, memory::AllocateMemory allocate_memory = nullptr, i32 left = 10, i32 top = 10) : settings{settings}, left{left}, top{top} {
        if (settings.line_count) {
            lines = (HUDLine*)allocate_memory(settings.line_count * sizeof(HUDLine));
            for (u32 i = 0; i < settings.line_count; i++)
                lines[i] = HUDLine{settings.default_color};
        }
    }
};
