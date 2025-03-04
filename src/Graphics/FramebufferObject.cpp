#include "FramebufferObject.h"
#include <iostream>

FramebufferObject::FramebufferObject(int width, int height, GLuint format, GLuint attachment)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);

    // glGenRenderbuffers(1, &rbo);
    // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

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

void FramebufferObject::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void FramebufferObject::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FramebufferObject::bindTexture(GLuint unit) const
{
    if (GL_TEXTURE0 + unit > GL_TEXTURE31)
    {
        std::cerr << "Invalid texture unit: " << unit << std::endl;
        return;
    }
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture);
}
