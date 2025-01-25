#include "MeshObject.h"
#include <Engine/Camera.h>

void GLClearError();
bool GLLogCall(const char *function, const char *file, int line);

#define ASSERT(x) \
    if (!(x))     \
        throw;
// #if DEBUG
#define GLCall(x)   \
    GLClearError(); \
    x;              \
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
// #else
// #define GLCall(x) x;
// #endif

inline void GLClearError()
{
    while (glGetError() != GL_NO_ERROR)
        ;
};
inline bool GLLogCall(const char *function, const char *file, int line)
{
    while (GLenum error = glGetError())
    {
        std::string error_name = "";
        switch (error)
        {
        case GL_INVALID_ENUM:
            error_name = "Invalid Enum";
            break;
        case GL_INVALID_VALUE:
            error_name = "Invalid Value";
            break;
        case GL_INVALID_OPERATION:
            error_name = "Invalid Operation";
            break;
        case GL_STACK_OVERFLOW:
            error_name = "Stack Overflow";
            break;
        case GL_STACK_UNDERFLOW:
            error_name = "Stack Underflow";
            break;
        case GL_OUT_OF_MEMORY:
            error_name = "Out Of Memory";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error_name = "Invalid Framebuffer Operation";
            break;
        default:
            error_name = std::to_string(error);
            break;
        }
        printf("[OpenGL Error]: (%s) %s %s: %i\n", error_name.c_str(), function, file, line);
        return false;
    }
    return true;
}

std::unordered_map<std::string, std::shared_ptr<RenderObject>> RenderObject::Meshes = {};

void pushVertexData(MeshGroup &group, std::vector<float> *vertexData, const std::vector<float> &positions, const std::vector<float> &uvs, const std::vector<float> &normals, const std::vector<float> &tangents)
{
    for (size_t i = 0; i < group.indices.size() / MeshData::INDEX_PER_VERTEX; i++)
    {
        unsigned int posIdx = group.indices[i * MeshData::INDEX_PER_VERTEX + MeshData::POSITION_OFFSET];
        unsigned int uvIdx = group.indices[i * MeshData::INDEX_PER_VERTEX + MeshData::UV_OFFSET];
        unsigned int normalIdx = group.indices[i * MeshData::INDEX_PER_VERTEX + MeshData::NORMAL_OFFSET];
        unsigned int tangentIdx = group.indices[i * MeshData::INDEX_PER_VERTEX + MeshData::TANGENT_OFFSET];

        // Push position
        vertexData->push_back(positions[posIdx * 3 + 0]);
        vertexData->push_back(positions[posIdx * 3 + 1]);
        vertexData->push_back(positions[posIdx * 3 + 2]);

        // Push UV
        vertexData->push_back(uvs[uvIdx * 2 + 0]);
        vertexData->push_back(uvs[uvIdx * 2 + 1]);

        // Push normal
        vertexData->push_back(normals[normalIdx * 3 + 0]);
        vertexData->push_back(normals[normalIdx * 3 + 1]);
        vertexData->push_back(normals[normalIdx * 3 + 2]);

        // Push tangent
        vertexData->push_back(tangents[tangentIdx * 3 + 0]);
        vertexData->push_back(tangents[tangentIdx * 3 + 1]);
        vertexData->push_back(tangents[tangentIdx * 3 + 2]);
    }
}

void defineVertexAttrib(int i, int count, size_t stride, size_t &offset)
{
    glVertexAttribPointer(i, count, GL_FLOAT, GL_FALSE, stride, (void *)offset);
    offset += count * sizeof(float);
    glEnableVertexAttribArray(i);
}

void RenderObject::extractGroups()
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

TextureObject *RenderObject::loadTexture(const std::string &texturePath)
{
    return TextureObject::getTextureByName(texturePath);
}

///////////////////////////////////////////////////////////////////////////

