#pragma once

#include <GLAD/glad.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

class TextureData;

class TextureArrayObject
{
public:
    // Constructor that takes a vector of file paths
    TextureArrayObject(const std::vector<std::string> &filePaths);

    // Destructor to clean up texture data
    ~TextureArrayObject();

    // Method to bind the texture array
    void bind(GLuint unit = 0) const;

    // Getter for the texture array ID
    GLuint getID() const { return textureArrayID; }

    const std::vector<std::shared_ptr<TextureData>> &getDatas() const;

private:
    GLuint textureArrayID; // OpenGL texture array ID
    std::vector<std::shared_ptr<TextureData>> datas; // Texture datas

    static std::map<std::string, TextureArrayObject *> TextureArrays; // Map of texture arrays

    // Loads the texture array from files and sets OpenGL parameters
    void loadTextureArray(const std::vector<std::string> &filePaths);

    void generateTextureArray(const std::vector<unsigned char *> &data, int width, int height, int layers, int channels);

    // Utility function for flipping the image vertically
    static void flipVertically(unsigned char *data, int width, int height, int channels);
};