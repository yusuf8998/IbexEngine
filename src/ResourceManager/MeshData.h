#pragma once

#include <map>
#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ResourceManager/Material.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GLAD/glad.h>

#include <splitString.h>

#include <ResourceManager/MaterialLibrary.h>

class MeshData
{
public:
    std::string filepath;
    std::string objectName = "Unnamed";
    std::map<std::string, std::vector<float>> vertexAttributes; // Vertex attributes like position, normal, uv
    std::map<std::string, std::vector<unsigned int>> indices;
    std::map<std::string, std::vector<std::string>> materials; // Material library, material name

    std::map<std::string, std::shared_ptr<MaterialLibrary>> materialLibraries;

    std::vector<std::string> getUsedTextures() const;

    bool loadFromOBJ(const std::string &filepath);

    // Helper function to access vertex attributes
    const std::vector<float> &getVertexAttribute(const std::string &name) const;

    size_t getFaceCount() const;

private:
    void parseOBJLine(const std::string &line);
};