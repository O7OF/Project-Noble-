#include "MemoryDiff.hpp"
#include "common/Logger.hpp"

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace noble {
namespace {

constexpr size_t kPage = 4096;
std::mutex s_scanMutex;
bool s_scanRunning = false;

bool HashPageSafe(const uint8_t* page, uint64_t& out) {
    __try {
        uint64_t hash = 1469598103934665603ull;   // FNV-1a
        for (size_t i = 0; i < kPage; i += 8) {
            uint64_t word = 0;
            std::memcpy(&word, page + i, sizeof(word));
            hash ^= word;
            hash *= 1099511628211ull;
        }
        out = hash;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

uint64_t HashPage(const uint8_t* page) {
    uint64_t hash = 1469598103934665603ull;   // FNV-1a
    for (size_t i = 0; i < kPage; i += 8) {
        uint64_t word = 0;
        std::memcpy(&word, page + i, sizeof(word));
        hash ^= word;
        hash *= 1099511628211ull;
    }
    return hash;
}

struct Range {
    uintptr_t address;
    size_t size;
    DWORD protect;
};

void CollectRanges(std::vector<Range>& ranges) {
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    uintptr_t address = reinterpret_cast<uintptr_t>(si.lpMinimumApplicationAddress);
    const uintptr_t maximum = reinterpret_cast<uintptr_t>(si.lpMaximumApplicationAddress);
    MEMORY_BASIC_INFORMATION info{};
    while (address < maximum && VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info)) == sizeof(info)) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(info.BaseAddress);
        const size_t size = info.RegionSize;
        address = base + size;
        if (info.State != MEM_COMMIT) continue;
        if (info.Protect & (PAGE_NOACCESS | PAGE_GUARD)) continue;
        const DWORD protect = info.Protect & 0xFF;
        if (protect != PAGE_READWRITE && protect != PAGE_WRITECOPY && protect != PAGE_EXECUTE_READWRITE &&
            protect != PAGE_EXECUTE_WRITECOPY) {
            continue;
        }
        ranges.push_back({ base, size, protect });
    }
}

} // namespace

void StopPageScan() {
    std::lock_guard<std::mutex> lock(s_scanMutex);
    s_scanRunning = false;
}

void StartPageScan(int seconds, std::filesystem::path outDir) {
    if (seconds <= 0) return;
    {
        std::lock_guard<std::mutex> lock(s_scanMutex);
        if (s_scanRunning) return;
        s_scanRunning = true;
    }
    std::thread([seconds, outDir] {
        std::error_code ec;
        std::filesystem::create_directories(outDir, ec);
        std::map<uintptr_t, uint64_t> previous;
        int index = 0;
        for (;;) {
            {
                std::lock_guard<std::mutex> lock(s_scanMutex);
                if (!s_scanRunning) return;
            }
            Sleep(static_cast<DWORD>(seconds) * 1000);
            std::vector<Range> ranges;
            CollectRanges(ranges);
            std::map<uintptr_t, uint64_t> current;
            for (const Range& range : ranges) {
                for (size_t offset = 0; offset < range.size; offset += kPage) {
                    const uint8_t* page = reinterpret_cast<const uint8_t*>(range.address + offset);
                    uint64_t hash = 0;
                    if (HashPageSafe(page, hash)) current[range.address + offset] = hash;
                }
            }
            ++index;
            if (!previous.empty()) {
                std::vector<uintptr_t> changed;
                for (const auto& [address, hash] : current) {
                    const auto it = previous.find(address);
                    if (it == previous.end() || it->second != hash) changed.push_back(address);
                }
                if (!changed.empty()) {
                    char name[64] = {};
                    std::snprintf(name, sizeof(name), "scan_%03d.txt", index);
                    std::ofstream report(outDir / name, std::ios::trunc);
                    size_t runStart = 0;
                    int runs = 0;
                    while (runStart < changed.size()) {
                        size_t runEnd = runStart;
                        while (runEnd + 1 < changed.size() && changed[runEnd + 1] == changed[runEnd] + kPage) ++runEnd;
                        const uintptr_t base = changed[runStart];
                        const size_t bytes = (runEnd - runStart + 1) * kPage;
                        report << "0x" << std::hex << base << " " << std::dec << bytes << "\n";
                        std::snprintf(name, sizeof(name), "scan_%03d_%llX.bin", index,
                                      static_cast<unsigned long long>(base));
                        std::ofstream blob(outDir / name, std::ios::binary | std::ios::trunc);
                        blob.write(reinterpret_cast<const char*>(base), static_cast<std::streamsize>(bytes));
                        ++runs;
                        runStart = runEnd + 1;
                    }
                    NOBLE_WARN("MemoryDiff: scan {} changed {} page(s) in {} run(s) -> '{}'", index, changed.size(),
                               runs, outDir.string());
                }
            }
            previous = std::move(current);
        }
    }).detach();
}

} // namespace noble
