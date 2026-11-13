// Headless map variant tool: parse an .mvar, repack it, and prove the round trip without starting the launcher.
//
//   mvar-tool <file.mvar | directory>        parse + repack + byte comparison (all files in a directory)
//   mvar-tool -info <file.mvar>              region layout and object summary
//   mvar-tool -save <file.mvar> <out.json>   write the Noble JSON form and verify it decodes back byte for byte
//
// Exit code 0 when everything matched, 1 when a file differed, 2 on bad usage.

#include "mvar/MapVariant.hpp"
#include "mvar/MapVariantWriter.hpp"
#include "mvar/MapVariantLibrary.hpp"
#include "mvar/ForgePlacementApply.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::vector<uint8_t> ReadFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Minimal base64 decoder for the JSON round-trip check (the encoder lives in the writer).
std::vector<uint8_t> Base64Decode(const std::string& text) {
    auto value = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return -1;
    };
    std::vector<uint8_t> out;
    uint32_t buffer = 0;
    int bits = 0;
    for (char c : text) {
        const int v = value(c);
        if (v < 0) continue;
        buffer = (buffer << 6) | static_cast<uint32_t>(v);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<uint8_t>((buffer >> bits) & 0xFF));
        }
    }
    return out;
}

std::string ExtractJsonData(const std::string& json) {
    const std::string key = "\"data\"";
    size_t at = json.find(key);
    if (at == std::string::npos) return {};
    at = json.find('"', json.find(':', at));
    if (at == std::string::npos) return {};
    const size_t end = json.find('"', at + 1);
    if (end == std::string::npos) return {};
    return json.substr(at + 1, end - at - 1);
}

int Info(const std::filesystem::path& path) {
    const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(path);
    if (!variant.valid) {
        std::printf("invalid: %s\n", variant.error.c_str());
        return 1;
    }
    const auto& h = variant.header;
    std::printf("%s\n", path.filename().string().c_str());
    std::printf("  chunk 0x%zX size 0x%zX  packed %u bytes\n", variant.chunkOffset, variant.chunkSize, h.packedLength);
    std::printf("  map %u  mode %d  engine %d  activity %d  type %d  category %u\n", h.mapId, h.gameMode, h.engine,
                h.activity, h.contentType, h.engineCategoryIndex);
    std::printf("  title '%ls'  description '%ls'\n", h.title.c_str(), h.description.c_str());
    std::printf("  by '%s' (%llu) / '%s' (%llu)\n", h.createdBy.name.c_str(),
                static_cast<unsigned long long>(h.createdBy.timestamp), h.modifiedBy.name.c_str(),
                static_cast<unsigned long long>(h.modifiedBy.timestamp));
    std::printf("  bounds x[%.1f,%.1f] y[%.1f,%.1f] z[%.1f,%.1f]  position bits %d/%d/%d\n", h.boundsMin[0],
                h.boundsMax[0], h.boundsMin[1], h.boundsMax[1], h.boundsMin[2], h.boundsMax[2], h.positionBits[0],
                h.positionBits[1], h.positionBits[2]);
    std::printf("  labels %u  objects %d/651 present  quotas %zu (field %u)\n", h.labelCount, h.presentObjects,
                h.quotas.size(), h.unk02B2);
    std::printf("  regions (chunk bits): header %zX, authors end %zX, labels %zX, objects %zX..%zX, quotas end %zX\n",
                h.headerBits, h.authorsEndBits, h.labelsBits, h.objectsBits, h.objectsEndBits, h.quotasEndBits);
    std::printf("  packed end %zX  slack %zu bits\n", static_cast<size_t>(0x24) * 8 + static_cast<size_t>(h.packedLength) * 8,
                static_cast<size_t>(0x24) * 8 + static_cast<size_t>(h.packedLength) * 8 - h.quotasEndBits);
    std::printf("  stored hash %02x%02x%02x%02x...\n", h.storedHash[0], h.storedHash[1], h.storedHash[2], h.storedHash[3]);
    return 0;
}

