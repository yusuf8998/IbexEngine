#pragma once

#include <GLAD/glad.h>
#include <stddef.h>
#include <string>
#include <cstring>

class UniformBufferObject
{
protected:
    GLuint ubo;
    GLuint index;
    GLuint bindingPoint;
    size_t size;
    std::string name;

public:
    UniformBufferObject(const std::string &name, size_t size, GLuint bindingPoint);
    ~UniformBufferObject();

    void bind();
    void unbind() const;
    void use(GLuint program);
    void setData(const void *data);
    void setSubData(const void *data, size_t offset, size_t length);
    void pushData(const void *data, size_t size, size_t &offset);
    GLuint getBindingPoint() const;
    size_t getSize() const;
    const std::string &getName() const;
};