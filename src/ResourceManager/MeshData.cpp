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

bool MeshData::loadFromOBJ(const std::string &filepath, bool calculate_tangents)
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
    initializeVertexAttributes();
    while (std::getline(file, line))
    {
        parseOBJLine(line);
    }
    file.close();
    if (calculate_tangents)
        calcTangentBitangentForMesh();
    return true;
}
bool MeshData::loadFromSource(const std::string &source, bool calculate_tangents)
{
    std::stringstream stream(source);
    std::string line;
    currentGroupName = "Unnamed";
    queuedMaterialLibrary = "";
    initializeVertexAttributes();
    while (std::getline(stream, line))
    {
        parseOBJLine(line);
    }
    if (calculate_tangents)
        calcTangentBitangentForMesh();
    return true;
}
#include "algorithm"
#include <glm/gtx/string_cast.hpp>
void MeshData::useMaterial(const std::string &materialName, MeshGroup &group)
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
        useMaterial(queuedMaterialLibrary, groups.back());
    }
}
void MeshData::initializeVertexAttributes()
{
    for (unsigned int i = 0; i < INDEX_PER_VERTEX; i++)
    {
        vertexAttributes[i] = VertexAttrib{ATTRIB_NAME[i], {}};
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
        vertexAttributes[POSITION_OFFSET].push(position);
    }
    else if (tokens[0] == "vn")
    { // Vertex normal
        if (tokens.size() != 4)
            throw std::runtime_error("Token size for normal definition is not correct");
        glm::vec3 normal(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
        vertexAttributes[NORMAL_OFFSET].push(normal);
    }
    else if (tokens[0] == "vt")
    { // Vertex texture coordinate
        if (tokens.size() != 3 && tokens.size() != 4)
            throw std::runtime_error("Token size for UV definition is not correct");
        glm::vec2 uv(std::stof(tokens[1]), std::stof(tokens[2]));
        vertexAttributes[UV_OFFSET].push(uv);
    }
    else if (tokens[0] == "f")
    { // Face (index list)
        if (!hasGroup(currentGroupName))
            generateGroup(currentGroupName);
        auto &g = getGroup(currentGroupName);
        if (g.vertexPerFace == 0)
            g.vertexPerFace = tokens.size() - 1;
        if (g.vertexPerFace == 2 || g.vertexPerFace == 1)
            throw std::runtime_error("Token size for face definition is not correct. Use l for line and p for point. vertexPerFace: " + std::to_string(g.vertexPerFace));
        if (tokens.size() != getGroup(currentGroupName).vertexPerFace + 1)
            throw std::runtime_error("Token size for face definition is not correct. vertexPerFace: " + std::to_string(g.vertexPerFace));
        for (size_t i = 1; i < tokens.size(); ++i)
        {
            std::vector<std::string> vertexData = splitString(tokens[i], '/');
            unsigned int posIdx, uvIdx = 0, normalIdx = 0;
            posIdx = std::stoi(vertexData[0]) - 1;
            if (vertexData.size() > 1)
                uvIdx = vertexData[1].empty() ? 0 : std::stoi(vertexData[1]) - 1;
            if (vertexData.size() > 2)
                normalIdx = vertexData[2].empty() ? 0 : std::stoi(vertexData[2]) - 1;
            g.indices.push_back({posIdx, uvIdx, normalIdx, 0});
        }
    }
    else if (tokens[0] == "l")
    { // Line (index list)
        if (!hasGroup(currentGroupName))
            generateGroup(currentGroupName);
        auto &g = getGroup(currentGroupName);
        if (g.vertexPerFace == 0)
            g.vertexPerFace = tokens.size() - 1;
        if (g.vertexPerFace == 3 || g.vertexPerFace == 1)
            throw std::runtime_error("Token size for line definition is not correct. Use f for face and p for point. vertexPerFace: " + std::to_string(g.vertexPerFace));
        if (tokens.size() != getGroup(currentGroupName).vertexPerFace + 1)
            throw std::runtime_error("Token size for line definition is not correct. vertexPerFace: " + std::to_string(g.vertexPerFace));
        for (size_t i = 1; i < tokens.size(); ++i)
        {
            std::vector<std::string> vertexData = splitString(tokens[i], '/');
            unsigned int posIdx, uvIdx = 0, normalIdx = 0;
            posIdx = std::stoi(vertexData[0]) - 1;
            if (vertexData.size() > 1)
                uvIdx = vertexData[1].empty() ? 0 : std::stoi(vertexData[1]) - 1;
            if (vertexData.size() > 2)
                normalIdx = vertexData[2].empty() ? 0 : std::stoi(vertexData[2]) - 1;
            g.indices.push_back({posIdx, uvIdx, normalIdx, 0});
        }
    }
    else if (tokens[0] == "p")
    { // Point (index list)
        if (!hasGroup(currentGroupName))
            generateGroup(currentGroupName);
        auto &g = getGroup(currentGroupName);
        if (g.vertexPerFace == 0)
            g.vertexPerFace = tokens.size() - 1;
        if (g.vertexPerFace == 3 || g.vertexPerFace == 2)
            throw std::runtime_error("Token size for point definition is not correct. Use f for face and l for line. vertexPerFace: " + std::to_string(g.vertexPerFace));
        if (tokens.size() != getGroup(currentGroupName).vertexPerFace + 1)
            throw std::runtime_error("Token size for point definition is not correct. vertexPerFace: " + std::to_string(g.vertexPerFace));
        for (size_t i = 1; i < tokens.size(); ++i)
        {
            std::vector<std::string> vertexData = splitString(tokens[i], '/');
            unsigned int posIdx, uvIdx = 0, normalIdx = 0;
            posIdx = std::stoi(vertexData[0]) - 1;
            if (vertexData.size() > 1)
                uvIdx = vertexData[1].empty() ? 0 : std::stoi(vertexData[1]) - 1;
            if (vertexData.size() > 2)
                normalIdx = vertexData[2].empty() ? 0 : std::stoi(vertexData[2]) - 1;
            g.indices.push_back({posIdx, uvIdx, normalIdx, 0});
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
            useMaterial(tokens[1], getGroup(currentGroupName));
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
        vertexAttributes[TANGENT_OFFSET].push(glm::vec3(0.f));
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

        vertexAttributes[TANGENT_OFFSET].push(tangent);

        group.indices[i * group.vertexPerFace + 0][TANGENT_OFFSET] = vertexAttributes[TANGENT_OFFSET].values.size() / 3 - 1;
        group.indices[i * group.vertexPerFace + 1][TANGENT_OFFSET] = vertexAttributes[TANGENT_OFFSET].values.size() / 3 - 1;
        group.indices[i * group.vertexPerFace + 2][TANGENT_OFFSET] = vertexAttributes[TANGENT_OFFSET].values.size() / 3 - 1;
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

void MeshData::removeDuplicateAttribute(unsigned int index, std::map<unsigned int, unsigned int> &map)
{
    auto stride = vertexAttributes[index].getStride();
    for (auto it = vertexAttributes[index].values.end() - stride; it != vertexAttributes[index].values.begin(); it -= stride)
    {
        for (auto jt = vertexAttributes[index].values.begin(); jt != it; jt += stride)
        {
            if (compareAttributes(it, jt, stride))
            {
                map[(it - vertexAttributes[index].values.begin()) / stride] = (jt - vertexAttributes[index].values.begin()) / stride;
                vertexAttributes[index].values.erase(it, it + stride);
                break;
            }
        }
    }
}

void MeshData::normalizeNormals()
{
    for (size_t i = 0; i < vertexAttributes[NORMAL_OFFSET].values.size() / 3; i++)
    {
        glm::vec3 normal(vertexAttributes[NORMAL_OFFSET].values[i * 3 + 0], vertexAttributes[NORMAL_OFFSET].values[i * 3 + 1], vertexAttributes[NORMAL_OFFSET].values[i * 3 + 2]);
        normal = glm::normalize(normal);
        vertexAttributes[NORMAL_OFFSET].values[i * 3 + 0] = normal.x;
        vertexAttributes[NORMAL_OFFSET].values[i * 3 + 1] = normal.y;
        vertexAttributes[NORMAL_OFFSET].values[i * 3 + 2] = normal.z;
    }
}

void MeshData::normalizeTangents()
{
    for (size_t i = 0; i < vertexAttributes[TANGENT_OFFSET].values.size() / 3; i++)
    {
        glm::vec3 tangent(vertexAttributes[TANGENT_OFFSET].values[i * 3 + 0], vertexAttributes[TANGENT_OFFSET].values[i * 3 + 1], vertexAttributes[TANGENT_OFFSET].values[i * 3 + 2]);
        tangent = glm::normalize(tangent);
        vertexAttributes[TANGENT_OFFSET].values[i * 3 + 0] = tangent.x;
        vertexAttributes[TANGENT_OFFSET].values[i * 3 + 1] = tangent.y;
        vertexAttributes[TANGENT_OFFSET].values[i * 3 + 2] = tangent.z;
    }
}

void MeshData::removeDuplicateAttributes()
{
    std::vector<std::map<unsigned int, unsigned int>> maps(INDEX_PER_VERTEX);
    for (unsigned int i = 0; i < INDEX_PER_VERTEX; i++)
        removeDuplicateAttribute(i, maps[i]);

    for (auto &group : groups)
    {
        for (size_t i = 0; i < group.indices.size(); i++)
        {
            for (unsigned int j = 0; j < INDEX_PER_VERTEX; j++)
            {
                auto idx = group.indices[i][j];
                if (maps[j].count(idx) > 0)
                    group.indices[i][j] = maps[j][idx];
            }
        }
    }

    for (auto &attrib : vertexAttributes)
        attrib.values.shrink_to_fit();
}

void MeshData::applyTransformation(const glm::mat4 &transformation)
{
    std::vector<unsigned int> posIndices, normalIndices, tangentIndices;
    for (unsigned int i = 0; i < getPositionOffset(); i++)
        posIndices.push_back(i);
    for (unsigned int i = 0; i < getNormalOffset(); i++)
        normalIndices.push_back(i);
    for (unsigned int i = 0; i < getTangentOffset(); i++)
        tangentIndices.push_back(i);
    applyTransformationToIndices(transformation, posIndices, normalIndices, tangentIndices);
}

void MeshData::applyTransformationToIndices(const glm::mat4 &transformation, const std::vector<unsigned int> &posIndices, const std::vector<unsigned int> &normalIndices, const std::vector<unsigned int> &tangentIndices)
{
    if (transformation == glm::mat4(1.f))
        return;
    for (unsigned int i = 0; i < posIndices.size(); i++)
    {
        auto index = posIndices[i];
        glm::vec4 position = glm::vec4(vertexAttributes[POSITION_OFFSET].getVector3(index), 1.f);
        position = transformation * position;
        vertexAttributes[POSITION_OFFSET].values[index * 3 + 0] = position.x;
        vertexAttributes[POSITION_OFFSET].values[index * 3 + 1] = position.y;
        vertexAttributes[POSITION_OFFSET].values[index * 3 + 2] = position.z;
    }
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transformation)));
    if (normalMatrix == glm::mat4(1.f))
        return;
    for (unsigned int i = 0; i < normalIndices.size(); i++)
    {
        auto index = normalIndices[i];
        glm::vec4 normal = glm::vec4(vertexAttributes[NORMAL_OFFSET].getVector3(index), 1.f);
        normal = glm::normalize(normalMatrix * normal);
        vertexAttributes[NORMAL_OFFSET].values[index * 3 + 0] = normal.x;
        vertexAttributes[NORMAL_OFFSET].values[index * 3 + 1] = normal.y;
        vertexAttributes[NORMAL_OFFSET].values[index * 3 + 2] = normal.z;
    }
    for (unsigned int i = 0; i < tangentIndices.size(); i++)
    {
        auto index = tangentIndices[i];
        glm::vec4 tangent = glm::vec4(vertexAttributes[TANGENT_OFFSET].getVector3(index), 1.0f);
        tangent = glm::normalize(normalMatrix * tangent);
        vertexAttributes[TANGENT_OFFSET].values[index * 3 + 0] = tangent.x;
        vertexAttributes[TANGENT_OFFSET].values[index * 3 + 1] = tangent.y;
        vertexAttributes[TANGENT_OFFSET].values[index * 3 + 2] = tangent.z;
    }
}

std::string MeshData::getMaterialName(const MeshGroup &group) const
{
    for (const auto &kvp : materials)
    {
        for (const auto &mat : kvp.second)
        {
            if (group.material == materialLibraries.at(kvp.first)->getMaterial(mat))
                return mat;
        }
    }
    throw std::runtime_error("Can't the material associated with given group!");
}

void MeshData::exportObject(const std::string &filepath) const
{
    std::ofstream outFile(filepath);
    if (!outFile.is_open())
    {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return;
    }

    // Write object name
    outFile << "o " << objectName << "\n";

    for (const auto &kvp : materialLibraries)
    {
        outFile << "mtllib " << kvp.first << "\n";
    }

    // Write vertex positions
    const auto &positions = vertexAttributes.at(POSITION_OFFSET);
    for (size_t i = 0; i < positions.values.size(); i += 3)
    {
        outFile << "v " << positions.values[i] << " " << positions.values[i + 1] << " " << positions.values[i + 2] << "\n";
    }

    // Write vertex texture coordinates
    const auto &uvs = vertexAttributes.at(UV_OFFSET);
    for (size_t i = 0; i < uvs.values.size(); i += 2)
    {
        outFile << "vt " << uvs.values[i] << " " << uvs.values[i + 1] << "\n";
    }

    // Write vertex normals
    const auto &normals = vertexAttributes.at(NORMAL_OFFSET);
    for (size_t i = 0; i < normals.values.size(); i += 3)
    {
        outFile << "vn " << normals.values[i] << " " << normals.values[i + 1] << " " << normals.values[i + 2] << "\n";
    }

    // Write faces
    for (const auto &group : groups)
    {
        outFile << "g " << group.name << "\n";
        outFile << "usemtl " << getMaterialName(group) << '\n';
        for (size_t i = 0; i < group.indices.size(); i += group.vertexPerFace)
        {
            outFile << "f";
            for (size_t j = 0; j < group.vertexPerFace; ++j)
            {
                unsigned int posIdx = group.indices[i + j][POSITION_OFFSET] + 1;
                unsigned int uvIdx = group.indices[i + j][UV_OFFSET] + 1;
                unsigned int normalIdx = group.indices[i + j][NORMAL_OFFSET] + 1;
                outFile << " " << posIdx << "/" << uvIdx << "/" << normalIdx;
            }
            outFile << "\n";
        }
    }

    outFile.close();
}

std::array<VertexAttrib, INDEX_PER_VERTEX> &MeshData::getAttribs()
{
    return vertexAttributes;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

std::shared_ptr<MeshData> MeshData::CombineMeshes(const MeshData &a, const glm::mat4 &a_tr, const MeshData &b, const glm::mat4 &b_tr)
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
    FlattenGroups(aGroups);
    FlattenGroups(bGroups);

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
        if (combinedGroups.count(bGroup.material) != 0)
            continue;
        bGroup.offsetIndices(result->getPositionOffset(), result->getUVOffset(), result->getNormalOffset(), result->getTangentOffset());
        uniqueGroups.push_back(bGroup);
    }

    // Set offsets for transformations
    unsigned int positionOffset = result->getPositionOffset();
    unsigned int normalOffset = result->getNormalOffset();
    unsigned int tangentOffset = result->getTangentOffset();

    // Set indices for transformation a
    std::vector<unsigned int> posIndices, normalIndices, tangentIndices;
    for (unsigned int i = 0; i < positionOffset; i++)
        posIndices.push_back(i);
    for (unsigned int i = 0; i < normalOffset; i++)
        normalIndices.push_back(i);
    for (unsigned int i = 0; i < tangentOffset; i++)
        tangentIndices.push_back(i);

    // Apply transformations a to vertices
    result->applyTransformationToIndices(a_tr, posIndices, normalIndices, tangentIndices);

    // Add vertex attributes from mesh b
    for (unsigned int i = 0; i < INDEX_PER_VERTEX; i++)
    {
        std::copy(b.vertexAttributes[i].values.begin(), b.vertexAttributes[i].values.end(), std::back_inserter(result->vertexAttributes[i].values));
    }

    // Update offsets and indices for transformation b
    posIndices.clear();
    normalIndices.clear();
    tangentIndices.clear();
    for (unsigned int i = positionOffset; i < result->getPositionOffset(); i++)
        posIndices.push_back(i);
    for (unsigned int i = normalOffset; i < result->getNormalOffset(); i++)
        normalIndices.push_back(i);
    for (unsigned int i = tangentOffset; i < result->getTangentOffset(); i++)
        tangentIndices.push_back(i);

    // Apply transformations b to vertices
    result->applyTransformationToIndices(b_tr, posIndices, normalIndices, tangentIndices);

    // Copy combined groups to result
    result->groups = std::vector<MeshGroup>();
    std::transform(combinedGroups.begin(), combinedGroups.end(), result->groups.begin(), [](const std::pair<std::shared_ptr<Material>, MeshGroup> &kvp)
                   { return kvp.second; });
    std::copy(uniqueGroups.begin(), uniqueGroups.end(), std::back_inserter(result->groups));

    // Recalculate tangents
    // result->vertexAttributes["tangent"].clear();
    // result->calcTangentBitangentForMesh();

    return result;
}

