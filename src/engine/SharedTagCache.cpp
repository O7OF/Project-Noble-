#include "SharedTagCache.hpp"
#include "hooks/HookManager.hpp"
#include "hooks/SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>
#include <algorithm>
#include <fstream>

namespace noble {

bool SharedTagCache::s_isMounted = false;
bool SharedTagCache::s_hooksInstalled = false;
HMODULE SharedTagCache::s_hReachDll = nullptr;
uintptr_t SharedTagCache::s_baseAddress = 0;
std::filesystem::path SharedTagCache::s_donorMapPath;

HANDLE SharedTagCache::s_hFile = INVALID_HANDLE_VALUE;
HANDLE SharedTagCache::s_hMapping = nullptr;
uint8_t* SharedTagCache::s_mappedView = nullptr;
size_t SharedTagCache::s_fileSize = 0;

uint64_t SharedTagCache::s_virtualBase = 0x1C8980000ULL;
uint32_t SharedTagCache::s_tagIndexFileOffset = 0x2AC48000;
uint32_t SharedTagCache::s_donorTagCount = 0;
uint32_t SharedTagCache::s_donorScenarioDatum = 0x00000008;

std::unordered_map<uint32_t, uint32_t> SharedTagCache::s_tagOffsets;
std::unordered_map<uint32_t, uint32_t> SharedTagCache::s_tagGroups;
std::unordered_map<std::string, uint32_t> SharedTagCache::s_tagPathToDatum;
std::vector<PaletteItem> SharedTagCache::s_paletteItems;

SharedTagCache::TagGetFn SharedTagCache::s_origTagGet = nullptr;

constexpr uintptr_t kStaticTagGetRva = 0x81BE8;
constexpr uintptr_t kPointerBasesRva = 0x4E39F20;

void* __fastcall SharedTagCache::HookedTagGet([[maybe_unused]] uint32_t groupTag, uint32_t datumIndex) {
    // 1. Check active match map first
    if (s_origTagGet) {
        void* result = s_origTagGet(groupTag, datumIndex);
        if (result != nullptr) {
            return result;
        }
    }

    // 2. If active map does not contain this tag, resolve from our mounted SharedTagCache!
    if (s_isMounted && s_mappedView) {
        auto it = s_tagOffsets.find(datumIndex);
        if (it != s_tagOffsets.end()) {
            uint32_t offset = it->second;
            if (offset < s_fileSize) {
                return s_mappedView + offset;
            }
        }
    }

    return nullptr;
}

void* SharedTagCache::ResolveTag([[maybe_unused]] uint32_t groupTag, uint32_t datumIndex) {
    if (!s_isMounted || !s_mappedView) return nullptr;

    auto it = s_tagOffsets.find(datumIndex);
    if (it != s_tagOffsets.end()) {
        uint32_t offset = it->second;
        if (offset < s_fileSize) {
            return s_mappedView + offset;
        }
    }
    return nullptr;
}

bool SharedTagCache::DiscoverDonorMap(const std::filesystem::path& dataDir, const std::filesystem::path& workshopDir) {
    std::vector<std::filesystem::path> searchCandidates;

    // 1. Steam Workshop directory for Forge World AI Battleground mod (ID 2899383598)
    if (!workshopDir.empty()) {
        searchCandidates.push_back(workshopDir / "2899383598" / "maps" / "forge_halo_normal.map");
        searchCandidates.push_back(workshopDir / "2899383598" / "maps" / "forge_halo_legendary.map");
    }

    // 2. Local mods directory in User Storage
    if (!dataDir.empty()) {
        searchCandidates.push_back(dataDir / "mods" / "maps" / "forge_halo_normal.map");
        searchCandidates.push_back(dataDir / "maps" / "forge_halo_normal.map");
        searchCandidates.push_back(dataDir / "mods" / "maps" / "forge_halo_legendary.map");
        searchCandidates.push_back(dataDir / "maps" / "forge_halo_legendary.map");
    }

    for (const auto& candidate : searchCandidates) {
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec)) {
            s_donorMapPath = std::filesystem::absolute(candidate, ec);
            NOBLE_INFO("SharedTagCache: Discovered donor AI map at '{}'", s_donorMapPath.string());
            return true;
        }
    }

    // 3. Fallback search: scan workshop directory for any forge_halo_normal.map
    if (!workshopDir.empty()) {
        std::error_code ec;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(workshopDir, ec)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename == "forge_halo_normal.map" || filename == "forge_halo_legendary.map") {
                    s_donorMapPath = std::filesystem::absolute(entry.path(), ec);
                    NOBLE_INFO("SharedTagCache: Discovered donor AI map via scan at '{}'", s_donorMapPath.string());
                    return true;
                }
            }
        }
    }

    NOBLE_WARN("SharedTagCache: No donor AI map (forge_halo_normal.map) found. Cross-map AI spawning will be inactive until mod is installed.");
    return false;
}

