#include "TextureData.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <iostream>

TextureData::TextureData(const std::string& filename)
    : filename(filename) {
    // Load the image using stb_image
    data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        throw std::runtime_error("Texture loading failed.");
    }
    std::cout << "Loaded texture: " << filename << " (" << width << "x" << height << ")" << std::endl;
}

TextureData::~TextureData() {
    if (data) {
        free(data);
    }
}

void TextureData::createData(int width, int height, int channels)
{
    this->width = width;
    this->height = height;
    this->channels = channels;
    this->data = (unsigned char *) malloc(width * height * channels);
}

void TextureData::uploadData(unsigned char *data, int size, int offset)
{
    assert(this->data);
    assert(size + offset <= width * height * channels);
    memcpy(this->data + offset, data, size);
}