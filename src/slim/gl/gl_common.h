#pragma once

namespace gl {
	const char* empty_fragment_shader = R"(#version 330
void main()
{
})";

	const char* base_vertex_shader = R"(#version 330
layout (location = 0) in vec3 pos;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp * vec4(pos, 1.0);
})";

    const char* uniform_color_fragment_shader = R"(#version 330
out vec4 out_color;

uniform vec3 uniform_color;

void main(void) {
	out_color = vec4(uniform_color, 1.0);
})";
}