#include "mvar/MapVariantWriter.hpp"
#include "mvar/Sha1.hpp"
#include <windows.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>

namespace noble::mvar {
namespace {

// Bungie's gen-3 security salt, prefixed to every hashed payload (saved films, game variants, map variants).
constexpr uint8_t kGen3Salt[] = {   // 34 bytes: EDD43009...B7714F1C36EC
    0xED, 0xD4, 0x30, 0x09, 0x66, 0x6D, 0x5C, 0x4A, 0x5C, 0x36, 0x57, 0xFA, 0xB4, 0x0E, 0x02, 0x2F, 0x53, 0x5A,
    0xC6, 0xC9, 0xEE, 0x47, 0x1F, 0x01, 0xF1, 0xA4, 0x47, 0x56, 0xB7, 0x71, 0x4F, 0x1C, 0x36, 0xEC,
};

class BitWriter {
public:
    size_t BitPosition() const { return m_bitPos; }

    void Write(uint64_t value, int bits) {
        for (int i = bits - 1; i >= 0; --i) {
            const size_t byteIndex = m_bitPos >> 3;
            if (byteIndex >= m_data.size()) m_data.push_back(0);
            if ((value >> i) & 1u) m_data[byteIndex] |= static_cast<uint8_t>(0x80u >> (m_bitPos & 7));
            ++m_bitPos;
        }
    }

    void CopyBits(const uint8_t* source, size_t fromBit, size_t toBit) {
        for (size_t bit = fromBit; bit < toBit; ++bit) {
            const uint8_t value = static_cast<uint8_t>((source[bit >> 3] >> (7 - (bit & 7))) & 1u);
            Write(value, 1);
        }
    }

    std::vector<uint8_t> Finish() {
        if (m_bitPos & 7) m_bitPos += 8 - (m_bitPos & 7);   // the stream is padded to a whole byte with zeros
        return m_data;
    }

private:
    std::vector<uint8_t> m_data;
    size_t m_bitPos = 0;
};

void WriteIndex(BitWriter& writer, int16_t value, int bits) {
    if (value < 0) {
        writer.Write(1, 1);
        return;
    }
    writer.Write(0, 1);
    writer.Write(static_cast<uint64_t>(value), bits);
}

void WriteObject(BitWriter& writer, const ForgeObject& object, const int positionBits[3]) {
    writer.Write(object.present ? 1 : 0, 1);
    if (!object.present) return;

    writer.Write(object.flags, 2);
    WriteIndex(writer, object.quotaIndex, 8);
    WriteIndex(writer, object.variantIndex, 5);
    writer.Write(object.positionInBounds ? 1 : 0, 1);
    for (int i = 0; i < 3; ++i) writer.Write(object.positionRaw[i], positionBits[i]);
    writer.Write(object.upIsGlobal ? 1 : 0, 1);
    if (!object.upIsGlobal) writer.Write(object.upRaw, 20);
    writer.Write(object.forwardAngleRaw, 14);
    writer.Write(object.spawnRelativeTo, 10);
    writer.Write(object.boundaryShape, 2);
    int boundaryFields = 0;
    if (object.boundaryShape == 1) boundaryFields = 1;
    else if (object.boundaryShape == 2) boundaryFields = 3;
    else if (object.boundaryShape == 3) boundaryFields = 4;
    for (int i = 0; i < boundaryFields; ++i) writer.Write(object.boundaryRaw[i], 11);
    writer.Write(object.userData, 8);
    writer.Write(object.spawnTime, 8);
    writer.Write(object.cachedType, 5);
    WriteIndex(writer, object.labelIndex, 8);
    writer.Write(object.placementFlags, 8);
    writer.Write(static_cast<uint64_t>(object.team + 1), 4);
    WriteIndex(writer, object.primaryColor, 3);
    switch (object.extraKind) {
        case 1:
            writer.Write(object.extraA, 8);
            break;
        case 2:
            writer.Write(object.extraA, 5);
            writer.Write(object.extraB, 5);
            break;
        case 3:
            writer.Write(object.extraValid ? 0 : 1, 1);
            if (object.extraValid) writer.Write(object.extraA, 8);
            break;
        default:
            break;
    }
}

void AppendBigU32(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(value & 0xFF));
}

std::string ToUtf8(const std::wstring& text) {
    if (text.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size, nullptr, nullptr);
    return out;
}

std::string JsonEscape(const std::string& text) {
    std::string out;
    out.reserve(text.size() + 8);
    for (unsigned char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                    out += buffer;
                } else {
                    out.push_back(static_cast<char>(c));
                }
        }
    }
    return out;
}

std::string Base64(const std::vector<uint8_t>& bytes) {
    static const char* kAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve((bytes.size() + 2) / 3 * 4);
    for (size_t i = 0; i < bytes.size(); i += 3) {
        const uint32_t b0 = bytes[i];
        const uint32_t b1 = i + 1 < bytes.size() ? bytes[i + 1] : 0;
        const uint32_t b2 = i + 2 < bytes.size() ? bytes[i + 2] : 0;
        const uint32_t triple = (b0 << 16) | (b1 << 8) | b2;
        out.push_back(kAlphabet[(triple >> 18) & 0x3F]);
        out.push_back(kAlphabet[(triple >> 12) & 0x3F]);
        out.push_back(i + 1 < bytes.size() ? kAlphabet[(triple >> 6) & 0x3F] : '=');
        out.push_back(i + 2 < bytes.size() ? kAlphabet[triple & 0x3F] : '=');
    }
    return out;
}

} // namespace

