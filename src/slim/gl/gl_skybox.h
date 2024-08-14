#pragma once


#include "gl_texture.h"
#include "gl_shader.h"
#include "gl_uniforms.h"
#include "../math/mat3.h"


struct GLSkyBox {
    static GLMatrix3Uniform camera_ray_matrix_uniform;
    static GLProgram program;
    static GLuint empty_vao;

    GLSkyBoxTexture texture;

    GLSkyBox(const SkyBoxImages &images) {
        texture.load(images);

        if (!program.id) {
            GLShader shaders[] = {
                {GL_VERTEX_SHADER, nullptr, R"(#version 330
out vec3 pos;

void main() {
    vec2 vertices[3] = vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));
    gl_Position = vec4(vertices[gl_VertexID], 0, 1);
    pos = vec3(vertices[gl_VertexID], 1);
})"
            },
            {GL_FRAGMENT_SHADER, nullptr, R"(#version 330
in vec3 pos;
out vec4 col;
uniform mat3 camera_ray_matrix;
uniform samplerCube skybox;

void main()
{
	col = texture(skybox, camera_ray_matrix * pos);
})"
                }
            };
            program.compile(shaders, 2);
            camera_ray_matrix_uniform.setLocation(program.id);
            glGenVertexArrays(1, &empty_vao);
        }
    }

    void draw(const mat3 &camera_ray_matrix) {
        glDepthMask(GL_FALSE);

        glUseProgram(program.id);
        
        camera_ray_matrix_uniform.update(camera_ray_matrix);
        texture.bind();
        
        glBindVertexArray(empty_vao); 
        glDrawArrays(GL_TRIANGLES, 0, 3);

	    glDepthMask(GL_TRUE);
    }
};


GLMatrix3Uniform GLSkyBox::camera_ray_matrix_uniform{"camera_ray_matrix"};
GLProgram GLSkyBox::program;
GLuint GLSkyBox::empty_vao;