std::shared_ptr<MeshData> MeshData::CombineMeshes(const std::vector<std::shared_ptr<MeshData>> &meshes, const std::vector<glm::mat4> &transforms)
{
    if (meshes.size() != transforms.size())
        throw std::runtime_error("Meshes and transforms size mismatch");
    std::shared_ptr<MeshData> result = std::make_shared<MeshData>(*meshes[0]);
    result->applyTransformation(transforms[0]);
    for (size_t i = 1; i < meshes.size(); i++)
    {
        result = CombineMeshes(*result, glm::mat4(1.0f), *meshes[i], transforms[i]);
    }
    return result;
}
std::shared_ptr<MeshData> MeshData::CombineMeshes(const std::vector<MeshData> &meshes, const std::vector<glm::mat4> &transforms)
{
    if (meshes.size() != transforms.size())
        throw std::runtime_error("Meshes and transforms size mismatch");
    std::shared_ptr<MeshData> result = std::make_shared<MeshData>(meshes[0]);
    result->applyTransformation(transforms[0]);
    for (size_t i = 1; i < meshes.size(); i++)
    {
        result = CombineMeshes(*result, glm::mat4(1.0f), meshes[i], transforms[i]);
    }
    return result;
}

void MeshData::FlattenGroups(std::vector<MeshGroup> &groups)
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

