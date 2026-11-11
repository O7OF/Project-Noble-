#include "mvar/MapVariant.hpp"
#include "mvar/BitReader.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>

namespace noble::mvar {
namespace {

constexpr uint32_t kChunkSize = 0x7028;   // every MCC Reach map variant uses this chunk size
constexpr size_t kMvarBodyHeader = 0x24;  // BLF chunk header (0x0C) + hash (0x14) + packed length (0x04)
constexpr int kMaximumVariantObjects = 0x28B;   // 651
constexpr int kMaximumVariantQuotas = 0x100;    // 256

uint32_t ReadBigU32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

// Walks the BLF chunk list. The mvar chunk is the one this whole module is about; finding it by walking beats
// hard-coding 0x2F0, which only holds while the preceding chunks keep their sizes.
size_t FindChunk(const std::vector<uint8_t>& bytes, const char* signature, uint32_t* sizeOut) {
    size_t offset = 0;
    while (offset + 12 <= bytes.size()) {
        const uint32_t size = ReadBigU32(bytes.data() + offset + 4);
        if (size < 12 || offset + size > bytes.size()) return SIZE_MAX;
        if (std::memcmp(bytes.data() + offset, signature, 4) == 0) {
            if (sizeOut) *sizeOut = size;
            return offset;
        }
        offset += size;
    }
    return SIZE_MAX;
}

// A content author: two 64-bit values, an ASCII name (at most 16 bytes, null terminated) and a 1-bit online flag.
// The name is bit-packed like everything else, so it is read as eight-bit units, not whole bytes.
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

// Per-axis bit count for a quantized position, from the map's bounds. Mirrors the engine's
// adjust_axis_encoding_bit_count_to_match_error_goals: a 21-bit encoding targets 1/120 world unit divided by 2^5,
// and each axis takes however many bits its range needs, capped at 26.
int AxisBits(int bitCount, double dimension) {
    constexpr double kInchesToWorldUnits = 1.0 / 120.0;
    const double maxError = bitCount <= 16 ? kInchesToWorldUnits * static_cast<double>(1 << (16 - bitCount))
                                           : kInchesToWorldUnits / static_cast<double>(1 << (bitCount - 16));
    const double maxValue = std::min(std::ceil(dimension / (maxError * 2.0)), 8388608.0);
    if (maxValue <= 1.0) return 0;
    return std::min(static_cast<int>(std::ceil(std::log2(maxValue))), 26);
}

// read_index: one presence bit, and the value only when the bit is clear. The invalid case is a real value (-1),
// not an error, and it is part of the record's width, so it has to be modelled.
void ReadIndex(BitReader& stream, int bits, int16_t& out) {
    if (stream.ReadBits(1)) {
        out = -1;
        return;
    }
    out = static_cast<int16_t>(stream.ReadBits(bits));
}

void ReadObject(BitReader& stream, ForgeObject& object, const int positionBits[3], const MapVariantHeader& header) {
    object.present = stream.ReadBits(1) != 0;
    if (!object.present) return;

    object.flags = static_cast<uint8_t>(stream.ReadBits(2));
    ReadIndex(stream, 8, object.quotaIndex);
    ReadIndex(stream, 5, object.variantIndex);

    object.positionInBounds = stream.ReadBits(1) != 0;
    for (int i = 0; i < 3; ++i) object.positionRaw[i] = static_cast<uint32_t>(stream.ReadBits(positionBits[i]));

    object.upIsGlobal = stream.ReadBits(1) != 0;
    if (!object.upIsGlobal) object.upRaw = static_cast<uint32_t>(stream.ReadBits(20));
    object.forwardAngleRaw = static_cast<uint32_t>(stream.ReadBits(14));
    object.spawnRelativeTo = static_cast<uint16_t>(stream.ReadBits(10));

    object.boundaryShape = static_cast<uint8_t>(stream.ReadBits(2));
    int boundaryFields = 0;
    if (object.boundaryShape == 1) boundaryFields = 1;
    else if (object.boundaryShape == 2) boundaryFields = 3;
    else if (object.boundaryShape == 3) boundaryFields = 4;
    for (int i = 0; i < boundaryFields; ++i) object.boundaryRaw[i] = static_cast<uint32_t>(stream.ReadBits(11));

    object.userData = static_cast<uint8_t>(stream.ReadBits(8));
    object.spawnTime = static_cast<uint8_t>(stream.ReadBits(8));
    object.cachedType = static_cast<uint8_t>(stream.ReadBits(5));
    ReadIndex(stream, 8, object.labelIndex);
    object.placementFlags = static_cast<uint8_t>(stream.ReadBits(8));
    object.team = static_cast<int8_t>(static_cast<int>(stream.ReadBits(4)) - 1);
    int16_t primaryColor = -1;
    ReadIndex(stream, 3, primaryColor);
    object.primaryColor = static_cast<int8_t>(primaryColor);

    switch (object.cachedType) {
        case 1:   // weapon: spare clips
            object.extraKind = 1;
            object.extraA = static_cast<uint8_t>(stream.ReadBits(8));
            break;
        case 12:  // teleporter: channel, passability
        case 13:
        case 14:
            object.extraKind = 2;
            object.extraA = static_cast<uint8_t>(stream.ReadBits(5));
            object.extraB = static_cast<uint8_t>(stream.ReadBits(5));
            break;
        case 19: { // location name
            object.extraKind = 3;
            int16_t index = -1;
            ReadIndex(stream, 8, index);
            object.extraValid = index >= 0;
            object.extraA = index < 0 ? 0 : static_cast<uint8_t>(index);
            break;
        }
        default:
            break;
    }

    // Decode the position for display only; the writer reuses positionRaw.
    for (int i = 0; i < 3; ++i) {
        const int bits = positionBits[i];
        const double range = static_cast<double>(header.boundsMax[i]) - static_cast<double>(header.boundsMin[i]);
        const double scale = bits > 0 ? range / static_cast<double>(1 << bits) : 0.0;
        object.position[i] = static_cast<float>((0.5 + static_cast<double>(object.positionRaw[i])) * scale +
                                                static_cast<double>(header.boundsMin[i]));
    }
}

} // namespace

MapVariant ReadMapVariantBytes(std::vector<uint8_t> bytes, const std::string& name) {
    (void)name;   // kept for callers that want to label a parse; the module itself does not log
    MapVariant out;
    if (bytes.size() < 0x40) {
        out.error = "file is too short for a map variant";
        return out;
    }
    if (std::memcmp(bytes.data(), "_blf", 4) != 0) {
        out.error = "not a BLF container (no '_blf')";
        return out;
    }

    uint32_t chunkSize = 0;
    const size_t chunkOffset = FindChunk(bytes, "mvar", &chunkSize);
    if (chunkOffset == SIZE_MAX) {
        out.error = "no 'mvar' chunk in the container";
        return out;
    }
    if (chunkOffset + chunkSize > bytes.size()) {
        out.error = "the 'mvar' chunk runs past the end of the file";
        return out;
    }
    out.chunkOffset = chunkOffset;
    out.chunkSize = chunkSize;

    BitReader stream(bytes.data() + chunkOffset, bytes.size() - chunkOffset);
    MapVariantHeader& h = out.header;
    h.signature = stream.ReadString(4);
    h.chunkSize = static_cast<uint32_t>(stream.ReadBits(32));
    h.chunkVersion = static_cast<uint16_t>(stream.ReadBits(16));
    h.chunkFlags = static_cast<uint16_t>(stream.ReadBits(16));
    for (int i = 0; i < 20; ++i) h.storedHash[i] = static_cast<uint8_t>(stream.ReadBits(8));
    h.packedLength = static_cast<uint32_t>(stream.ReadBits(32));
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

    // Title and description: 128 wide characters each, stored up to the terminator.
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
        h.description = ReadWideText(stream, 128, h.titleEndianBig);
    }
    h.titleEndBits = stream.BitPosition();

