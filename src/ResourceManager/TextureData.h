#pragma once
#include <string>

// Texture class to hold texture data
class TextureData
{
public:
    TextureData(const std::string &filename);
    ~TextureData();

    // Getters for texture properties
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    unsigned char *getData() const { return data; }
    int getChannels() const { return channels; }
    std::string getName() const { return filename; }

private:
    std::string filename;
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char *data = nullptr;
};