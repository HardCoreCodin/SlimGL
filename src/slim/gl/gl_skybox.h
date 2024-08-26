#pragma once


#include "gl_texture.h"
#include "gl_shader.h"
#include "gl_uniforms.h"
#include "../math/mat3.h"


union CubeMapSet {
	struct {
		CubeMapImages skybox;
		CubeMapImages radiance;
		CubeMapImages irradiance;
	};
	CubeMapImages array[3];

    CubeMapSet() {}
};


namespace gl {    
    namespace skybox {
        const char* vertex_shader = R"(#version 330
out vec3 pos;
uniform mat3 camera_ray_matrix;

void main() {
    vec2 vertices[3] = vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));
    gl_Position = vec4(vertices[gl_VertexID], 0, 1);
    pos = camera_ray_matrix * vec3(vertices[gl_VertexID], 1);
})";
        const char* fragment_shader = R"(#version 330
in vec3 pos;
out vec4 col;
uniform samplerCube skybox;

void main()
{
	col = texture(skybox, pos);
})";

        GLMatrix3Uniform camera_ray_matrix_uniform{"camera_ray_matrix"};
        GLProgram program;
        GLuint empty_vao;

        void init() {
            if (program.id) return;
            
            GLShader shaders[] = {
                {GL_VERTEX_SHADER, nullptr, vertex_shader},
                {GL_FRAGMENT_SHADER, nullptr, fragment_shader}
            };
            program.compile(shaders, 2);
            camera_ray_matrix_uniform.setLocation(program.id);
            glGenVertexArrays(1, &empty_vao);
        }

        void draw(const GLCubeMapTexture &texture, const mat3 &camera_ray_matrix) {
            glDepthMask(GL_FALSE);

            glUseProgram(program.id);
        
            camera_ray_matrix_uniform.update(camera_ray_matrix);
            texture.bind();
        
            glBindVertexArray(empty_vao); 
            glDrawArrays(GL_TRIANGLES, 0, 3);

	        glDepthMask(GL_TRUE);
        }
    }
}