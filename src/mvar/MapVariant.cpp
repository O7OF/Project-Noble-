#include "mvar/MapVariant.hpp"
#include "mvar/BitReader.hpp"
#include "common/Logger.hpp"

#include <windows.h>

#include <cstring>
#include <fstream>
#include <sstream>

#include <cmath>

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


// --- Object records ---------------------------------------------------------------------------------------------
// 651 of these follow the label table (0x28B, fixed count - which is why every variant file is the same size).
// Ported from ReachVariantTool's LoadedForgeObject / LFOUnk30.

static int HighestBitSet(uint32_t value) {
    int r = 0;
    while (value >>= 1) ++r;
    return r;
}

// Per-axis bit counts for a quantized position, from the map's bounds (reference: _sub4DC8E0 with bitcount 21).
void PositionBitCounts(const float boundsMin[3], const float boundsMax[3], int out[3]) {
    constexpr int bitCount = 21;
    float minStep = 0.00833333333f / static_cast<float>(1 << (bitCount - 16));
    minStep *= 2.0f;
    for (int i = 0; i < 3; ++i) {
        const float range = boundsMax[i] - boundsMin[i];
        uint32_t steps = static_cast<uint32_t>(std::ceil(range / minStep));
        if (steps > 0x800000) steps = 0x800000;
        out[i] = steps ? HighestBitSet(steps) : bitCount;
    }
}

struct ForgeObject {
    bool present = false;
    uint8_t subcat = 0xFF;
    uint8_t type = 0xFF;
    float position[3] = {};
    float axis[3] = { 0, 0, 1 };
    uint16_t angleRaw = 0;
    uint8_t shapeType = 0;
    uint8_t spawnSequence = 0;
    uint8_t respawnTime = 0;
    uint8_t unk13 = 0;
    int16_t forgeLabelIndex = -1;
};

// LFOUnk30: the object's shape/behaviour tail. Variable length, so it has to be walked exactly.
static void ReadObjectTail(BitReader& stream, ForgeObject& object) {
    const uint32_t shape = static_cast<uint32_t>(stream.ReadBits(2));
    object.shapeType = static_cast<uint8_t>(shape);
    switch (shape) {
        case 1:
        case 2:
        case 3:
            stream.ReadBits(11);
            if (shape == 3) stream.ReadBits(11);
            if (shape != 1) stream.ReadBits(11);
            break;
        default:
            if (shape == 0) {
                // 2-bit type 0 ends the tail immediately.
                break;
            }
            stream.ReadBits(11);
            break;
    }
    if (shape == 2 || shape == 3) stream.ReadBits(11);
    if (shape == 3) {
        stream.ReadBits(11);
        stream.ReadBits(11);
    }
    // Common tail.
    object.spawnSequence = static_cast<uint8_t>(stream.ReadBits(8));
    object.respawnTime = static_cast<uint8_t>(stream.ReadBits(8));
    object.unk13 = static_cast<uint8_t>(stream.ReadBits(5));
    if (!stream.ReadBits(1)) {
        object.forgeLabelIndex = static_cast<int16_t>(stream.ReadBits(8));
    } else {
        object.forgeLabelIndex = -1;
    }
    stream.ReadBits(8);   // unk16
    stream.ReadBits(4);   // unk17 (object colour)
    if (!stream.ReadBits(1)) stream.ReadBits(3);   // team
    if (object.unk13 == 1) {
        stream.ReadBits(8);
    } else if (object.unk13 > 0xB) {
        if (object.unk13 <= 0xE) {
            stream.ReadBits(5);
            stream.ReadBits(5);
        } else if (object.unk13 == 0x13) {
            stream.ReadBits(8);
        }
    }
}