GLenum RenderGroup::getDrawMode() const
{
    if (data->getVertexPerFace(name) == 0)
        return 0;
    if (data->getVertexPerFace(name) == 3)
        return GL_TRIANGLES;
    if (data->getVertexPerFace(name) == 4)
        return GL_TRIANGLE_STRIP;
    throw std::runtime_error("Data has invalid vertexPerFace value. vertexPerFace:" + std::to_string(data->getVertexPerFace(name)));
}

void RenderGroup::generateOpenGLBuffers()
{
    // Create a VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);
    // Create a VBO (Vertex Buffer Object)
    glGenBuffers(1, &VBO);
    // Create an EBO (Element Buffer Object)
    glGenBuffers(1, &EBO);

    indices = generateIndices(data->getFaceCount(name) * data->getVertexPerFace(name));
}

void RenderGroup::populateOpenGLBuffers()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Flatten vertex data (positions, normals, and UVs)
    std::vector<float> vertexData;
    const std::vector<float> &positions = data->getVertexAttribute("position");
    const std::vector<float> &uvs = data->getVertexAttribute("uv");
    const std::vector<float> &normals = data->getVertexAttribute("normal");
    const std::vector<float> &tangents = data->getVertexAttribute("tangent");

    pushVertexData(data->getGroup(name), &vertexData, positions, uvs, normals, tangents);

    // Populate the VBO with interleaved data
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Enable the vertex attributes
    // Position attribute
    const size_t stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec3);
    size_t offset = 0;

    defineVertexAttrib(0, 3, stride, offset); // position
    defineVertexAttrib(1, 2, stride, offset); // UV
    defineVertexAttrib(2, 3, stride, offset); // normal
    defineVertexAttrib(3, 3, stride, offset); // tangent

    // Unbind the VAO and buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    textureArray = std::make_shared<TextureArrayObject>(data->getUsedTextures());
}

void RenderGroup::render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation)
{
    // Bind the VAO for rendering
    glBindVertexArray(VAO);

    shader->use();

    // Set material properties (e.g., diffuse color)
    for (auto &kvp : data->materials)
    {
        const auto material = data->materialLibraries[kvp.first]->getMaterial(kvp.second[0]);
        // Set shader uniform for material properties (diffuse, specular, etc.)
        shader->setVec3("material.diffuse", material->diffuse);
        shader->setVec3("material.specular", material->specular);
        shader->setFloat("material.shininess", material->shininess);

        shader->setInt("material.diffuseIndex", -1);
        shader->setInt("material.specularIndex", -1);
        shader->setInt("material.normalIndex", -1);

        for (size_t i = 0; i < textureArray->getFilePaths().size(); i++)
        {
            if (textureArray->getFilePaths()[i] == material->diffuseTexture)
            {
                shader->setInt("material.diffuseIndex", i);
            }
            else if (textureArray->getFilePaths()[i] == material->specularTexture)
            {
                shader->setInt("material.specularIndex", i);
            }
            else if (textureArray->getFilePaths()[i] == material->normalMap)
            {
                shader->setInt("material.normalIndex", i);
            }
        }
    }

    textureArray->bind(GL_TEXTURE0);
    shader->setInt("material.textures", 0);

    shader->setMat4("model", transformation);

    shader->setVec3("light.direction", glm::vec3(0.f, -1.f, 0.f));
    shader->setVec3("light.ambient", glm::vec3(0.125f, 0.125f, 0.125f));
    shader->setVec3("light.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    shader->setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));

    shader->setVec3("viewPos", mainCamera.position);

    // Draw the mesh
    GLCall(glDrawElements(getDrawMode(), indices.size(), GL_UNSIGNED_INT, indices.data()));

    // Unbind the VAO
    glBindVertexArray(0);
}

void RenderGroup::renderRaw()
{
    // Bind the VAO for rendering
    glBindVertexArray(VAO);

    // Draw the mesh
    GLCall(glDrawElements(getDrawMode(), indices.size(), GL_UNSIGNED_INT, indices.data()));

    // Unbind the VAO
    glBindVertexArray(0);
}