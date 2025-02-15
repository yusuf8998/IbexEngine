#pragma once

#ifndef TEXTURE_H
#define TEXTURE_H

#include <GLAD/glad.h>
#include <string>
#include <map>

class TextureData;

class TextureObject
{
public:
    // Constructor that takes a file path
    TextureObject(const std::string &filePath);
    TextureObject(const TextureData &data);

    // Destructor to clean up texture data
    ~TextureObject();

    // Method to bind the texture
    void bind(GLuint unit = 0) const;

    // Getter for the texture ID
    GLuint getID() const { return textureID; }

    // Getter for a specific texture by name
    static TextureObject *getTextureByName(const std::string &name);

private:
    GLuint textureID;     // OpenGL texture ID
    std::string filePath; // Texture file path

    static std::map<std::string, TextureObject *> Textures;

    // Loads the texture from file and sets OpenGL parameters
    void loadTexture();

    void generateTexture(unsigned char *data, int width, int height, int channels);
};

#endif
