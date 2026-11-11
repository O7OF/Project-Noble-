#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace noble::mvar {

struct MapVariantHeader {
    std::string signature;          // "mvar"
    uint32_t chunkSize = 0;
    uint16_t chunkVersion = 0;
    uint16_t chunkFlags = 0;
    int32_t contentType = -1;
    uint32_t fileLength = 0;
    int32_t activity = -1;
    int32_t gameMode = -1;
    int32_t engine = -1;
    uint32_t mapId = 0;
    uint32_t engineCategoryIndex = 0;
    std::wstring title;
    std::wstring description;
    uint32_t secondMapId = 0;
    float boundsMin[3] = {};
    float boundsMax[3] = {};
    size_t headerBits = 0;          // where the object data starts, for the writer to repack
};

struct MapVariant {
    bool valid = false;
    std::string error;
    MapVariantHeader header;
};

// Reads the "mvar" chunk out of an MCC map variant file (the BLF container with "chdr" at +0x30 and "mvar" at +0x2F0)
// and decodes its header. This is the first piece of the variant writer: the header has to be understood before the
// object list can be re-emitted.
MapVariant ReadMapVariant(const std::filesystem::path& path);

// Human-readable summary, for the "-mvar-parse" diagnostic.
std::string Describe(const MapVariant& variant);

} // namespace noble::mvar
