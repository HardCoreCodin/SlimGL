#pragma once

#include "./gl_uniforms.h"
#include "../scene/material.h"

namespace gl {
    struct GLMaterial {
        GLVector3Uniform albedo;
        GLVector3Uniform F0;
        GLFloatUniform roughness;
        GLFloatUniform metalness;
        GLFloatUniform normal_strength;
        GLUIntUniform flags;

        explicit GLMaterial(const char *variable_name, int index = -1) :
            albedo{variable_name, "albedo", nullptr, index},
            F0{variable_name, "F0", nullptr, index},
            roughness{variable_name, "roughness", nullptr, index},
            metalness{variable_name, "metalness", nullptr, index},
            normal_strength{variable_name, "normal_strength", nullptr, index},
            flags{variable_name, "flags", nullptr, index}
        {}

        void init(const GLuint shader_program_id) {
            albedo.setLocation(shader_program_id);
            F0.setLocation(shader_program_id);
            roughness.setLocation(shader_program_id);
            metalness.setLocation(shader_program_id);
            normal_strength.setLocation(shader_program_id);
            flags.setLocation(shader_program_id);
        }

        void update(const Material &material) {
            albedo.update(material.albedo);
            F0.update(material.reflectivity);
            roughness.update(material.roughness);
            metalness.update(material.metalness);
            normal_strength.update(material.normal_magnitude );
            flags.update(material.flags);
        }

        void setBases(const char* bases)
        {
            albedo.bases = F0.bases = roughness.bases = metalness.bases = normal_strength.bases = flags.bases = bases;
        }
    };
}