#pragma once

#include <GLAD/glad.h>

void GLClearError();
bool GLLogCall(const char *function, const char *file, int line);

#define ASSERT(x) \
    if (!(x))     \
        throw;
#if DEBUG
#define GLCall(x)   \
    GLClearError(); \
    x;              \
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
#define GLCall(x) x;
#endif

inline void GLClearError()
{
    while (glGetError() != GL_NO_ERROR)
        ;
};
inline bool GLLogCall(const char *function, const char *file, int line)
{
    while (GLenum error = glGetError())
    {
        std::string error_name = "";
        switch (error)
        {
        case GL_INVALID_ENUM:
            error_name = "Invalid Enum";
            break;
        case GL_INVALID_VALUE:
            error_name = "Invalid Value";
            break;
        case GL_INVALID_OPERATION:
            error_name = "Invalid Operation";
            break;
        case GL_STACK_OVERFLOW:
            error_name = "Stack Overflow";
            break;
        case GL_STACK_UNDERFLOW:
            error_name = "Stack Underflow";
            break;
        case GL_OUT_OF_MEMORY:
            error_name = "Out Of Memory";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error_name = "Invalid Framebuffer Operation";
            break;
        default:
            error_name = std::to_string(error);
            break;
        }
        printf("[OpenGL Error]: (%s) %s %s: %i\n", error_name.c_str(), function, file, line);
        return false;
    }
    return true;
}