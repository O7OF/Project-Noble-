#include "mvar/MapVariant.hpp"
#include "mvar/BitReader.hpp"
#include "common/Logger.hpp"

#include <windows.h>

#include <cstring>
#include <fstream>
#include <sstream>

namespace noble::mvar {
namespace {
constexpr size_t kMvarChunkOffset = 0x2F0;   // "mvar" inside the BLF container
constexpr uint32_t kChunkSize = 0x7028;      // the one size every MCC Reach map variant uses
} // namespace

// Field order follows ReachVariantTool's reference parser (map_variant_mvar.js) one read at a time: the bit fields and
// the byte reads are not interchangeable, and mixing them up shifts everything after the first mistake.
MapVariant ReadMapVariant(const std::filesystem::path& path) {
    MapVariant out;
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        out.error = "cannot open the file";
        return out;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (data.size() < kMvarChunkOffset + 0x20) {
        out.error = "file is too short for a map variant";
        return out;
    }
    if (std::memcmp(data.data(), "_blf", 4) != 0) {
        out.error = "not a BLF container (no '_blf')";
        return out;
    }
    if (std::memcmp(data.data() + kMvarChunkOffset, "mvar", 4) != 0) {
        out.error = "no 'mvar' chunk at +0x2F0";
        return out;
    }

    BitReader stream(data.data() + kMvarChunkOffset, data.size() - kMvarChunkOffset);
    MapVariantHeader& h = out.header;
    h.signature = stream.ReadString(4);
    h.chunkSize = stream.ReadUInt32Big();          // big-endian on disk
    h.chunkVersion = stream.ReadUInt16Big();
    h.chunkFlags = stream.ReadUInt16Big();
    stream.ReadBytes(0x14);                        // SHA-1
    stream.ReadBytes(0x4);                         // content length
    h.contentType = static_cast<int32_t>(stream.ReadBits(4)) - 1;
    h.fileLength = stream.ReadUInt32();            // little-endian from here on
    stream.ReadUInt64(); stream.ReadUInt64();      // unk08, unk10
    stream.ReadUInt64(); stream.ReadUInt64();      // unk18, unk20
    h.activity = static_cast<int32_t>(stream.ReadBits(3)) - 1;
    h.gameMode = static_cast<int32_t>(stream.ReadBits(3));
    h.engine = static_cast<int32_t>(stream.ReadBits(3));
    // The map id is a *bit* field, read straight after the three 3-bit fields with no byte alignment in between.
    h.mapId = static_cast<uint32_t>(stream.ReadBits(32));
    h.engineCategoryIndex = static_cast<uint32_t>(stream.ReadBits(8));
    h.headerBits = stream.BitPosition();
    NOBLE_INFO("Mvar: '{}' signature='{}' size=0x{:X} version=0x{:X} flags=0x{:X} type={} mode={} engine={} mapId={} "
               "bits={}", path.filename().string(), h.signature, h.chunkSize, h.chunkVersion, h.chunkFlags,
               h.contentType, h.gameMode, h.engine, h.mapId, h.headerBits);
    out.valid = !stream.Overran() && h.chunkSize == kChunkSize && h.signature == "mvar";
    if (!out.valid) {
        std::ostringstream why;
        why << "chunk size 0x" << std::hex << h.chunkSize << " signature '" << h.signature << "'";
        if (stream.Overran()) why << ", ran past the end";
        out.error = why.str();
    }
    return out;
}

std::string Describe(const MapVariant& variant) {
    if (!variant.valid) return "invalid: " + variant.error;
    std::ostringstream out;
    out << "map " << variant.header.mapId << ", mode " << variant.header.gameMode << ", engine " << variant.header.engine
        << ", version 0x" << std::hex << variant.header.chunkVersion;
    return out.str();
}

} // namespace noble::mvar
