#include "RenderObject.h"
#include <Engine/Camera.h>
#include <Graphics/GL.h>

RenderObject::RenderObject(const std::string &filepath)
    : RenderObject(ResourceManager::instance().getResource<MeshData>(filepath))
{
}
RenderObject::RenderObject(const std::shared_ptr<MeshData> &data)
{
    if (Meshes[data->filepath] != nullptr)
        throw std::runtime_error("Mesh already loaded");
    extractGroups(data);
}

std::shared_ptr<RenderObject> RenderObject::GetRenderObject(const std::string &name)
{
    if (Meshes.count(name) != 0)
        return Meshes[name];
    Meshes[name] = std::make_shared<RenderObject>(name);
    return Meshes[name];
}

std::shared_ptr<RenderObject> RenderObject::AddRenderObject(const std::string &name, std::shared_ptr<RenderObject> object)
{
    Meshes[name] = object;
    return Meshes[name];
}

bool RenderObject::HasRenderObject(const std::string &name)
{
    return Meshes.count(name) != 0;
}

void RenderObject::ReleaseAllMeshes()
{
    Meshes.clear();
}

void RenderObject::Purge()
{
    std::vector<std::string> purgeList = {};
    for (auto &kvp : Meshes)
    {
        if (kvp.second.use_count() <= 1)
        {
            purgeList.push_back(kvp.first);
        }
    }

    for (auto &name : purgeList)
    {
        Meshes.erase(name);
        printf("Purged render object %s\n", name.c_str());
    }
}

void RenderObject::DebugUseCounts()
{
    for (auto &kvp : Meshes)
    {
        printf("Render object %s has %ld uses\n", kvp.first.c_str(), kvp.second.use_count());
    }
}

std::unordered_map<std::string, std::shared_ptr<RenderObject>> RenderObject::Meshes = {};

void pushVertexData(MeshGroup &group, std::vector<float> *vertexData, const std::array<VertexAttrib, INDEX_PER_VERTEX> &attribs)
{
    for (size_t i = 0; i < group.indices.size(); i++)
    {
        for (unsigned int j = 0; j < INDEX_PER_VERTEX; j++)
        {
            unsigned int idx = group.indices[i][j];
            unsigned stride = attribs[j].getStride();
            for (unsigned int k = 0; k < stride; k++)
            {
                if (attribs[j].values.size() != 0)
                    vertexData->push_back(attribs[j].values[idx * stride + k]);
                else
                    vertexData->push_back(0.f);
            }
        }
    }
}

void RenderObject::extractGroups(const std::shared_ptr<MeshData> &data)
{
    for (const auto &g : data->groups)
    {
        groups.push_back(RenderGroup(data, g.name));
    }
}

void RenderObject::render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation)
{
    for (auto &g : groups)
    {
        g.render(shader, transformation);
    }
}

void RenderObject::renderRaw()
{
    for (auto &g : groups)
    {
        g.renderRaw();
    }
}

///////////////////////////////////////////////////////////////////////////

RenderGroup::RenderGroup(const std::shared_ptr<MeshData> &data, const std::string &name)
    : data(data), name(name)
{
    generateOpenGLBuffers();
    populateOpenGLBuffers();
}

GLenum RenderGroup::getDrawMode() const
{
    if (data->getVertexPerFace(name) == 0)
        return 0;
    if (data->getVertexPerFace(name) == 1)
        return GL_POINTS;
    if (data->getVertexPerFace(name) == 2)
        return GL_LINES;
    if (data->getVertexPerFace(name) == 3)
        return GL_TRIANGLES;
    if (data->getVertexPerFace(name) == 4)
        return GL_TRIANGLE_STRIP;
    throw std::runtime_error("Data has invalid vertexPerFace value. vertexPerFace:" + std::to_string(data->getVertexPerFace(name)));
}

void RenderGroup::generateOpenGLBuffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

void RenderGroup::populateOpenGLBuffers()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Flatten vertex data (positions, normals, and UVs)
    std::vector<float> vertexData;
    pushVertexData(data->getGroup(name), &vertexData, data->getAttribs());

    // Populate the VBO with interleaved data
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Enable the vertex attributes
    // Position attribute
    const size_t stride = data->getVertexStride();
    size_t offset = 0;

    for (unsigned int i = 0; i < INDEX_PER_VERTEX; i++)
        defineVertexAttrib(i, ATTRIB_STRIDE[i], stride, offset);

    // Unbind the VAO and buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    textureArray = std::make_shared<TextureArrayObject>(data->getGroup(name).getUsedTextures());
}

void RenderGroup::reuploadToGLBuffers()
{
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    generateOpenGLBuffers();
    populateOpenGLBuffers();
}

#include <ResourceManager/TextureData.h>

void RenderGroup::render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation)
{
    // Check for OpenGL errors
    GLClearError();

    // Bind the VAO for rendering
    GLCall(glBindVertexArray(VAO));

    shader->use();

    // Set material properties (e.g., diffuse color)
    const auto &material = data->getGroup(name).material;
    shader->setVec3("material.diffuse", material->diffuse);
    shader->setVec3("material.specular", material->specular);
    shader->setFloat("material.shininess", material->shininess);

    shader->setInt("material.diffuseIndex", -1);
    shader->setInt("material.specularIndex", -1);
    shader->setInt("material.normalIndex", -1);
    shader->setInt("material.displacementIndex", -1);

    for (size_t i = 0; i < textureArray->getDatas().size(); i++)
    {
        if (textureArray->getDatas()[i]->getName() == material->diffuseTexture)
        {
            shader->setInt("material.diffuseIndex", i);
        }
        else if (textureArray->getDatas()[i]->getName() == material->specularTexture)
        {
            shader->setInt("material.specularIndex", i);
        }
        else if (textureArray->getDatas()[i]->getName() == material->normalMap)
        {
            shader->setInt("material.normalIndex", i);
        }
        else if (textureArray->getDatas()[i]->getName() == material->dispMap)
        {
            shader->setInt("material.displacementIndex", i);
        }
    }

    textureArray->bind(0);
    shader->setInt("material.textures", 0);

    shader->setMat4("model", transformation);

    shader->setVec3("light.direction", glm::vec3(0.f, -1.f, 0.f));
    shader->setVec3("light.ambient", glm::vec3(0.125f, 0.125f, 0.125f));
    shader->setVec3("light.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    shader->setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));

    shader->setVec3("viewPos", mainCamera.position);

    // Draw the mesh
    GLCall(glDrawArrays(getDrawMode(), 0, data->getFaceCount(name) * data->getVertexPerFace(name)));

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    ASSERT(GLLogCall("glDrawElements", __FILE__, __LINE__));
}

void RenderGroup::renderRaw()
{
    // Bind the VAO for rendering
    glBindVertexArray(VAO);

    // Draw the mesh
    GLCall(glDrawArrays(getDrawMode(), 0, data->getFaceCount(name) * data->getVertexPerFace(name)));

    // Unbind the VAO
    glBindVertexArray(0);
}