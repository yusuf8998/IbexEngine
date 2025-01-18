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

class MeshObject
{
public:
    std::shared_ptr<MeshData> data;
    GLuint VAO, VBO, EBO;

    inline MeshObject(const std::string &filepath)
        : MeshObject(ResourceManager::instance().getResource<MeshData>(filepath))
    {}
    inline MeshObject(std::shared_ptr<MeshData> data)
        : data(data)
    {
        if (Meshes[data->filepath] != nullptr)
            throw std::runtime_error("Mesh already loaded");
        generateOpenGLBuffers();
        populateOpenGLBuffers();
    }

    void generateOpenGLBuffers();
    void populateOpenGLBuffers();
    void render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation);
    void renderRaw();
    TextureObject *loadTexture(const std::string &texturePath);

    static std::shared_ptr<MeshObject> GetMeshObject(const std::string &name)
    {
        if (Meshes.count(name) != 0)
            return Meshes[name];
        Meshes[name] = std::make_shared<MeshObject>(name);
        return Meshes[name];
    }

    static void ReleaseAllMeshes()
    {
        Meshes.clear();
    }
private:
    std::vector<unsigned int> indices = {};

    std::shared_ptr<TextureArrayObject> textureArray;

    static std::unordered_map<std::string, std::shared_ptr<MeshObject>> Meshes;

    void pushVertexData(const std::string &groupName, std::vector<float> *vertexData, const std::vector<float> &positions, const std::vector<float> &uvs, const std::vector<float> &normals, const std::vector<float> &tangents, const std::vector<float> &bitangents);

    GLenum getDrawMode() const;
};

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