    if (h.activity == 2) stream.ReadBits(16);            // hopper id
    if (h.gameMode == 1) {
        stream.ReadBits(8); stream.ReadBits(2); stream.ReadBits(2); stream.ReadBits(8); stream.ReadBits(32);
    } else if (h.gameMode == 2) {
        stream.ReadBits(2); stream.ReadBits(32);
    }
    stream.ReadBits(8);                                   // unk02B0 (0x1F in every file)
    stream.ReadBits(32);                                  // unk02DC
    stream.ReadBits(32);                                  // unk02E0
    h.unk02B2 = static_cast<uint32_t>(stream.ReadBits(9));   // placeable-object quota count
    h.secondMapIdBits = stream.BitPosition();
    h.secondMapId = static_cast<uint32_t>(stream.ReadBits(32));
    stream.ReadBits(1);
    stream.ReadBits(1);
    // Six big-endian floats: x.min, x.max, y.min, y.max, z.min, z.max.
    h.boundsMin[0] = stream.ReadFloatBitsBig();
    h.boundsMax[0] = stream.ReadFloatBitsBig();
    h.boundsMin[1] = stream.ReadFloatBitsBig();
    h.boundsMax[1] = stream.ReadFloatBitsBig();
    h.boundsMin[2] = stream.ReadFloatBitsBig();
    h.boundsMax[2] = stream.ReadFloatBitsBig();
    stream.ReadBits(32);                                  // unk02D0
    stream.ReadBits(32);                                  // unk02D4

