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

std::string Narrow(const std::wstring& wide) {
    if (wide.empty()) return {};
    char buf[1024] = {};
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), buf, sizeof(buf) - 1, nullptr, nullptr);
    return buf;
}
} // namespace

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
    h.chunkSize = stream.ReadUInt32(false);
    h.chunkVersion = stream.ReadUInt16(false);
    h.chunkFlags = stream.ReadUInt16(false);
    stream.ReadBytes(0x14);              // SHA1
    stream.ReadBytes(0x4);               // content length
    // GameVariantHeader: 4-bit content type, then byte-aligned fields.
    h.contentType = static_cast<int32_t>(stream.ReadBits(4)) - 1;
    h.fileLength = static_cast<uint32_t>(stream.ReadBits(32));
    stream.ReadBits(32); stream.ReadBits(32);   // unk08
    stream.ReadBits(32); stream.ReadBits(32);   // unk10
    stream.ReadBits(32); stream.ReadBits(32);   // unk18
    stream.ReadBits(32); stream.ReadBits(32);   // unk20
    h.activity = static_cast<int32_t>(stream.ReadBits(3)) - 1;
    h.gameMode = static_cast<int32_t>(stream.ReadBits(3));
    h.engine = static_cast<int32_t>(stream.ReadBits(3));
    stream.Align();
    h.mapId = static_cast<uint32_t>(stream.ReadBits(32));
    h.engineCategoryIndex = static_cast<uint32_t>(stream.ReadBits(8));
    // createdBy / modifiedBy (VariantContentAuthor) come next; the title follows them.
    // Their size is not fixed here, so the title is decoded once the author block is understood - for now the
    // header fields above are what identifies the variant, and they are enough to check the reader against a file.
    h.headerBits = stream.BitPosition();
    out.valid = !stream.Overran() && h.chunkSize == kChunkSize;
    if (!out.valid) {
        std::ostringstream why;
        why << "chunk size 0x" << std::hex << h.chunkSize << " (expected 0x" << kChunkSize << ")";
        if (stream.Overran()) why << ", ran past the end";
        out.error = why.str();
    }
    NOBLE_INFO("Mvar: '{}' signature='{}' size=0x{:X} version=0x{:X} flags=0x{:X} type={} mode={} engine={} mapId={} "
               "headerBits={}", path.filename().string(), h.signature, h.chunkSize, h.chunkVersion, h.chunkFlags,
               h.contentType, h.gameMode, h.engine, h.mapId, h.headerBits);
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