#pragma GCC diagnostic pop

void MeshData::CalcTangentBitangentForTri(const std::array<glm::vec3, 3> &positions, const std::array<glm::vec2, 3> &uvs, const std::array<glm::vec3, 3> &normals, glm::vec3 &tangent, glm::vec3 &bitangent)
{
    glm::vec3 face_normal = normals[0];
    if (normals[0] != normals[1] || normals[1] != normals[2])
        face_normal = (normals[0] + normals[1] + normals[2]) / 3.0f;

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
    size_t startIndex = face_index * group.vertexPerFace;
    for (size_t i = 0; i < group.vertexPerFace; i++)
    {
        std::vector<float> vertex;
        for (size_t j = 0; j < INDEX_PER_VERTEX; j++)
        {
            vertex.push_back(group.indices[startIndex + i][j]);
        }
        face.push_back(vertex);
    }
    return face;
}

const std::vector<float> &MeshData::getVertexAttribute(const std::string &name) const
{
    auto index = FindAttribIndex(name.c_str());
    if (index != -1)
        return vertexAttributes[index].values;
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
    for (const auto &attrib : vertexAttributes)
        stride += attrib.getStride() * sizeof(float);
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
    return group.indices.size() / (group.vertexPerFace);
}

MeshGroup &MeshData::addGroup(const std::string &groupName)
{
    groups.push_back(MeshGroup());
    groups[groups.size() - 1].name = groupName;
    return getGroup(groupName);
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

    std::vector<VertexIndex> indices;

    for (size_t i = 0; i < a.indices.size(); i++)
    {
        indices.push_back(a.indices[i]);
    }

    for (size_t i = 0; i < b.indices.size(); i++)
    {
        indices.push_back({b.indices[i][POSITION_OFFSET] + positionOffset,
                           b.indices[i][UV_OFFSET] + uvOffset,
                           b.indices[i][NORMAL_OFFSET] + normalOffset,
                           b.indices[i][TANGENT_OFFSET] + tangentOffset});
    }

    return MeshGroup{a.name + "+" + b.name, indices, a.material, a.vertexPerFace};
}

bool MeshGroup::canCombine(const MeshGroup &other) const
{
    return vertexPerFace == other.vertexPerFace && material == other.material;
}

void MeshGroup::offsetIndices(unsigned int positionOffset, unsigned int uvOffset, unsigned int normalOffset, unsigned int tangentOffset)
{
    for (size_t i = 0; i < indices.size(); i++)
    {
        indices[i][POSITION_OFFSET] += positionOffset;
        indices[i][UV_OFFSET] += uvOffset;
        indices[i][NORMAL_OFFSET] += normalOffset;
        indices[i][TANGENT_OFFSET] += tangentOffset;
    }
}