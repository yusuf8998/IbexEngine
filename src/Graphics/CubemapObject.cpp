#include "CubemapObject.h"
#include <iostream>

void CubemapObject::loadCubemap()
{
    unsigned char *data[6];
    int width = 0, height = 0, channels = 0;
    for (int i = 0; i < 6; i++)
    {
    }
    generateCubemap(data, width, height, channels);
}

void CubemapObject::generateCubemap(unsigned char *data[6], int width, int height, int channels)
{
    // Generate a new cube map
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Set the texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Detect the format based on channels
    GLenum format = (channels == 3) ? GL_RGB : (channels == 4 ? GL_RGBA : GL_RED);

    // Generate the cube map
    glTexImage3D(GL_TEXTURE_CUBE_MAP, 0, format, width, height, 6, 0, format, GL_UNSIGNED_BYTE, 0);

    // Populate the texture array with the image data
    for (int i = 0; i < 6; i++)
    {
        glTexSubImage3D(GL_TEXTURE_CUBE_MAP, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, data[i]);
    }

    // Unbind the cube map
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubemapObject::CubemapObject(const std::string filePaths[6])
{
    for (int i = 0; i < 6; i++)
        this->filePaths[i] = filePaths[i];
    loadCubemap();
}

CubemapObject::~CubemapObject()
{
    glDeleteTextures(1, &textureID);
}

void CubemapObject::bind(GLuint unit) const
{
    if (unit >= GL_TEXTURE0 && unit <= GL_TEXTURE31)
    {
        glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    }
    else
    {
        std::cerr << "Invalid texture unit: " << unit << std::endl;
    }
}