static ForgeObject ReadObject(BitReader& stream, const int positionBits[3], const float boundsMin[3],
                              const float boundsMax[3]) {
    ForgeObject object;
    if (!stream.ReadBits(1)) return object;   // presence bit
    object.present = true;
    stream.ReadBits(2);                        // unk00
    if (!stream.ReadBits(1)) object.subcat = static_cast<uint8_t>(stream.ReadBits(8));
    if (!stream.ReadBits(1)) object.type = static_cast<uint8_t>(stream.ReadBits(5));

    // Position: one "bounds set" bit, then three quantized fields.
    if (stream.ReadBits(1)) {
        // The reference falls back to static values here; the bit still has to be consumed.
    }
    for (int i = 0; i < 3; ++i) {
        const uint32_t raw = static_cast<uint32_t>(stream.ReadBits(positionBits[i]));
        const float range = boundsMax[i] - boundsMin[i];
        object.position[i] = (0.5f + static_cast<float>(raw)) * (range / static_cast<float>(1 << positionBits[i])) +
                             boundsMin[i];
    }

    // Rotation: an axis-angle. A single bit means "yaw only".
    if (!stream.ReadBits(1)) {
        stream.ReadBits(20);   // packed unit vector
    }
    object.angleRaw = static_cast<uint16_t>(stream.ReadBits(14));
    stream.ReadBits(10);       // unk2C
    ReadObjectTail(stream, object);
    return object;
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
    // Six big-endian floats: x.min, x.max, y.min, y.max, z.min, z.max.
    h.boundsMin[0] = stream.ReadFloatBitsBig();
    h.boundsMax[0] = stream.ReadFloatBitsBig();
    h.boundsMin[1] = stream.ReadFloatBitsBig();
    h.boundsMax[1] = stream.ReadFloatBitsBig();
    h.boundsMin[2] = stream.ReadFloatBitsBig();
    h.boundsMax[2] = stream.ReadFloatBitsBig();
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

    int positionBits[3] = { 21, 21, 21 };
    PositionBitCounts(h.boundsMin, h.boundsMax, positionBits);
    int present = 0;
    for (int i = 0; i < 0x28B; ++i) {
        const size_t recordStart = stream.BitPosition();
        const ForgeObject object = ReadObject(stream, positionBits, h.boundsMin, h.boundsMax);
        if (!object.present) continue;
        if (present < 6) {
            NOBLE_INFO("Mvar: object[{}] type {} subcat {} at ({:.1f}, {:.1f}, {:.1f}) shape {}", i, object.type,
                       object.subcat, object.position[0], object.position[1], object.position[2], object.shapeType);
        }
        if (present < 6) {
            NOBLE_INFO("Mvar:   record {} consumed {} bits", i, stream.BitPosition() - recordStart);
        }
        ++present;
    }
    h.presentObjects = present;
    h.objectsEndBits = stream.BitPosition();
    NOBLE_INFO("Mvar: {} of {} object slots used, position bits {}/{}/{}, objects end at bit {}", present, 0x28B,
               positionBits[0], positionBits[1], positionBits[2], h.objectsEndBits);

    NOBLE_INFO("Mvar: '{}' sig='{}' size=0x{:X} ver=0x{:X} flags=0x{:X} type={} mode={} engine={} mapId={} "
               "category={} headerBits={} authorsEnd={} author='{}'",
               path.filename().string(), h.signature, h.chunkSize, h.chunkVersion, h.chunkFlags, h.contentType,
               h.gameMode, h.engine, h.mapId, h.engineCategoryIndex, h.headerBits, h.authorsEndBits,
               h.createdBy.name);
    NOBLE_INFO("Mvar: title='{}' desc='{}' secondMapId={} (bit {}) labels={} (bit {}) objects at bit {}",
               std::string(h.title.begin(), h.title.end()), std::string(h.description.begin(), h.description.end()),
               h.secondMapId, h.secondMapIdBits, h.labelCount, h.labelsBits, h.objectsBits);
    NOBLE_INFO("Mvar: activity={} bounds x[{:.1f},{:.1f}] y[{:.1f},{:.1f}] z[{:.1f},{:.1f}]",
               h.activity, h.boundsMin[0], h.boundsMax[0], h.boundsMin[1], h.boundsMax[1], h.boundsMin[2], h.boundsMax[2]);
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
