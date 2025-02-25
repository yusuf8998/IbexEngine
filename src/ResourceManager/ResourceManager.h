#pragma once

#include <string>
#include <map>
#include <memory>
#include <stdexcept>
#include <iostream>

// Forward declaration for resource types
class TextureData;
class ShaderData;
class MeshData;
class MaterialLibrary;

class ResourceManager
{
public:
    // Singleton pattern to ensure one resource manager instance
    static ResourceManager &instance()
    {
        static ResourceManager instance_ptr;
        return instance_ptr;
    }

    // Template function for loading resources
    template <typename ResourceType>
    std::shared_ptr<ResourceType> loadResource(const std::string &filename);

    // Function to get a resource from cache
    template <typename ResourceType>
    std::shared_ptr<ResourceType> getResource(const std::string &filename);

    template <typename ResourceType>
    void purge();

    void purgeAll();

    // Cleanup
    void clear();

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    // Map for storing resources by filename
    std::map<std::string, std::shared_ptr<TextureData>> textureCache;
    std::map<std::string, std::shared_ptr<ShaderData>> shaderCache;
    std::map<std::string, std::shared_ptr<MeshData>> meshCache;
    std::map<std::string, std::shared_ptr<MaterialLibrary>> mtlCache;

    template <typename ResourceType>
    std::map<std::string, std::shared_ptr<ResourceType>> &getCache();

    // Disable copy/move operations for the singleton
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;
};