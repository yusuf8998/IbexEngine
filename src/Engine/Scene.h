#pragma once

#include <memory>
#include "SceneGraph.h"
#include <ResourceManager/ResourceManager.h>

struct Scene
{
private:
    std::vector<std::string> materials;
    std::vector<std::string> textures;
    std::vector<std::string> shaders;
    std::vector<std::string> meshes;

public:
    std::shared_ptr<Node> root;
    std::shared_ptr<Node> activeCamera;
    std::shared_ptr<ShaderObject> defaultShader;

    void extractUsedResources();

    inline void load(const std::string &filename)
    {
        loadSceneGraph(filename, root);
        extractUsedResources();
    }

    inline void save(const std::string &filename)
    {
        saveSceneGraph(filename, root);
    }
};