bool SharedTagCache::MountFile(const std::filesystem::path& mapPath) {
    if (s_isMounted) return true;

    s_hFile = CreateFileW(
        mapPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (s_hFile == INVALID_HANDLE_VALUE) {
        NOBLE_ERROR("SharedTagCache: Failed to open donor map '{}' (Win32 error: {})",
            mapPath.string(), GetLastError());
        return false;
    }

    LARGE_INTEGER liSize;
    if (!GetFileSizeEx(s_hFile, &liSize)) {
        CloseHandle(s_hFile);
        s_hFile = INVALID_HANDLE_VALUE;
        return false;
    }
    s_fileSize = static_cast<size_t>(liSize.QuadPart);

    s_hMapping = CreateFileMappingW(
        s_hFile,
        nullptr,
        PAGE_READONLY,
        0,
        0,
        nullptr
    );

    if (!s_hMapping) {
        NOBLE_ERROR("SharedTagCache: CreateFileMapping failed for donor map (Win32 error: {})", GetLastError());
        CloseHandle(s_hFile);
        s_hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    s_mappedView = static_cast<uint8_t*>(MapViewOfFile(
        s_hMapping,
        FILE_MAP_READ,
        0,
        0,
        0
    ));

    if (!s_mappedView) {
        NOBLE_ERROR("SharedTagCache: MapViewOfFile failed for donor map (Win32 error: {})", GetLastError());
        CloseHandle(s_hMapping);
        CloseHandle(s_hFile);
        s_hMapping = nullptr;
        s_hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    s_isMounted = true;
    NOBLE_INFO("SharedTagCache: Successfully mounted donor map into virtual memory ({} MB mapped, zero-copy).",
        s_fileSize / (1024 * 1024));
    return true;
}

bool SharedTagCache::ParseDonorTags() {
    if (!s_mappedView || s_fileSize < 0x800) return false;

    // Verify map header magic: 'daeh' (0x68656164)
    uint32_t magic = *reinterpret_cast<const uint32_t*>(s_mappedView);
    if (magic != 0x68656164 && magic != 0x64616568) {
        NOBLE_ERROR("SharedTagCache: Invalid donor map header signature: 0x{:X}", magic);
        return false;
    }

    s_tagIndexFileOffset = *reinterpret_cast<const uint32_t*>(s_mappedView + 0x10);
    s_donorTagCount = *reinterpret_cast<const uint32_t*>(s_mappedView + 0x20);
    s_virtualBase = *reinterpret_cast<const uint64_t*>(s_mappedView + 0x2E0);
    if (s_virtualBase == 0) s_virtualBase = 0x1C8980000ULL;

    NOBLE_INFO("SharedTagCache: Donor header parsed: tagIndexOffset=0x{:X}, tagCount={}, virtualBase=0x{:X}",
        s_tagIndexFileOffset, s_donorTagCount, s_virtualBase);

    if (s_tagIndexFileOffset >= s_fileSize || s_donorTagCount == 0 || s_donorTagCount > 50000) {
        NOBLE_WARN("SharedTagCache: Unusual tag index parameters in donor map; falling back to static bounds.");
        s_tagIndexFileOffset = 0x2AC48000;
        s_donorTagCount = 17707;
    }

    // Locate Tag Instances Array
    // Tag index header is located at s_tagIndexFileOffset + 0x4375D4
    // Tag instances array is located at s_tagIndexFileOffset + 0x413DCC
    uint32_t instancesOffset = s_tagIndexFileOffset + 0x413DCC;
    if (instancesOffset + s_donorTagCount * 8 <= s_fileSize) {
        const uint8_t* pInstances = s_mappedView + instancesOffset;
        for (uint32_t i = 0; i < s_donorTagCount; ++i) {
            int16_t group = *reinterpret_cast<const int16_t*>(pInstances + i * 8);
            uint16_t salt = *reinterpret_cast<const uint16_t*>(pInstances + i * 8 + 2);
            uint32_t packed = *reinterpret_cast<const uint32_t*>(pInstances + i * 8 + 4);

            if (group >= 0 && salt != 0xFFFF && packed != 0) {
                uint32_t datum = (static_cast<uint32_t>(salt) << 16) | i;
                uint32_t offset = (packed & 0x0FFFFFFF) << 2;
                if (offset < s_fileSize) {
                    s_tagOffsets[datum] = offset;
                    s_tagGroups[datum] = static_cast<uint32_t>(group);
                }
            }
        }
        NOBLE_INFO("SharedTagCache: Indexed {} donor tags from instances array.", s_tagOffsets.size());
    }

    return !s_tagOffsets.empty();
}

bool SharedTagCache::InstallHooks(HMODULE hReachDll) {
    if (s_hooksInstalled || !hReachDll) return s_hooksInstalled;

    s_hReachDll = hReachDll;
    s_baseAddress = reinterpret_cast<uintptr_t>(hReachDll);

    // 1. Hook tag_get at RVA 0x81BE8
    void* tagGetTarget = reinterpret_cast<void*>(s_baseAddress + kStaticTagGetRva);

    // Quick verification of function preamble bytes
    // Expected: 48 89 5C 24 08 48 89 74 24 10 (mov [rsp+8], rbx; mov [rsp+10h], rsi)
    const uint8_t* p = static_cast<const uint8_t*>(tagGetTarget);
    if (p[0] == 0x48 && p[1] == 0x89 && p[2] == 0x5C) {
        if (HookManager::InstallHook(tagGetTarget, reinterpret_cast<void*>(&HookedTagGet),
                                     reinterpret_cast<void**>(&s_origTagGet), "tag_get")) {
            s_hooksInstalled = true;
            NOBLE_INFO("SharedTagCache: Successfully hooked tag_get at RVA 0x{:X}. Cross-map tag virtualization ENABLED.",
                kStaticTagGetRva);
        } else {
            NOBLE_ERROR("SharedTagCache: Failed to install hook for tag_get at RVA 0x{:X}!", kStaticTagGetRva);
        }
    } else {
        NOBLE_WARN("SharedTagCache: tag_get preamble at RVA 0x{:X} does not match expected bytes; searching via signature...",
            kStaticTagGetRva);
        MODULEINFO modInfo = { 0 };
        if (GetModuleInformation(GetCurrentProcess(), hReachDll, &modInfo, sizeof(modInfo))) {
            uintptr_t match = SigScanner::FindPattern(
                s_baseAddress,
                modInfo.SizeOfImage,
                "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 83 CE FF 44 0F B7 CA"
            );
            if (match != 0) {
                if (HookManager::InstallHook(reinterpret_cast<void*>(match), reinterpret_cast<void*>(&HookedTagGet),
                                             reinterpret_cast<void**>(&s_origTagGet), "tag_get")) {
                    s_hooksInstalled = true;
                    NOBLE_INFO("SharedTagCache: Hooked tag_get via signature scan at RVA 0x{:X}.", match - s_baseAddress);
                }
            }
        }
    }

    // 2. Set base pointer in haloreach.dll kPointerBases table (Slot 2)
    // Table at haloreach.dll + 0x4E39F20 contains 16 base pointers
    if (s_mappedView && s_isMounted) {
        uint64_t* bases = reinterpret_cast<uint64_t*>(s_baseAddress + kPointerBasesRva);
        DWORD oldProtect = 0;
        if (VirtualProtect(&bases[2], sizeof(uint64_t), PAGE_READWRITE, &oldProtect)) {
            bases[2] = reinterpret_cast<uint64_t>(s_mappedView + s_tagIndexFileOffset);
            VirtualProtect(&bases[2], sizeof(uint64_t), oldProtect, &oldProtect);
            NOBLE_INFO("SharedTagCache: Registered donor memory base in pointer table slot 2.");
        }
    }

    return s_hooksInstalled;
}

bool SharedTagCache::Initialize(HMODULE hReachDll, const std::filesystem::path& dataDir, const std::filesystem::path& workshopDir) {
    NOBLE_INFO("SharedTagCache: Initializing Cross-Map Shared Tag Cache (Approach 1)...");

    if (!DiscoverDonorMap(dataDir, workshopDir)) {
        return false;
    }

    if (!MountFile(s_donorMapPath)) {
        return false;
    }

    ParseDonorTags();
    InstallHooks(hReachDll);

    return s_isMounted;
}

void SharedTagCache::Shutdown() {
    if (s_mappedView) {
        UnmapViewOfFile(s_mappedView);
        s_mappedView = nullptr;
    }
    if (s_hMapping) {
        CloseHandle(s_hMapping);
        s_hMapping = nullptr;
    }
    if (s_hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(s_hFile);
        s_hFile = INVALID_HANDLE_VALUE;
    }
    s_isMounted = false;
    s_tagOffsets.clear();
    s_tagGroups.clear();
    NOBLE_INFO("SharedTagCache: Shutdown complete.");
}

bool SharedTagCache::InjectScenarioPalette(uint8_t* activeTagTableMemory) {
    if (!s_isMounted || !activeTagTableMemory) return false;

    // The scenario tag ('scnr' = 0x73636E72) holds the sandbox palette for Forge.
    // TagTable helper accesses tag count at +0x3FFF8 and entries at +i*8.
    int32_t tagCount = *reinterpret_cast<const int32_t*>(activeTagTableMemory + 0x3FFF8);
    if (tagCount <= 0 || tagCount > 0x8000) return false;

    NOBLE_INFO("SharedTagCache: Active map has {} tags in memory; checking scenario palette...", tagCount);
    return true;
}

bool SharedTagCache::SpawnObject(uint32_t datumIndex, const std::string& tagPath) {
    if (!s_baseAddress) return false;

    NOBLE_INFO("SharedTagCache: Requesting spawn for object datum 0x{:X} ('{}')", datumIndex, tagPath);

    // Call the engine's native Forge spawn function at RVA 0x692C5C
    // Preamble: forge_spawn_object(rcx=objDef, rdx=subType, r8=owner, r9=flags, stackArg=transform)
    using ForgeSpawnFn = int64_t(__fastcall*)(uint32_t, uint32_t, uint32_t, uint32_t);
    auto pSpawn = reinterpret_cast<ForgeSpawnFn>(s_baseAddress + 0x692C5C);
    if (pSpawn) {
        __try {
            int64_t result = pSpawn(datumIndex, 0, 0, 0);
            NOBLE_INFO("SharedTagCache: forge_spawn_object returned 0x{:X}", result);
            return result >= 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            NOBLE_WARN("SharedTagCache: Exception caught during forge_spawn_object call.");
            return false;
        }
    }
    return false;
}

} // namespace noble
