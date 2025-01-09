#include <Graphics/CubemapObject.h>
#include <iostream>
#include <stb/stb_image.h>
#include "CubemapObject.h"

void CubemapObject::loadCubemap()
{
    unsigned char *data[6];
    int width = 0, height = 0, channels = 0;
    for (int i = 0; i < 6; i++)
    {
        data[i] = stbi_load(filePaths[i].c_str(), &width, &height, &channels, 0);
        if (!data[i])
            throw;
    }
    generateCubemap(data, width, height, channels);
    for (int i = 0; i < 6; i++)
        stbi_image_free(data[i]);
}

void CubemapObject::generateCubemap(unsigned char *data[6], int width, int height, int channels)
{
    // Generate a new cube map
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Set the texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Detect the format based on channels
    GLenum format = (channels == 3) ? GL_RGB : (channels == 4 ? GL_RGBA : GL_RED);

    // Populate the texture array with the image data
    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data[i]);
    }

    // Unbind the cube map
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubemapObject::CubemapObject(const std::array<std::string, 6> &filePaths)
{
    for (int i = 0; i < 6; i++)
        this->filePaths[i] = filePaths[i];
    loadCubemap();
}
CubemapObject::CubemapObject(const std::string &cubemapDir, const std::string &extension)
    : CubemapObject(std::array<std::string, 6>{
        cubemapDir + "/right." + extension,
        cubemapDir + "/left." + extension,
        cubemapDir + "/top." + extension,
        cubemapDir + "/bottom." + extension,
        cubemapDir + "/back." + extension,
        cubemapDir + "/front." + extension
    })
{
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
