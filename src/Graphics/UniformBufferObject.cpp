#include "UniformBufferObject.h"
#include <string.h>

UniformBufferObject::UniformBufferObject(const std::string &name, size_t size)
    : name(name), size(size), bindingPoint(0)
{
    data = new char[size];
    glGenBuffers(1, &ubo);
    bind();
    setData(data);
    unbind();
}

UniformBufferObject::~UniformBufferObject()
{
    glDeleteBuffers(1, &ubo);
    delete[] data;
}

void UniformBufferObject::bind(GLuint program)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    bindingPoint = glGetUniformBlockIndex(program, name.c_str());
    // glUniformBlockBinding(program, )
}

void UniformBufferObject::unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferObject::setData(const void *data) const
{
    bind();
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
    memcpy(this->data, data, size);
}

void UniformBufferObject::setSubData(const void *data, size_t offset, size_t length) const
{
    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, offset, length, data);
    memcpy(this->data + offset, data, length);
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

char *UniformBufferObject::getData() const
{
    return data;
}