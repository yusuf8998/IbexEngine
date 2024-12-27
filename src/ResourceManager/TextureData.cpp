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
        stbi_image_free(data);
    }
}