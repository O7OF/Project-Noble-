#include "mvar/MapVariantLibrary.hpp"

#include "mvar/MapVariant.hpp"
#include "mvar/MapVariantWriter.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

namespace noble::mvar {
namespace {

constexpr size_t kTitleOffset = 0xC0;   // the byte-aligned title copy in the chdr chunk
constexpr size_t kTitleChars = 64;      // 64 UTF-16 units, null terminated

void SetContainerTitle(std::vector<uint8_t>& bytes, const std::string& name) {
    if (bytes.size() < kTitleOffset + kTitleChars * 2) return;
    std::memset(bytes.data() + kTitleOffset, 0, kTitleChars * 2);
    const size_t count = std::min(name.size(), kTitleChars - 1);
    for (size_t i = 0; i < count; ++i) {
        bytes[kTitleOffset + i * 2] = static_cast<uint8_t>(name[i]);
        bytes[kTitleOffset + i * 2 + 1] = 0;
    }
}

} // namespace

std::string SanitizeVariantName(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (unsigned char c : name) {
        if (c < 0x20 || c == 0x7F || std::strchr("\\/:*?\"<>|", c)) out.push_back('_');
        else out.push_back(static_cast<char>(c));
    }
    // No leading/trailing spaces or dots: Windows silently drops them, which would make the file name differ from the
    // title shown in the menu.
    while (!out.empty() && (out.back() == ' ' || out.back() == '.')) out.pop_back();
    const size_t start = out.find_first_not_of(' ');
    if (start == std::string::npos) out.clear();
    else if (start > 0) out.erase(0, start);
    if (out.size() > 60) out.resize(60);
    if (out.empty()) out = "Forge Map";
    return out;
}

std::string VariantTitle(const std::filesystem::path& sourceMvar) {
    const MapVariant variant = ReadMapVariant(sourceMvar);
    if (!variant.valid) return {};
    std::string out;
    out.reserve(variant.header.title.size());
    for (wchar_t c : variant.header.title) out.push_back(c < 0x80 ? static_cast<char>(c) : '?');
    return out;
}

std::filesystem::path SaveVariantChunkToLibrary(const std::vector<uint8_t>& mvarChunk,
                                                const std::filesystem::path& templateMvar, const std::string& name,
                                                const std::filesystem::path& directory, std::string& detail) {
    if (mvarChunk.size() != 0x7028) {
        detail = "the captured chunk is not 0x7028 bytes";
        return {};
    }
    std::ifstream file(templateMvar, std::ios::binary);
    if (!file) {
        detail = "no template variant to wrap the chunk in ('" + templateMvar.string() + "')";
        return {};
    }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Walk the template's BLF chunks to the mvar chunk it already has.
    size_t offset = 0, chunkOffset = SIZE_MAX;
    while (offset + 12 <= bytes.size()) {
        const uint32_t size = (static_cast<uint32_t>(bytes[offset + 4]) << 24) |
                              (static_cast<uint32_t>(bytes[offset + 5]) << 16) |
                              (static_cast<uint32_t>(bytes[offset + 6]) << 8) | bytes[offset + 7];
        if (size < 12 || offset + size > bytes.size()) break;
        if (std::memcmp(bytes.data() + offset, "mvar", 4) == 0) { chunkOffset = offset; break; }
        offset += size;
    }
    if (chunkOffset == SIZE_MAX || chunkOffset + 0x7028 > bytes.size()) {
        detail = "the template has no usable mvar chunk";
        return {};
    }
    std::memcpy(bytes.data() + chunkOffset, mvarChunk.data(), mvarChunk.size());

    const std::string safe = SanitizeVariantName(name);
    SetContainerTitle(bytes, safe);

    MapVariant variant = ReadMapVariantBytes(std::move(bytes), safe);
    if (!variant.valid) {
        detail = "the assembled variant did not parse: " + variant.error;
        return {};
    }

    // The maps menu groups a variant by the base-map id in the chdr, while the engine loads the map and objects from
    // the mvar chunk. Writing the chunk's id over it keeps the two copies agreeing when the template is another map.
    if (variant.fileBytes.size() > 0x70) {
        const uint32_t mapId = variant.header.mapId;
        variant.fileBytes[0x6C] = static_cast<uint8_t>(mapId & 0xFF);
        variant.fileBytes[0x6D] = static_cast<uint8_t>((mapId >> 8) & 0xFF);
        variant.fileBytes[0x6E] = static_cast<uint8_t>((mapId >> 16) & 0xFF);
        variant.fileBytes[0x6F] = static_cast<uint8_t>((mapId >> 24) & 0xFF);
    }

    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    const std::filesystem::path out = directory / (safe + ".json");
    std::string error;
    // The chunk came from the engine's serializer: keep its bytes, hash and trailing padding exactly as written.
    if (!SaveContainerJson(variant.fileBytes, variant.header.mapId, safe, out, error)) {
        detail = error;
        return {};
    }
    detail = "saved";
    return out;
}

std::filesystem::path SaveVariantToLibrary(const std::filesystem::path& sourceMvar, const std::string& name,
                                           const std::filesystem::path& directory, std::string& detail) {
    std::ifstream file(sourceMvar, std::ios::binary);
    if (!file) {
        detail = "cannot open '" + sourceMvar.string() + "'";
        return {};
    }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.size() < 0x7329) {
        detail = "'" + sourceMvar.string() + "' is too short to be a map variant";
        return {};
    }

    const std::string safe = SanitizeVariantName(name);
    SetContainerTitle(bytes, safe);

    // Parse after patching so the object holds the bytes that are written (and the writer preserves the stored hash
    // for an unchanged payload, which is the common case for a save-as-copy).
    MapVariant variant = ReadMapVariantBytes(std::move(bytes), safe);
    if (!variant.valid) {
        detail = "the variant did not parse: " + variant.error;
        return {};
    }

    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    const std::filesystem::path out = directory / (safe + ".json");
    std::string error;
    if (!SaveMapVariantJson(variant, out, safe, error)) {
        detail = error;
        return {};
    }
    detail = "saved";
    return out;
}

} // namespace noble::mvar
