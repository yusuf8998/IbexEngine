#pragma once

#ifndef MATERIAL_LIBRARY_H
#define MATERIAL_LIBRARY_H

#include <unordered_map>
#include <string>
#include <ResourceManager/Material.h>
#include <splitString.h>
#include <memory>

class MaterialLibrary {
public:
    // Add a material to the library
    void addMaterial(const std::string& name, const Material& material);

    // Get a material from the library
    Material *getMaterial(const std::string& name);

    // Check if a material exists in the library
    bool hasMaterial(const std::string& name) const;

    bool loadMaterialsFromMTL(const std::string &mtlFilePath);

private:
    std::unordered_map<std::string, Material> materials;

    void parseMTLLine(const std::string &line, Material &currentMaterial, std::string &currentName);
};

#endif // MATERIAL_LIBRARY_H