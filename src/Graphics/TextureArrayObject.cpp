#include "TextureArrayObject.h"
#include <vendor/stb/stb_image.h>
#include <iostream>
#include "ResourceManager/TextureData.h"
#include "ResourceManager/ResourceManager.h"

TextureArrayObject::TextureArrayObject(const std::vector<std::string> &filePaths)
    : filePaths(filePaths)
{
    loadTextureArray();
}

TextureArrayObject::~TextureArrayObject()
{
    glDeleteTextures(1, &textureArrayID);
}

void TextureArrayObject::bind(GLuint unit) const
{
    if (unit >= 0 && unit <= 31)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
    }
    else
    {
        std::cerr << "Invalid texture unit: " << unit << std::endl;
    }
}

void TextureArrayObject::loadTextureArray()
{
    std::vector<unsigned char *> datas = {};
    int arr_width = 0, arr_height = 0, arr_channels = 0;
    for (auto filePath : filePaths)
    {
        auto texture = ResourceManager::instance().getResource<TextureData>(filePath);
        if (arr_width == 0)
        {
            arr_width = texture->getWidth();
            arr_height = texture->getHeight();
            arr_channels = texture->getChannels();
        }
        else if (texture->getWidth() != arr_width || texture->getHeight() != arr_height || texture->getChannels() != arr_channels)
        {
            std::cerr << "Texture dimensions do not match: " << filePath << std::endl;
            return;
        }
        datas.push_back(texture->getData());
    }
    generateTextureArray(datas, arr_width, arr_height, datas.size(), arr_channels);
}

void TextureArrayObject::generateTextureArray(const std::vector<unsigned char *> &data, int width, int height, int layers, int channels)
{
    // Generate a new texture array
    glGenTextures(1, &textureArrayID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);

    // Set the texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Detect the format based on channels
    GLenum format = (channels == 3) ? GL_RGB : (channels == 4 ? GL_RGBA : GL_RED);

    // Generate the texture array
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, width, height, layers, 0, format, GL_UNSIGNED_BYTE, 0);

    // Populate the texture array with the image data
    for (int i = 0; i < layers; i++)
    {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, data[i]);
    }

    // Unbind the texture array
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArrayObject::flipVertically(unsigned char *data, int width, int height, int channels)
{
}
