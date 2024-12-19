#pragma once

#include <GLAD/glad.h>
#include <vector>
#include <string>

struct Vertex
{
    float position[3]; // Vertex positions (x, y, z)
    float color[3];    // Color (r, g, b)
    float uv[2];

    Vertex() = default;
    Vertex(const glm::vec3 &pos, const glm::vec3 &clr, const glm::vec2 &_uv)
    {
        position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
        color[0] = clr.r; color[1] = clr.g; color[2] = clr.b;
        uv[0] = _uv.x; uv[1] = _uv.y;
    }

    void operator*=(const glm::mat4 &mat)
    {
        auto pos = glm::vec4(position[0], position[1], position[2], 1.f);
        pos = pos * mat;
        position[0] = pos.x;
        position[1] = pos.y;
        position[2] = pos.z;
    }

    Vertex operator*(const glm::mat4 &mat) const
    {
        auto pos = glm::vec4(position[0], position[1], position[2], 1.f);
        pos = mat * pos;
        return Vertex{glm::vec3{pos.x, pos.y, pos.z}, glm::vec3{color[0], color[1], color[2]}, glm::vec2{uv[0], uv[1]}};
    }
};

class Mesh
{
public:
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<Vertex> transformedVertices;
    std::vector<unsigned int> indices;

    GLuint VAO, VBO, EBO;

    inline Mesh(const std::string &name, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
        : name(name), vertices(vertices), indices(indices)
    {
        setupMesh();
    }

    inline void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    inline void applyTransformation(const glm::mat4 &mat)
    {
        transformedVertices.clear();
        for (Vertex v : vertices)
        {
            transformedVertices.push_back(v * mat);
        }
    }

    inline void draw()
    {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, transformedVertices.size() * sizeof(Vertex), &transformedVertices[0], GL_DYNAMIC_DRAW);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

inline Mesh combineMeshes(const std::string &new_name, const std::vector<Mesh> &meshes)
{
    std::vector<Vertex> combinedVertices;
    std::vector<unsigned int> combinedIndices;

    unsigned int vertexOffset = 0; // This will be used to adjust the indices in the other meshes

    // Combine vertices and indices from all meshes
    for (const Mesh &mesh : meshes)
    {
        // Append the vertices
        combinedVertices.insert(combinedVertices.end(), mesh.vertices.begin(), mesh.vertices.end());

        // Append the indices, adjusting them by the current offset
        for (unsigned int index : mesh.indices)
        {
            combinedIndices.push_back(index + vertexOffset);
        }

        // Update the vertex offset for the next mesh (the number of vertices in the current mesh)
        vertexOffset += mesh.vertices.size();
    }

    // Create and return the new combined mesh
    return Mesh(new_name, combinedVertices, combinedIndices);
}
