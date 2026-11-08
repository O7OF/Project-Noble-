#include "PoolReport.hpp"

#include <algorithm>
#include <bit>
#include <chrono>
#include <cstring>
#include <map>
#include <thread>

#ifdef NOBLE_POOLS_CORE_ONLY
#define NOBLE_INFO(...) ((void)0)
#define NOBLE_WARN(...) ((void)0)
#else
#include "common/Logger.hpp"
#include <windows.h>
#include <shellapi.h>
#endif

namespace noble::pools {

namespace {

constexpr uint32_t kSignature = 0x64407440;   // written at header+0x28 by the data-array initializer
constexpr size_t kHeaderSize = 0x70;

bool PlausibleName(const uint8_t* p) {
    size_t n = 0;
    for (; n < 32 && p[n]; ++n) {
        const uint8_t c = p[n];
        if (c < 0x20 || c > 0x7E) return false;
    }
    return n > 0 && n < 32;
}

// POD form of a parsed header (no std::string, so it can be filled inside __try).
struct PoolInfoRaw {
    char name[32];
    uint32_t capacity;
    uint32_t used;          // UINT32_MAX when the bitmap could not be read
    uint64_t recordSize;
};

bool ParseHeaderRaw(const uint8_t* regionBase, size_t regionSize, size_t offset, PoolInfoRaw& out) {
    if (offset < 0x28 || offset - 0x28 + kHeaderSize > regionSize) return false;
    const uint8_t* h = regionBase + (offset - 0x28);
    if (!PlausibleName(h)) return false;
    uint64_t recordSize = 0;
    uint32_t capacity = 0;
    int64_t bitmapOffset = 0;
    std::memcpy(&recordSize, h + 0x20, 8);
    std::memcpy(&capacity, h + 0x2C, 4);
    std::memcpy(&bitmapOffset, h + 0x68, 8);
    if (capacity == 0 || capacity > 4'000'000 || recordSize == 0 || recordSize > 0x100000) return false;
    std::memset(out.name, 0, sizeof(out.name));
    std::memcpy(out.name, h, 31);
    out.capacity = capacity;
    out.recordSize = recordSize;
    out.used = UINT32_MAX;
    const size_t headerPos = offset - 0x28;
    const uint64_t bitmapBytes = (static_cast<uint64_t>(capacity) + 31) / 32 * 4;
    if (bitmapOffset >= static_cast<int64_t>(kHeaderSize) &&
        headerPos + static_cast<uint64_t>(bitmapOffset) + bitmapBytes <= regionSize) {
        const uint8_t* bm = h + bitmapOffset;
        uint32_t used = 0;
        for (uint64_t i = 0; i < bitmapBytes; i += 4) {
            uint32_t word = 0;
            std::memcpy(&word, bm + i, 4);
            used += static_cast<uint32_t>(std::popcount(word));
        }
        if (used <= capacity) out.used = used;
    }
    return true;
}

// Parses one header candidate inside [regionBase, regionBase + regionSize). Returns false if implausible.
bool ParseHeader(const uint8_t* regionBase, size_t regionSize, size_t offset, PoolInfo& out) {
    if (offset < 0x28 || offset - 0x28 + kHeaderSize > regionSize) return false;
    const uint8_t* h = regionBase + (offset - 0x28);
    if (!PlausibleName(h)) return false;
    uint64_t recordSize = 0;
    uint32_t capacity = 0;
    int64_t dataOffset = 0, bitmapOffset = 0;
    std::memcpy(&recordSize, h + 0x20, 8);
    std::memcpy(&capacity, h + 0x2C, 4);
    std::memcpy(&dataOffset, h + 0x60, 8);
    std::memcpy(&bitmapOffset, h + 0x68, 8);
    if (capacity == 0 || capacity > 4'000'000 || recordSize == 0 || recordSize > 0x100000) return false;

    out = PoolInfo{};
    out.name.assign(reinterpret_cast<const char*>(h));
    out.capacity = capacity;
    out.recordSize = recordSize;
    out.used = UINT32_MAX;
    const size_t headerPos = offset - 0x28;
    const uint64_t bitmapBytes = (static_cast<uint64_t>(capacity) + 31) / 32 * 4;
    if (bitmapOffset >= static_cast<int64_t>(kHeaderSize) &&
        headerPos + static_cast<uint64_t>(bitmapOffset) + bitmapBytes <= regionSize) {
        const uint8_t* bm = h + bitmapOffset;
        uint32_t used = 0;
        for (uint64_t i = 0; i < bitmapBytes; i += 4) {
            uint32_t word = 0;
            std::memcpy(&word, bm + i, 4);
            used += static_cast<uint32_t>(std::popcount(word));
        }
        if (used <= capacity) out.used = used;
    }
    return true;
}

std::string Grouped(uint64_t v) {
    std::string s = std::to_string(v);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(static_cast<size_t>(i), ",");
    return s;
}

} // namespace

#ifndef NOBLE_POOLS_CORE_ONLY
// Raw, exception-guarded scan of one memory region. The engine frees and remaps memory while maps load, so
// a block VirtualQuery called readable can disappear before it is read; __try turns that into "skip the
// rest of this region" instead of a crash. Kept free of C++ objects (required for __try).
struct RawHit {
    uintptr_t header;
    char name[32];
    uint64_t recordSize;
    uint32_t capacity;
    uint32_t used;
};

size_t ScanRegionGuarded(const uint8_t* base, size_t size, RawHit* hits, size_t maxHits) {
    size_t count = 0;
    __try {
        for (size_t off = 0x28; off + 4 <= size && count < maxHits; off += 4) {
            if (*reinterpret_cast<const uint32_t*>(base + off) != kSignature) continue;
            PoolInfoRaw raw;
            if (!ParseHeaderRaw(base, size, off, raw)) continue;
            RawHit& h = hits[count++];
            h.header = reinterpret_cast<uintptr_t>(base + off - 0x28);
            std::memcpy(h.name, raw.name, sizeof(h.name));
            h.recordSize = raw.recordSize;
            h.capacity = raw.capacity;
            h.used = raw.used;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // Region went away mid-scan: keep what was found before it.
    }
    return count;
}

std::vector<PoolInfo> Scan(uintptr_t engineBase, size_t engineSize) {
    std::map<std::string, PoolInfo> best;
    std::vector<RawHit> hits(4096);
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    auto* addr = static_cast<uint8_t*>(si.lpMinimumApplicationAddress);
    auto* const end = static_cast<uint8_t*>(si.lpMaximumApplicationAddress);
    MEMORY_BASIC_INFORMATION mbi{};
    while (addr < end && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        const DWORD prot = mbi.Protect & 0xFF;
        const bool readable = mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_GUARD) &&
            (prot == PAGE_READWRITE || prot == PAGE_WRITECOPY || prot == PAGE_EXECUTE_READWRITE || prot == PAGE_READONLY);
        if (readable && mbi.RegionSize >= kHeaderSize) {
            const size_t n = ScanRegionGuarded(static_cast<const uint8_t*>(mbi.BaseAddress), mbi.RegionSize, hits.data(), hits.size());
            for (size_t i = 0; i < n; ++i) {
                PoolInfo info;
                info.name.assign(hits[i].name, strnlen(hits[i].name, sizeof(hits[i].name)));
                info.capacity = hits[i].capacity;
                info.used = hits[i].used;
                info.recordSize = hits[i].recordSize;
                info.address = hits[i].header;
                info.inEngineImage = info.address >= engineBase && info.address < engineBase + engineSize;
                auto it = best.find(info.name);
                // Prefer a live copy (outside the DLL image), then the busier one.
                if (it == best.end() ||
                    (it->second.inEngineImage && !info.inEngineImage) ||
                    (it->second.inEngineImage == info.inEngineImage && info.used != UINT32_MAX &&
                     (it->second.used == UINT32_MAX || info.used > it->second.used))) {
                    best[info.name] = info;
                }
            }
        }
        addr = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
    }
    std::vector<PoolInfo> out;
    out.reserve(best.size());
    for (auto& [name, info] : best) out.push_back(info);
    return out;
}
#else
std::vector<PoolInfo> Scan(uintptr_t, size_t) { return {}; }
#endif

namespace {

const char* const kKeyPools[] = { "object", "actor", "prop", "squad", "swarm", "command scripts", "object list header",
                                  "effect", "effect event", "effect location", "particle system", "particle", "light volume",
                                  "decal", "contrail", "player", "hs thread" };

uint32_t UsedOf(const std::vector<PoolInfo>& pools, const char* name) {
    for (const auto& p : pools) if (p.name == name) return p.used;
    return UINT32_MAX - 1;
}

void Log(const std::vector<PoolInfo>& pools, const std::string& profile, bool full) {
    // One machine-readable line per pool (tools\limits\test-limits.ps1 parses "POOL|name|used|capacity").
    std::string summary;
    for (const char* key : kKeyPools) {
        for (const auto& p : pools) {
            if (p.name != key) continue;
            summary += (summary.empty() ? "" : " | ") + p.name + " " +
                (p.used == UINT32_MAX ? std::string("?") : Grouped(p.used)) + " / " + Grouped(p.capacity);
        }
    }
    NOBLE_INFO("Engine pools [{}]: {}", profile, summary.empty() ? "(no game state yet)" : summary);
    if (!full) return;
    std::vector<PoolInfo> sorted = pools;
    std::sort(sorted.begin(), sorted.end(), [](const PoolInfo& a, const PoolInfo& b) { return a.name < b.name; });
    for (const auto& p : sorted) {
        NOBLE_INFO("POOL|{}|{}|{}|{}|{}", p.name, p.used == UINT32_MAX ? std::string("?") : std::to_string(p.used),
            p.capacity, p.recordSize, p.inEngineImage ? "image" : "live");
    }
    NOBLE_INFO("Engine pools [{}]: {} pools found", profile, pools.size());
}

} // namespace

void StartReporter(uintptr_t engineBase, size_t engineSize, const std::string& profileName) {
#if defined(NOBLE_DEV_BUILD) && !defined(NOBLE_POOLS_CORE_ONLY)
    int exitAfter = 0;
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (_wcsicmp(argv[i], L"-pool-report-exit") == 0) exitAfter = _wtoi(argv[i + 1]);
        }
        if (argv) LocalFree(argv);
    }
    std::thread([engineBase, engineSize, profileName, exitAfter]() {
        using namespace std::chrono_literals;
        if (exitAfter > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(exitAfter));
            const auto pools = Scan(engineBase, engineSize);
            Log(pools, profileName, true);
            NOBLE_INFO("POOL-REPORT-DONE|{}", profileName);
            spdlog::default_logger()->flush();
            TerminateProcess(GetCurrentProcess(), 0);
            return;
        }
        std::map<std::string, uint32_t> lastCapacity;
        uint32_t lastObjects = UINT32_MAX - 2, lastActors = UINT32_MAX - 2;
        std::this_thread::sleep_for(10s);
        for (;;) {
            const auto pools = Scan(engineBase, engineSize);
            std::map<std::string, uint32_t> capacity;
            for (const auto& p : pools) capacity[p.name] = p.capacity;
            const bool layoutChanged = capacity != lastCapacity;
            const uint32_t objects = UsedOf(pools, "object"), actors = UsedOf(pools, "actor");
            if (!pools.empty() && (layoutChanged || objects != lastObjects || actors != lastActors)) {
                Log(pools, profileName, layoutChanged);
                lastCapacity = std::move(capacity);
                lastObjects = objects;
                lastActors = actors;
            }
            std::this_thread::sleep_for(15s);
        }
    }).detach();
    NOBLE_INFO("Engine pools: reporter started ({})", exitAfter > 0 ? "exit after " + std::to_string(exitAfter) + " s" : "every 15 s");
#else
    (void)engineBase; (void)engineSize; (void)profileName;
    (void)&Log; (void)&UsedOf;
#endif
}

} // namespace noble::pools
