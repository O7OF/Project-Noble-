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

// A placeable object quota: the Forge budget table that follows the 651 object slots.
struct VariantQuota {
    uint8_t minimum = 0;
    uint8_t maximum = 0;
    uint8_t placedOnMap = 0;
};

// One of the 651 Forge object slots. Every field is kept at the width it is read, and quantized values are kept raw
// (not as floats), so re-encoding is bit-exact: the writer never has to round a float back onto the wire.
//
// The MCC record is not the Xbox 360 one ReachVariantTool documents. It is:
//   presence 1
//   flags 2, quota index {1+8}, variant index {1+5}      (the extra bit is read_index's "index invalid" flag)
//   position: in-bounds 1 + per-axis quantized fields
//   axes: up-is-global 1 {+20}, forward angle 14
//   spawn-relative-to 10
//   multiplayer properties: boundary 2 {+11 x N}, user data 8, spawn time 8, cached type 5,
//                           label index {1+8}, placement flags 8, team 4, colour {1+3}, type-specific tail
struct ForgeObject {
    bool present = false;
    uint8_t flags = 0;
    int16_t quotaIndex = -1;      // -1 = the "index invalid" flag was set
    int16_t variantIndex = -1;
    bool positionInBounds = false;
    uint32_t positionRaw[3] = {};
    bool upIsGlobal = true;
    uint32_t upRaw = 0;
    uint32_t forwardAngleRaw = 0;
    uint16_t spawnRelativeTo = 0;
    uint8_t boundaryShape = 0;
    uint32_t boundaryRaw[4] = {};
    uint8_t userData = 0;
    uint8_t spawnTime = 0;
    uint8_t cachedType = 0;
    int16_t labelIndex = -1;
    uint8_t placementFlags = 0;
    int8_t team = -1;
    int8_t primaryColor = -1;
    uint8_t extraKind = 0;        // 0 none, 1 weapon, 2 teleporter, 3 location name
    bool extraValid = true;       // location name only: read_index's presence bit
    uint8_t extraA = 0;
    uint8_t extraB = 0;
    float position[3] = {};       // decoded, for display only
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
    size_t headerBits = 0;        // where the author blocks begin
    size_t authorsEndBits = 0;    // where the title/description begin
    size_t titleEndBits = 0;
    size_t secondMapIdBits = 0;
    size_t labelsBits = 0;
    size_t objectsBits = 0;       // first object slot (chunk-relative bits)
    size_t objectsEndBits = 0;    // first quota
    size_t quotasEndBits = 0;     // end of the last quota; 0-7 zero bits of padding follow
    uint32_t packedLength = 0;    // declared length of the packed bitstream in bytes
    uint8_t storedHash[20] = {};  // the 20-byte hash as it sits in the file
    int presentObjects = 0;
    std::wstring title, description;
    bool titleEndianBig = false;
    uint32_t unk02B2 = 0;         // the 9-bit placeable-object-quota count
    uint32_t secondMapId = 0;
    uint32_t labelCount = 0;
    float boundsMin[3] = {};
    float boundsMax[3] = {};
    int positionBits[3] = { 21, 21, 21 };
    Author createdBy, modifiedBy;
    std::vector<VariantQuota> quotas;
    std::vector<ForgeObject> objects;   // always 0x28B entries
};

struct MapVariant {
    bool valid = false;
    std::string error;
    MapVariantHeader header;
    std::vector<uint8_t> fileBytes;   // the whole container, so the writer can copy what it does not model
    size_t chunkOffset = 0;
    size_t chunkSize = 0;
};

// Decodes the "mvar" chunk of an MCC map variant: header, authors, title/description, label table, the 651 object
// records and the object-quota table.
MapVariant ReadMapVariant(const std::filesystem::path& path);
MapVariant ReadMapVariantBytes(std::vector<uint8_t> bytes, const std::string& name);
std::string Describe(const MapVariant& variant);

} // namespace noble::mvar