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

    RenderGroup(const std::shared_ptr<MeshData> &data, const std::string &name);

private:
    std::shared_ptr<MeshData> data;
    std::string name;
    GLuint VAO, VBO, EBO;
    std::vector<unsigned int> elementIndices = {};

    void generateOpenGLBuffers();
    void populateOpenGLBuffers();

    void reuploadToGLBuffers();

    void render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation);
    void renderRaw();

    std::shared_ptr<TextureArrayObject> textureArray;
};

class RenderObject
{
public:
    std::shared_ptr<MeshData> data;
    std::vector<RenderGroup> groups;

    RenderObject(const std::string &filepath);
    RenderObject(std::shared_ptr<MeshData> data);

    void render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation);
    void renderRaw();
    TextureObject *loadTexture(const std::string &texturePath);

    static std::shared_ptr<RenderObject> GetRenderObject(const std::string &name);
    static std::shared_ptr<RenderObject> AddRenderObject(const std::string &name, std::shared_ptr<RenderObject> object);
    static void ReleaseAllMeshes();

    void reuploadToGLBuffers();

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