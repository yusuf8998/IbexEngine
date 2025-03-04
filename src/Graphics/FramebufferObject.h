#pragma once

#include <GLAD/glad.h>

class FramebufferObject
{
public:
    FramebufferObject(int width, int height, GLuint format = GL_RGB, GLuint attachment = GL_COLOR_ATTACHMENT0);
    ~FramebufferObject();

    void bind();
    void unbind();

    void bindTexture(GLuint unit = 0) const;

    GLuint getID() const { return fbo; }
    GLuint getTextureID() const { return texture; }

private:
    GLuint fbo;
    GLuint texture;
    GLuint rbo;

};