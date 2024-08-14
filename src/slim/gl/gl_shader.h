#pragma once

#include "./gl_core.h"
#include "../core/string.h"


struct GLShader {
    GLenum type;
    const char* code = nullptr;
    const char* file = nullptr;
    GLuint id = 0;

    GLShader(GLenum type, const char* file, const char* code = nullptr) : type{type}, code{code}, file{file} {
        compile();
    }

    u64 read(const char* new_file = nullptr) {
        if (new_file) file = new_file;

        u64 code_size = 0;
        code = (char*)os::readEntireFile(file, &code_size);
        if (!code_size) code = nullptr;

        return code_size;
    }

    GLuint compile() {
        id = 0;

        u64 code_size = 0;
        if (!code) {
            if (file) {
                code_size = read();
                if (code_size)
                    return 0;
            } else
                return 0;
        }
        if (!code_size) while (code[code_size]) code_size++;

        id = glCreateShader(type);

        const GLchar* code_array[1];
        code_array[0] = code;

        GLint code_sizes[1];
        code_sizes[0] = (GLint)code_size;

        glShaderSource(id, 1, code_array, code_sizes);
        glCompileShader(id);

        GLint result = 0;
        GLchar eLog[1024] = { 0 };

        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (!result)
        {
            glGetShaderInfoLog(id, sizeof(eLog), NULL, eLog);
//            printf("Error compiling the %d shader: '%s'\n", type, eLog);
            id = 0;
            return 0;
        }

        return id;
    }
};

struct GLProgram {
    GLuint id = 0;

    GLProgram(GLShader *shaders = nullptr, u8 shader_count = 0) {
        if (shader_count != 0 && shaders != nullptr)
            compile(shaders, shader_count);
    }

    GLuint compile(GLShader *shaders, u8 shader_count) {
        id = 0;
        if (shader_count == 0 || shaders == nullptr) return 0;

        id = glCreateProgram();

        if (!id)
        {
//            printf("Error creating shader program!\n");
            return 0;
        }

        GLShader *shader = shaders;
        for (u8 i = 0; i < shader_count; i++, shader++) {
            if (!shader->id) {
                if (!shader->compile()) {
                    id = 0;
                    return 0;
                }
            }
            glAttachShader(id, shader->id);
        }

        GLint result = 0;
        GLchar eLog[1024] = { 0 };

        glLinkProgram(id);
        glGetProgramiv(id, GL_LINK_STATUS, &result);
        if (!result)
        {
            glGetProgramInfoLog(id, sizeof(eLog), NULL, eLog);
//            printf("Error linking program: '%s'\n", eLog);
            id = 0;
            return 0;
        }

        glValidateProgram(id);
        glGetProgramiv(id, GL_VALIDATE_STATUS, &result);
        if (!result)
        {
            glGetProgramInfoLog(id, sizeof(eLog), NULL, eLog);
//            printf("Error validating program: '%s'\n", eLog);
            id = 0;
            return 0;
        }

//        if (uniform_locations != nullptr && uniform_location_count != 0)
//            getLocations(uniform_locations, uniform_location_count);

        return id;
    }

//    bool getLocations(GLUniformLocation *uniform_locations, u8 uniform_location_count) const {
//        if (uniform_location_count == 0 || uniform_locations == nullptr)
//            return false;
//
//        GLUniformLocation *location = uniform_locations;
//        for (u8 i = 0; i < uniform_location_count; i++, location++) {
//            if (location->name) {
//                location->id = glGetUniformLocation(id, location->name);
//            } else
//                return false;
//        }
//        return true;
//    }

    void destroy() {
        if (id != 0)
        {
            glDeleteProgram(id);
            id = 0;
        }
    }
};