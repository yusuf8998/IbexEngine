#include "AssetPack.h"
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cassert>

#include <zlib.h>
#include <stb/stb_image.h>

size_t AssetPack::size() const
{
    return sizeof(version) + sizeof(asset_count) + asset_count * ASSET_IDENTIFIER_SIZE + std::accumulate(identifiers.begin(), identifiers.end(), 0, [](size_t sum, const AssetIdentifier &identifier)
                                                                                                         { return sum + identifier.size; });
}

void AssetPack::clear()
{
    identifiers.clear();
    for (auto &asset : assets)
    {
        delete[] asset;
    }
    assets.clear();
}

void AssetPack::addAsset(const std::string &name, const char *data, size_t size, AssetType type, char _metadata[32])
{
    assert(type != AssetType::MAX_TYPE);
    AssetIdentifier identifier;
    strncpy(identifier.name, name.c_str(), sizeof(identifier.name));
    if (identifiers.size() == 0)
        identifier.offset = 0;
    else
        identifier.offset = identifiers[identifiers.size() - 1].offset + identifiers[identifiers.size() - 1].size;
    identifier.size = size;
    identifier.type = type;
    memcpy(identifier.metadata, _metadata, sizeof(identifier.metadata));
    identifiers.push_back(identifier);

    char *asset = new char[size];
    memcpy(asset, data, size);
    assets.push_back(asset);
    asset_count++;
}

void AssetPack::removeAsset(const std::string &name)
{
    for (size_t i = 0; i < asset_count; i++)
    {
        if (identifiers[i].name == name)
        {
            delete[] assets[i];
            assets.erase(assets.begin() + i);
            identifiers.erase(identifiers.begin() + i);
            asset_count--;
            return;
        }
    }
}

std::pair<AssetIdentifier, char *> AssetPack::getAsset(const std::string &name) const
{
    for (size_t i = 0; i < asset_count; i++)
    {
        if (identifiers[i].name == name)
        {
            return {identifiers[i], assets[i]};
        }
    }
    return {AssetIdentifier(), nullptr};
}

char *AssetPack::serializeAssetPack() const
{
    char *buffer = new char[size()];
    char *ptr = buffer;
    memcpy(ptr, &version, sizeof(version));
    ptr += sizeof(version);
    memcpy(ptr, &asset_count, sizeof(asset_count));
    ptr += sizeof(asset_count);
    for (size_t i = 0; i < asset_count; i++)
    {
        const AssetIdentifier &id = identifiers[i];
        memcpy(ptr, &id.name, sizeof(id.name));
        ptr += sizeof(id.name);
        memcpy(ptr, &id.offset, sizeof(id.offset));
        ptr += sizeof(id.offset);
        memcpy(ptr, &id.size, sizeof(id.size));
        ptr += sizeof(id.size);
        memcpy(ptr, &id.type, sizeof(id.type));
        ptr += sizeof(id.type);
        memcpy(ptr, id.metadata, sizeof(id.metadata));
        ptr += sizeof(id.metadata);
    }
    for (size_t i = 0; i < asset_count; i++)
    {
        memcpy(ptr, assets[i], identifiers[i].size);
        ptr += identifiers[i].size;
    }
    return buffer;
}

void AssetPack::deserializeAssetPack(const char *buffer)
{
    const char *ptr = buffer;
    memcpy(&version, ptr, sizeof(version));
    ptr += sizeof(version);
    memcpy(&asset_count, ptr, sizeof(asset_count));
    ptr += sizeof(asset_count);
    for (size_t i = 0; i < asset_count; i++)
    {
        AssetIdentifier id;
        memcpy(&id.name, ptr, sizeof(id.name));
        ptr += sizeof(id.name);
        memcpy(&id.offset, ptr, sizeof(id.offset));
        ptr += sizeof(id.offset);
        memcpy(&id.size, ptr, sizeof(id.size));
        ptr += sizeof(id.size);
        memcpy(&id.type, ptr, sizeof(id.type));
        ptr += sizeof(id.type);
        memcpy(id.metadata, ptr, sizeof(id.metadata));
        ptr += sizeof(id.metadata);
        identifiers.push_back(id);
    }
    for (size_t i = 0; i < asset_count; i++)
    {
        char *asset = new char[identifiers[i].size];
        memcpy(asset, ptr, identifiers[i].size);
        ptr += identifiers[i].size;
        assets.push_back(asset);
    }
}

