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

class MeshData;

constexpr unsigned int POSITION_OFFSET = 0;
constexpr unsigned int UV_OFFSET = 1;
constexpr unsigned int NORMAL_OFFSET = 2;
constexpr unsigned int TANGENT_OFFSET = 3;

constexpr unsigned int INDEX_PER_VERTEX = 4;

// Just keep in mind
// typedef glm::vec<INDEX_PER_VERTEX, unsigned int, glm::defaultp> Vertex; // Position, UV, Normal, Tangent

class MeshGroup
{
public:
    std::string name;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material;

    short vertexPerFace = 0;

    std::vector<std::string> getUsedTextures() const;

    friend class MeshData;

private:
    static MeshGroup CombineGroups(const MeshGroup &a, const MeshGroup &b, unsigned int positionOffset, unsigned int uvOffset, unsigned int normalOffset, unsigned int tangentOffset);
    bool canCombine(const MeshGroup &other) const;
};

class RenderObject;

class MeshData
{
public:
    std::string filepath;
    std::string objectName = "Unnamed";

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

    unsigned int getPositionOffset() const;
    unsigned int getUVOffset() const;
    unsigned int getNormalOffset() const;
    unsigned int getTangentOffset() const;

    void removeDuplicateAttributes();

    friend class RenderObject;
    static std::shared_ptr<MeshData> CombineMeshes(const MeshData &a, const MeshData &b);
    static void FlattenGroupVector(std::vector<MeshGroup> &groups);

private:
    std::map<std::string, std::vector<float>> vertexAttributes; // Vertex attributes: position, uv, normal, tangent, bitangent
    std::vector<MeshGroup> groups;

    void parseOBJLine(const std::string &line);

    void generateGroup(const std::string &name);
    void UseMaterial(const std::string &materialName, MeshGroup &group);

    bool compareAttributes(std::vector<float>::const_iterator &it, std::vector<float>::const_iterator &jt, unsigned int stride);
    bool compareAttributes(std::vector<float>::iterator &it, std::vector<float>::iterator &jt, unsigned int stride);
    void removeDuplicateAttribute(const std::string &name, unsigned int stride, std::map<unsigned int, unsigned int> &map);

    void calcTangentBitangentForMesh();
    void calcTangentBitangentForGroup(const std::string &groupName);
    void calcTangentBitangentForGroup(MeshGroup &group);

    static void CalcTangentBitangentForTri(const std::array<glm::vec3, 3> &positions, const std::array<glm::vec2, 3> &uvs, const std::array<glm::vec3, 3> &normals, glm::vec3 &tangent, glm::vec3 &bitangent);

    std::vector<std::vector<float>> getFace(const std::string &groupName, unsigned int face_index);
    std::vector<std::vector<float>> getFace(const MeshGroup &group, unsigned int face_index);
};