#pragma once

#include "gl_common.h"
#include "gl_shader.h"
#include "gl_uniforms.h"
#include "../math/mat4.h"


namespace gl {
    static const char* inline_quad_line_vertex_shader = R"(#version 330
uniform mat4 mvp;

void main() {
    vec3 vertices[8] = vec3[8](
        vec3(-1, -1, 0), vec3(-1,  1, 0), 
        vec3(-1,  1, 0), vec3( 1,  1, 0), 
        vec3( 1,  1, 0), vec3( 1, -1, 0), 
        vec3( 1, -1, 0), vec3(-1, -1, 0)
    );
	gl_Position = mvp * vec4(vertices[gl_VertexID], 1.0);
})";

    static const char* inline_cube_line_vertex_shader = R"(#version 330
uniform mat4 mvp;

void main() {
    vec3 vertices[24] = vec3[24](
        vec3(-1, -1, -1), vec3(-1,  1, -1), 
        vec3(-1,  1, -1), vec3( 1,  1, -1), 
        vec3( 1,  1, -1), vec3( 1, -1, -1), 
        vec3( 1, -1, -1), vec3(-1, -1, -1),

        vec3(-1, -1,  1), vec3(-1,  1,  1), 
        vec3(-1,  1,  1), vec3( 1,  1,  1), 
        vec3( 1,  1,  1), vec3( 1, -1,  1), 
        vec3( 1, -1,  1), vec3(-1, -1,  1),

        vec3(-1, -1, -1), vec3(-1, -1,  1), 
        vec3(-1,  1, -1), vec3(-1,  1,  1), 
        vec3( 1,  1, -1), vec3( 1,  1,  1), 
        vec3( 1, -1, -1), vec3( 1, -1,  1)
    );
	gl_Position = mvp * vec4(vertices[gl_VertexID], 1.0);
})";


    namespace quad {
        GLProgram program;
        GLMatrix4Uniform mvp{"mvp"};
        GLVector3Uniform line_color{"line_color"};
        GLuint empty_vao;

        void init() { 
            if (program.id) return;

            GLShader shaders[] = {
                {GL_VERTEX_SHADER, nullptr, inline_quad_line_vertex_shader},
                {GL_FRAGMENT_SHADER, nullptr, uniform_color_fragment_shader}
            };
            program.compile(shaders, 2);
            mvp.setLocation(program.id);
            line_color.setLocation(program.id);
            glGenVertexArrays(1, &empty_vao);
        }

        void draw(const mat4 &mvp_matrix, const vec3& color = {1}) {
		    init();
            glUseProgram(program.id);

            mvp.update(mvp_matrix);
            line_color.update(color);
        
            glBindVertexArray(empty_vao);
            glDrawArrays(GL_LINES, 0, 8);
        }    
    }
    
    namespace cube {
        GLProgram program;
        GLMatrix4Uniform mvp{"mvp"};
        GLVector3Uniform line_color{"line_color"};
        GLuint empty_vao;

        void init() { 
            if (program.id) return;

            GLShader shaders[] = {
                {GL_VERTEX_SHADER, nullptr, inline_cube_line_vertex_shader},
                {GL_FRAGMENT_SHADER, nullptr, uniform_color_fragment_shader}
            };
            program.compile(shaders, 2);
            mvp.setLocation(program.id);
            line_color.setLocation(program.id);
            glGenVertexArrays(1, &empty_vao);
        }

        void draw(const mat4 &mvp_matrix, const vec3& color = {1}) {
		    if (!program.id) init();
            glUseProgram(program.id);

            mvp.update(mvp_matrix);
            line_color.update(color);
        
            glBindVertexArray(empty_vao);
            glDrawArrays(GL_LINES, 0, 24);
        }    
    }

    namespace edge {
        GLProgram program;
        GLMatrix4Uniform mvp{"mvp"};
        GLVector3Uniform line_color{"line_color"};
        GLuint empty_vao;

        void init() { 
            if (program.id) return;

            GLShader shaders[] = {
                {GL_VERTEX_SHADER, nullptr, base_vertex_shader},
                {GL_FRAGMENT_SHADER, nullptr, uniform_color_fragment_shader}
            };
            program.compile(shaders, 2);
            mvp.setLocation(program.id);
            line_color.setLocation(program.id);
        }
        
        void draw(GLuint VAO, GLuint IBO, GLsizei count, const mat4 &mvp_matrix, const vec3& color = {1}) {
            init();
            glUseProgram(program.id);

            mvp.update(mvp_matrix);
            line_color.update(color);
        
            glBindVertexArray(VAO);
            if (IBO) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
                glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            } else 
                glDrawArrays(GL_LINES, 0, count);
        }
    }
}


struct GLEdges {
    GLsizei count = 0;
	GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint IBO = 0;    

    GLEdges() = default;

    GLEdges(const EdgeVertexIndices *line_vertex_indices, u32 line_count, const vec3 *vertex_positions, u32 vertex_count) {
        load(line_vertex_indices, line_count, vertex_positions, vertex_count);
    }

    GLEdges(const vec3 *line_vertices, u32 line_vertices_count) {
        load(line_vertices, line_vertices_count);
    }

    void draw(const mat4 &mvp_matrix, const vec3& color = {1}) {
        if (VAO && count) gl::edge::draw(VAO, IBO, count, mvp_matrix, color);
    }

    void load(const vec3 *line_vertices, u32 line_vertices_count) {
        destroy();
        count = line_vertices_count;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * count, line_vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0);
    }

    void load(const EdgeVertexIndices *line_vertex_indices, u32 line_count, const vec3 *vertex_positions, u32 vertex_count, bool use_index_buffer = true) {       
        if (!use_index_buffer) {
            u32 line_vertices_count = line_count * 2;
            vec3 *line_vertices = new vec3[line_vertices_count];

            for (int i = 0; i < line_count; i++) {
                line_vertices[i * 2 + 0] = vertex_positions[line_vertex_indices[i].from];
                line_vertices[i * 2 + 1] = vertex_positions[line_vertex_indices[i].to  ];
            }

            load(line_vertices, line_vertices_count);
        
            delete[] line_vertices;
        
            return;
        }

        destroy();
        count = line_count * 2;
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * count, (GLuint*)line_vertex_indices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertex_count, vertex_positions, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0);
    }

    void destroy() {
        if (IBO) glDeleteBuffers(1, &IBO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
        IBO = VBO = VAO = count = 0;
    }
};
