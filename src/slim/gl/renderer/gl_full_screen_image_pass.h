#pragma once

#pragma once

#include "gl_uniforms.h"
#include "gl_shader.h"
#include "gl_texture.h"
#include "gl_text.h"

namespace gl {
	namespace renderer {
		namespace full_screen_image_pass {
			const char* vertex_shader = R"VERTEX_SHADER(
#version 330

out vec3 in_position;

void main()
{
    vec2 vertices[3] = vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));
    vec2 uv = vertices[gl_VertexID];

    in_position = vec3(uv, 1);
    gl_Position = vec4(uv, 0, 1);
}
)VERTEX_SHADER";

			const char* fragment_shader = R"FRAGMENT_SHADER(
#version 330

in vec3 in_position;
out vec4 out_color;

uniform sampler2D image;

void main()
{
	out_color = vec4(vec3(texture(image, in_position.xy * 0.5f + 0.5f)), 1.0f);
}
)FRAGMENT_SHADER";

			GLProgram program;
        	GLuint empty_vao;

			void init() {
				if (program.id) return;

				GLShader shaders[] = {
					{GL_VERTEX_SHADER, nullptr, vertex_shader},
					{GL_FRAGMENT_SHADER, nullptr, fragment_shader}
				};
				program.compile(shaders, 2);
				glUseProgram(program.id);
            	glGenVertexArrays(1, &empty_vao);

				glUseProgram(0);
			}

			void draw(const GLTexture &texture) {
				glDepthMask(GL_FALSE);

				glUseProgram(program.id);

				texture.bind(GL_TEXTURE0);

				glBindVertexArray(empty_vao);
				glDrawArrays(GL_TRIANGLES, 0, 3);

				glDepthMask(GL_TRUE);
			}
		}
	}
}
