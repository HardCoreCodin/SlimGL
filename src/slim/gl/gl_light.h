#pragma once

#include "./gl_shadow_map.h"

namespace gl {

    struct GLBaseLight {
        GLVector3Uniform color;
        GLFloatUniform intensity;

        explicit GLBaseLight(const char *variable_name, int index = -1) :
            color{variable_name, "color", nullptr, index},
            intensity{variable_name, "intensity", nullptr, index}
        {}

        void init(
            const GLuint shader_program_id, 
            u32 shadow_map_width = 2048,
            u32 shadow_map_height = 2048
        ) {
            color.setLocation(shader_program_id);
            intensity.setLocation(shader_program_id);
        }

        void update(const BaseLight &light) {
            color.update(light.color);
            intensity.update(light.intensity);
        }

        void setBases(const char* bases)
        {
            color.bases = intensity.bases = bases;
        }
    };


    struct GLDirectionalLight : GLBaseLight {
        GLVector3Uniform direction;
        GLTextureUniform shadow_map_texture{"shadow_map"};
        GLMatrix4Uniform transform{"directionalLightTransform"};
        char _base_light_namespace[64] = {""};

        explicit GLDirectionalLight(const char *variable_name, const char* base_light_namespace = nullptr, int index = -1) :
            GLBaseLight{variable_name, index},
            direction{variable_name, "direction", nullptr, index}
        {
            if (base_light_namespace)
                setSubBases(base_light_namespace);
        }

        void setSubBases(const char* base_light_namespace) {
            char current_bases[64];
            for (u8 i = 0; i < 64; i++) current_bases[i] = _base_light_namespace[i];

            snprintf(_base_light_namespace, sizeof(_base_light_namespace), "%s.%s", current_bases, base_light_namespace);

            GLBaseLight::setBases(_base_light_namespace);
        }

        void setBases(const char *base) {
            direction.bases = base;
        }

        void init(const GLuint shader_program_id) {
            GLBaseLight::init(shader_program_id);
            direction.setLocation(shader_program_id);
            shadow_map_texture.setLocation(shader_program_id);
            transform.setLocation(shader_program_id);
        }

        void update(const DirectionalLight &light) {
            GLBaseLight::update(light);
            direction.update(-Mat3(light.orientation).forward);
            transform.update(light.shadowMapMatrix());
        }
    };

    struct GLPointLight : GLBaseLight {
        GLVector3Uniform position;
        GLFloatUniform constant;
        GLFloatUniform linear;
        GLFloatUniform exponent;
        GLFloatUniform shadow_map_far_plane;
        GLTextureUniform shadow_map_texture;
        GLOmniDirectionalShadowMap shadow_map;
        char _base_light_namespace[64] = {""};

        explicit GLPointLight(const char *variable_name = "", const char* base_light_namespace = nullptr, int index = -1, int shadow_map_index_offset = 0) :
            GLBaseLight{variable_name, index},
            position{variable_name, "position", nullptr, index},
            constant{variable_name, "constant", nullptr, index},
            linear{variable_name, "linear", nullptr, index},
            exponent{variable_name, "exponent", nullptr, index},
            shadow_map_far_plane{"omniShadowMaps", "farPlane", nullptr, index + shadow_map_index_offset},
            shadow_map_texture{"omniShadowMaps", "shadowMap", nullptr, index + shadow_map_index_offset}
        {
            if (base_light_namespace)
                setSubBases(base_light_namespace);
        }

        void setSubBases(const char* base_light_namespace) {
            char current_bases[64];
            for (u8 i = 0; i < 64; i++) current_bases[i] = _base_light_namespace[i];

            snprintf(_base_light_namespace, sizeof(_base_light_namespace), "%s.%s", current_bases, base_light_namespace);

            GLBaseLight::setBases(_base_light_namespace);
        }

        void setBases(const char *base) {
            position.bases = constant.bases = linear.bases = exponent.bases = base;
        }

        void init(const GLuint shader_program_id) {
            GLBaseLight::init(shader_program_id);
            position.setLocation(shader_program_id);
            constant.setLocation(shader_program_id);
            linear.setLocation(shader_program_id);
            exponent.setLocation(shader_program_id);
            shadow_map_texture.setLocation(shader_program_id);
            shadow_map_far_plane.setLocation(shader_program_id);
            shadow_map.init();
        }

        void update(const PointLight &light, GLint texture_unit) {
            GLBaseLight::update(light);
            position.update(light.position);
            constant.update(light.attenuation.constant);
            linear.update(light.attenuation.linear);
            exponent.update(light.attenuation.exponent);
            shadow_map.read(GL_TEXTURE0 + texture_unit);
            shadow_map_texture.update(texture_unit);
            shadow_map_far_plane.update(light.shadow_bounds.far_distance);
        }
    };

    struct GLSpotLight : GLPointLight {
        GLVector3Uniform direction;
        GLFloatUniform edge;
        char _point_light_namespace[64] = {""};

        explicit GLSpotLight(const char *variable_name = "", const char* point_light_namespace = nullptr, const char* base_light_namespace = nullptr, int index = -1, int shadow_map_index_offset = 0) :
            GLPointLight{variable_name, base_light_namespace, index, shadow_map_index_offset},
            direction{variable_name, "direction", nullptr, index},
            edge{variable_name, "edge", nullptr, index}
        {
            if (point_light_namespace)
                setSubBases(point_light_namespace);
        }

        void setSubBases(const char* point_light_namespace) {
            char current_bases[64];
            for (u8 i = 0; i < 64; i++) current_bases[i] = _point_light_namespace[i];

            snprintf(_point_light_namespace, sizeof(_point_light_namespace), "%s.%s", current_bases, point_light_namespace);

            GLPointLight::setSubBases(point_light_namespace);
            GLPointLight::setBases(_point_light_namespace);
        }

        void setBases(const char *base) {
            position.bases = constant.bases = linear.bases = exponent.bases = base;
        }

        void init(const GLuint shader_program_id) {
            GLPointLight::init(shader_program_id);
            direction.setLocation(shader_program_id);
            edge.setLocation(shader_program_id);
        }

        void update(const SpotLight &light, GLint texture_unit) {
            GLPointLight::update(light, texture_unit);
            direction.update(Mat3(light.orientation).forward);
            edge.update(cosf(DEG_TO_RAD * light.edge));
        }
    };

}