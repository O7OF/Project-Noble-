#include "EngineLimits.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <system_error>

#ifdef NOBLE_LIMITS_CORE_ONLY
// Off-device tests build only the hashing / parsing / applying core.
#define NOBLE_INFO(...) ((void)0)
#define NOBLE_WARN(...) ((void)0)
#else
#include "common/Logger.hpp"
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace noble::limits {

// ---------------------------------------------------------------- SHA-256 (FIPS 180-4)
namespace {

constexpr uint32_t kK[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

inline uint32_t Rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

void Block(uint32_t h[8], const uint8_t* p) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (uint32_t(p[i * 4]) << 24) | (uint32_t(p[i * 4 + 1]) << 16) | (uint32_t(p[i * 4 + 2]) << 8) | uint32_t(p[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        const uint32_t s0 = Rotr(w[i - 15], 7) ^ Rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const uint32_t s1 = Rotr(w[i - 2], 17) ^ Rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int i = 0; i < 64; ++i) {
        const uint32_t t1 = hh + (Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25)) + ((e & f) ^ (~e & g)) + kK[i] + w[i];
        const uint32_t t2 = (Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
}

uint32_t ReadU32(const Bytes& b, size_t pos) {
    return uint32_t(b[pos]) | (uint32_t(b[pos + 1]) << 8) | (uint32_t(b[pos + 2]) << 16) | (uint32_t(b[pos + 3]) << 24);
}

} // namespace

Digest Sha256(const uint8_t* data, size_t size) {
    uint32_t h[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
    size_t i = 0;
    for (; i + 64 <= size; i += 64) Block(h, data + i);
    uint8_t tail[128] = {};
    const size_t rem = size - i;
    if (rem) std::memcpy(tail, data + i, rem);
    tail[rem] = 0x80;
    const size_t tailLen = rem < 56 ? 64 : 128;
    const uint64_t bits = uint64_t(size) * 8;
    for (int k = 0; k < 8; ++k) tail[tailLen - 1 - k] = uint8_t(bits >> (8 * k));
    Block(h, tail);
    if (tailLen == 128) Block(h, tail + 64);
    Digest out{};
    for (int k = 0; k < 8; ++k) {
        out[k * 4] = uint8_t(h[k] >> 24); out[k * 4 + 1] = uint8_t(h[k] >> 16);
        out[k * 4 + 2] = uint8_t(h[k] >> 8); out[k * 4 + 3] = uint8_t(h[k]);
    }
    return out;
}

std::string DigestHex(const Digest& d) {
    static const char* kHex = "0123456789abcdef";
    std::string s;
    s.reserve(64);
    for (uint8_t b : d) { s += kHex[b >> 4]; s += kHex[b & 15]; }
    return s;
}

// ---------------------------------------------------------------- .nlp parsing / applying
bool ParseNlp(const Bytes& nlp, NlpPatch& out, std::string& error) {
    out = NlpPatch{};
    constexpr size_t kHeader = 4 + 4 * 3 + (4 + 32) * 2 + 4 * 2;   // 104 bytes
    if (nlp.size() < kHeader || std::memcmp(nlp.data(), "NLP1", 4) != 0) {
        error = "not an NLP1 patch file";
        return false;
    }
    size_t pos = 4;
    const uint32_t version = ReadU32(nlp, pos); pos += 4;
    if (version != 1) { error = "unsupported patch format version " + std::to_string(version); return false; }
    out.profileId = ReadU32(nlp, pos); pos += 4;
    const uint32_t rowCount = ReadU32(nlp, pos); pos += 4;
    out.stockLength = ReadU32(nlp, pos); pos += 4;
    std::memcpy(out.stockSha.data(), &nlp[pos], 32); pos += 32;
    out.outputLength = ReadU32(nlp, pos); pos += 4;
    std::memcpy(out.outputSha.data(), &nlp[pos], 32); pos += 32;
    const uint32_t appendOffset = ReadU32(nlp, pos); pos += 4;
    out.appendLength = ReadU32(nlp, pos); pos += 4;

    if (appendOffset != out.stockLength || uint64_t(out.stockLength) + out.appendLength != out.outputLength) {
        error = "patch header lengths are inconsistent";
        return false;
    }
    out.rows.reserve(rowCount);
    uint64_t prevEnd = 0;
    for (uint32_t r = 0; r < rowCount; ++r) {
        if (pos + 8 > nlp.size()) { error = "patch file is truncated (rows)"; return false; }
        NlpPatch::Row row{};
        row.offset = ReadU32(nlp, pos); pos += 4;
        row.length = ReadU32(nlp, pos); pos += 4;
        if (row.length == 0 || pos + uint64_t(row.length) * 2 > nlp.size()) { error = "patch file is truncated (row data)"; return false; }
        if (row.offset < prevEnd || uint64_t(row.offset) + row.length > out.stockLength) {
            error = "patch rows overlap or run past the stock file";
            return false;
        }
        prevEnd = uint64_t(row.offset) + row.length;
        row.stockPos = pos; pos += row.length;
        row.newPos = pos; pos += row.length;
        out.rows.push_back(row);
    }
    if (pos + out.appendLength != nlp.size()) { error = "patch file size does not match its header"; return false; }
    out.appendPos = pos;
    return true;
}

bool ApplyNlp(const Bytes& stock, const Bytes& nlp, const NlpPatch& patch, Bytes& output, std::string& error) {
    if (stock.size() != patch.stockLength) {
        error = "installed haloreach.dll is a different build (size " + std::to_string(stock.size()) + ")";
        return false;
    }
    if (Sha256(stock.data(), stock.size()) != patch.stockSha) {
        error = "installed haloreach.dll is a different build (hash mismatch)";
        return false;
    }
    output.assign(stock.begin(), stock.end());
    for (const NlpPatch::Row& row : patch.rows) {
        if (std::memcmp(&output[row.offset], &nlp[row.stockPos], row.length) != 0) {
            char buf[96];
            std::snprintf(buf, sizeof(buf), "stock bytes differ at file offset 0x%08X", row.offset);
            error = buf;
            output.clear();
            return false;
        }
        std::memcpy(&output[row.offset], &nlp[row.newPos], row.length);
    }
    output.insert(output.end(), nlp.begin() + patch.appendPos, nlp.begin() + patch.appendPos + patch.appendLength);
    if (output.size() != patch.outputLength || Sha256(output.data(), output.size()) != patch.outputSha) {
        error = "patched result does not match the expected hash";
        output.clear();
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- profiles
namespace {
Profile s_active = Profile::Stock;
}

Profile ActiveProfile() { return s_active; }

const char* ProfileName(Profile p) {
    switch (p) {
    case Profile::V16: return "Extended (V16)";
    case Profile::V16Particles: return "Extended + Particles";
    case Profile::Full: return "Full (V16 + particles + light volumes)";
    default: return "Stock";
    }
}

// The network id of an engine profile: the game-state layout depends both on the patch profile and (with
// an Extended profile) on the AI actor count, so host and clients have to match on both or they desync.
// High byte = profile id, low 16 bits = actor count (0 = the stock 96, 1,024 = the experimental value).
uint32_t ProfileNetworkId(Profile p, int aiActors) {
    const uint32_t actors = aiActors == 256 || aiActors == 512 || aiActors == 1024 ? static_cast<uint32_t>(aiActors) : 0u;
    return (static_cast<uint32_t>(p) << 24) | actors;
}

uint32_t LocalNetworkId() {
    int actors = 0;
    wchar_t env[16] = {};
    if (s_active != Profile::Stock && GetEnvironmentVariableW(L"NOBLE_AI_ACTORS", env, 16) > 0) actors = _wtoi(env);
    return ProfileNetworkId(s_active, actors);
}

std::string SaveFolderSuffix(Profile p) {
    switch (p) {
    case Profile::V16: return "_ext_v16";
    case Profile::V16Particles: return "_ext_v16p";
    case Profile::Full: return "_ext_full";
    default: return "";
    }
}

Profile ProfileFromInt(int value) {
    switch (value) {
    case 1: return Profile::V16;
    case 2: return Profile::V16Particles;
    case 3: return Profile::Full;
    default: return Profile::Stock;
    }
}

#ifndef NOBLE_LIMITS_CORE_ONLY
// RtlAddFunctionTable lives in ntdll; winnt.h declares the RUNTIME_FUNCTION layout.
extern "C" NTSYSAPI BOOLEAN NTAPI RtlAddFunctionTable(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD_PTR BaseAddress);

void RegisterUnwindTables(void* engineBase) {
    if (s_active != Profile::Full || !engineBase) return;
    // New code in .deltrac (RVA, end RVA, original UNWIND_INFO RVA). The unwind RVAs are the stock routines'
    // entries: make_v16_particles.py copies the code unchanged, so the prologues still match.
    struct LvFunction { uint32_t begin, end, unwind; };
    static const LvFunction kFunctions[] = {
        { 0xAAE8460, 0xAAE8976, 0xAA49D8 },   // effect / particle initialization
        { 0xAAE8980, 0xAAE89FB, 0xA9A878 },   // message queue admission
        { 0xAAE8A00, 0xAAE8AB7, 0xA9A83C },   // message queue reset
        { 0xAAE8AC0, 0xAAE8C1B, 0xA9ABF4 },   // message queue relocation callback
        { 0xAAE8C20, 0xAAE8CE2, 0xA9A880 },   // message queue consumer
        { 0xAAE8DA0, 0xAAE8E5E, 0xA9ABD8 },   // message queue filtering
    };
    const uint8_t* code = static_cast<const uint8_t*>(engineBase);
    if (code[kFunctions[0].begin] != 0x48 || code[kFunctions[0].begin + 1] != 0x8B ||
        code[kFunctions[0].begin + 2] != 0xC4) {
        NOBLE_WARN("Engine limits: light-volume code not found at 0x{:X}; unwind tables not registered",
                   kFunctions[0].begin);
        return;
    }
    static RUNTIME_FUNCTION table[std::size(kFunctions)] = {};
    for (size_t i = 0; i < std::size(kFunctions); ++i)
        table[i] = RUNTIME_FUNCTION{ kFunctions[i].begin, kFunctions[i].end, kFunctions[i].unwind };
    if (RtlAddFunctionTable(table, static_cast<DWORD>(std::size(kFunctions)), reinterpret_cast<DWORD_PTR>(engineBase)))
        NOBLE_INFO("Engine limits: registered {} unwind entries for the light-volume routines", std::size(kFunctions));
    else
        NOBLE_WARN("Engine limits: RtlAddFunctionTable failed for the light-volume routines");
}
#else
void RegisterUnwindTables(void*) {}
#endif

// ---------------------------------------------------------------- file helpers / PrepareEngineDll
namespace {

bool ReadAll(const fs::path& path, Bytes& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    const std::streamoff size = f.tellg();
    if (size < 0) return false;
    out.resize(static_cast<size_t>(size));
    f.seekg(0, std::ios::beg);
    return size == 0 || static_cast<bool>(f.read(reinterpret_cast<char*>(out.data()), size));
}

const char* ProfileFileStem(Profile p) {
    switch (p) {
    case Profile::V16: return "v16";
    case Profile::V16Particles: return "v16p";
    case Profile::Full: return "full";
    default: return "";
    }
}

#ifndef NOBLE_LIMITS_CORE_ONLY
fs::path ExeDirectory() {
    wchar_t buf[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, buf, MAX_PATH) == 0) return {};
    return fs::path(buf).parent_path();
}

// %LOCALAPPDATA%\ProjectNoble\engine, or <exe>\data\cache\engine when LOCALAPPDATA is unavailable.
fs::path CacheRoot() {
    wchar_t buf[MAX_PATH] = {};
    const DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) return fs::path(buf) / L"ProjectNoble" / L"engine";
    return ExeDirectory() / L"data" / L"cache" / L"engine";
}

// Writes the patched copy next to `target` and moves it into place. The menu process can still have an
// older copy loaded while a relaunched match process rebuilds it, so the replace is retried; if the file
// stays locked, the verified bytes are kept under a per-process name and loaded from there instead.
bool WriteCopy(const fs::path& target, const Bytes& data, fs::path& written, std::string& error) {
    std::error_code ec;
    fs::create_directories(target.parent_path(), ec);
    const std::wstring pid = std::to_wstring(GetCurrentProcessId());
    fs::path tmp = target;
    tmp += L".tmp" + pid;
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f || !f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()))) {
            error = "could not write " + tmp.string();
            return false;
        }
    }
    DWORD err = 0;
    for (int attempt = 0; attempt < 5; ++attempt) {
        if (MoveFileExW(tmp.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            written = target;
            // Clean up per-process copies left by earlier runs (skipped while still loaded).
            for (const auto& entry : fs::directory_iterator(target.parent_path(), ec)) {
                const std::wstring name = entry.path().filename().wstring();
                if (name.rfind(L"haloreach.", 0) == 0 && name != L"haloreach.dll" && entry.path().extension() == L".dll") {
                    std::error_code rmEc;
                    fs::remove(entry.path(), rmEc);
                }
            }
            return true;
        }
        err = GetLastError();
        Sleep(150);
    }
    const fs::path alt = target.parent_path() / (L"haloreach." + pid + L".dll");
    if (MoveFileExW(tmp.c_str(), alt.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        NOBLE_WARN("Engine limits: {} is in use (Win32 error {}); loading the verified copy {}", target.string(),
            static_cast<unsigned long>(err), alt.string());
        written = alt;
        return true;
    }
    err = GetLastError();
    fs::remove(tmp, ec);
    error = "could not move the patched copy into place (Win32 error " + std::to_string(err) + ")";
    return false;
}
#endif

#ifndef NOBLE_LIMITS_CORE_ONLY
PrepareResult Fallback(const fs::path& stockDll, Profile requested, std::string reason) {
    NOBLE_WARN("Engine limits: requested {}, using Stock: {}", ProfileName(requested), reason);
    s_active = Profile::Stock;
    return PrepareResult{ stockDll, Profile::Stock, std::move(reason) };
}
#endif

} // namespace

PrepareResult PrepareEngineDll(const fs::path& stockDll, Profile requested) {
#ifdef NOBLE_LIMITS_CORE_ONLY
    (void)ProfileFileStem;
    (void)ReadAll;
    s_active = Profile::Stock;
    return PrepareResult{ stockDll, Profile::Stock, requested == Profile::Stock ? "" : "core-only build" };
#else
    s_active = Profile::Stock;
    if (requested == Profile::Stock) {
        NOBLE_INFO("Engine limits: Stock (installed haloreach.dll, unmodified)");
        return PrepareResult{ stockDll, Profile::Stock, {} };
    }
    try {
        const fs::path nlpPath = ExeDirectory() / L"data" / L"limits" / (std::string(ProfileFileStem(requested)) + ".nlp");
        Bytes nlp;
        if (!ReadAll(nlpPath, nlp)) return Fallback(stockDll, requested, "patch file missing: " + nlpPath.string());
        NlpPatch patch;
        std::string error;
        if (!ParseNlp(nlp, patch, error)) return Fallback(stockDll, requested, nlpPath.filename().string() + ": " + error);
        if (patch.profileId != static_cast<uint32_t>(requested)) {
            return Fallback(stockDll, requested, nlpPath.filename().string() + " is for a different profile");
        }

        const std::string outHex = DigestHex(patch.outputSha);
        const fs::path target = CacheRoot() / (std::string(ProfileFileStem(requested)) + "-" + outHex.substr(0, 12)) / L"haloreach.dll";

        // Reuse a copy built earlier, but only if it is exactly the expected output.
        std::error_code ec;
        if (fs::exists(target, ec) && fs::file_size(target, ec) == patch.outputLength) {
            Bytes cached;
            if (ReadAll(target, cached) && Sha256(cached.data(), cached.size()) == patch.outputSha) {
                NOBLE_INFO("Engine limits: {} (cached copy {}, sha256 {})", ProfileName(requested), target.string(), outHex);
                s_active = requested;
                return PrepareResult{ target, requested, {} };
            }
            NOBLE_WARN("Engine limits: cached copy {} is damaged; rebuilding", target.string());
        }

        Bytes stock;
        if (!ReadAll(stockDll, stock)) return Fallback(stockDll, requested, "could not read " + stockDll.string());
        Bytes output;
        if (!ApplyNlp(stock, nlp, patch, output, error)) return Fallback(stockDll, requested, error);
        fs::path written;
        if (!WriteCopy(target, output, written, error)) return Fallback(stockDll, requested, error);

        NOBLE_INFO("Engine limits: {} (built {} from stock sha256 {}, output sha256 {})", ProfileName(requested),
            written.string(), DigestHex(patch.stockSha), outHex);
        s_active = requested;
        return PrepareResult{ written, requested, {} };
    } catch (const std::exception& e) {
        return Fallback(stockDll, requested, std::string("unexpected error: ") + e.what());
    }
#endif
}

} // namespace noble::limits