int RoundTrip(const std::vector<std::filesystem::path>& files) {
    int identical = 0, different = 0, failed = 0;
    for (const auto& path : files) {
        const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(path);
        if (!variant.valid) {
            std::printf("FAIL  %s: %s\n", path.filename().string().c_str(), variant.error.c_str());
            ++failed;
            continue;
        }
        std::vector<uint8_t> bytes;
        bool hashRecomputed = false;
        std::string error;
        if (!noble::mvar::WriteMapVariant(variant, bytes, hashRecomputed, error)) {
            std::printf("FAIL  %s: %s\n", path.filename().string().c_str(), error.c_str());
            ++failed;
            continue;
        }
        if (bytes == variant.fileBytes) {
            ++identical;
        } else {
            size_t at = 0;
            while (at < bytes.size() && at < variant.fileBytes.size() && bytes[at] == variant.fileBytes[at]) ++at;
            std::printf("DIFF  %s at 0x%zX (%zu vs %zu bytes)%s\n", path.filename().string().c_str(), at, bytes.size(),
                        variant.fileBytes.size(), hashRecomputed ? " [hash recomputed]" : "");
            ++different;
        }
    }
    std::printf("round trip: %d identical, %d different, %d failed\n", identical, different, failed);
    return (different || failed) ? 1 : 0;
}

