#include "MvarDump.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

namespace noble {

// Finds the loaded map variant inside the running process.
//
// A map variant is a BLF container: "_blf" at +0x00, "chdr" at +0x30 and the "mvar" body at +0x2F0, then the variant
// itself (0x7329 bytes upward). The engine has to keep the one it loaded (Forge re-saves it), so walking our own
// committed memory for that signature finds it whatever structure the engine wraps it in.
int DumpMapVariantCandidates(const std::filesystem::path& outDir) {
    std::error_code ec;
    std::filesystem::create_directories(outDir, ec);

    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    const uintptr_t minAddr = reinterpret_cast<uintptr_t>(si.lpMinimumApplicationAddress);
    const uintptr_t maxAddr = reinterpret_cast<uintptr_t>(si.lpMaximumApplicationAddress);

    int hits = 0;
    int regions = 0;
    uintptr_t addr = minAddr;
    MEMORY_BASIC_INFORMATION mbi{};
    while (addr < maxAddr && VirtualQuery(reinterpret_cast<void*>(addr), &mbi, sizeof(mbi)) == sizeof(mbi)) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        const size_t size = mbi.RegionSize;
        addr = base + size;
        if (mbi.State != MEM_COMMIT) continue;
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) continue;
        const DWORD p = mbi.Protect & 0xFF;
        if (p != PAGE_READONLY && p != PAGE_READWRITE && p != PAGE_WRITECOPY &&
            p != PAGE_EXECUTE_READ && p != PAGE_EXECUTE_READWRITE && p != PAGE_EXECUTE_WRITECOPY) continue;
        ++regions;

        // Only the signature's first bytes are needed to find a candidate.
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(base);
        const size_t limit = size >= 0x300 ? size - 0x300 : 0;
        for (size_t i = 0; i + 0x300 < limit; i += 4) {
            if (bytes[i] != '_' || std::memcmp(bytes + i, "_blf", 4) != 0) continue;
            if (std::memcmp(bytes + i + 0x30, "chdr", 4) != 0) continue;
            if (std::memcmp(bytes + i + 0x2F0, "mvar", 4) != 0) continue;
            const size_t avail = size - i;
            const size_t dumpSize = avail < 0x10000 ? avail : 0x10000;
            wchar_t name[64] = {};
            swprintf_s(name, L"mvar_%04d_%p.bin", hits, reinterpret_cast<void*>(base + i));
            const std::filesystem::path out = outDir / name;
            std::ofstream file(out, std::ios::binary | std::ios::trunc);
            file.write(reinterpret_cast<const char*>(bytes + i), static_cast<std::streamsize>(dumpSize));
            file.close();
            int32_t mapId = 0;
            std::memcpy(&mapId, bytes + i + 0x6C, sizeof(mapId));
            NOBLE_INFO("MvarDump: candidate {} at 0x{:X} ({} bytes, base map id {}) -> '{}'", hits,
                       base + i, dumpSize, mapId, out.string());
            ++hits;
            if (hits >= 32) break;
        }
        if (hits >= 32) break;
    }
    NOBLE_INFO("MvarDump: scanned {} committed region(s), {} map variant candidate(s) in '{}'", regions, hits,
               outDir.string());
    return hits;
}

} // namespace noble
