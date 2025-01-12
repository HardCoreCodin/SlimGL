#pragma once

#include "gl_uniforms.h"
#include "gl_shader.h"
#include "gl_texture.h"
#include "gl_text.h"

#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (128  / FONT_COLS)
#define FONT_CHAR_HEIGHT (64 / FONT_ROWS)
#define FONT_SCALE 5

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

namespace gl {
	namespace renderer {
		namespace tiled_text_pass {
			const char* vertex_shader = R"(#version 330
#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)

uniform vec2 resolution;
uniform vec2 scale;

layout(location = 0) in ivec2 tile;
layout(location = 1) in int ch;
layout(location = 2) in vec4 fg_color;
layout(location = 3) in vec4 bg_color;

out vec2 uv;
flat out int glyph_ch;
out vec4 glyph_fg_color;
out vec4 glyph_bg_color;

vec2 project_point(vec2 point)
{
    return 2.0 * point / resolution;
}

void main() {
    uv = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
    vec2 char_size = vec2(float(FONT_CHAR_WIDTH), float(FONT_CHAR_HEIGHT));
    vec2 pos = tile * char_size * scale;
    gl_Position = vec4(project_point(uv * char_size * scale + pos), 0.0, 1.0);
    glyph_ch = ch;

    glyph_fg_color = fg_color;
    glyph_bg_color = bg_color;
})";

			const char* fragment_shader = R"(#version 330
#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_CHAR_WIDTH_UV  (float(FONT_CHAR_WIDTH) / float(FONT_WIDTH))
#define FONT_CHAR_HEIGHT_UV (float(FONT_CHAR_HEIGHT) / float(FONT_HEIGHT))

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

uniform sampler2D font;

in vec2 uv;
flat in int glyph_ch;
in vec4 glyph_fg_color;
in vec4 glyph_bg_color;

void main() {
    int ch = glyph_ch;
    if (!(ASCII_DISPLAY_LOW <= ch && ch <= ASCII_DISPLAY_HIGH)) {
        ch = 63;
    }

    int index = ch - 32;
    float x = float(index % FONT_COLS) * FONT_CHAR_WIDTH_UV;
    float y = float(index / FONT_COLS) * FONT_CHAR_HEIGHT_UV;
    vec2 pos = vec2(x, y + FONT_CHAR_HEIGHT_UV);
    vec2 size = vec2(FONT_CHAR_WIDTH_UV, -FONT_CHAR_HEIGHT_UV);
    vec2 t = pos + size * uv;

    vec4 tc = texture(font, t);
    gl_FragColor = glyph_bg_color * (1.0 - tc.x) + tc.x * glyph_fg_color;
})";
			GLProgram program;
			GLVector2Uniform resolution{"resolution"};
			GLVector2Uniform scale{"scale"};

			void init() {
				if (program.id) return;

				GLShader shaders[] = {
					{GL_VERTEX_SHADER, nullptr, vertex_shader},
					{GL_FRAGMENT_SHADER, nullptr, fragment_shader}
				};
				program.compile(shaders, 2);
				glUseProgram(program.id);

				resolution.setLocation(program.id);
				scale.setLocation(program.id);
				scale.update(5.0f);

				glUseProgram(0);
			}

			void render(GLint x, GLint y, GLsizei width, GLsizei height, GLTexture *font_texture, const GLTiledText *texts, u32 texts_count) {
				if (!program.id) return;

				glUseProgram(program.id);
				glViewport(x , y, width, height);

				glClear(GL_DEPTH_BUFFER_BIT);
//				resolution.update({});
                glUniform2f(resolution.id, (GLfloat)width, (GLfloat)height);
				font_texture->bind(GL_TEXTURE0);

				for (u32 i = 0; i < texts_count; i++) {
					texts[i].render();
				}
			}
		}
	}
}