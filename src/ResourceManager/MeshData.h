#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Graphics/Material.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GLAD/glad.h>

class MeshData
{
public:
    std::unordered_map<std::string, std::vector<float>> vertexAttributes; // Vertex attributes like position, normal, uv
    std::vector<unsigned int> indices;                                    // Indices for faces
    std::unordered_map<std::string, Material> materials;                  // Map of material names to materials
    std::vector<std::string> materialNames;                               // Names of the materials applied to faces

    bool loadFromOBJ(const std::string &filepath);
    bool loadMaterialsFromMTL(const std::string &mtlFilePath);

    // Helper function to access vertex attributes
    const std::vector<float> &getVertexAttribute(const std::string &name) const;

private:
    void parseOBJLine(const std::string &line);
    void parseMTLLine(const std::string &line, Material &currentMaterial, std::string &currentName);
};

// Helper function to split a string by delimiter
inline std::vector<std::string> splitString(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}