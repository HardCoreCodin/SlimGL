#pragma once

#include "gl_uniforms.h"
#include "gl_shader.h"
#include "gl_texture.h"
#include "gl_text.h"
#include "math/mat3_constructurs.h"

namespace gl {
	namespace renderer {
		namespace text_pass {
			const char* vertex_shader = R"(#version 330
uniform mat3 mvp;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 size;
layout(location = 2) in vec2 glyph_uv_pos;
layout(location = 3) in vec2 glyph_uv_size;
layout(location = 4) in vec3 glyph_color;

out vec2 uv;
out vec3 color;

void main() {
    uv = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
	gl_Position = vec4(vec3((mvp * vec3(uv * size + position, 1.0)).xy, 0.0), 1.0);
	gl_Position.y = -gl_Position.y;
    uv *= glyph_uv_size;
    uv += glyph_uv_pos;
    color = glyph_color;
})";

			const char* fragment_shader = R"(#version 330
uniform sampler2D font;

in vec2 uv;
in vec3 color;

void main() {
    float d = texture(font, uv).r;
    float aaf = fwidth(d);
    float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);
	if (alpha < 0.0001) discard;
    gl_FragColor = vec4(color, alpha);
})";

			GLProgram program;
			GLMatrix3Uniform mvp{"mvp"};

			void init() {
				if (program.id) return;

				GLShader shaders[] = {
					{GL_VERTEX_SHADER, nullptr, vertex_shader},
					{GL_FRAGMENT_SHADER, nullptr, fragment_shader}
				};
				program.compile(shaders, 2);
				glUseProgram(program.id);

				mvp.setLocation(program.id);

				glUseProgram(0);
			}

			void render(GLint x, GLint y, GLsizei width, GLsizei height, const GLTexture &font_texture, const GLText *texts, u32 texts_count) {
				if (!program.id) return;

				glUseProgram(program.id);
				glViewport(x , y, width, height);
				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glCullFace(GL_FRONT);
				mat3 view_projection{
					2.0f / (f32)width, 0.0f, 0.0f,
					0.0f, 2.0f / (f32)height, 0.0f,
					0.0f, 0.0f, 1.0f
				};
                font_texture.bind(GL_TEXTURE0);

				for (u32 i = 0; i < texts_count; i++) {
					mvp.update(Mat3(texts[i].text2D->transform) * view_projection);
					texts[i].render();
				}
				glCullFace(GL_BACK);
			}
		}
	}
}