std::array<uint8_t, 20> ComputeChunkHash(const std::vector<uint8_t>& packedData, uint32_t packedLength) {
    std::vector<uint8_t> input;
    input.reserve(sizeof(kGen3Salt) + 4 + packedData.size());
    input.insert(input.end(), std::begin(kGen3Salt), std::end(kGen3Salt));
    AppendBigU32(input, packedLength);
    const size_t count = std::min<size_t>(packedLength, packedData.size());
    input.insert(input.end(), packedData.begin(), packedData.begin() + static_cast<std::ptrdiff_t>(count));
    return Sha1(input.data(), input.size());
}

bool WriteMapVariant(const MapVariant& variant, std::vector<uint8_t>& out, bool& hashRecomputed, std::string& error) {
    hashRecomputed = false;
    if (!variant.valid) {
        error = "the variant did not decode: " + variant.error;
        return false;
    }
    const MapVariantHeader& h = variant.header;
    const size_t chunkOffset = variant.chunkOffset;
    const size_t chunkSize = variant.chunkSize;
    if (chunkSize != 0x7028) {
        error = "unexpected mvar chunk size";
        return false;
    }

    BitWriter writer;
    writer.CopyBits(variant.fileBytes.data() + chunkOffset, 0, h.objectsBits);
    for (const ForgeObject& object : h.objects) WriteObject(writer, object, h.positionBits);
    writer.CopyBits(variant.fileBytes.data() + chunkOffset, h.objectsEndBits, h.quotasEndBits);
    // The writer's bit 0 is the chunk's first byte; the packed bitstream starts after the 0x24-byte body header
    // (BLF chunk header + hash + packed length), which is byte-aligned, so the prefix is a plain slice.
    std::vector<uint8_t> packedAll = writer.Finish();
    std::vector<uint8_t> packed(packedAll.begin() + 0x24, packedAll.end());
    const uint32_t packedLength = static_cast<uint32_t>(packed.size());
    if (packedLength > 0x7000) {
        error = "the re-encoded variant does not fit the 0x7000-byte packed storage";
        return false;
    }

    // The stored hash is a legacy one in shipped MCC files (it does not match the canonical construction), so keep it
    // when the payload is untouched, and write the canonical hash once the payload actually changes.
    std::vector<uint8_t> originalPacked(h.packedLength);
    if (h.packedLength) {
        std::memcpy(originalPacked.data(), variant.fileBytes.data() + chunkOffset + 0x24, h.packedLength);
    }
    std::array<uint8_t, 20> hash{};
    if (packedLength == h.packedLength && packed == originalPacked) {
        std::memcpy(hash.data(), h.storedHash, hash.size());
    } else {
        hash = ComputeChunkHash(packed, packedLength);
        hashRecomputed = true;
    }

    std::vector<uint8_t> chunk;
    chunk.reserve(chunkSize);
    chunk.insert(chunk.end(), variant.fileBytes.begin() + static_cast<std::ptrdiff_t>(chunkOffset),
                 variant.fileBytes.begin() + static_cast<std::ptrdiff_t>(chunkOffset + 0x0C));
    chunk.insert(chunk.end(), hash.begin(), hash.end());
    AppendBigU32(chunk, packedLength);
    chunk.insert(chunk.end(), packed.begin(), packed.end());
    chunk.resize(0x24 + 0x7000, 0);   // packed storage slack
    chunk.resize(chunkSize, 0);       // the 4-byte tail pad
    if (chunk.size() != chunkSize) {
        error = "internal: chunk assembly produced the wrong size";
        return false;
    }

    out.clear();
    out.reserve(variant.fileBytes.size());
    out.insert(out.end(), variant.fileBytes.begin(), variant.fileBytes.begin() + static_cast<std::ptrdiff_t>(chunkOffset));
    out.insert(out.end(), chunk.begin(), chunk.end());
    out.insert(out.end(), variant.fileBytes.begin() + static_cast<std::ptrdiff_t>(chunkOffset + chunkSize),
               variant.fileBytes.end());
    return true;
}

bool SaveContainerJson(const std::vector<uint8_t>& container, uint32_t baseMapId, const std::string& name,
                       const std::filesystem::path& jsonPath, std::string& error) {
    std::ofstream file(jsonPath, std::ios::binary | std::ios::trunc);
    if (!file) {
        error = "cannot open '" + jsonPath.string() + "' for writing";
        return false;
    }
    const std::string safeName = name.empty() ? std::string("Forge Map") : name;
    file << "{\n  \"version\": 1,\n  \"name\": \"" << JsonEscape(safeName) << "\",\n  \"base_map_id\": "
         << baseMapId << ",\n  \"mode\": \"forge\",\n  \"data\": \"" << Base64(container) << "\"\n}\n";
    file.close();
    if (!file) {
        error = "could not write '" + jsonPath.string() + "'";
        return false;
    }
    return true;
}

bool SaveMapVariantJson(const MapVariant& variant, const std::filesystem::path& jsonPath, const std::string& name,
                        std::string& error) {
    std::vector<uint8_t> bytes;
    bool hashRecomputed = false;
    if (!WriteMapVariant(variant, bytes, hashRecomputed, error)) return false;

    std::string safeName = name.empty() ? ToUtf8(variant.header.title) : name;
    if (safeName.empty()) safeName = "Forge Map";

    std::ofstream file(jsonPath, std::ios::binary | std::ios::trunc);
    if (!file) {
        error = "cannot open '" + jsonPath.string() + "' for writing";
        return false;
    }
    file << "{\n  \"version\": 1,\n  \"name\": \"" << JsonEscape(safeName) << "\",\n  \"base_map_id\": "
         << variant.header.mapId << ",\n  \"mode\": \"forge\",\n  \"data\": \"" << Base64(bytes) << "\"\n}\n";
    file.close();
    if (!file) {
        error = "could not write '" + jsonPath.string() + "'";
        return false;
    }
    return true;
}

} // namespace noble::mvar