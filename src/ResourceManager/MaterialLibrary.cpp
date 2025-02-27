#include "MaterialLibrary.h"
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

void MaterialLibrary::addMaterial(const std::string &name, const std::shared_ptr<Material> &material)
{
    materials[name] = material;
}

std::shared_ptr<Material> MaterialLibrary::getMaterial(const std::string &name)
{
    if (materials.count(name) == 0)
        return 0;
    return materials.at(name);
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
    std::shared_ptr<Material> currentMaterial;
    std::string currentName;

    while (std::getline(file, line))
    {
        parseMTLLine(line, currentMaterial, currentName);
    }
    materials[currentName] = currentMaterial;
    file.close();
    std::cout << "Loaded material library: " << mtlFilePath << std::endl;
    return true;
}

std::string MaterialLibrary::findMaterialName(const std::shared_ptr<Material> &mat) const
{
    for (const auto &kvp : materials)
    {
        if (mat == kvp.second)
            return kvp.first;
    }
    throw std::runtime_error("Can't find name from given material");
}

void MaterialLibrary::parseMTLLine(const std::string &line, std::shared_ptr<Material> &currentMaterial, std::string &currentName)
{
    if (line.empty())
        return;
    std::string uncomment = splitString(line, '#')[0];
    std::vector<std::string> tokens = splitString(uncomment, ' ');
    if (tokens.empty())
        return;

    if (tokens[0] == "newmtl")
    { // New material definition
        if (currentMaterial)
        {
            materials[currentName] = currentMaterial; // Store the previous material
        }
        currentName = tokens[1];
        currentMaterial = std::make_shared<Material>();
    }
    else if (tokens[0] == "illum")
    { // Illumination model
        currentMaterial->illum = std::stoi(tokens[1]);
    }
    else if (tokens[0] == "Ka")
    { // Ambient color
        currentMaterial->ambient = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Kd")
    { // Diffuse color
        currentMaterial->diffuse = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Ks")
    { // Specular color
        currentMaterial->specular = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
    }
    else if (tokens[0] == "Ns")
    { // Shininess
        currentMaterial->shininess = std::stof(tokens[1]);
    }
    else if (tokens[0] == "map_Kd")
    { // Diffuse texture map
        currentMaterial->diffuseTexture = tokens[1];
    }
    else if (tokens[0] == "map_Ks")
    { // Specular texture map
        currentMaterial->specularTexture = tokens[1];
    }
    else if (tokens[0] == "norm" || tokens[0] == "map_Bump")
    { // Normal map
        currentMaterial->normalMap = tokens[1];
    }
    else if (tokens[0] == "disp")
    { // Displacement map
        currentMaterial->dispMap = tokens[1];
    }
}