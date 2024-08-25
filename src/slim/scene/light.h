#pragma once

#define LIGHT_RADIUS_INTENSITY_FACTOR 32.0f


#include "../math/mat4_constructurs.h"


enum class LightType {
    Directional,
    Point,
    Spot
};


struct LightAttenuation {
    f32 constant = 0.0f;
    f32 linear = 0.0f;
    f32 exponent = 1.0f;
};


struct ShadowBounds {
    f32 near_distance = 0.1f;
    f32 far_distance = 100.0f;
    f32 width = 40.0f;
    f32 height = 40.0f;
    
    void setMatrices(const vec3 &position, mat4 *matrices) const {
        static mat3 R{-vec3::Z, -vec3::Y,  vec3::X};
        static mat3 L{ vec3::Z, -vec3::Y, -vec3::X};
        static mat3 T{ vec3::X,  vec3::Z,  vec3::Y};
        static mat3 B{ vec3::X, -vec3::Z, -vec3::Y};
        static mat3 K{-vec3::X, -vec3::Y, -vec3::Z};
        static mat3 F{ vec3::X, -vec3::Y,  vec3::Z};

        ProjectionParams projection_params = ProjectionParams::makePerspective(
            1, 
            1, 
            near_distance, 
            far_distance
        );
        mat4 projection_matrix = Mat4(projection_params);
        matrices[0] = Mat4(R, position).inverted() * projection_matrix;
        matrices[1] = Mat4(L, position).inverted() * projection_matrix;
        matrices[2] = Mat4(T, position).inverted() * projection_matrix;
        matrices[3] = Mat4(B, position).inverted() * projection_matrix;
        matrices[4] = Mat4(F, position).inverted() * projection_matrix;
        matrices[5] = Mat4(K, position).inverted() * projection_matrix;
    }
};


struct BaseLight {
    LightType type;
    LightAttenuation attenuation;
    ShadowBounds shadow_bounds;
    Color color;
    f32 intensity;
    vec3 position;

    /*
    f32 scale() const { return intensity.diffuse / LIGHT_RADIUS_INTENSITY_FACTOR; }
    virtual mat4 matrix() const { return Mat4({}, {scale()}, position); }*/
};


struct DirectionalLight : BaseLight {
    OrientationUsingQuaternion orientation;

    DirectionalLight(
        const OrientationUsingQuaternion &orientation = {}, 
        const Color &color = White,
        const float intensity = 1.0f,
        const vec3 &position = {0.0f},
        const LightAttenuation &attenuation = {},
        const ShadowBounds &shadow_bounds = {}
    ) : BaseLight{
            LightType::Directional,
            attenuation,
            shadow_bounds,
            color,
            intensity,
            position
        },
        orientation{orientation}
    {}

    mat4 shadowProjectionMatrix() const {
        return Mat4(ProjectionParams::makeOrthographic(
            shadow_bounds.width, 
            shadow_bounds.height, 
            shadow_bounds.near_distance, 
            shadow_bounds.far_distance
        ));
    }

    mat4 transformationMatrix() const { 
        return Mat4(Mat3(orientation), position); 
    }

    mat4 shadowMapMatrix() const {
        return transformationMatrix().inverted() * shadowProjectionMatrix();
    }

    mat4 shadowBoundsMatrix() const {
        return shadowProjectionMatrix().inverted() * transformationMatrix();
    }
};


struct PointLight : BaseLight {
    PointLight(
        const vec3 &position = {0.0f},
        const Color &color = White,
        const float intensity = 1.0f,
        const LightAttenuation &attenuation = {},
        const ShadowBounds &shadow_bounds = {}
    ) : BaseLight{
            LightType::Point,
            attenuation,
            shadow_bounds,
            color,
            intensity,
            position
        }
    {}

    mat4 transformationMatrix() const { 
        return Mat4({}, position); 
    }

    void setShadowMapMatrices(mat4 *matrices) const {
        return shadow_bounds.setMatrices(position, matrices);
    }
};


struct SpotLight : PointLight {
    OrientationUsingQuaternion orientation;
    f32 edge = 0.0f;

    SpotLight(
        const OrientationUsingQuaternion &orientation = {},
        const vec3 &position = {0.0f},
        const Color &color = White,
        const float intensity = 1.0f,
        const LightAttenuation &attenuation = {},
        const ShadowBounds &shadow_bounds = {},
        f32 edge = 20.0f
    ) : PointLight{
            position,
            color,
            intensity,
            attenuation,
            shadow_bounds
        },
        orientation{orientation},
        edge{edge}
    {
        type = LightType::Spot;
    }

    void setShadowMatrices(mat4 *matrices) {
        return shadow_bounds.setMatrices(position, matrices);
    }

    mat4 transformationMatrix() const { 
        return Mat4(orientation, {1.0f}, position); 
    }
};