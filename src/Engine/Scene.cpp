#include "Scene.h"
#include <Graphics/RenderObject.h>

void Scene::extractUsedResources()
{
    materials.clear();
    textures.clear();
    meshes.clear();

    auto lambda = [&](Node *node)
    {
        if (auto *cast = dynamic_cast<Renderable *>(node))
        {
            auto obj = RenderObject::GetRenderObject(cast->render_name);
            for (auto &group : obj->groups)
            {
                auto usedTextures = group.getData()->getGroup(group.getName()).getUsedTextures();
                textures.insert(textures.end(), usedTextures.begin(), usedTextures.end());
            }
            materials.insert(materials.end(), obj->groups[0].getData()->materialLibraries.begin(), obj->groups[0].getData()->materialLibraries.end());
            meshes.push_back(obj->groups[0].getData());
        }
    };

    root->traverse(lambda);
}