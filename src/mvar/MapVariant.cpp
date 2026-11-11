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

// A content author: two 64-bit values, an ASCII name (max 16 bytes, null terminated, read as 8-bit units because the
// whole block is bit-packed) and a 1-bit online flag. Its variable length is why offsets differ between files.
Author ReadAuthor(BitReader& stream) {
    Author author;
    author.timestamp = stream.ReadBits(64);
    author.xuid = stream.ReadBits(64);
    for (int i = 0; i < 16; ++i) {
        const uint8_t c = static_cast<uint8_t>(stream.ReadBits(8));
        if (c == 0) break;
        author.name.push_back(static_cast<char>(c));
    }
    author.onlineId = stream.ReadBits(1) != 0;
    return author;
}
} // namespace

// Field order mirrors ReachVariantTool's reference parser read for read (see tools/mvar_oracle): everything after the
// chunk header is bit-packed, so every field is a bit read - byte-aligning any of them shifts the rest.
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
    // These three are big-endian on disk, and a 32-bit MSB-first bit read already yields them in that order.
    h.chunkSize = static_cast<uint32_t>(stream.ReadBits(32));
    h.chunkVersion = static_cast<uint16_t>(stream.ReadBits(16));
    h.chunkFlags = static_cast<uint16_t>(stream.ReadBits(16));
    stream.ReadBytes(0x14);                        // SHA-1
    stream.ReadBytes(0x4);                         // content length
    h.contentType = static_cast<int32_t>(stream.ReadBits(4)) - 1;
    h.fileLength = static_cast<uint32_t>(stream.ReadBits(32));
    h.unk08 = stream.ReadBits(64);
    h.unk10 = stream.ReadBits(64);
    h.unk18 = stream.ReadBits(64);
    h.unk20 = stream.ReadBits(64);
    h.activity = static_cast<int32_t>(stream.ReadBits(3)) - 1;
    h.gameMode = static_cast<int32_t>(stream.ReadBits(3));
    h.engine = static_cast<int32_t>(stream.ReadBits(3));
    h.mapId = static_cast<uint32_t>(stream.ReadBits(32));
    h.engineCategoryIndex = static_cast<uint32_t>(stream.ReadBits(8));
    h.headerBits = stream.BitPosition();

    h.createdBy = ReadAuthor(stream);
    h.modifiedBy = ReadAuthor(stream);
    h.authorsEndBits = stream.BitPosition();

    NOBLE_INFO("Mvar: '{}' sig='{}' size=0x{:X} ver=0x{:X} flags=0x{:X} type={} mode={} engine={} mapId={} "
               "category={} headerBits={} authorsEnd={} author='{}'",
               path.filename().string(), h.signature, h.chunkSize, h.chunkVersion, h.chunkFlags, h.contentType,
               h.gameMode, h.engine, h.mapId, h.engineCategoryIndex, h.headerBits, h.authorsEndBits,
               h.createdBy.name);
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
        << ", version 0x" << std::hex << variant.header.chunkVersion << ", by '" << variant.header.createdBy.name << "'";
    return out.str();
}

} // namespace noble::mvar
