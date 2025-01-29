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
            glm::vec3(positions[face.at(2).at(POSITION_OFFSET) * 3 + 0], positions[face.at(2).at(POSITION_OFFSET) * 3 + 1], positions[face.at(2).at(POSITION_OFFSET) * 3 + 2])};
        std::array<glm::vec2, 3> face_uvs = {
            glm::vec2(uvs[face.at(0).at(UV_OFFSET) * 2 + 0], uvs[face.at(0).at(UV_OFFSET) * 2 + 1]),
            glm::vec2(uvs[face.at(1).at(UV_OFFSET) * 2 + 0], uvs[face.at(1).at(UV_OFFSET) * 2 + 1]),
            glm::vec2(uvs[face.at(2).at(UV_OFFSET) * 2 + 0], uvs[face.at(2).at(UV_OFFSET) * 2 + 1])};
        std::array<glm::vec3, 3> face_normals = {
            glm::vec3(normals[face.at(0).at(NORMAL_OFFSET) * 3 + 0], normals[face.at(0).at(NORMAL_OFFSET) * 3 + 1], normals[face.at(0).at(NORMAL_OFFSET) * 3 + 2]),
            glm::vec3(normals[face.at(1).at(NORMAL_OFFSET) * 3 + 0], normals[face.at(1).at(NORMAL_OFFSET) * 3 + 1], normals[face.at(1).at(NORMAL_OFFSET) * 3 + 2]),
            glm::vec3(normals[face.at(2).at(NORMAL_OFFSET) * 3 + 0], normals[face.at(2).at(NORMAL_OFFSET) * 3 + 1], normals[face.at(2).at(NORMAL_OFFSET) * 3 + 2])};

        glm::vec3 tangent, bitangent;

        currentGroupName = group.name;
        CalcTangentBitangentForTri(face_positions, face_uvs, face_normals, tangent, bitangent);

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

unsigned int MeshData::getPositionOffset() const
{
    return getVertexAttribute("position").size() / 3;
}
unsigned int MeshData::getUVOffset() const
{
    return getVertexAttribute("uv").size() / 2;
}
unsigned int MeshData::getNormalOffset() const
{
    return getVertexAttribute("normal").size() / 3;
}
unsigned int MeshData::getTangentOffset() const
{
    return getVertexAttribute("tangent").size() / 3;
}

bool MeshData::compareAttributes(std::vector<float>::const_iterator &it, std::vector<float>::const_iterator &jt, unsigned int stride)
{
    for (unsigned int i = 0; i < stride; i++)
    {
        if (it[i] != jt[i])
            return false;
    }
    return true;
}

bool MeshData::compareAttributes(std::vector<float>::iterator &it, std::vector<float>::iterator &jt, unsigned int stride)
{
    for (unsigned int i = 0; i < stride; i++)
    {
        if (it[i] != jt[i])
            return false;
    }
    return true;
}

void MeshData::removeDuplicateAttribute(const std::string &name, unsigned int stride, std::map<unsigned int, unsigned int> &map)
{
    for (auto it = vertexAttributes[name].end() - stride; it != vertexAttributes[name].begin(); it -= stride)
    {
        for (auto jt = vertexAttributes[name].begin(); jt != it; jt += stride)
        {
            if (compareAttributes(it, jt, stride))
            {
                map[(it - vertexAttributes[name].begin()) / stride] = (jt - vertexAttributes[name].begin()) / stride;
                vertexAttributes[name].erase(it, it + stride);
                break;
            }
        }
    }
}

void MeshData::removeDuplicateAttributes()
{
    std::map<unsigned int, unsigned int> positionMap, uvMap, normalMap, tangentMap;
    removeDuplicateAttribute("position", 3, positionMap);
    removeDuplicateAttribute("uv", 2, uvMap);
    removeDuplicateAttribute("normal", 3, normalMap);
    removeDuplicateAttribute("tangent", 3, tangentMap);

    for (auto &group : groups)
    {
        for (size_t i = 0; i < group.indices.size(); i += INDEX_PER_VERTEX)
        {
            auto posIdx = group.indices[i + POSITION_OFFSET];
            auto uvIdx = group.indices[i + UV_OFFSET];
            auto normalIdx = group.indices[i + NORMAL_OFFSET];
            auto tangentIdx = group.indices[i + TANGENT_OFFSET];

            if (positionMap.count(posIdx) > 0)
                group.indices[i + POSITION_OFFSET] = positionMap[posIdx];
            if (uvMap.count(uvIdx) > 0)
                group.indices[i + UV_OFFSET] = uvMap[uvIdx];
            if (normalMap.count(normalIdx) > 0)
                group.indices[i + NORMAL_OFFSET] = normalMap[normalIdx];
            if (tangentMap.count(tangentIdx) > 0)
                group.indices[i + TANGENT_OFFSET] = tangentMap[tangentIdx];
        }   
    }

    vertexAttributes["position"].shrink_to_fit();
    vertexAttributes["uv"].shrink_to_fit();
    vertexAttributes["normal"].shrink_to_fit();
    vertexAttributes["tangent"].shrink_to_fit();
}

