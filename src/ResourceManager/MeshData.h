#pragma once

#include <map>
#include <vector>
#include <array>
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

class MeshGroup
{
public:
    std::string name;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material;

    short vertexPerFace = 0;
};

class RenderObject;

class MeshData
{
public:
    std::string filepath;
    std::string objectName = "Unnamed";

    static const unsigned int POSITION_OFFSET = 0;
    static const unsigned int UV_OFFSET = 1;
    static const unsigned int NORMAL_OFFSET = 2;
    static const unsigned int TANGENT_OFFSET = 3;

    static const unsigned int INDEX_PER_VERTEX = 4;

    std::map<std::string, std::vector<std::string>> materials; // Material library, material names
    std::map<std::string, std::shared_ptr<MaterialLibrary>> materialLibraries;

    std::vector<std::string> getUsedTextures() const;

    bool loadFromOBJ(const std::string &filepath);

    // Helper function to access vertex attributes
    const std::vector<float> &getVertexAttribute(const std::string &name) const;

    short getVertexPerFace(const std::string &groupName) const;
    short getVertexPerFace(const MeshGroup &group) const;
    size_t getVertexStride() const;
    size_t getFaceCount() const;
    size_t getFaceCount(const std::string &groupName) const;
    size_t getFaceCount(const MeshGroup &group) const;

    MeshGroup &getGroup(const std::string &groupName);
    const MeshGroup &getGroup(const std::string &groupName) const;

    bool hasGroup(const std::string &groupName) const;

    friend class RenderObject;

private:
    std::map<std::string, std::vector<float>> vertexAttributes; // Vertex attributes: position, uv, normal, tangent, bitangent
    std::vector<MeshGroup> groups;

    void parseOBJLine(const std::string &line);

    void generateGroup(const std::string &name);
    void UseMaterial(const std::string &materialName, MeshGroup &group);

    void calcTangentBitangentForMesh();
    void calcTangentBitangentForGroup(const std::string &groupName);
    void calcTangentBitangentForGroup(MeshGroup &group);

    static void calcTangentBitangentForTri(const std::array<glm::vec3, 3> &positions, const std::array<glm::vec2, 3> &uvs, const std::array<glm::vec3, 3> &normals, glm::vec3 &tangent, glm::vec3 &bitangent);

    std::vector<std::vector<float>> getFace(const std::string &groupName, unsigned int face_index);
    std::vector<std::vector<float>> getFace(const MeshGroup &group, unsigned int face_index);
};