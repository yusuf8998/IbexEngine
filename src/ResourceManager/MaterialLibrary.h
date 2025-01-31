#pragma once

#ifndef MATERIAL_LIBRARY_H
#define MATERIAL_LIBRARY_H

#include <unordered_map>
#include <string>
#include <ResourceManager/Material.h>
#include <splitString.h>
#include <memory>

class MaterialLibrary
{
public:

    void addMaterial(const std::string &name, const std::shared_ptr<Material> &material);

    // Get a material from the library
    std::shared_ptr<Material> getMaterial(const std::string &name);

    // Check if a material exists in the library
    bool hasMaterial(const std::string &name) const;

    bool loadMaterialsFromMTL(const std::string &mtlFilePath);

    std::string findMaterialName(const std::shared_ptr<Material> &mat) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;

    void parseMTLLine(const std::string &line, std::shared_ptr<Material> &currentMaterial, std::string &currentName);
};

#endif // MATERIAL_LIBRARY_H