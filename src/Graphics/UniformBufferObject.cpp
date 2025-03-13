#include <Graphics/UniformBufferObject.h>
#include "UniformBufferObject.h"

UniformBufferObject::UniformBufferObject(const std::string &name, size_t size, GLuint bindingPoint)
    : name(name), size(size), bindingPoint(bindingPoint)
{
    glGenBuffers(1, &ubo);
    bind();
    setData(nullptr);
    unbind();
}

UniformBufferObject::~UniformBufferObject()
{
    glDeleteBuffers(1, &ubo);
}

void UniformBufferObject::bind()
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
}

void UniformBufferObject::unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferObject::use(GLuint program)
{
    bind();
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
    index = glGetUniformBlockIndex(program, name.c_str());
    glUniformBlockBinding(program, index, bindingPoint);
}

void UniformBufferObject::setData(const void *data)
{
    bind();
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void UniformBufferObject::setSubData(const void *data, size_t offset, size_t length)
{
    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, offset, length, data);
}

void UniformBufferObject::pushData(const void *data, size_t size, size_t &offset)
{
    setSubData(data, offset, size);
    size_t padding = 0;
    if (size > 16)
        padding = size % 16;
    else
        padding = 16 - size;
    offset += size + padding;
}
GLuint UniformBufferObject::getBindingPoint() const
{
    return bindingPoint;
}

size_t UniformBufferObject::getSize() const
{
    return size;
}

const std::string &UniformBufferObject::getName() const
{
    return name;
}