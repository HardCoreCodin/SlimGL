#pragma once

#include "../math/mat4_constructurs.h"
#include "../scene/selection.h"
#include "../gl/gl_edges.h"


void drawSelection(Selection &selection, const mat4 &view_projection_matrix, const Mesh *meshes) {
    if (
        !(
            controls::is_pressed::alt && 
            !mouse::is_captured && (
                selection.geometry || 
                selection.light
            )
          )
        )
        return;

    if (selection.geometry) {
        selection.xform = selection.geometry->transform;
        if (selection.geometry->type == GeometryType_Mesh)
            selection.xform.scale *= meshes[selection.geometry->id].aabb.max;
    } else {
        selection.xform.position = selection.light->position;
        selection.xform.scale = 1.0f; //selection.light->scale();
        switch (selection.light->type)
        {
        case LightType::Point      : selection.xform.orientation.reset();
        case LightType::Directional: selection.xform.orientation = ((DirectionalLight*)selection.light)->orientation;
        case LightType::Spot       : selection.xform.orientation = ((SpotLight*)selection.light)->orientation;
        }
    }

    static mat3 L{ vec3::Z,  vec3::Y, -vec3::X};
    static mat3 R{-vec3::Z,  vec3::Y,  vec3::X};
    static mat3 T{ vec3::X, -vec3::Z,  vec3::Y};
    static mat3 B{ vec3::X,  vec3::Z, -vec3::Y};
    static mat3 F{-vec3::X,  vec3::Y, -vec3::Z};
    static mat3 K{ vec3::X,  vec3::Y,  vec3::Z};

    mat4 mvp_matrix = Mat4(selection.xform) * view_projection_matrix;
    gl::cube::draw(mvp_matrix, BrightYellow);
    if (selection.box_side) {
        ColorID color = White;
        mat3* rot;
        vec3 pos;
        switch (selection.box_side) {
        case BoxSide_Left  : rot = &L; pos.x = -1; color = Red;   break;
        case BoxSide_Right : rot = &R; pos.x = +1; color = Red;   break;
        case BoxSide_Bottom: rot = &B; pos.y = -1; color = Green; break;
        case BoxSide_Top   : rot = &T; pos.y = +1; color = Green; break;
        case BoxSide_Back  : rot = &K; pos.z = -1; color = Blue;  break;
        case BoxSide_Front : rot = &F; pos.z = +1; color = Blue;  break; 
        case BoxSide_None  : break;
        }
        gl::quad::draw(Mat4(*rot, pos * 1.01f) * mvp_matrix, color);
    }
}
