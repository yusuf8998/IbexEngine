#include "TextureArrayObject.h"
#include <vendor/stb/stb_image.h>
#include <iostream>
#include "ResourceManager/TextureData.h"
#include "ResourceManager/ResourceManager.h"
#include "Renderer.h"

TextureArrayObject::TextureArrayObject(const std::vector<std::string> &filePaths)
{
    loadTextureArray(filePaths);
}

TextureArrayObject::~TextureArrayObject()
{
    glDeleteTextures(1, &textureArrayID);
}

const std::vector<std::shared_ptr<TextureData>> &TextureArrayObject::getDatas() const { return datas; }

void TextureArrayObject::bind() const
{
    Renderer::instance().slotTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
}

void TextureArrayObject::loadTextureArray(const std::vector<std::string> &filePaths)
{
    std::vector<unsigned char *> raw_datas = {};
    datas = std::vector<std::shared_ptr<TextureData>>(filePaths.size());
    int arr_width = 0, arr_height = 0, arr_channels = 0;
    for (size_t i = 0; i < filePaths.size(); i++)
    {
        const std::string filePath = filePaths[i];
        datas[i] = ResourceManager::instance().getResource<TextureData>(filePath);
        if (arr_width == 0)
        {
            arr_width = datas[i]->getWidth();
            arr_height = datas[i]->getHeight();
            arr_channels = datas[i]->getChannels();
        }
        else if (datas[i]->getWidth() != arr_width || datas[i]->getHeight() != arr_height || datas[i]->getChannels() != arr_channels)
        {
            std::cerr << "Texture dimensions do not match: " << filePath << std::endl;
            return;
        }
        raw_datas.push_back(datas[i]->getData());
    }
    generateTextureArray(raw_datas, arr_width, arr_height, raw_datas.size(), arr_channels);
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
