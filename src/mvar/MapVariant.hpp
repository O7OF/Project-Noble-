#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace noble::mvar {

inline uint16_t ByteSwap16(uint16_t v) { return static_cast<uint16_t>((v << 8) | (v >> 8)); }
inline uint32_t ByteSwap32(uint32_t v) {
    return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v >> 8) & 0xFF00) | ((v >> 24) & 0xFF);
}

struct Author {
    uint64_t timestamp = 0;
    uint64_t xuid = 0;
    std::string name;
    bool onlineId = false;
};

struct MapVariantHeader {
    std::string signature;
    uint32_t chunkSize = 0;
    uint16_t chunkVersion = 0;
    uint16_t chunkFlags = 0;
    int32_t contentType = -1;
    uint32_t fileLength = 0;
    uint64_t unk08 = 0, unk10 = 0, unk18 = 0, unk20 = 0;
    int32_t activity = -1;
    int32_t gameMode = -1;
    int32_t engine = -1;
    uint32_t mapId = 0;
    uint32_t engineCategoryIndex = 0;
    size_t headerBits = 0;      // where the author blocks begin
    size_t authorsEndBits = 0;  // where the title/description begin
    size_t titleEndBits = 0;
    size_t secondMapIdBits = 0;
    size_t labelsBits = 0;
    size_t objectsBits = 0;
    std::wstring title, description;
    bool titleEndianBig = false;
    uint32_t unk02B2 = 0;
    uint32_t secondMapId = 0;
    uint32_t labelCount = 0;
    float boundsMin[3] = {};
    float boundsMax[3] = {};
    Author createdBy, modifiedBy;
};

struct MapVariant {
    bool valid = false;
    std::string error;
    MapVariantHeader header;
};

// Decodes the "mvar" chunk of an MCC map variant: header, authors, and (next) the forge labels and object list.
MapVariant ReadMapVariant(const std::filesystem::path& path);
std::string Describe(const MapVariant& variant);

} // namespace noble::mvar
