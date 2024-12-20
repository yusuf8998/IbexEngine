#include "TextureObject.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <iostream>
#include "ResourceManager/TextureData.h"

std::map<std::string, TextureObject *> TextureObject::Textures = {};
TextureObject *TextureObject::getTextureByName(const std::string &name)
{
    auto it = Textures.find(name);
    if (it != Textures.end())
    {
        return it->second;
    }
    return new TextureObject(name);
}

TextureObject::TextureObject(const std::string &filePath)
    : filePath(filePath), textureID(0)
{
    loadTexture();
    Textures[filePath] = this;
}
TextureObject::TextureObject(const TextureData &data)
    : filePath(data.getName()), textureID(0)
{
    generateTexture(data.getData(), data.getWidth(), data.getHeight(), data.getChannels());
    Textures[filePath] = this;
}

TextureObject::~TextureObject()
{
    // if (textureID != 0) {
    //     glDeleteTextures(1, &textureID);
    // }
    Textures.erase(filePath);
}

void TextureObject::bind(GLuint unit) const
{
    if (unit >= GL_TEXTURE0 && unit <= GL_TEXTURE31)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    else
    {
        std::cerr << "Invalid texture unit: " << unit << std::endl;
    }
}

void TextureObject::loadTexture()
{
    // Load the image using stb_image
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); // Optional: flip the image vertically on load
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

    if (data)
    {
        generateTexture(data, width, height, channels);
        // Free the image data after use
        stbi_image_free(data);

        std::cout << "Loaded texture: " << filePath << " (" << width << "x" << height << ")" << std::endl;
    }
    else
    {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
    }
}

void TextureObject::generateTexture(unsigned char *data, int width, int height, int channels)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Detect the format based on channels
    GLenum format = (channels == 3) ? GL_RGB : (channels == 4 ? GL_RGBA : GL_RED);

    // Generate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void TextureObject::flipVertically(unsigned char *data, int width, int height, int channels)
{
    // Flip the image vertically (if needed, this is done using stb_image).
    // This function is just for illustration and can be ignored if flipping is handled by stb_image.
}