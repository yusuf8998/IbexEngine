#include "TextureObject.h"
#include <iostream>
#include "ResourceManager/TextureData.h"
#include "ResourceManager/ResourceManager.h"
#include <Graphics/GL.h>

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
    : textureID(0)
{
    data = ResourceManager::instance().getResource<TextureData>(filePath);
    generateTexture(data->getData(), data->getWidth(), data->getHeight(), data->getChannels());
    Textures[data->getName()] = this;
}
TextureObject::TextureObject(const std::shared_ptr<TextureData> &_data)
    : data(_data), textureID(0)
{
    generateTexture(data->getData(), data->getWidth(), data->getHeight(), data->getChannels());
    Textures[data->getName()] = this;
}

TextureObject::~TextureObject()
{
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
    Textures.erase(data->getName());
}

void TextureObject::bind(GLuint unit) const
{
    if (unit >= 0 && unit <= 31)
    {
        GLCall(glActiveTexture(GL_TEXTURE0 + unit));
        GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
    }
    else
    {
        std::cerr << "Invalid texture unit: " << unit << std::endl;
    }
}

void TextureObject::loadTexture()
{
    generateTexture(data->getData(), data->getWidth(), data->getHeight(), data->getChannels());
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
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
    GLCall(glGenerateMipmap(GL_TEXTURE_2D));
}