#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GLAD/glad.h>
#include <glm/glm.hpp>
#include <memory>

class ShaderProgram;

class ShaderObject
{
public:
    // Constructor
    ShaderObject(const std::shared_ptr<ShaderProgram> &program);

    // Use the shader program
    void use() const;

    GLuint getID() const { return programID; }

    // Set uniform variables
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &value) const;

private:
    // Shader program ID
    GLuint programID;

    std::shared_ptr<ShaderProgram> program;

    // Helper functions for shader compilation and program linking
    std::string readFile(const std::string &filePath) const;
    GLuint compileShader(const std::string &source, GLenum shaderType) const;
    void linkProgram(GLuint vertexShader, GLuint fragmentShader);
    void linkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint geometryShader);
    void checkCompileErrors(GLuint shader, const std::string &type) const;
    void checkLinkErrors(GLuint program) const;
};

#endif // SHADER_H