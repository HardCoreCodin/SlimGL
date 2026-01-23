#pragma once

#include "gl_uniforms.h"
#include "gl_shader.h"
#include "gl_texture.h"
#include "gl_text.h"
#include "math/mat3_constructurs.h"

namespace gl {
	namespace renderer {
		namespace text_pass {
			const char* vertex_shader2D = R"(#version 330
uniform mat3 mvp;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 size;
layout(location = 2) in vec2 glyph_uv_pos;
layout(location = 3) in vec2 glyph_uv_size;
layout(location = 4) in vec3 glyph_color;

out vec2 uv;
out vec3 color;

void main() {
    uv = vec2(float((gl_VertexID >> 1) & 1), float(gl_VertexID & 1));
	gl_Position = vec4(vec3((mvp * vec3(uv * size + position, 1.0)).xy, 0.0), 1.0);
	gl_Position.y = -gl_Position.y;
	uv.y = 1.0 - uv.y;
    uv *= glyph_uv_size;
    uv += glyph_uv_pos;
    color = glyph_color;
})";
			const char* vertex_shader3D = R"(#version 330
uniform mat4 mvp;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 size;
layout(location = 2) in vec2 glyph_uv_pos;
layout(location = 3) in vec2 glyph_uv_size;
layout(location = 4) in vec3 glyph_color;

out vec2 uv;
out vec3 color;

void main() {
    uv = vec2(float((gl_VertexID >> 1) & 1), float(gl_VertexID & 1));
	gl_Position = mvp * vec4(vec3((vec3(uv * size + position, 1.0)).xy, 0.0), 1.0);
	uv.y = 1.0 - uv.y;
    uv *= glyph_uv_size;
    uv += glyph_uv_pos;
    color = glyph_color;
})";

			const char* fragment_shader = R"(#version 330
uniform sampler2D font;

in vec2 uv;
in vec3 color;


float screenPxRange() {
	vec2 unitRange = vec2(2.0)/vec2(textureSize(font, 0));
	vec2 screenTexSize = vec2(1.0)/fwidth(uv);
	return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

void main() {
	vec3 msd = texture(font, uv).rgb;
	float sd = median(msd.r, msd.g, msd.b);
	float screenPxDistance = screenPxRange()*(sd - 0.5);
	float alpha = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	if (alpha < 0.0001) discard;
    gl_FragColor = vec4(color, alpha);
})";

			GLProgram program2D;
			GLProgram program3D;
			GLMatrix4Uniform mvp3D{"mvp"};
			GLMatrix3Uniform mvp2D{"mvp"};

			void init() {
				if (program2D.id) return;

				GLShader shaders[] = {
					{GL_VERTEX_SHADER, nullptr, vertex_shader2D},
						{GL_FRAGMENT_SHADER, nullptr, fragment_shader},
						{GL_VERTEX_SHADER, nullptr, vertex_shader3D}
				};

				program2D.compile(shaders, 2);
				glUseProgram(program2D.id);
				mvp2D.setLocation(program2D.id);
				glUseProgram(0);

				program3D.compile(shaders+1, 2);
				glUseProgram(program3D.id);
				mvp3D.setLocation(program3D.id);
				glUseProgram(0);
			}

			void render2D(GLint x, GLint y, GLsizei width, GLsizei height, const GLTexture &font_texture, const GLText *texts, u32 texts_count) {
				if (!program2D.id) return;

				glUseProgram(program2D.id);
				glViewport(x , y, width, height);
				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				mat3 view_transform{
					2.0f / (f32)width, 0.0f, 0.0f,
					0.0f, 2.0f / (f32)height, 0.0f,
					0.0f, 0.0f, 1.0f
				};
                font_texture.bind(GL_TEXTURE0);

				for (u32 i = 0; i < texts_count; i++) {
					if (texts[i].text->is2D) {
						mvp2D.update(Mat3(texts[i].text->transform2D) * view_transform);
						texts[i].render();
					}
				}
			}

			void render3D(GLint x, GLint y, GLsizei width, GLsizei height, const mat4 &view_projection_matrix, const GLTexture &font_texture, const GLText *texts, u32 texts_count) {
				if (!program3D.id) return;

				glUseProgram(program3D.id);
				glViewport(x , y, width, height);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				font_texture.bind(GL_TEXTURE0);

				for (u32 i = 0; i < texts_count; i++) {
					if (!texts[i].text->is2D) {
						Transform model = texts[i].text->transform3D;
						model.scale *= {-1,-1,1};
						mvp3D.update(Mat4(model) * view_projection_matrix);
						texts[i].render();
					}
				}
			}
		}
	}
}