std::shared_ptr<MeshData> MeshData::CombineMeshes(const MeshData &a, const MeshData &b)
{
    std::shared_ptr<MeshData> result = std::make_shared<MeshData>();
    result->objectName = a.objectName + "+" + b.objectName;
    result->filepath = a.filepath + "+" + b.filepath;
    result->materials = a.materials;
    result->materialLibraries = a.materialLibraries;
    result->vertexAttributes = a.vertexAttributes;

    // Combine materials and material libraries
    for (const auto &kvp : b.materials)
    {
        if (result->materials.count(kvp.first) == 0)
            result->materials[kvp.first] = kvp.second;
        else
            std::copy(kvp.second.begin(), kvp.second.end(), std::inserter(result->materials[kvp.first], result->materials[kvp.first].end()));
    }
    for (auto &kvp : result->materials)
    {
        std::sort(kvp.second.begin(), kvp.second.end());
        kvp.second.erase(std::unique(kvp.second.begin(), kvp.second.end()), kvp.second.end());
    }
    for (const auto &kvp : b.materialLibraries)
        if (result->materialLibraries.count(kvp.first) == 0)
            result->materialLibraries[kvp.first] = kvp.second;

    std::vector<MeshGroup> aGroups = a.groups, bGroups = b.groups;
    std::map<std::shared_ptr<Material>, MeshGroup> combinedGroups;
    std::vector<MeshGroup> uniqueGroups;

    // Combine groups in meshes seperately
    FlattenGroupVector(aGroups);
    FlattenGroupVector(bGroups);
    
    // This doesn't work

    // Combine groups between meshes
    for (auto &aGroup : aGroups)
    {
        for (auto &bGroup : bGroups)
        {
            if (aGroup.canCombine(bGroup))
            {
                MeshGroup combinedGroup = MeshGroup::CombineGroups(aGroup, bGroup, result->getPositionOffset(), result->getUVOffset(), result->getNormalOffset(), result->getTangentOffset());

                combinedGroups[combinedGroup.material] = combinedGroup;
            }
        }
    }
    for (auto &aGroup : aGroups)
    {
        if (combinedGroups.count(aGroup.material) == 0)
            uniqueGroups.push_back(aGroup);
    }
    for (auto &bGroup : bGroups)
    {
        if (combinedGroups.count(bGroup.material) == 0)
            uniqueGroups.push_back(bGroup);
    }
    for (const auto &kvp : result->vertexAttributes)
    {
        std::copy(b.vertexAttributes.at(kvp.first).begin(), b.vertexAttributes.at(kvp.first).end(), std::inserter(result->vertexAttributes.at(kvp.first), result->vertexAttributes.at(kvp.first).end()));
    }
    
    // Copy combined groups to result
    result->groups = std::vector<MeshGroup>(combinedGroups.size());
    std::transform(combinedGroups.begin(), combinedGroups.end(), result->groups.begin(), [](const std::pair<std::shared_ptr<Material>, MeshGroup> &kvp) { return kvp.second; });
    std::copy(uniqueGroups.begin(), uniqueGroups.end(), std::back_inserter(result->groups));
    return result;
}

void MeshData::FlattenGroupVector(std::vector<MeshGroup> &groups)
{
    for (auto it = groups.begin(); it != groups.end(); ++it)
    {
        for (auto jt = it + 1; jt != groups.end();)
        {
            if (it->canCombine(*jt))
            {
                *it = MeshGroup::CombineGroups(*it, *jt, 0, 0, 0, 0);
                jt = groups.erase(jt);
            }
            else
            {
                ++jt;
            }
        }
    }
}

void MeshData::CalcTangentBitangentForTri(const std::array<glm::vec3, 3> &positions, const std::array<glm::vec2, 3> &uvs, const std::array<glm::vec3, 3> &normals, glm::vec3 &tangent, glm::vec3 &bitangent)
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

std::vector<std::string> MeshGroup::getUsedTextures() const
{
    std::vector<std::string> textures;
    if (material != nullptr)
    {
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
    return textures;
}

MeshGroup MeshGroup::CombineGroups(const MeshGroup &a, const MeshGroup &b, unsigned int positionOffset, unsigned int uvOffset, unsigned int normalOffset, unsigned int tangentOffset)
{
    if (a.vertexPerFace != b.vertexPerFace)
        throw std::runtime_error("Vertex per face mismatch");
    if (a.material != nullptr && b.material != nullptr && a.material != b.material)
        throw std::runtime_error("Material mismatch");

    std::vector<unsigned int> indices;

    for (size_t i = 0; i < a.indices.size() / INDEX_PER_VERTEX; i++)
    {
        indices.push_back(a.indices[i * INDEX_PER_VERTEX + POSITION_OFFSET]);
        indices.push_back(a.indices[i * INDEX_PER_VERTEX + UV_OFFSET]);
        indices.push_back(a.indices[i * INDEX_PER_VERTEX + NORMAL_OFFSET]);
        indices.push_back(a.indices[i * INDEX_PER_VERTEX + TANGENT_OFFSET]);
    }

    for (size_t i = 0; i < b.indices.size() / INDEX_PER_VERTEX; i++)
    {
        indices.push_back(b.indices[i * INDEX_PER_VERTEX + POSITION_OFFSET] + positionOffset);
        indices.push_back(b.indices[i * INDEX_PER_VERTEX + UV_OFFSET] + uvOffset);
        indices.push_back(b.indices[i * INDEX_PER_VERTEX + NORMAL_OFFSET] + normalOffset);
        indices.push_back(b.indices[i * INDEX_PER_VERTEX + TANGENT_OFFSET] + tangentOffset);
    }

    return MeshGroup{a.name + "+" + b.name, indices, a.material, a.vertexPerFace};
}

bool MeshGroup::canCombine(const MeshGroup &other) const
{
    return vertexPerFace == other.vertexPerFace && material == other.material;
}