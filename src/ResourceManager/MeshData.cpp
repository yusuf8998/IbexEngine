#include "MeshData.h"
#include <stb/stb_image.h>

bool MeshData::loadFromOBJ(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return false;
    }

    std::string line;

    while (std::getline(file, line))
    {
        parseOBJLine(line);
    }

    file.close();
    return true;
}

void MeshData::parseOBJLine(const std::string &line)
{
    if (line.empty())
        return;
    std::string uncomment = splitString(line, '#')[0];
    std::vector<std::string> tokens = splitString(uncomment, ' ');
    if (tokens.empty())
        return;

    if (tokens[0] == "v")
    { // Vertex position
        if (tokens.size() == 4)
        {
            glm::vec3 position(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
            vertexAttributes["position"].push_back(position.x);
            vertexAttributes["position"].push_back(position.y);
            vertexAttributes["position"].push_back(position.z);
        }
    }
    else if (tokens[0] == "vn")
    { // Vertex normal
        if (tokens.size() == 4)
        {
            glm::vec3 normal(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
            vertexAttributes["normal"].push_back(normal.x);
            vertexAttributes["normal"].push_back(normal.y);
            vertexAttributes["normal"].push_back(normal.z);
        }
    }
    else if (tokens[0] == "vt")
    { // Vertex texture coordinate
        if (tokens.size() == 3)
        {
            glm::vec2 uv(std::stof(tokens[1]), std::stof(tokens[2]));
            vertexAttributes["uv"].push_back(uv.x);
            vertexAttributes["uv"].push_back(uv.y);
        }
    }
    else if (tokens[0] == "f")
    { // Face (index list)
        if (tokens.size() >= 4)
        {
            for (size_t i = 1; i < tokens.size(); ++i)
            {
                std::vector<std::string> vertexData = splitString(tokens[i], '/');
                unsigned int posIdx = std::stoi(vertexData[0]) - 1;
                unsigned int uvIdx = std::stoi(vertexData[1]) - 1;
                unsigned int normalIdx = std::stoi(vertexData[2]) - 1;
                indices.push_back(posIdx);
                indices.push_back(uvIdx);
                indices.push_back(normalIdx);
            }
        }
    }
    else if (tokens[0] == "usemtl")
    { // Material reference
        if (tokens.size() == 2)
        {
            materialNames.push_back(tokens[1]); // Store the material name for subsequent faces
        }
    }
}

bool MeshData::loadMaterialsFromMTL(const std::string &mtlFilePath)
{
    std::ifstream file(mtlFilePath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open material file " << mtlFilePath << std::endl;
        return false;
    }

    std::string line;
    Material currentMaterial;
    std::string currentName;

    while (std::getline(file, line))
    {
        parseMTLLine(line, currentMaterial, currentName);
    }
    materials[currentName] = currentMaterial;
    file.close();
    return true;
}

void MeshData::parseMTLLine(const std::string &line, Material &currentMaterial, std::string &currentName)
{
    std::vector<std::string> tokens = splitString(line, ' ');

    if (tokens.empty())
        return;

    if (tokens[0] == "newmtl")
    { // New material definition
        if (currentMaterial.diffuse != glm::vec3(0.f))
        {
            materials[currentName] = currentMaterial; // Store the previous material
        }
        currentName = tokens[1];
        currentMaterial = Material();
    }
    else if (tokens[0] == "Ka")
    { // Ambient color
        currentMaterial.ambient = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Kd")
    { // Diffuse color
        currentMaterial.diffuse = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Ks")
    { // Specular color
        currentMaterial.specular = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Ns")
    { // Shininess
        currentMaterial.shininess = std::stof(tokens[1]);
    }
    else if (tokens[0] == "map_Kd")
    { // Diffuse texture map
        currentMaterial.diffuseTexture = tokens[1];
    }
}

const std::vector<float> &MeshData::getVertexAttribute(const std::string &name) const
{
    auto it = vertexAttributes.find(name);
    if (it != vertexAttributes.end())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("Attribute not found: " + name);
    }
}