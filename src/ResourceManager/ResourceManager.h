#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <iostream>

// Forward declaration for resource types
class TextureData;
class ShaderData;

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

    // Cleanup
    void clear();

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    // Map for storing resources by filename
    std::unordered_map<std::string, std::shared_ptr<TextureData>> textureCache;
    std::unordered_map<std::string, std::shared_ptr<ShaderData>> shaderCache;

    // Disable copy/move operations for the singleton
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;
};