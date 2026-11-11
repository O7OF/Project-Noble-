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


// Reads a 128-character wide string. MCC stores the variant's title/description as UTF-16; the reference parser reads
// them big-endian, but the bytes are the same either way, so try both orders and keep the one that reads as text.
std::wstring ReadWideText(BitReader& stream, int chars, bool bigEndian) {
    std::wstring out;
    for (int i = 0; i < chars; ++i) {
        const uint16_t a = static_cast<uint16_t>(stream.ReadBits(8));
        const uint16_t b = static_cast<uint16_t>(stream.ReadBits(8));
        const uint16_t ch = bigEndian ? static_cast<uint16_t>((a << 8) | b) : static_cast<uint16_t>((b << 8) | a);
        if (ch == 0) break;
        out.push_back(static_cast<wchar_t>(ch));
    }
    return out;
}

bool LooksLikeText(const std::wstring& text) {
    if (text.empty()) return false;
    for (wchar_t c : text) {
        if (c < 0x20 || c > 0x7E) return false;
    }
    return true;
}

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

    // Title and description: 128 wide characters each.
    {
        const size_t start = stream.BitPosition();
        std::wstring little = ReadWideText(stream, 128, false);
        const size_t afterLittle = stream.BitPosition();
        stream.Seek(start);
        std::wstring big = ReadWideText(stream, 128, true);
        const size_t afterBig = stream.BitPosition();
        if (LooksLikeText(little)) {
            h.title = little;
            h.titleEndianBig = false;
            stream.Seek(afterLittle);
        } else {
            h.title = big;
            h.titleEndianBig = true;
            stream.Seek(afterBig);
        }
        const size_t descStart = stream.BitPosition();
        std::wstring dLittle = ReadWideText(stream, 128, h.titleEndianBig);
        (void)dLittle;
        h.description = dLittle;
        (void)descStart;
    }
    h.titleEndBits = stream.BitPosition();
    if (h.activity == 2) stream.ReadBits(16);            // hopper id
    if (h.gameMode == 1) {
        stream.ReadBits(8); stream.ReadBits(2); stream.ReadBits(2); stream.ReadBits(8); stream.ReadBits(32);
    } else if (h.gameMode == 2) {
        stream.ReadBits(2); stream.ReadBits(32);
    }
    stream.ReadBits(8);                                   // unk02B0
    stream.ReadBits(32);                                  // unk02DC
    stream.ReadBits(32);                                  // unk02E0
    h.unk02B2 = static_cast<uint32_t>(stream.ReadBits(9));
    h.secondMapIdBits = stream.BitPosition();
    h.secondMapId = static_cast<uint32_t>(stream.ReadBits(32));
    stream.ReadBits(1);                                   // unk02D9
    stream.ReadBits(1);                                   // unk02DA
    for (float& f : h.boundsMin) f = stream.ReadFloatBig();
    for (int i = 0; i < 3; ++i) { stream.ReadFloatBig(); stream.ReadFloatBig(); }   // max, then y/z pairs
    stream.ReadBits(32);                                  // unk02D0
    stream.ReadBits(32);                                  // unk02D4
    h.labelsBits = stream.BitPosition();
    h.labelCount = static_cast<uint32_t>(stream.ReadBits(9));
    for (uint32_t i = 0; i < h.labelCount; ++i) {
        if (!stream.ReadBits(1)) continue;
        stream.ReadBits(12);
    }
    if (h.labelCount > 0) {
        const uint32_t dataLength = static_cast<uint32_t>(stream.ReadBits(13));
        const bool compressed = stream.ReadBits(1) != 0;
        if (!compressed) {
            for (uint32_t i = 0; i < dataLength; ++i) stream.ReadBits(8);
        } else {
            const uint32_t compSize = static_cast<uint32_t>(stream.ReadBits(13));
            for (uint32_t i = 0; i < compSize; ++i) stream.ReadBits(8);
        }
    }
    h.objectsBits = stream.BitPosition();

    NOBLE_INFO("Mvar: '{}' sig='{}' size=0x{:X} ver=0x{:X} flags=0x{:X} type={} mode={} engine={} mapId={} "
               "category={} headerBits={} authorsEnd={} author='{}'",
               path.filename().string(), h.signature, h.chunkSize, h.chunkVersion, h.chunkFlags, h.contentType,
               h.gameMode, h.engine, h.mapId, h.engineCategoryIndex, h.headerBits, h.authorsEndBits,
               h.createdBy.name);
    NOBLE_INFO("Mvar: title='{}' desc='{}' secondMapId={} (bit {}) labels={} (bit {}) objects at bit {}",
               std::string(h.title.begin(), h.title.end()), std::string(h.description.begin(), h.description.end()),
               h.secondMapId, h.secondMapIdBits, h.labelCount, h.labelsBits, h.objectsBits);
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