    // Forge label table.
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

    for (int i = 0; i < 3; ++i) {
        h.positionBits[i] = AxisBits(21, static_cast<double>(h.boundsMax[i]) - static_cast<double>(h.boundsMin[i]));
    }

    h.objectsBits = stream.BitPosition();
    h.objects.resize(kMaximumVariantObjects);
    int present = 0;
    for (int i = 0; i < kMaximumVariantObjects; ++i) {
        ReadObject(stream, h.objects[i], h.positionBits, h);
        if (h.objects[i].present) ++present;
    }
    h.presentObjects = present;
    h.objectsEndBits = stream.BitPosition();

    const uint32_t quotaCount = std::min<uint32_t>(kMaximumVariantQuotas, h.unk02B2);
    h.quotas.resize(quotaCount);
    for (uint32_t i = 0; i < quotaCount; ++i) {
        h.quotas[i].minimum = static_cast<uint8_t>(stream.ReadBits(8));
        h.quotas[i].maximum = static_cast<uint8_t>(stream.ReadBits(8));
        h.quotas[i].placedOnMap = static_cast<uint8_t>(stream.ReadBits(8));
    }
    h.quotasEndBits = stream.BitPosition();

    const size_t packedEnd = kMvarBodyHeader * 8 + static_cast<size_t>(h.packedLength) * 8;
    const size_t slack = packedEnd > h.quotasEndBits ? packedEnd - h.quotasEndBits : 0;
    const bool overran = stream.Overran() || h.packedLength > 0x7000 || h.quotasEndBits > packedEnd || slack > 7;

    out.valid = !overran && h.chunkSize == kChunkSize && h.signature == "mvar";
    if (!out.valid) {
        std::ostringstream why;
        why << "signature '" << h.signature << "', chunk size 0x" << std::hex << h.chunkSize;
        if (stream.Overran()) why << ", ran past the end";
        else if (h.quotasEndBits > packedEnd) why << ", object/quota data runs " << std::dec
                                                  << (h.quotasEndBits - packedEnd) << " bits past the packed length";
        else why << ", " << std::dec << slack << " bits of padding (expected 0-7)";
        out.error = why.str();
    }

    if (!out.valid) return out;

    out.fileBytes = std::move(bytes);
    return out;
}

MapVariant ReadMapVariant(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        MapVariant out;
        out.error = "cannot open the file";
        return out;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return ReadMapVariantBytes(std::move(data), path.filename().string());
}

std::string Describe(const MapVariant& variant) {
    if (!variant.valid) return "invalid: " + variant.error;
    std::ostringstream out;
    out << "map " << variant.header.mapId << ", mode " << variant.header.gameMode << ", engine " << variant.header.engine
        << ", version 0x" << std::hex << variant.header.chunkVersion << std::dec << ", "
        << variant.header.presentObjects << "/" << kMaximumVariantObjects << " objects, "
        << variant.header.quotas.size() << " quotas, by '" << variant.header.createdBy.name << "'";
    return out.str();
}

} // namespace noble::mvar