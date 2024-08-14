#pragma once

#include <stdio.h>
#include "./gl_shader.h"


struct GLUniformLocation {
    const char* variable_name = nullptr;
    const char* field_name = nullptr;
    const char* bases = nullptr;
    int index = -1;

    mutable GLint id = 0;

    explicit GLUniformLocation(const char* variable_name = nullptr, const char* field_name = nullptr, const char* bases = nullptr, int index = -1) :
        variable_name{variable_name}, field_name{field_name}, bases{bases}, index{index}, id{0} {}

    void setLocation(const GLuint shader_program_id) const {
        static char resolved_name[256];

        const char* _variable_name = variable_name == nullptr ? "" : variable_name;
        if (field_name || bases) {
            const char* _field_name = field_name == nullptr ? "" : field_name;
            const char* _bases = bases == nullptr ? "" : bases;
            if (index == -1)
                snprintf(resolved_name, sizeof(resolved_name), "%s%s.%s", _variable_name, _bases, _field_name);
            else
                snprintf(resolved_name, sizeof(resolved_name), "%s[%d]%s.%s", _variable_name, index, _bases, _field_name);
        } else {
            if (index == -1)
                snprintf(resolved_name, sizeof(resolved_name), "%s", _variable_name);
            else
                snprintf(resolved_name, sizeof(resolved_name), "%s[%d]", _variable_name, index);
        }

        id = glGetUniformLocation(shader_program_id, resolved_name);
    }
};

struct GLFloatUniform : GLUniformLocation {
    explicit GLFloatUniform(const char* variable_name = nullptr, const char* field_name = nullptr, const char* bases = nullptr, int index = -1) :
        GLUniformLocation{variable_name, field_name, bases, index} {}

    void update(GLfloat value) {
        glUniform1f(id, value);
    }
};

struct GLIntUniform : GLUniformLocation {
    explicit GLIntUniform(const char* variable_name = nullptr, const char* field_name = nullptr, const char* bases = nullptr, int index = -1) :
        GLUniformLocation{variable_name, field_name, bases, index} {}

    void update(GLint value) {
        glUniform1i(id, value);
    }
};

struct GLVector3Uniform : GLUniformLocation {
    explicit GLVector3Uniform(const char* variable_name = nullptr, const char* field_name = nullptr, const char* bases = nullptr, int index = -1) :
        GLUniformLocation{variable_name, field_name, bases, index} {}

    void update(const vec3 &vector) {
        glUniform3f(id, vector.x, vector.y, vector.z);
    }
};

struct GLMatrix3Uniform : GLUniformLocation {
    explicit GLMatrix3Uniform(const char* variable_name = nullptr, const char* field_name = nullptr, const char* bases = nullptr, int index = -1) :
        GLUniformLocation{variable_name, field_name, bases, index} {}

    void update(const mat3 &matrix) const {
        glUniformMatrix3fv(id, 1, GL_FALSE, &matrix.X.x);
    }
};

struct GLMatrix4Uniform : GLUniformLocation {
    explicit GLMatrix4Uniform(const char* variable_name = nullptr, const char* field_name = nullptr, const char* bases = nullptr, int index = -1) :
        GLUniformLocation{variable_name, field_name, bases, index} {}

    void update(const mat4 &matrix) const {
        glUniformMatrix4fv(id, 1, GL_FALSE, &matrix.X.x);
    }
};

struct GLTextureUniform : GLUniformLocation {
    explicit GLTextureUniform(const char* variable_name = nullptr, const char* field_name = nullptr, const char* bases = nullptr, int index = -1) :
        GLUniformLocation{variable_name, field_name, bases, index} {}

    void update(GLint texture_unit) const {
        glUniform1i(id, texture_unit);
    }
};