int WriteBytes(const std::filesystem::path& in, const std::filesystem::path& out) {
    const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(in);
    std::vector<uint8_t> bytes;
    bool hashRecomputed = false;
    std::string error;
    if (!noble::mvar::WriteMapVariant(variant, bytes, hashRecomputed, error)) {
        std::printf("FAIL  %s: %s\n", in.filename().string().c_str(), error.c_str());
        return 1;
    }
    std::ofstream file(out, std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    std::printf("%s: %zu bytes, hash %s\n", out.filename().string().c_str(), bytes.size(),
                hashRecomputed ? "recomputed" : "preserved");
    return 0;
}

// Moves one object by a single position quantum and checks the container through a re-parse. This is the smallest
// real edit: it changes the object region length-neutral, so everything after it must shift and the hash must change.
int Nudge(const std::filesystem::path& in, const std::filesystem::path& out, int slot, int axis) {
    noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(in);
    if (!variant.valid) {
        std::printf("FAIL  %s: %s\n", in.filename().string().c_str(), variant.error.c_str());
        return 1;
    }
    if (slot < 0 || slot >= static_cast<int>(variant.header.objects.size()) || !variant.header.objects[slot].present) {
        std::printf("FAIL  slot %d is not a present object\n", slot);
        return 1;
    }
    if (axis < 0 || axis > 2) {
        std::printf("FAIL  axis must be 0, 1 or 2\n");
        return 1;
    }
    const uint32_t before = variant.header.objects[slot].positionRaw[axis];
    variant.header.objects[slot].positionRaw[axis] = before + 1;

    std::vector<uint8_t> bytes;
    bool hashRecomputed = false;
    std::string error;
    if (!noble::mvar::WriteMapVariant(variant, bytes, hashRecomputed, error)) {
        std::printf("FAIL  %s: %s\n", in.filename().string().c_str(), error.c_str());
        return 1;
    }
    std::ofstream file(out, std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    file.close();

    const noble::mvar::MapVariant check = noble::mvar::ReadMapVariantBytes(bytes, out.filename().string());
    const bool ok = check.valid && check.header.objects[slot].positionRaw[axis] == before + 1 &&
                    check.header.objects.size() == variant.header.objects.size() && hashRecomputed;
    std::printf("%s: slot %d axis %d raw %u -> %u, %zu bytes, hash %s, reparse %s (%s)\n", ok ? "OK  " : "FAIL", slot,
                axis, before, before + 1, bytes.size(), hashRecomputed ? "recomputed" : "preserved",
                check.valid ? "valid" : "invalid", check.error.c_str());
    return ok ? 0 : 1;
}

int Assemble(const std::filesystem::path& chunkFile, const std::filesystem::path& tmpl,
             const std::filesystem::path& dir, const std::string& name) {
    const std::vector<uint8_t> chunk = ReadFile(chunkFile);
    std::string detail;
    const std::filesystem::path out = noble::mvar::SaveVariantChunkToLibrary(chunk, tmpl, name, dir, detail);
    if (out.empty()) {
        std::printf("FAIL  %s\n", detail.c_str());
        return 1;
    }
    std::printf("OK  %s\n", out.string().c_str());
    return 0;
}

// Offline check of the placement applier: pretend the first present object was a load-time placement of tag T, then
// place another object with the same tag next to it. Expect one new record at that position.
int PlaceSim(const std::filesystem::path& in, const std::filesystem::path& dir) {
    noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(in);
    if (!variant.valid) {
        std::printf("FAIL  %s\n", variant.error.c_str());
        return 1;
    }
    int first = -1;
    for (size_t i = 0; i < variant.header.objects.size(); ++i) {
        if (variant.header.objects[i].present) { first = static_cast<int>(i); break; }
    }
    if (first < 0) { std::printf("FAIL  no objects\n"); return 1; }
    const auto& object = variant.header.objects[first];
    std::vector<noble::mvar::WorldPlacement> placements(2);
    placements[0].tag = 0xDEADBEEF;
    std::memcpy(placements[0].position, object.position, sizeof(placements[0].position));
    placements[1].tag = 0xDEADBEEF;
    placements[1].position[0] = object.position[0] + 1.0f;
    placements[1].position[1] = object.position[1];
    placements[1].position[2] = object.position[2];
    std::string error;
    const int added = noble::mvar::ApplyWorldPlacements(variant, placements, error);
    std::printf("added %d, present %d, error '%s'\n", added, variant.header.presentObjects, error.c_str());
    std::string saveError;
    const std::filesystem::path out = dir / "placed.json";
    if (!noble::mvar::SaveMapVariantJson(variant, out, "Placed", saveError)) {
        std::printf("FAIL  %s\n", saveError.c_str());
        return 1;
    }
    std::printf("OK  %s (expected present %d)\n", out.string().c_str(), variant.header.presentObjects);
    return added == 1 ? 0 : 1;
}

int Library(const std::filesystem::path& in, const std::filesystem::path& dir, const std::string& name) {
    std::string detail;
    const std::string useName = name.empty() ? noble::mvar::VariantTitle(in) : name;
    const std::filesystem::path out = noble::mvar::SaveVariantToLibrary(in, useName, dir, detail);
    if (out.empty()) {
        std::printf("FAIL  %s: %s\n", in.filename().string().c_str(), detail.c_str());
        return 1;
    }
    std::printf("OK  %s -> %s\n", in.filename().string().c_str(), out.string().c_str());
    return 0;
}

int SaveJson(const std::filesystem::path& in, const std::filesystem::path& out) {
    const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(in);
    std::string error;
    if (!noble::mvar::SaveMapVariantJson(variant, out, std::string(), error)) {
        std::printf("FAIL  %s: %s\n", in.filename().string().c_str(), error.c_str());
        return 1;
    }
    // Read the JSON back through the same base64 field FrontEndMenu uses and compare with the container.
    std::ifstream file(out, std::ios::binary);
    const std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const std::vector<uint8_t> decoded = Base64Decode(ExtractJsonData(json));
    const bool match = decoded == variant.fileBytes;
    std::printf("%s: %s (%zu bytes of JSON, %zu bytes of variant)\n", match ? "OK  " : "FAIL",
                out.filename().string().c_str(), json.size(), decoded.size());
    return match ? 0 : 1;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::printf("usage: mvar-tool <file.mvar | directory> | -info <file.mvar> | -save <in.mvar> <out.json>\n");
        return 2;
    }
    const std::string mode = argv[1];
    if (mode == "-info" && argc >= 3) return Info(argv[2]);
    if (mode == "-save" && argc >= 4) return SaveJson(argv[2], argv[3]);
    if (mode == "-write" && argc >= 4) return WriteBytes(argv[2], argv[3]);
    if (mode == "-nudge" && argc >= 6) return Nudge(argv[2], argv[3], std::atoi(argv[4]), std::atoi(argv[5]));
    if (mode == "-library" && argc >= 4) return Library(argv[2], argv[3], argc >= 5 ? argv[4] : "");
    if (mode == "-assemble" && argc >= 6) return Assemble(argv[2], argv[3], argv[4], argv[5]);
    if (mode == "-place-sim" && argc >= 4) return PlaceSim(argv[2], argv[3]);

    std::vector<std::filesystem::path> files;
    std::error_code ec;
    const std::filesystem::path target = argv[1];
    if (std::filesystem::is_directory(target, ec)) {
        for (const auto& entry : std::filesystem::directory_iterator(target, ec)) {
            if (entry.is_regular_file(ec) && entry.path().extension() == ".mvar") files.push_back(entry.path());
        }
        std::sort(files.begin(), files.end());
    } else {
        files.push_back(target);
    }
    if (files.empty()) {
        std::printf("no .mvar files found\n");
        return 1;
    }
    return RoundTrip(files);
}