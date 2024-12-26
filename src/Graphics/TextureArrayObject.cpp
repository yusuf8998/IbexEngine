#include "TextureArrayObject.h"
#include <vendor/stb/stb_image.h>
#include <iostream>

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
    if (unit >= GL_TEXTURE0 && unit <= GL_TEXTURE31)
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
        // Load the image using stb_image
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true); // Optional: flip the image vertically on load
        unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
        if (arr_width == 0)
        {
            arr_width = width;
            arr_height = height;
            arr_channels = channels;
        }
        else if (width != arr_width || height != arr_height || channels != arr_channels)
        {
            std::cerr << "Texture dimensions do not match: " << filePath << std::endl;
            return;
        }

        if (data)
        {
            datas.push_back(data);
            // stbi_image_free(data);

            std::cout << "Loaded texture: " << filePath << " (" << width << "x" << height << ")" << std::endl;
        }
        else
        {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
        }
    }
    generateTextureArray(datas, arr_width, arr_height, datas.size(), arr_channels);
    for (int i = 0; i < datas.size(); i++)
    {
        stbi_image_free(datas[i]);
    }
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

    // Generate the texture array
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, layers, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Populate the texture array with the image data
    for (int i = 0; i < layers; i++)
    {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
    }

    // Unbind the texture array
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArrayObject::flipVertically(unsigned char *data, int width, int height, int channels)
{
}
