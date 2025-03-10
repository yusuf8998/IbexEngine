#include "FramebufferObject.h"
#include <iostream>
#include "Renderer.h"

FramebufferObject::FramebufferObject(int width, int height, bool overrideTexture, GLuint format, GLuint attachment)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (!overrideTexture)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
    }
    else
        texture = 0;

    // glGenRenderbuffers(1, &rbo);
    // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (!overrideTexture && glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FramebufferObject::FramebufferObject(int width, int height, GLuint texture, GLuint textureType, GLuint format, GLuint attachment)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureType, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FramebufferObject::~FramebufferObject()
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
    // glDeleteRenderbuffers(1, &rbo);
}

void FramebufferObject::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void FramebufferObject::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FramebufferObject::bindTexture() const
{
    Renderer::instance().slotTexture(GL_TEXTURE_2D, texture);
}

void FramebufferObject::setTexture(GLuint id, GLuint type, GLuint attachement)
{
    texture = id;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachement, type, texture, 0);
}