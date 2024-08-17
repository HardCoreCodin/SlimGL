#pragma once

#include "./gl_uniforms.h"


struct BaseLight {
    vec3 color = {1.0f};
    vec3 position = {0.0f};
    float ambient_intensity = 1.0f;
    float diffuse_intensity = 1.0f;
    const Projection projection;

    BaseLight(const vec3 &color, const vec3 &position, float ambient_intensity, float diffuse_intensity, const ProjectionParams &projection_params) :
        color{color},
        position{position},
        ambient_intensity{ambient_intensity},
        diffuse_intensity{diffuse_intensity},
        projection{projection_params}
    {}
};

struct DirectionalLight : BaseLight {
    mat3 orientation{};

    DirectionalLight(const vec3 &color, float ambient_intensity, float diffuse_intensity, const vec3 &direction, const vec3 &position = {0}, 
                     float shadow_near = 0.1f, float shadow_far = 100.0f, float shadow_width = 40.0f, float shadow_height = 40.0f) :
        BaseLight{color, position, ambient_intensity, diffuse_intensity, ProjectionParams::makeOrthographic(shadow_width, shadow_height, shadow_near, shadow_far)} {}

    mat4 transformationMatrix() const
    {
        /*vec3 Z{direction.normalized()};
        vec3 X{vec3::Y.cross(Z).normalized()};
        vec3 Y{Z.cross(X).normalized()};
        return Mat4(mat3(X, Y, Z), position) * Mat4(projection);
        */
        return Mat4(orientation, position).inverted() * Mat4(projection);
    }
};

struct PointLight : BaseLight {
    float constant = 1.0;
    float linear = 0.0f;
    float exponent = 0.0f;
    float far_plane = 100.0f;

    PointLight(const vec3 &color, float ambient_intensity, float diffuse_intensity,
               const vec3 &position, float constant, float linear, float exponent, 
               float shadow_near = 0.1f, float shadow_far = 100.0f) :
        BaseLight{color, position, ambient_intensity, diffuse_intensity, 
              ProjectionParams::makePerspective(1, 1, shadow_near, shadow_far)},
        constant{constant}, linear{linear}, exponent{exponent} {}

    void setTransforms(mat4 *trasnforms)
    {
        static mat3 R{-vec3::Z, -vec3::Y,  vec3::X};
        static mat3 L{ vec3::Z, -vec3::Y, -vec3::X};
        static mat3 T{ vec3::X,  vec3::Z,  vec3::Y};
        static mat3 B{ vec3::X, -vec3::Z, -vec3::Y};
        static mat3 K{-vec3::X, -vec3::Y, -vec3::Z};
        static mat3 F{ vec3::X, -vec3::Y,  vec3::Z};

        mat4 projection_matrix = Mat4(projection);
        trasnforms[0] = Mat4(R, position).inverted() * projection_matrix;
        trasnforms[1] = Mat4(L, position).inverted() * projection_matrix;
        trasnforms[2] = Mat4(T, position).inverted() * projection_matrix;
        trasnforms[3] = Mat4(B, position).inverted() * projection_matrix;
        trasnforms[4] = Mat4(F, position).inverted() * projection_matrix;
        trasnforms[5] = Mat4(K, position).inverted() * projection_matrix;
    }
};

struct SpotLight : PointLight {
    vec3 direction = {0.0f, -1.0f, 0.0f};
    float edge = 0.0f;

    SpotLight(const vec3 &color, float ambient_intensity, float diffuse_intensity,
              const vec3 &position, float constant, float linear, float exponent,
              const vec3 &direction, float edge, float shadow_near = 0.1f, float shadow_far = 100.0f) :
        PointLight{color, ambient_intensity, diffuse_intensity, position, constant, linear, exponent, shadow_near, shadow_far},
        direction{direction.normalized()}, edge{edge} {}
};


