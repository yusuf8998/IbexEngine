#include "ResourceManager.h"
#include "TextureData.h"
#include "ShaderData.h"
#include <stb/stb_image.h>

template <>
std::shared_ptr<TextureData> ResourceManager::loadResource<TextureData>(const std::string& filename) {
    // Check if resource is already loaded
    if (textureCache.find(filename) != textureCache.end()) {
        return textureCache[filename];
    }

    // Load new texture
    auto texture = std::make_shared<TextureData>(filename);
    textureCache[filename] = texture;
    return texture;
}
template <>
std::shared_ptr<TextureData> ResourceManager::getResource<TextureData>(const std::string& filename) {
    // Check if the resource is in cache
    auto it = textureCache.find(filename);
    if (it != textureCache.end()) {
        return it->second;
    }

    // If not in cache, load it
    return loadResource<TextureData>(filename);
}

template <>
std::shared_ptr<ShaderData> ResourceManager::loadResource<ShaderData>(const std::string& filename) {
    // Check if resource is already loaded
    if (shaderCache.find(filename) != shaderCache.end()) {
        return shaderCache[filename];
    }

    // Load new texture
    auto shader = std::make_shared<ShaderData>(filename);
    shaderCache[filename] = shader;
    return shader;
}
template <>
std::shared_ptr<ShaderData> ResourceManager::getResource<ShaderData>(const std::string& filename) {
    // Check if the resource is in cache
    auto it = shaderCache.find(filename);
    if (it != shaderCache.end()) {
        return it->second;
    }

    // If not in cache, load it
    return loadResource<ShaderData>(filename);
}

void ResourceManager::clear() {
    textureCache.clear();
    shaderCache.clear();
}
