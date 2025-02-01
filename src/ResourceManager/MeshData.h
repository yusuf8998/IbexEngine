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
#include <string.h>

class MeshData;

constexpr unsigned int POSITION_OFFSET = 0;
constexpr unsigned int UV_OFFSET = 1;
constexpr unsigned int NORMAL_OFFSET = 2;
constexpr unsigned int TANGENT_OFFSET = 3;

constexpr unsigned int INDEX_PER_VERTEX = 4;

constexpr const char *ATTRIB_NAME[INDEX_PER_VERTEX] = {
    "position",
    "uv",
    "normal",
    "tangent"
};

constexpr unsigned int ATTRIB_STRIDE[INDEX_PER_VERTEX] = {
    3,
    2,
    3,
    3
};

constexpr bool const_strcmp(const char *a, const char *b)
{
    for (;*a || *b;){
        if (*a++!=*b++)
            return false;
    }
    return true;
}

constexpr unsigned int FindAttribIndex(const char *name)
{
    for (unsigned int i = 0; i < INDEX_PER_VERTEX; i++)
    {
        if (const_strcmp(name, ATTRIB_NAME[i]))
            return i;
    }
    return -1;
}

// Just keep in mind
typedef std::array<unsigned int, INDEX_PER_VERTEX> VertexIndex; // Position, UV, Normal, Tangent

// typedef std::array<std::vector<float>, INDEX_PER_VERTEX> Vertex; // Position, UV, Normal, Tangent

struct VertexAttrib
{
public:
    std::string name;
    std::vector<float> values;

    inline unsigned int getStride() const
    {
        return ATTRIB_STRIDE[FindAttribIndex(name.c_str())];
    }

    inline void push(const glm::vec4 &v)
    {
        assert(getStride() == 4);
        values.push_back(v.x);
        values.push_back(v.y);
        values.push_back(v.z);
        values.push_back(v.w);
    }
    inline void push(const glm::vec3 &v)
    {
        assert(getStride() == 3);
        values.push_back(v.x);
        values.push_back(v.y);
        values.push_back(v.z);
    }
    inline void push(const glm::vec2 &v)
    {
        assert(getStride() == 2);
        values.push_back(v.x);
        values.push_back(v.y);
    }

    inline glm::vec4 getVector4(unsigned int index)
    {
        assert(getStride() == 4);
        return glm::vec4(values[index * 4 + 0], values[index * 4 + 1], values[index * 4 + 2], values[index * 4 + 3]);
    }
    inline glm::vec3 getVector3(unsigned int index)
    {
        assert(getStride() == 3);
        return glm::vec3(values[index * 3 + 0], values[index * 3 + 1], values[index * 3 + 2]);
    }
    inline glm::vec2 getVector2(unsigned int index)
    {
        assert(getStride() == 2);
        return glm::vec2(values[index * 2 + 0], values[index * 2 + 1]);
    }
};

class MeshGroup
{
public:
    std::string name;
    std::vector<VertexIndex> indices;
    std::shared_ptr<Material> material;

    short vertexPerFace = 0;

    std::vector<std::string> getUsedTextures() const;

    friend class MeshData;

private:
    [[deprecated("Combine 'em yourself")]]
    static MeshGroup CombineGroups(const MeshGroup &a, const MeshGroup &b, unsigned int positionOffset, unsigned int uvOffset, unsigned int normalOffset, unsigned int tangentOffset);
    bool canCombine(const MeshGroup &other) const;
    void offsetIndices(unsigned int positionOffset, unsigned int uvOffset, unsigned int normalOffset, unsigned int tangentOffset);
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

    void applyTransformation(const glm::mat4 &transformation);
    void applyTransformationToIndices(const glm::mat4 &transformation, const std::vector<unsigned int> &posIndices, const std::vector<unsigned int> &normalIndices, const std::vector<unsigned int> &tangentIndices);

    std::string getMaterialName(const MeshGroup &group) const;

    void exportObject(const std::string &filepath) const;

    std::array<VertexAttrib, INDEX_PER_VERTEX> &getAttribs();

    [[deprecated("Combine 'em yourself")]]
    static std::shared_ptr<MeshData> CombineMeshes(const MeshData &a, const glm::mat4 &a_tr, const MeshData &b, const glm::mat4 &b_tr);
    [[deprecated("Combine 'em yourself")]]
    static std::shared_ptr<MeshData> CombineMeshes(const std::vector<std::shared_ptr<MeshData>> &meshes, const std::vector<glm::mat4> &transforms);
    [[deprecated("Combine 'em yourself")]]
    static std::shared_ptr<MeshData> CombineMeshes(const std::vector<MeshData> &meshes, const std::vector<glm::mat4> &transforms);
    [[deprecated("Combine 'em yourself")]]
    static void FlattenGroups(std::vector<MeshGroup> &groups);

    friend class RenderObject;

private:
    // std::map<std::string, std::vector<float>> vertexAttributes; // Vertex attributes: position, uv, normal, tangent
    std::array<VertexAttrib, INDEX_PER_VERTEX> vertexAttributes;
    std::vector<MeshGroup> groups;

    void initializeVertexAttributes();

    void parseOBJLine(const std::string &line);

    void generateGroup(const std::string &name);
    void useMaterial(const std::string &materialName, MeshGroup &group);

    bool compareAttributes(std::vector<float>::const_iterator &it, std::vector<float>::const_iterator &jt, unsigned int stride);
    bool compareAttributes(std::vector<float>::iterator &it, std::vector<float>::iterator &jt, unsigned int stride);
    void removeDuplicateAttribute(unsigned int index, std::map<unsigned int, unsigned int> &map);

    void normalizeNormals();
    void normalizeTangents();

    void calcTangentBitangentForMesh();
    void calcTangentBitangentForGroup(const std::string &groupName);
    void calcTangentBitangentForGroup(MeshGroup &group);

    static void CalcTangentBitangentForTri(const std::array<glm::vec3, 3> &positions, const std::array<glm::vec2, 3> &uvs, const std::array<glm::vec3, 3> &normals, glm::vec3 &tangent, glm::vec3 &bitangent);

    std::vector<std::vector<float>> getFace(const std::string &groupName, unsigned int face_index);
    std::vector<std::vector<float>> getFace(const MeshGroup &group, unsigned int face_index);
};