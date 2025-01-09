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

std::unordered_map<std::string, std::shared_ptr<MeshObject>> MeshObject::Meshes = {};

void MeshObject::generateOpenGLBuffers()
{
    // Create a VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);
    // Create a VBO (Vertex Buffer Object)
    glGenBuffers(1, &VBO);
    // Create an EBO (Element Buffer Object)
    glGenBuffers(1, &EBO);

    tri_indices = generateTriangleIndices(data->getFaceCount());
}

void MeshObject::pushVertexData(const std::string &groupName, std::vector<float> *vertexData, const std::vector<float> &positions, const std::vector<float> &uvs, const std::vector<float> &normals)
{
    for (size_t i = 0; i < data->indices[groupName].size() / 3; i++)
    {
        unsigned int posIdx = data->indices[groupName][i * 3];
        unsigned int uvIdx = data->indices[groupName][i * 3 + 1];
        unsigned int normalIdx = data->indices[groupName][i * 3 + 2];

        // glm::vec4 pos = glm::vec4(positions[posIdx * 3], positions[posIdx * 3 + 1], positions[posIdx * 3 + 2], 1.f);
        // pos = pos * transformation;
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
    }
}

void MeshObject::populateOpenGLBuffers()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Flatten vertex data (positions, normals, and UVs)
    std::vector<float> vertexData;
    const std::vector<float> &positions = data->getVertexAttribute("position");
    const std::vector<float> &uvs = data->getVertexAttribute("uv");
    const std::vector<float> &normals = data->getVertexAttribute("normal");

    for (auto &kvp : data->indices)
    {
        pushVertexData(kvp.first, &vertexData, positions, uvs, normals);
    }

    // Populate the VBO with interleaved data
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tri_indices.size() * sizeof(unsigned int), tri_indices.data(), GL_STATIC_DRAW);

    // Enable the vertex attributes
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // UV attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind the VAO and buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    textureArray = std::make_shared<TextureArrayObject>(data->getUsedTextures());
}

void MeshObject::render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation)
{
    // Bind the VAO for rendering
    glBindVertexArray(VAO);

    shader->use();

    // Set material properties (e.g., diffuse color)
    for (auto &kvp : data->materials)
    {
        const auto *material = data->materialLibraries[kvp.first]->getMaterial(kvp.second[0]);
        // Set shader uniform for material properties (diffuse, specular, etc.)
        shader->setVec3("material.diffuse", material->diffuse);
        shader->setVec3("material.specular", material->specular);
        shader->setFloat("material.shininess", material->shininess);

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
        }
    }

    textureArray->bind(GL_TEXTURE0);
    shader->setInt("material.textures", 0);

    shader->setMat4("model", transformation);

    shader->setVec3("dirLight.direction", glm::vec3(0.f, -1.0f, 0.f));
    shader->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    shader->setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
    shader->setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));

    shader->setVec3("viewPos", mainCamera.position);

    // Draw the mesh
    GLCall(glDrawElements(GL_TRIANGLES, tri_indices.size(), GL_UNSIGNED_INT, tri_indices.data()));

    // Unbind the VAO
    glBindVertexArray(0);
}

void MeshObject::renderRaw()
{
    // Bind the VAO for rendering
    glBindVertexArray(VAO);
    // Draw the mesh
    GLCall(glDrawElements(GL_TRIANGLES, tri_indices.size(), GL_UNSIGNED_INT, tri_indices.data()));
    // Unbind the VAO
    glBindVertexArray(0);
}

TextureObject *MeshObject::loadTexture(const std::string &texturePath)
{
    return TextureObject::getTextureByName(texturePath);
}