struct GLShadowMap {
    const bool is_omni = false;
    GLuint id = 0;
    GLuint FBO = 0;
    GLsizei width = 2048;
    GLsizei height = 2048;

    ~GLShadowMap()
    {
        if (FBO)
        {
            glDeleteFramebuffers(1, &FBO);
        }

        if (id)
        {
            glDeleteTextures(1, &id);
        }
    }

    bool init(u32 shadow_map_width = 2048, u32 shadow_map_height = 2048) {
        width = shadow_map_width;
        height = shadow_map_height;

        glGenFramebuffers(1, &FBO);
        glGenTextures(1, &id);

        if (is_omni)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, id);

            for (size_t i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, id, 0);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id, 0);
        }
        
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (Status != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Framebuffer error: %s\n", Status);
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    void write() const {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    }

    void read(GLenum texture_unit) const {
        glActiveTexture(texture_unit);
        glBindTexture(is_omni ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, id);
    }
};


struct GLBaseLight {
    GLVector3Uniform color;
    GLFloatUniform ambient_intensity;
    GLFloatUniform diffuse_intensity;

    explicit GLBaseLight(const char *variable_name, int index = -1) :
        color{variable_name, "colour", nullptr, index},
        ambient_intensity{variable_name, "ambientIntensity", nullptr, index},
        diffuse_intensity{variable_name, "diffuseIntensity", nullptr, index}
    {}

    void init(const GLuint shader_program_id, u32 shadow_map_width = 2048, u32 shadow_map_height = 2048) {
        color.setLocation(shader_program_id);
        ambient_intensity.setLocation(shader_program_id);
        diffuse_intensity.setLocation(shader_program_id);
    }

    void update(const BaseLight &light) {
        color.update(light.color);
        ambient_intensity.update(light.ambient_intensity);
        diffuse_intensity.update(light.diffuse_intensity);
    }

    void setBases(const char* bases)
    {
        color.bases = ambient_intensity.bases = diffuse_intensity.bases = bases;
    }
};


struct GLDirectionalLight : GLBaseLight {
    GLVector3Uniform direction;
    GLTextureUniform shadow_map_texture{"directionalShadowMap"};
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
        direction.update(-light.orientation.forward);
        transform.update(light.transformationMatrix());
    }
};

struct GLPointLight : GLBaseLight {
    GLVector3Uniform position;
    GLFloatUniform constant;
    GLFloatUniform linear;
    GLFloatUniform exponent;
    GLFloatUniform shadow_map_far_plane;
    GLTextureUniform shadow_map_texture;
    GLShadowMap shadow_map{true};
    char _base_light_namespace[64] = {""};

    explicit GLPointLight(const char *variable_name, const char* base_light_namespace = nullptr, int index = -1, int shadow_map_index_offset = 0) :
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
        shadow_map.init();
        position.setLocation(shader_program_id);
        constant.setLocation(shader_program_id);
        linear.setLocation(shader_program_id);
        exponent.setLocation(shader_program_id);
        shadow_map_texture.setLocation(shader_program_id);
        shadow_map_far_plane.setLocation(shader_program_id);
    }

    void update(const PointLight &light, GLint texture_unit) {
        GLBaseLight::update(light);
        position.update(light.position);
        constant.update(light.constant);
        linear.update(light.linear);
        exponent.update(light.exponent);
        shadow_map.read(GL_TEXTURE0 + texture_unit);
        shadow_map_texture.update(texture_unit);
        shadow_map_far_plane.update(light.projection.params.far_distance);
    }
};

struct GLSpotLight : GLPointLight {
    GLVector3Uniform direction;
    GLFloatUniform edge;
    char _point_light_namespace[64] = {""};

    explicit GLSpotLight(const char *variable_name, const char* point_light_namespace = nullptr, const char* base_light_namespace = nullptr, int index = -1, int shadow_map_index_offset = 0) :
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
        direction.update(light.direction);
        edge.update(cosf(DEG_TO_RAD * light.edge));
    }
};