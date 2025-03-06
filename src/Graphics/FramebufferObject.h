#pragma once

#include <GLAD/glad.h>

class FramebufferObject
{
public:
    FramebufferObject(int width, int height, bool overrideTexture = false, GLuint format = GL_RGB, GLuint attachment = GL_COLOR_ATTACHMENT0);
    FramebufferObject(int width, int height, GLuint texture, GLuint textureType, GLuint format = GL_RGB, GLuint attachment = GL_COLOR_ATTACHMENT0);
    ~FramebufferObject();

    void bind() const;
    void unbind() const;

    void bindTexture() const;

    GLuint getID() const { return fbo; }
    GLuint getTextureID() const { return texture; }

    void setTexture(GLuint id, GLuint type = GL_TEXTURE_2D, GLuint attachement = GL_COLOR_ATTACHMENT0);

private:
    GLuint fbo;
    GLuint texture;
    GLuint rbo;

};