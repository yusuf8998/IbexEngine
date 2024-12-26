#include "MaterialLibrary.h"
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

void MaterialLibrary::addMaterial(const std::string &name, const Material &material)
{
    if (materials.count(name) == 0)
        materials[name] = material;
    else
        std::cout << "Warning: Material " << name << " already exists in the library" << std::endl;
}

Material *MaterialLibrary::getMaterial(const std::string &name)
{
    if (materials.count(name) == 0)
        return 0;
    return &materials.at(name);
}

bool MaterialLibrary::hasMaterial(const std::string &name) const
{
    return materials.count(name) != 0;
}

bool MaterialLibrary::loadMaterialsFromMTL(const std::string &mtlFilePath)
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

void MaterialLibrary::parseMTLLine(const std::string &line, Material &currentMaterial, std::string &currentName)
{
    if (line.empty())
        return;
    std::string uncomment = splitString(line, '#')[0];
    std::vector<std::string> tokens = splitString(uncomment, ' ');
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
    else if (tokens[0] == "map_Ks")
    { // Specular texture map
        currentMaterial.specularTexture = tokens[1];
    }
}