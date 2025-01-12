#pragma once

#include "gl_base.h"
#include "math/vec2.h"
#include "math/vec4.h"
#include "core/text.h"

#define GLYPH_BUFFER_CAP 1024
//
// struct GLTiledText {
//     struct Glyph {
//         vec2i tile;
//         int ch;
//         vec4 fg_color;
//         vec4 bg_color;
//
//         static void defineAttributes()
//         {
//             glEnableVertexAttribArray(0);
//     	    glVertexAttribIPointer(0, 2, GL_INT, sizeof(Glyph), (void*)offsetof(Glyph, tile));
//     	    glVertexAttribDivisor(0, 1);
//
//     	    glEnableVertexAttribArray(1);
//     	    glVertexAttribIPointer(1, 1, GL_INT, sizeof(Glyph), (void*)offsetof(Glyph, ch));
//     	    glVertexAttribDivisor(1, 1);
//
//     	    glEnableVertexAttribArray(2);
//     	    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Glyph), (void*)offsetof(Glyph, fg_color));
//     	    glVertexAttribDivisor(2, 1);
//
//     	    glEnableVertexAttribArray(3);
//     	    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Glyph), (void*)offsetof(Glyph, bg_color));
//     	    glVertexAttribDivisor(3, 1);
//         }
//     };
//
//     Glyph glyphs[GLYPH_BUFFER_CAP];
// 	GLsizei glyphs_count = 0;
//
//     GLuint VAO = 0;
//     GLuint VBO = 0;
//
// 	void create() {
//     	glGenVertexArrays(1, &VAO);
//     	glBindVertexArray(VAO);
//
//     	glGenBuffers(1, &VBO);
//     	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     	glBufferData(GL_ARRAY_BUFFER, sizeof(glyphs), glyphs, GL_DYNAMIC_DRAW);
//
//         Glyph::defineAttributes();
//
//     	glBindBuffer(GL_ARRAY_BUFFER, 0);
//     	glBindVertexArray(0);
//     }
//
//     void update(const Text2D &text) {
//         for (u32 i = 0; i < text.string.length; i++) {
//             Glyph &glyph = glyphs[glyphs_count++];
//             glyph.tile = vec2i((i32)i + (i32)text.transform.position.x, (i32)text.transform.position.y);
//             glyph.ch = text.string.char_ptr[i];
//             glyph.fg_color = Vec4(text.foreground_color);
//             glyph.bg_color = Vec4(text.background_color);
//         }
//
//     	glBindVertexArray(VAO);
//     	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     	glBufferSubData(GL_ARRAY_BUFFER, 0, glyphs_count * sizeof(Glyph), glyphs);
//     	glBindBuffer(GL_ARRAY_BUFFER, 0);
//     	glBindVertexArray(0);
//     }
//
//     void render() const
//     {
// 		glBindVertexArray(VAO);
// 		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, glyphs_count);
// 		glBindVertexArray(0);
//     }
// };


struct GLText {
    struct Glyph {
        vec2 pos;
        vec2 size;
        vec2 uv_pos;
        vec2 uv_size;
        Color color;

        static void defineAttributes()
        {
    	    glEnableVertexAttribArray(0);
    	    glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, sizeof(Glyph), (void*)offsetof(Glyph, pos));
    	    glVertexAttribDivisor(0, 1);

    	    glEnableVertexAttribArray(1);
    	    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, sizeof(Glyph), (void*)offsetof(Glyph, size));
    	    glVertexAttribDivisor(1, 1);

    	    glEnableVertexAttribArray(2);
    	    glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, sizeof(Glyph), (void*)offsetof(Glyph, uv_pos));
    	    glVertexAttribDivisor(2, 1);

    	    glEnableVertexAttribArray(3);
    	    glVertexAttribPointer(3,2, GL_FLOAT, GL_FALSE, sizeof(Glyph), (void*)offsetof(Glyph, uv_size));
    	    glVertexAttribDivisor(3, 1);

		    glEnableVertexAttribArray(4);
		    glVertexAttribPointer(4,3, GL_FLOAT, GL_FALSE, sizeof(Glyph), (void*)offsetof(Glyph, color));
		    glVertexAttribDivisor(4, 1);
        }
    };

    Glyph glyphs[GLYPH_BUFFER_CAP];
	GLsizei glyphs_count = 0;

    GLuint VAO = 0;
    GLuint VBO = 0;
	const Text2D *text2D = nullptr;

	void create() {
    	glGenVertexArrays(1, &VAO);
    	glBindVertexArray(VAO);

    	glGenBuffers(1, &VBO);
    	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    	glBufferData(GL_ARRAY_BUFFER, sizeof(glyphs), glyphs, GL_DYNAMIC_DRAW);

        Glyph::defineAttributes();

    	glBindBuffer(GL_ARRAY_BUFFER, 0);
    	glBindVertexArray(0);
    }

    void update(const Text2D &text) {
		text2D = &text;

        vec2 pos{};

        for (size_t i = 0; i < text.string.length; ++i) {
            const Font::GlyphMetric &metric = text.font.metrics[(int)text.string.char_ptr[i]];

            Glyph &glyph = glyphs[glyphs_count++];
            glyph.pos.x = pos.x + metric.bl;
            glyph.pos.y = pos.y - metric.bt;
            glyph.size.x = metric.bw;
            glyph.size.y = metric.bh;
            glyph.uv_pos.x = metric.tx;
            glyph.uv_pos.y = 0.0f;
            glyph.uv_size.x = metric.bw / (f32)text.font.atlas_width;
            glyph.uv_size.y = metric.bh / (f32)text.font.atlas_height;
            glyph.color = text.color;
            pos.x += metric.ax;
            pos.y += metric.ay;
        }

    	glBindVertexArray(VAO);
    	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    	glBufferSubData(GL_ARRAY_BUFFER, 0, glyphs_count * sizeof(Glyph), glyphs);
    	glBindBuffer(GL_ARRAY_BUFFER, 0);
    	glBindVertexArray(0);
    }

    void render() const
    {
		glBindVertexArray(VAO);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, glyphs_count);
		glBindVertexArray(0);
    }
};