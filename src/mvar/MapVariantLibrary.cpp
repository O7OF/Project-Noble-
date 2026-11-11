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
