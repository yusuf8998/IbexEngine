#pragma once

#include <GLAD/glad.h>
#include <vector>
#include <string>

#include <ResourceManager/ResourceManager.h>
#include <ResourceManager/MeshData.h>
#include <Graphics/ShaderObject.h>
#include <Graphics/TextureObject.h>

class MeshObject
{
public:
    MeshData *data;
    GLuint VAO, VBO, EBO;

    inline MeshObject(const std::string &filepath)
        : MeshObject(ResourceManager::instance().getResource<MeshData>(filepath).get())
    {}
    inline MeshObject(MeshData *data)
        : data(data)
    {
        if (Meshes[data->filepath] != nullptr)
            throw std::runtime_error("Mesh already loaded");
        Meshes[data->filepath] = this;
        generateOpenGLBuffers();
        populateOpenGLBuffers();
    }

    void generateOpenGLBuffers();
    void populateOpenGLBuffers();
    void render(ShaderObject *shader, const glm::mat4 &transformation);
    TextureObject *loadTexture(const std::string &texturePath);

    static MeshObject *GetMeshObject(const std::string &name)
    {
        if (Meshes.count(name) != 0)
            return Meshes[name];
        return new MeshObject(name);
    }
private:
    std::vector<unsigned int> tri_indices = {};

    static std::unordered_map<std::string, MeshObject *> Meshes;
};

inline std::vector<unsigned int> generateTriangleIndices(int numVertices)
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