void addTextureToPack(const std::string &path, AssetPack &pack, const std::string &name)
{
    size_t texture_data_size = 0;
    char *texture_data = 0;
    {
        std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file: " + path);
        texture_data_size = file.tellg();
        file.seekg(0, std::ios::beg);

        texture_data = new char[texture_data_size];

        if (!file.read(texture_data, texture_data_size))
            throw std::runtime_error("Failed to read file: " + path);

        file.close();
    }

    char metadata[32];
    {
        int x = 0, y = 0, bpp = 0;
        auto *inprt_texture = stbi_load_from_memory((const stbi_uc *)texture_data, texture_data_size, &x, &y, &bpp, 4);
        stbi_image_free(inprt_texture);
        char *ptr = metadata;
        memcpy(ptr, &x, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &y, sizeof(int));
        ptr += sizeof(int);
        memcpy(ptr, &bpp, sizeof(int));
        ptr += sizeof(int);
    }

    pack.addAsset(name, texture_data, texture_data_size, AssetType::TEXTURE, metadata);
    delete[] texture_data;
}

char *compressPack(const AssetPack &pack, size_t &new_size)
{
    new_size = compressBound(pack.size());
    char *compressed = new char[new_size + sizeof(size_t)];
    char *ptr = compressed;
    size_t pack_size = pack.size();
    memcpy(ptr, &pack_size, sizeof(size_t));
    ptr += sizeof(size_t);
    int result = compress((Bytef *)ptr, (uLongf *)&new_size, (const Bytef *)pack.serializeAssetPack(), (uLong)pack.size());
    new_size += sizeof(size_t);

    if (result == Z_OK)
        printf("Compressed size: %lu\n", new_size);
    else if (result == Z_MEM_ERROR)
        printf("Compression failed, not enough memory\n");
    else if (result == Z_BUF_ERROR)
        printf("Compression failed, not enough room in output buffer\n");
    else if (result == Z_STREAM_ERROR)
        printf("Compression failed, level parameter invalid\n");
    else
        printf("I don't know man...\n");

    return compressed;
}

void uncompressPack(AssetPack &pack, const char *compressed, size_t compressed_size, size_t &new_size)
{
    const char *ptr = compressed;
    memcpy(&new_size, ptr, sizeof(size_t));
    ptr += sizeof(size_t);

    char *uncompressed = new char[new_size];

    int result = uncompress((Bytef *)uncompressed, (uLongf *)&new_size, (const Bytef *)ptr, (uLong)(compressed_size - sizeof(size_t)));

    if (result == Z_OK)
        printf("Uncompression successfull\n");
    else if (result == Z_MEM_ERROR)
        printf("Uncompression failed, not enough memory\n");
    else if (result == Z_BUF_ERROR)
        printf("Uncompression failed, not enough room in output buffer\n");
    else if (result == Z_DATA_ERROR)
        printf("Uncompression failed, data corrupted\n");
    else
        printf("I don't know man...\n");

    pack.deserializeAssetPack(uncompressed);
    delete[] uncompressed;
}

void loadAssetPack(const std::string &path, AssetPack &pack)
{
    size_t compressedPackSize = 0;
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + path);
    compressedPackSize = file.tellg();
    file.seekg(0, std::ios::beg);

    char *compressedPack = new char[compressedPackSize];

    if (!file.read(compressedPack, compressedPackSize))
        throw std::runtime_error("Failed to read file: " + path);

    file.close();
    size_t uncompressedPackSize = 0;
    uncompressPack(pack, compressedPack, compressedPackSize, uncompressedPackSize);
    delete[] compressedPack;
}

void saveAssetPack(const std::string &path, const AssetPack &pack)
{
    size_t compressedPackSize = 0;
    char *compressedPack = compressPack(pack, compressedPackSize);

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + path);

    file.write(compressedPack, compressedPackSize);

    file.close();
}
