#include "ResourceManager.h"
#include "TextureData.h"
#include "ShaderProgram.h"
#include "MeshData.h"
#include "MaterialLibrary.h"
#include <stb/stb_image.h>

template <>
std::shared_ptr<TextureData> ResourceManager::loadResource<TextureData>(const std::string &filename)
{
    // Check if resource is already loaded
    if (textureCache.find(filename) != textureCache.end())
    {
        return textureCache[filename];
    }

    // Load new texture
    auto texture = std::make_shared<TextureData>(filename);
    textureCache[filename] = texture;
    return texture;
}
template <>
std::shared_ptr<TextureData> ResourceManager::getResource<TextureData>(const std::string &filename)
{
    // Check if the resource is in cache
    auto it = textureCache.find(filename);
    if (it != textureCache.end())
    {
        return it->second;
    }

    // If not in cache, load it
    return loadResource<TextureData>(filename);
}

template <>
std::shared_ptr<ShaderProgram> ResourceManager::loadResource<ShaderProgram>(const std::string &filename)
{
    // Check if resource is already loaded
    if (shaderCache.find(filename) != shaderCache.end())
    {
        return shaderCache[filename];
    }

    // Load new shader program
    auto shader = std::make_shared<ShaderProgram>(filename);
    shaderCache[filename] = shader;
    return shader;
}
template <>
std::shared_ptr<ShaderProgram> ResourceManager::getResource<ShaderProgram>(const std::string &filename)
{
    // Check if the resource is in cache
    auto it = shaderCache.find(filename);
    if (it != shaderCache.end())
    {
        return it->second;
    }

    // If not in cache, load it
    return loadResource<ShaderProgram>(filename);
}

template <>
std::shared_ptr<MeshData> ResourceManager::loadResource<MeshData>(const std::string &filename)
{
    // Check if resource is already loaded
    auto it = meshCache.find(filename);
    if (it != meshCache.end())
    {
        return it->second;
    }

    // Load new mesh
    auto mesh = std::make_shared<MeshData>();
    try
    {
        if (!mesh->loadFromOBJ(filename))
        {
            throw std::runtime_error("Failed to load mesh from " + filename);
        }

        // Cache the loaded mesh
        meshCache[filename] = mesh;
        return mesh;
    }
    catch (const std::exception &e)
    {
        // Handle loading failure, and potentially log or clean up
        throw std::runtime_error("Exception while loading mesh from " + filename + ": " + e.what());
    }
}
template <>
std::shared_ptr<MeshData> ResourceManager::getResource<MeshData>(const std::string &filename)
{
    // Check if the resource is in cache
    auto it = meshCache.find(filename);
    if (it != meshCache.end())
    {
        return it->second;
    }

    // If not in cache, load it
    return loadResource<MeshData>(filename);
}

template <>
std::shared_ptr<MaterialLibrary> ResourceManager::loadResource<MaterialLibrary>(const std::string &filename)
{
    // Check if resource is already loaded
    if (mtlCache.find(filename) != mtlCache.end())
    {
        return mtlCache[filename];
    }

    // Load new material library
    auto mtl = std::make_shared<MaterialLibrary>();
    if (!mtl->loadMaterialsFromMTL(filename))
    {
        throw std::runtime_error("Failed to load material library from " + filename);
    }
    mtlCache[filename] = mtl;
    return mtl;
}
template <>
std::shared_ptr<MaterialLibrary> ResourceManager::getResource<MaterialLibrary>(const std::string &filename)
{
    // Check if the resource is in cache
    auto it = mtlCache.find(filename);
    if (it != mtlCache.end())
    {
        return it->second;
    }

    // If not in cache, load it
    return loadResource<MaterialLibrary>(filename);
}

void ResourceManager::purgeAll()
{
    purge<TextureData>();
    purge<ShaderProgram>();
    purge<MeshData>();
    purge<MaterialLibrary>();
}

void ResourceManager::debugUseCounts()
{
    for (auto &kvp : textureCache)
    {
        printf("Texture %s has %ld uses\n", kvp.first.c_str(), kvp.second.use_count());
    }
    for (auto &kvp : shaderCache)
    {
        printf("Shader %s has %ld uses\n", kvp.first.c_str(), kvp.second.use_count());
    }
    for (auto &kvp : meshCache)
    {
        printf("Mesh %s has %ld uses\n", kvp.first.c_str(), kvp.second.use_count());
    }
    for (auto &kvp : mtlCache)
    {
        printf("Material Library %s has %ld uses\n", kvp.first.c_str(), kvp.second.use_count());
    }
}

template <>
std::map<std::string, std::shared_ptr<TextureData>> &ResourceManager::getCache()
{
    return textureCache;
}
template <>
std::map<std::string, std::shared_ptr<ShaderProgram>> &ResourceManager::getCache()
{
    return shaderCache;
}
template <>
std::map<std::string, std::shared_ptr<MeshData>> &ResourceManager::getCache()
{
    return meshCache;
}
template <>
std::map<std::string, std::shared_ptr<MaterialLibrary>> &ResourceManager::getCache()
{
    return mtlCache;
}

void ResourceManager::clear()
{
    textureCache.clear();
    shaderCache.clear();
    meshCache.clear();
    mtlCache.clear();
}
