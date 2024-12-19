#pragma once

#include <vector>
#include "Node.h"

class SceneGraph
{
public:
    std::vector<Node *> staticObjects;  // Store static objects
    std::vector<Node *> dynamicObjects; // Store dynamic objects

    inline ~SceneGraph()
    {
        for (auto *node : staticObjects)
            delete node;
        for (auto *node : dynamicObjects)
            delete node;
    }

    // Add nodes to the scene graph
    inline void addStaticObject(Node *node)
    {
        staticObjects.push_back(node);
    }

    inline void addDynamicObject(Node *node)
    {
        dynamicObjects.push_back(node);
    }

    // Render static objects in batches (shared vertex buffer)
    inline void renderStaticObjects()
    {
        for (auto *node : staticObjects)
        {
            node->render();
        }
    }

    // Render dynamic objects one by one
    inline void renderDynamicObjects()
    {
        for (auto *node : dynamicObjects)
        {
            node->render();
        }
    }

    // Perform simple frustum culling (stub, for simplicity)
    inline void cullObjects()
    {
        // In a more complete system, you'd check if objects are within the camera's frustum
        // and only add them to the render list if they are visible.
    }

    // Render everything in the scene graph
    inline void render()
    {
        renderStaticObjects();
        renderDynamicObjects();
    }
};