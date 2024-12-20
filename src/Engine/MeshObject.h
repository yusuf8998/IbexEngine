#pragma once

#include <GLAD/glad.h>
#include <vector>
#include <string>

#include <ResourceManager/MeshData.h>
#include <Graphics/ShaderObject.h>
#include <Graphics/TextureObject.h>

class MeshObject
{
public:
    MeshData *data;
    GLuint VAO, VBO, EBO;

    inline MeshObject(MeshData *data)
        : data(data)
    {
    }

    void generateOpenGLBuffers();
    void populateOpenGLBuffers(const glm::mat4 &transformation);
    void render(ShaderObject *shader, const glm::mat4 &transformation);
    TextureObject *loadTexture(const std::string &texturePath);
private:
    std::vector<unsigned int> tri_indices = {};
};

inline std::vector<unsigned int> generateTriangleIndices(int numVertices)
{
    std::vector<unsigned int> indices;

    if (numVertices < 3)
    {
        // A valid polygon must have at least 3 vertices (triangle).
        return indices;
    }

    // Let's assume vertices are ordered in a circular fashion.
    // Example: We have a polygon with 'numVertices' vertices, and we want to triangulate it.

    // Loop through the vertices, connecting the center vertex (0) to form triangles.
    // for (int i = 1; i < numVertices - 1; ++i)
    // {
    //     indices.push_back(0);     // Center vertex (assuming 0 is the center vertex)
    //     indices.push_back(i);     // First vertex of the triangle
    //     indices.push_back(i + 1); // Second vertex of the triangle
    // }

    for (int i = 0; i < numVertices; i++)
    {
        indices.push_back(i);
    }

    return indices;
}