#pragma once

#include <GLAD/glad.h>
#include <vector>
#include <string>
#include <memory>

#include <ResourceManager/ResourceManager.h>
#include <ResourceManager/MeshData.h>
#include <Graphics/ShaderObject.h>
#include <Graphics/TextureObject.h>
#include "TextureArrayObject.h"

class RenderObject;

class RenderGroup
{
public:
    GLenum getDrawMode() const;

    friend class RenderObject;

    RenderGroup(const std::shared_ptr<MeshData> &data, const std::string &name)
        : data(data), name(name)
    {
        generateOpenGLBuffers();
        populateOpenGLBuffers();
    }

private:
    std::shared_ptr<MeshData> data;
    std::string name;
    GLuint VAO, VBO, EBO;
    std::vector<unsigned int> indices = {};

    void generateOpenGLBuffers();
    void populateOpenGLBuffers();

    void render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation);
    void renderRaw();

    std::shared_ptr<TextureArrayObject> textureArray;
};

class RenderObject
{
public:
    std::shared_ptr<MeshData> data;
    std::vector<RenderGroup> groups;

    inline RenderObject(const std::string &filepath)
        : RenderObject(ResourceManager::instance().getResource<MeshData>(filepath))
    {}
    inline RenderObject(std::shared_ptr<MeshData> data)
        : data(data)
    {
        if (Meshes[data->filepath] != nullptr)
            throw std::runtime_error("Mesh already loaded");
        extractGroups();
    }

    void render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation);
    void renderRaw();
    TextureObject *loadTexture(const std::string &texturePath);

    static std::shared_ptr<RenderObject> GetRenderObject(const std::string &name)
    {
        if (Meshes.count(name) != 0)
            return Meshes[name];
        Meshes[name] = std::make_shared<RenderObject>(name);
        return Meshes[name];
    }

    static void AddRenderObject(const std::string &name, std::shared_ptr<RenderObject> object)
    {
        Meshes[name] = object;
    }

    static void ReleaseAllMeshes()
    {
        Meshes.clear();
    }
private:
    void extractGroups();

    static std::unordered_map<std::string, std::shared_ptr<RenderObject>> Meshes;
};

void pushVertexData(MeshGroup &group, std::vector<float> *vertexData, const std::vector<float> &positions, const std::vector<float> &uvs, const std::vector<float> &normals, const std::vector<float> &tangents);

inline std::vector<unsigned int> generateIndices(int numVertices)
{
    std::vector<unsigned int> indices;

    if (numVertices < 3)
    {
        // A valid polygon must have at least 3 vertices (triangle).
        return indices;
    }

    for (int i = 0; i < numVertices; i++)
    {
        indices.push_back(i);
    }

    return indices;
}