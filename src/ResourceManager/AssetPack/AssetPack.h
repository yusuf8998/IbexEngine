#pragma once

#include <vector>
#include <string>
#include <utility>
#include <numeric>

enum class AssetType : unsigned char
{
    TEXTURE = 0,
    SHADER,
    MESH,
    MATERIAL,
    SCENE,
    FONT,
    AUDIO,
    SCRIPT,
    PARTICLE,
    CUBEMAP,
    MAX_TYPE
};

struct AssetIdentifier
{
    char name[32];
    size_t offset;
    size_t size;
    AssetType type;
    char metadata[32];
};
constexpr size_t ASSET_IDENTIFIER_SIZE = sizeof(AssetIdentifier::name) + sizeof(AssetIdentifier::offset) + sizeof(AssetIdentifier::size) + sizeof(AssetIdentifier::type) + sizeof(AssetIdentifier::metadata);

struct AssetPack
{
    int version;
    size_t asset_count;
    std::vector<AssetIdentifier> identifiers;
    std::vector<char *> assets;

    AssetPack() : version(2), asset_count(0), identifiers(), assets() {}
    ~AssetPack()
    {
        for (auto &asset : assets)
        {
            delete[] asset;
        }
    }

    size_t size() const;
    void clear();

    void addAsset(const std::string &name, const char *data, size_t size, AssetType type, char _metadata[32]);
    void removeAsset(const std::string &name);
    std::pair<AssetIdentifier, char *> getAsset(const std::string &name) const;

    char *serializeAssetPack() const;
    void deserializeAssetPack(const char *buffer);
};

void addTextureToPack(const std::string &path, AssetPack &pack, const std::string &name);

char *compressPack(const AssetPack &pack, size_t &new_size);
void uncompressPack(AssetPack &pack, const char *compressed, size_t compressed_size, size_t &new_size);

void loadAssetPack(const std::string &path, AssetPack &pack);
void saveAssetPack(const std::string &path, const AssetPack &pack);