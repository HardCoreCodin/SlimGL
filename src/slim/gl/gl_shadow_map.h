#pragma once

#include "./gl_uniforms.h"


namespace gl {    
    struct GLDepthFrameBufferTexture {
        GLsizei width = 2048;
        GLsizei height = 2048;
        GLuint FBO = 0;
        GLuint id = 0;

        void write() const {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        }
    
        void destroy() {
            if (FBO) glDeleteFramebuffers(1, &FBO);
            if (id) glDeleteTextures(1, &id);
        }

    protected:
        
        void _init(const bool is_omni, GLsizei new_width, GLsizei new_height) {
            width = new_width;
            height = new_height; 
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

            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status == GL_FRAMEBUFFER_COMPLETE)
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            else
                printf("Framebuffer error: %s\n", status);
        }

        void _read(GLenum texture_unit, const bool is_omni) const {
            glActiveTexture(texture_unit);
            glBindTexture(is_omni ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, id);
        }
    };

    struct GLDirectionalShadowMap : GLDepthFrameBufferTexture {
        void init(GLsizei width = 2048, GLsizei height = 2048) { _init(false, width, height); }
        void read(GLenum texture_unit) const { _read(texture_unit, false); }    
    };

    struct GLOmniDirectionalShadowMap : GLDepthFrameBufferTexture {
        void init(GLsizei width = 2048, GLsizei height= 2048) { _init(true, width, height); }
        void read(GLenum texture_unit) const { _read(texture_unit, true); }    
    };
}