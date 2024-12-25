#include "ShaderObject.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "ResourceManager/ShaderData.h"

ShaderObject::ShaderObject(const std::string &vertexPath, const std::string &fragmentPath)
{
    // Step 1: Read the shader files
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    // Step 2: Compile the shaders
    GLuint vertexShader = compileShader(vertexCode, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentCode, GL_FRAGMENT_SHADER);

    // Step 3: Link the shaders into a program
    linkProgram(vertexShader, fragmentShader);

    // Step 4: Delete shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

ShaderObject::ShaderObject(const std::shared_ptr<ShaderData> &vertex, const std::shared_ptr<ShaderData> &fragment)
    : vertex(vertex), fragment(fragment)
{
    // Step 2: Compile the shaders
    GLuint vertexShader = compileShader(vertex->getSource(), GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragment->getSource(), GL_FRAGMENT_SHADER);

    // Step 3: Link the shaders into a program
    linkProgram(vertexShader, fragmentShader);

    // Step 4: Delete shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// ShaderObject::ShaderObject(const ShaderData &vertexData, const ShaderData &fragData)
// {
//     // Step 2: Compile the shaders
//     GLuint vertexShader = compileShader(vertexData.getSource(), GL_VERTEX_SHADER);
//     GLuint fragmentShader = compileShader(fragData.getSource(), GL_FRAGMENT_SHADER);

//     // Step 3: Link the shaders into a program
//     linkProgram(vertexShader, fragmentShader);

//     // Step 4: Delete shaders after linking
//     glDeleteShader(vertexShader);
//     glDeleteShader(fragmentShader);
// }

void ShaderObject::use() const
{
    glUseProgram(programID);
}

void ShaderObject::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
}

void ShaderObject::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void ShaderObject::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void ShaderObject::setVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}

void ShaderObject::setMat4(const std::string &name, const glm::mat4 &value) const
{
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

std::string ShaderObject::readFile(const std::string &filePath) const
{
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint ShaderObject::compileShader(const std::string &source, GLenum shaderType) const
{
    GLuint shader = glCreateShader(shaderType);
    const char *shaderCode = source.c_str();
    glShaderSource(shader, 1, &shaderCode, nullptr);
    glCompileShader(shader);
    checkCompileErrors(shader, shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    return shader;
}

void ShaderObject::linkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);
    checkLinkErrors(programID);
}

void ShaderObject::checkCompileErrors(GLuint shader, const std::string &type) const
{
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << type << " SHADER COMPILATION ERROR: " << infoLog << std::endl;
    }
}

void ShaderObject::checkLinkErrors(GLuint program) const
{
    GLint success;
    GLchar infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "PROGRAM LINKING ERROR: " << infoLog << std::endl;
    }
}
