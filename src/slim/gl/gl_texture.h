#pragma once

#include "./gl_base.h"


union CubeMapImages {
	struct {
		RawImage pox_x;
		RawImage neg_x;
		RawImage pos_y;
		RawImage neg_y;
		RawImage pos_z;
		RawImage neg_z;
	};
	RawImage array[6];

    CubeMapImages() {}
};


struct GLTexture {
    GLuint id = 0;
    
    GLTexture() = default;

    GLTexture(const RawImage &image) {
        load(image);
    }

    bool load(const RawImage &image) {
        if (id) destroy();

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0,                 
            image.flags.alpha ? GL_RGBA : GL_RGB, 
            image.width, 
            image.height, 0, 
            image.flags.alpha ? GL_RGBA : GL_RGB, 
            GL_UNSIGNED_BYTE, 
            image.content);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }

    void bind(GLenum slot = GL_TEXTURE1) const {
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void destroy() {
        glDeleteTextures(1, &id);
        id = 0;
    }
};


struct GLCubeMapTexture {
    GLuint id = 0;

    GLCubeMapTexture() = default;

    GLCubeMapTexture(const CubeMapImages &images) {
        load(images);
    }

    bool load(const CubeMapImages &images) {
        if (id) destroy();

	    glGenTextures(1, &id);
	    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        const RawImage *image = images.array;
	    for (size_t i = 0; i < 6; i++, image++)
	    {
		    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
                image->flags.alpha ? GL_RGBA : GL_RGB, 
                image->width, 
                image->height, 0, 
                image->flags.alpha ? GL_RGBA : GL_RGB, 
                GL_UNSIGNED_BYTE, 
                image->content);
	    }

	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return true;
    }
    
    void bind(GLenum slot = GL_TEXTURE0) const {
        glActiveTexture(slot);
	    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    }

    void destroy() {
        glDeleteTextures(1, &id);
        id = 0;
    }
};