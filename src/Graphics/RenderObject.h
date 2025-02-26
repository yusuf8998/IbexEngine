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

    const std::shared_ptr<MeshData> getData() const { return data; }
    const std::string &getName() const { return name; }

private:
    std::shared_ptr<MeshData> data;
    std::string name;
    GLuint VAO, VBO;

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
    std::vector<RenderGroup> groups;

    RenderObject(const std::string &filepath);
    RenderObject(const std::shared_ptr<MeshData> &data);

    void render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation);
    void renderRaw();

    static std::shared_ptr<RenderObject> GetRenderObject(const std::string &name);
    static std::shared_ptr<RenderObject> AddRenderObject(const std::string &name, std::shared_ptr<RenderObject> object);
    static bool HasRenderObject(const std::string &name);
    static void ReleaseAllMeshes();

    static void Purge();

private:
    void extractGroups(const std::shared_ptr<MeshData> &data);

    static std::unordered_map<std::string, std::shared_ptr<RenderObject>> Meshes;
};

void pushVertexData(MeshGroup &group, std::vector<float> *vertexData, const std::array<VertexAttrib, INDEX_PER_VERTEX> &attribs);