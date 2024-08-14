#pragma once

#include "./gl_core.h"


struct GLMaterial {
    GLfloat specular_intensity;
    GLfloat shininess;

    GLMaterial(GLfloat specular_intensity, GLfloat shininess) :
            specular_intensity{specular_intensity}, shininess{shininess} {};

    void bind(GLuint specular_intensity_location, GLuint shininess_location) {
        glUniform1f(specular_intensity_location, specular_intensity);
        glUniform1f(shininess_location, shininess);
    }
};