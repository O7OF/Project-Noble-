#include "MvarDump.hpp"
#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"
#include "mvar/MapVariantLibrary.hpp"
#include "mvar/MapVariant.hpp"

#include <map>
#include <set>
#include <functional>
#include <cstdio>

#include <windows.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

namespace noble {

// The engine's save leaves a zero-byte temp behind, so watch what it actually writes: hook WriteFile in this process,
// and when the engine hands over a map-variant-sized buffer for a file under the profile, keep a copy. That tells us
// whether the bytes exist at the point of writing (then the failure is only the file) or never get produced.
namespace {
using WriteFileFn = BOOL(WINAPI*)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
WriteFileFn s_origWriteFile = nullptr;

std::string HandlePath(HANDLE h) {
    wchar_t buf[MAX_PATH * 4] = {};
    const DWORD len = GetFinalPathNameByHandleW(h, buf, static_cast<DWORD>(std::size(buf)), FILE_NAME_NORMALIZED);
    if (len == 0 || len >= std::size(buf)) return {};
    char out[MAX_PATH * 4] = {};
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, out, sizeof(out) - 1, nullptr, nullptr);
    return out;
}

// Capture itself: plain C++ (ofstream unwinds), so it must stay outside the __try below.
//
// The engine writes its saves in chunks (a header, then records), so every write to a file under the player's save
// folder is appended to one capture per handle: the concatenation is the file the engine would have produced.
static void WriteCaptureFile(HANDLE hFile, const void* buffer, DWORD nBytes, BOOL ok) {
    static std::map<uintptr_t, std::pair<std::string, std::string>> s_files;   // handle -> (path, capture file)
    const std::string path = hFile ? HandlePath(hFile) : std::string();
    const std::filesystem::path out = std::filesystem::path("data") / "mvar_write";
    std::error_code ec;
    std::filesystem::create_directories(out, ec);

    bool interesting = false;
    for (const char* marker : {"HaloReach", "map_variants", "autosave"}) {
        if (path.find(marker) != std::string::npos) { interesting = true; break; }
    }
    if (!interesting && nBytes < 0x4000) return;

    std::string capture;
    auto it = s_files.find(reinterpret_cast<uintptr_t>(hFile));
    if (it != s_files.end()) {
        capture = it->second.second;
    } else {
        static int index = 0;
        char name[64] = {};
        std::snprintf(name, sizeof(name), "capture_%03d.bin", ++index);
        capture = (out / name).string();
        s_files[reinterpret_cast<uintptr_t>(hFile)] = {path, capture};
        NOBLE_INFO("MvarWrite: handle 0x{:X} is '{}' -> capture '{}'", reinterpret_cast<uintptr_t>(hFile), path, capture);
    }
    std::ofstream file(capture, std::ios::binary | std::ios::app);
    file.write(static_cast<const char*>(buffer), nBytes);
    file.close();
    const unsigned char* first = static_cast<const unsigned char*>(buffer);
    const bool container = nBytes >= 4 && std::memcmp(first, "_blf", 4) == 0;
    NOBLE_INFO("MvarWrite: +{} byte(s) to '{}' (ok={}, starts '_blf'={}) total capture '{}'", nBytes, path, ok ? 1 : 0,
               container ? 1 : 0, capture);

    // If the engine's own save produced a complete container, adopt it into Noble's library. That is the path that
    // carries Forge edits: the game writes its commit, and the file shows up in the maps menu without a restart.
    static std::set<std::string> s_adopted;
    std::error_code sizeEc;
    if (std::filesystem::file_size(capture, sizeEc) >= 0x7329 && !sizeEc) {
        std::ifstream in(capture, std::ios::binary);
        std::vector<uint8_t> bytes(0x7329);
        in.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (in.gcount() == static_cast<std::streamsize>(bytes.size()) &&
            std::memcmp(bytes.data(), "_blf", 4) == 0 && std::memcmp(bytes.data() + 0x2F0, "mvar", 4) == 0) {
            const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariantBytes(bytes, "capture");
            if (variant.valid) {
                const std::string key = std::to_string(variant.header.mapId) + ":" + std::to_string(bytes.size()) + ":" +
                                        std::to_string(std::hash<std::string>{}(std::string(
                                            reinterpret_cast<const char*>(bytes.data() + 0x2F0), 0x100)));
                if (s_adopted.insert(key).second) {
                    std::error_code ec2;
                    const std::filesystem::path library = std::filesystem::path("data") / "map_variants";
                    std::filesystem::create_directories(library, ec2);
                    char name[64] = {};
                    std::snprintf(name, sizeof(name), "captured_%08X.mvar", variant.header.mapId);
                    const std::filesystem::path adopted = library / name;
                    std::ofstream(library / name, std::ios::binary | std::ios::trunc)
                        .write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
                    NOBLE_INFO("MvarWrite: adopted the engine's save as '{}'", adopted.string());
                }
            }
        }
    }
}

static void CaptureWrite(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytes, BOOL ok) {
    // Only raw reads here: a bad pointer must not take the process down, and SEH cannot unwind C++ objects.
    __try {
        if (lpBuffer && nBytes > 0) {
            volatile unsigned char probe = *static_cast<const unsigned char*>(lpBuffer);
            (void)probe;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
    WriteCaptureFile(hFile, lpBuffer, nBytes, ok);
}

BOOL WINAPI HookWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytes, LPDWORD lpWritten, LPOVERLAPPED ov) {
    const BOOL ok = s_origWriteFile(hFile, lpBuffer, nBytes, lpWritten, ov);
    // Every write is considered; CaptureWrite drops the ones that are neither large nor in a save folder.
    if (nBytes > 0 && lpBuffer) CaptureWrite(hFile, lpBuffer, nBytes, ok);
    return ok;
}
} // namespace

bool InstallWriteWatch() {
    if (s_origWriteFile) return true;
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!kernel32) return false;
    void* target = reinterpret_cast<void*>(GetProcAddress(kernel32, "WriteFile"));
    if (!target) return false;
    if (!HookManager::InstallHook(target, reinterpret_cast<void*>(&HookWriteFile),
                                  reinterpret_cast<void**>(&s_origWriteFile), "trace_write_file")) {
        NOBLE_WARN("MvarWrite: could not hook WriteFile");
        return false;
    }
    NOBLE_INFO("MvarWrite: watching WriteFile for map variant buffers");
    return true;
}


// Finds the loaded map variant inside the running process.
//
// A map variant is a BLF container: "_blf" at +0x00, "chdr" at +0x30 and the "mvar" body at +0x2F0, then the variant
// itself (0x7329 bytes upward). The engine has to keep the one it loaded (Forge re-saves it), so walking our own
// committed memory for that signature finds it whatever structure the engine wraps it in.
bool InstallWriteWatch();

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
