#include "MeshData.h"
#include "ResourceManager.h"
#include <stb/stb_image.h>

std::vector<std::string> MeshData::getUsedTextures() const
{
    std::vector<std::string> textures;
    for (const auto &mat : materials)
    {
        auto it = materialLibraries.find(mat.first);
        if (it == materialLibraries.end())
            continue;

        for (const auto &mat_name : mat.second)
        {
            auto material = it->second->getMaterial(mat_name);
            if (!material->diffuseTexture.empty())
            {
                textures.push_back(material->diffuseTexture);
            }
            if (!material->specularTexture.empty())
            {
                textures.push_back(material->specularTexture);
            }
            if (!material->normalMap.empty())
            {
                textures.push_back(material->normalMap);
            }
        }
    }
    return textures;
}
std::string currentGroupName;
std::string queuedMaterialLibrary;

bool MeshData::loadFromOBJ(const std::string &filepath)
{
    this->filepath = filepath;
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return false;
    }

    std::string line;
    currentGroupName = "Unnamed";
    queuedMaterialLibrary = "";
    while (std::getline(file, line))
    {
        parseOBJLine(line);
    }
    file.close();
    calcTangentBitangentForMesh();
    return true;
}
#include "algorithm"
#include <glm/gtx/string_cast.hpp>
void MeshData::UseMaterial(const std::string &materialName, MeshGroup &group)
{
    for (auto it = materialLibraries.begin(); it != materialLibraries.end(); it++)
    {
        if (it->second->hasMaterial(materialName))
        {
            materials[it->first].push_back(materialName);
            group.material = std::shared_ptr<Material>(it->second->getMaterial(materialName));
            return;
        }
    }
}
void MeshData::generateGroup(const std::string &name)
{
    groups.push_back(MeshGroup{currentGroupName});
    if (queuedMaterialLibrary != "")
    {
        UseMaterial(queuedMaterialLibrary, groups.back());
    }
}
void MeshData::parseOBJLine(const std::string &line)
{
    if (line.empty())
        return;
    std::string uncomment = splitString(line, '#')[0];
    uncomment.erase(std::remove(uncomment.begin(), uncomment.end(), '\t'), uncomment.end());
    std::vector<std::string> tokens = splitString(uncomment, ' ');
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const std::string &s)
                                { return s.empty(); }),
                 tokens.end());
    if (tokens.empty())
        return;
    if (tokens[0] == "o")
    {
        if (tokens.size() != 2)
            throw std::runtime_error("Token size for object definition is not correct");
        objectName = tokens[1];
    }
    else if (tokens[0] == "g")
    {
        if (tokens.size() > 1)
            currentGroupName = tokens[1];
        generateGroup(currentGroupName);
    }
    else if (tokens[0] == "v")
    { // Vertex position
        if (tokens.size() != 4)
            throw std::runtime_error("Token size for position definition is not correct");
        glm::vec3 position(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
        vertexAttributes["position"].push_back(position.x);
        vertexAttributes["position"].push_back(position.y);
        vertexAttributes["position"].push_back(position.z);
    }
    else if (tokens[0] == "vn")
    { // Vertex normal
        if (tokens.size() != 4)
            throw std::runtime_error("Token size for normal definition is not correct");
        glm::vec3 normal(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
        vertexAttributes["normal"].push_back(normal.x);
        vertexAttributes["normal"].push_back(normal.y);
        vertexAttributes["normal"].push_back(normal.z);
    }
    else if (tokens[0] == "vt")
    { // Vertex texture coordinate
        if (tokens.size() != 3 && tokens.size() != 4)
            throw std::runtime_error("Token size for UV definition is not correct");
        glm::vec2 uv(std::stof(tokens[1]), std::stof(tokens[2]));
        vertexAttributes["uv"].push_back(uv.x);
        vertexAttributes["uv"].push_back(uv.y);
    }
    else if (tokens[0] == "f")
    { // Face (index list)
        if (!hasGroup(currentGroupName))
            generateGroup(currentGroupName);
        if (getGroup(currentGroupName).vertexPerFace == 0)
            getGroup(currentGroupName).vertexPerFace = tokens.size() - 1;
        if (tokens.size() != getGroup(currentGroupName).vertexPerFace + 1)
            throw std::runtime_error("Token size for face definition is not correct. vertexPerFace: " + std::to_string(getGroup(currentGroupName).vertexPerFace));
        for (size_t i = 1; i < tokens.size(); ++i)
        {
            std::vector<std::string> vertexData = splitString(tokens[i], '/');
            unsigned int posIdx = std::stoi(vertexData[0]) - 1;
            unsigned int uvIdx = vertexData[1].empty() ? 0 : std::stoi(vertexData[1]) - 1;
            unsigned int normalIdx = vertexData[2].empty() ? 0 : std::stoi(vertexData[2]) - 1;
            getGroup(currentGroupName).indices.push_back(posIdx);
            getGroup(currentGroupName).indices.push_back(uvIdx);
            getGroup(currentGroupName).indices.push_back(normalIdx);
            getGroup(currentGroupName).indices.push_back(0); // tangent
        }
    }
    else if (tokens[0] == "mtllib")
    { // Material Library reference
        if (tokens.size() != 2)
            throw std::runtime_error("Token size for mtllib is not correct");
        materialLibraries[tokens[1]] = ResourceManager::instance().loadResource<MaterialLibrary>(tokens[1]);
    }
    else if (tokens[0] == "usemtl")
    { // Material reference
        if (tokens.size() != 2)
            throw std::runtime_error("Token size for material usage is not correct");
        if (hasGroup(currentGroupName))
            UseMaterial(tokens[1], getGroup(currentGroupName));
        else
            queuedMaterialLibrary = tokens[1];
    }
}

void MeshData::calcTangentBitangentForMesh()
{
    for (auto &g : groups)
        calcTangentBitangentForGroup(g);
}

void MeshData::calcTangentBitangentForGroup(const std::string &groupName)
{
    calcTangentBitangentForGroup(getGroup(groupName));
}
void MeshData::calcTangentBitangentForGroup(MeshGroup &group)
{
    const std::vector<float> &positions = getVertexAttribute("position");
    const std::vector<float> &uvs = getVertexAttribute("uv");
    const std::vector<float> &normals = getVertexAttribute("normal");

    if (normals.size() == 3 && glm::vec3(normals[0], normals[1], normals[2]) == glm::vec3(0.f) || group.vertexPerFace == 4)
    {
        vertexAttributes["tangent"].push_back(0);
        vertexAttributes["tangent"].push_back(0);
        vertexAttributes["tangent"].push_back(0);
        return;
    }

    for (size_t i = 0; i < getFaceCount(group); i++)
    {
        auto face = getFace(group, i);

        std::array<glm::vec3, 3> face_positions = {
            glm::vec3(positions[face.at(0).at(POSITION_OFFSET) * 3 + 0], positions[face.at(0).at(POSITION_OFFSET) * 3 + 1], positions[face.at(0).at(POSITION_OFFSET) * 3 + 2]),
            glm::vec3(positions[face.at(1).at(POSITION_OFFSET) * 3 + 0], positions[face.at(1).at(POSITION_OFFSET) * 3 + 1], positions[face.at(1).at(POSITION_OFFSET) * 3 + 2]),
            glm::vec3(positions[face.at(2).at(POSITION_OFFSET) * 3 + 0], positions[face.at(2).at(POSITION_OFFSET) * 3 + 1], positions[face.at(2).at(POSITION_OFFSET) * 3 + 2])
        };
        std::array<glm::vec2, 3> face_uvs = {
            glm::vec2(uvs[face.at(0).at(UV_OFFSET) * 2 + 0], uvs[face.at(0).at(UV_OFFSET) * 2 + 1]),
            glm::vec2(uvs[face.at(1).at(UV_OFFSET) * 2 + 0], uvs[face.at(1).at(UV_OFFSET) * 2 + 1]),
            glm::vec2(uvs[face.at(2).at(UV_OFFSET) * 2 + 0], uvs[face.at(2).at(UV_OFFSET) * 2 + 1])
        };
        std::array<glm::vec3, 3> face_normals = {
            glm::vec3(normals[face.at(0).at(NORMAL_OFFSET) * 3 + 0], normals[face.at(0).at(NORMAL_OFFSET) * 3 + 1], normals[face.at(0).at(NORMAL_OFFSET) * 3 + 2]),
            glm::vec3(normals[face.at(1).at(NORMAL_OFFSET) * 3 + 0], normals[face.at(1).at(NORMAL_OFFSET) * 3 + 1], normals[face.at(1).at(NORMAL_OFFSET) * 3 + 2]),
            glm::vec3(normals[face.at(2).at(NORMAL_OFFSET) * 3 + 0], normals[face.at(2).at(NORMAL_OFFSET) * 3 + 1], normals[face.at(2).at(NORMAL_OFFSET) * 3 + 2])
        };

        glm::vec3 tangent, bitangent;

        currentGroupName = group.name;
        calcTangentBitangentForTri(face_positions, face_uvs, face_normals, tangent, bitangent);

        vertexAttributes["tangent"].push_back(tangent.x);
        vertexAttributes["tangent"].push_back(tangent.y);
        vertexAttributes["tangent"].push_back(tangent.z);
        
        group.indices[((i * group.vertexPerFace + 0) * INDEX_PER_VERTEX) + TANGENT_OFFSET] = vertexAttributes["tangent"].size() / 3 - 1;
        group.indices[((i * group.vertexPerFace + 1) * INDEX_PER_VERTEX) + TANGENT_OFFSET] = vertexAttributes["tangent"].size() / 3 - 1;
        group.indices[((i * group.vertexPerFace + 2) * INDEX_PER_VERTEX) + TANGENT_OFFSET] = vertexAttributes["tangent"].size() / 3 - 1;
    }
}

MeshGroup &MeshData::getGroup(const std::string &groupName)
{
    for (auto &g : groups)
    {
        if (g.name == groupName)
            return const_cast<MeshGroup &>(g);
    }
    throw std::runtime_error("Group not found: " + groupName);
}

const MeshGroup &MeshData::getGroup(const std::string &groupName) const
{
    for (const auto &g : groups)
    {
        if (g.name == groupName)
            return g;
    }
    throw std::runtime_error("Group not found: " + groupName);
}

bool MeshData::hasGroup(const std::string &groupName) const
{
    for (const auto &g : groups)
    {
        if (g.name == groupName)
            return true;
    }
    return false;
}

void MeshData::calcTangentBitangentForTri(const std::array<glm::vec3, 3> &positions, const std::array<glm::vec2, 3> &uvs, const std::array<glm::vec3, 3> &normals, glm::vec3 &tangent, glm::vec3 &bitangent)
{
    // glm::vec3 face_normal = normals[0];
    // if (normals[0] != normals[1] || normals[1] != normals[2])
    //     face_normal = (normals[0] + normals[1] + normals[2]) / 3.0f;
    
    glm::vec3 edge1 = positions[1] - positions[0];
    glm::vec3 edge2 = positions[2] - positions[0];
    glm::vec2 deltaUV1 = uvs[1] - uvs[0];
    glm::vec2 deltaUV2 = uvs[2] - uvs[0];

    float denominator = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
    if (denominator == 0.0f || !std::isfinite(1.0f / denominator))
        throw std::runtime_error("Denominator for tangent/bitangent calculation is zero or infinite");
    float f = 1.0f / denominator;

    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

    tangent = glm::normalize(tangent);
    bitangent = glm::normalize(bitangent);

    // if (glm::dot(tangent, bitangent) > 0.01f || glm::dot(tangent, face_normal) > 0.01f || glm::dot(bitangent, face_normal) > 0.01f)
    //     throw std::runtime_error("Tangent, bitangent, and normal are not orthogonal");
}

std::vector<std::vector<float>> MeshData::getFace(const std::string &groupName, unsigned int face_index)
{
    return getFace(getGroup(groupName), face_index);
}

std::vector<std::vector<float>> MeshData::getFace(const MeshGroup &group, unsigned int face_index)
{
    std::vector<std::vector<float>> face;
    size_t startIndex = face_index * group.vertexPerFace * INDEX_PER_VERTEX;
    for (size_t i = 0; i < group.vertexPerFace; i++)
    {
        std::vector<float> vertex;
        for (size_t j = 0; j < INDEX_PER_VERTEX; j++)
        {
            vertex.push_back(group.indices[startIndex + i * INDEX_PER_VERTEX + j]);
        }
        face.push_back(vertex);
    }
    return face;
}

const std::vector<float> &MeshData::getVertexAttribute(const std::string &name) const
{
    auto it = vertexAttributes.find(name);
    if (it != vertexAttributes.end())
        return it->second;
    else
        throw std::runtime_error("Attribute not found: " + name);
}

short MeshData::getVertexPerFace(const std::string &groupName) const
{
    return getVertexPerFace(getGroup(groupName));
}

short MeshData::getVertexPerFace(const MeshGroup &group) const
{
    return group.vertexPerFace;
}

size_t MeshData::getVertexStride() const
{
    size_t stride = 0;
    for (const auto &kvp : vertexAttributes)
        stride += kvp.second.size() * sizeof(float);
    return stride;
}

size_t MeshData::getFaceCount() const
{
    size_t result = 0;
    for (const auto &g : groups)
    {
        result += getFaceCount(g);
    }
    return result;
}

size_t MeshData::getFaceCount(const std::string &groupName) const
{
    return getFaceCount(getGroup(groupName));
}

size_t MeshData::getFaceCount(const MeshGroup &group) const
{
    return group.indices.size() / (INDEX_PER_VERTEX * group.vertexPerFace);
}
