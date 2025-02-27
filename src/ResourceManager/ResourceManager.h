#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>

// Forward declaration for resource types
class TextureData;
class ShaderProgram;
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
    void unloadResource(const std::string &filename);

    template <typename ResourceType>
    void purge();

    void purgeAll();

    void debugUseCounts();

    // Cleanup
    void clear();

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    // Map for storing resources by filename
    std::map<std::string, std::shared_ptr<TextureData>> textureCache;
    std::map<std::string, std::shared_ptr<ShaderProgram>> shaderCache;
    std::map<std::string, std::shared_ptr<MeshData>> meshCache;
    std::map<std::string, std::shared_ptr<MaterialLibrary>> mtlCache;

    template <typename ResourceType>
    std::map<std::string, std::shared_ptr<ResourceType>> &getCache();

    // Disable copy/move operations for the singleton
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;
};

template <typename ResourceType>
inline void ResourceManager::unloadResource(const std::string &filename)
{
    auto &cache = getCache<ResourceType>();
    auto it = cache.find(filename);
    if (it != cache.end())
    {
        printf("Unloaded resource %s\n", filename.c_str());
        cache.erase(it);
    }
}

template <typename ResourceType>
inline void ResourceManager::purge()
{
    auto &cache = getCache<ResourceType>();
    std::vector<std::string> purgeList = {};
    for (auto &kvp : cache)
    {
        if (kvp.second.use_count() <= 1)
        {
            purgeList.push_back(kvp.first);
        }
    }
    for (auto &name : purgeList)
    {
        unloadResource<ResourceType>(name);
    }
}