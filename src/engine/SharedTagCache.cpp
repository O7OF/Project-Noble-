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

bool SharedTagCache::DiscoverDonorMap(const std::filesystem::path& dataDir, const std::filesystem::path& workshopDir, const std::filesystem::path& reachDir) {
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

    // 3. Native stock Campaign maps from local MCC installation (Zero mod download required!)
    // m20.map (Sword Base) contains Spartans, Marines, Troopers, ODSTs, Grunts, Jackals, Skirmishers, Elites, Hunters, and the Pelican!
    if (!reachDir.empty()) {
        searchCandidates.push_back(reachDir / "maps" / "m20.map");
        searchCandidates.push_back(reachDir / "maps" / "m45.map");
        searchCandidates.push_back(reachDir / "maps" / "campaign.map");
        searchCandidates.push_back(reachDir / "maps" / "shared.map");
    }

    for (const auto& candidate : searchCandidates) {
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec)) {
            s_donorMapPath = std::filesystem::absolute(candidate, ec);
            NOBLE_INFO("SharedTagCache: Discovered donor AI map at '{}'", s_donorMapPath.string());
            return true;
        }
    }

    // 4. Fallback search: scan workshop directory for any forge_halo_normal.map
    if (!workshopDir.empty()) {
        std::error_code ec;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(workshopDir, ec)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename == "forge_halo_normal.map" || filename == "forge_halo_legendary.map" || filename == "m20.map") {
                    s_donorMapPath = std::filesystem::absolute(entry.path(), ec);
                    NOBLE_INFO("SharedTagCache: Discovered donor AI map via scan at '{}'", s_donorMapPath.string());
                    return true;
                }
            }
        }
    }

    NOBLE_WARN("SharedTagCache: No donor AI map found in search candidates.");
    return false;
}

bool SharedTagCache::MountFile(const std::filesystem::path& mapPath) {
    if (s_isMounted) return true;

    s_hFile = CreateFileW(
        mapPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (s_hFile == INVALID_HANDLE_VALUE) {
        s_hFile = CreateFileW(
            mapPath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
    }

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

    // Use PAGE_WRITECOPY so changes (such as the wraith_d seat patch) are private in RAM and never written to disk
    s_hMapping = CreateFileMappingW(
        s_hFile,
        nullptr,
        PAGE_WRITECOPY,
        0,
        0,
        nullptr
    );

    if (!s_hMapping) {
        s_hMapping = CreateFileMappingW(
            s_hFile,
            nullptr,
            PAGE_READONLY,
            0,
            0,
            nullptr
        );
    }

    if (!s_hMapping) {
        NOBLE_ERROR("SharedTagCache: CreateFileMapping failed for donor map (Win32 error: {})", GetLastError());
        CloseHandle(s_hFile);
        s_hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    s_mappedView = static_cast<uint8_t*>(MapViewOfFile(
        s_hMapping,
        FILE_MAP_COPY,
        0,
        0,
        0
    ));

    if (!s_mappedView) {
        s_mappedView = static_cast<uint8_t*>(MapViewOfFile(
            s_hMapping,
            FILE_MAP_READ,
            0,
            0,
            0
        ));
    }

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

    PatchPelicanSeatLabel();

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

void SharedTagCache::PatchPelicanSeatLabel() {
    if (!s_mappedView || s_fileSize < 0x1000) return;

    // StringId for 'pelican_d' in Reach: 10417 (0x000028B1)
    // StringId for 'wraith_d': 5275 (0x0000149B)
    constexpr uint32_t kPelicanD = 10417;
    constexpr uint32_t kWraithD = 5275;

    int patchCount = 0;
    uint32_t* pDwords = reinterpret_cast<uint32_t*>(s_mappedView);
    size_t count = s_fileSize / sizeof(uint32_t);
    for (size_t i = 0; i < count; ++i) {
        if (pDwords[i] == kPelicanD) {
            pDwords[i] = kWraithD;
            ++patchCount;
        }
    }
    if (patchCount > 0) {
        NOBLE_INFO("SharedTagCache: Patched {} occurrences of 'pelican_d' -> 'wraith_d' in RAM (Pelican is flyable).", patchCount);
    }
}

bool SharedTagCache::Initialize(HMODULE hReachDll, const std::filesystem::path& dataDir, const std::filesystem::path& workshopDir, const std::filesystem::path& reachDir) {
    NOBLE_INFO("SharedTagCache: Initializing Cross-Map Shared Tag Cache (Approach 1)...");

    if (!DiscoverDonorMap(dataDir, workshopDir, reachDir)) {
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

static std::mutex s_spawnMutex;
static noble::SharedTagCache::SpawnRequest s_pendingSpawn;
static std::atomic<int> s_lastSpawnStatus{0};

void SharedTagCache::QueueSpawn(uint32_t datumIndex, const std::string& tagPath) {
    std::lock_guard<std::mutex> lock(s_spawnMutex);
    s_pendingSpawn.datumIndex = datumIndex;
    s_pendingSpawn.tagPath = tagPath;
    s_pendingSpawn.pending = true;
    s_lastSpawnStatus.store(0);
    NOBLE_INFO("SharedTagCache: Queued spawn request for '{}' (datum: 0x{:X})", tagPath, datumIndex);
}

bool SharedTagCache::HasPendingSpawn() {
    return s_pendingSpawn.pending;
}

int SharedTagCache::GetLastSpawnStatus() {
    return s_lastSpawnStatus.load();
}

static int64_t SafeCallForgeSpawn(void* pFn, uint32_t datum) {
    __try {
        using ForgeSpawnFn = int64_t(__fastcall*)(uint32_t, uint32_t, uint32_t, uint32_t);
        auto fn = reinterpret_cast<ForgeSpawnFn>(pFn);
        return fn(datum, 0, 0, 0);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }
}

#pragma warning(push)
#pragma warning(disable: 4324)
struct alignas(16) EnginePlacementData {
    uint32_t tagDatum = 0xFFFFFFFF;
    uint32_t variant = 0;
    uint32_t unknown08 = 0;
    uint32_t unknown0C = 0;
    uint64_t playerContext = 0;
    uint16_t team = 0;
    uint16_t playerIndex = 0;
    uint64_t unknown1C = 0;
    uint64_t unknown24 = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    uint32_t unknown34 = 0;
    uint32_t unknown38 = 0;
    float posZ = 0.0f;
    float yaw = 0.0f;
    uint8_t pad[0x40] = {};
};
#pragma warning(pop)

static int64_t SafeCallObjectCreate(void* pFn, EnginePlacementData* pData) {
    __try {
        using Fn = int64_t(__fastcall*)(EnginePlacementData*);
        return reinterpret_cast<Fn>(pFn)(pData);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }
}

bool SharedTagCache::ProcessPendingSpawn() {
    SpawnRequest req;
    {
        std::lock_guard<std::mutex> lock(s_spawnMutex);
        if (!s_pendingSpawn.pending) return false;
        req = s_pendingSpawn;
        s_pendingSpawn.pending = false;
    }

    if (!s_baseAddress) {
        s_lastSpawnStatus.store(-1);
        return false;
    }

    NOBLE_INFO("SharedTagCache: Processing simulation spawn for '{}' (datum: 0x{:X})", req.tagPath, req.datumIndex);

    uint32_t datum = req.datumIndex;
    if (datum == 0xFFFFFFFF && !req.tagPath.empty()) {
        auto it = s_tagPathToDatum.find(req.tagPath);
        if (it != s_tagPathToDatum.end()) {
            datum = it->second;
        } else {
            std::string alt = req.tagPath;
            for (char& c : alt) {
                if (c == '/') c = '\\';
                else if (c == '\\') c = '/';
            }
            it = s_tagPathToDatum.find(alt);
            if (it != s_tagPathToDatum.end()) {
                datum = it->second;
            }
        }
    }

    // Determine spawn position directly in front of the player/camera
    float spawnPos[3] = { 0.0f, 0.0f, 2.0f };
    auto pGetLocalUnit = reinterpret_cast<int32_t(__fastcall*)(int32_t)>(s_baseAddress + 0x53EF8);
    auto pGetHeadPos = reinterpret_cast<float*(__fastcall*)(int32_t, float*)>(s_baseAddress + 0x472184);
    if (pGetLocalUnit && pGetHeadPos) {
        int32_t unitDatum = pGetLocalUnit(0);
        if (unitDatum != -1 && unitDatum != 0) {
            float headPos[3] = {};
            if (pGetHeadPos(unitDatum, headPos)) {
                spawnPos[0] = headPos[0] + 3.5f;
                spawnPos[1] = headPos[1] + 3.5f;
                spawnPos[2] = headPos[2] + 0.5f;
            }
        }
    }

    EnginePlacementData params;
    params.tagDatum = datum;
    params.posX = spawnPos[0];
    params.posY = spawnPos[1];
    params.posZ = spawnPos[2];

    void* pCreate = reinterpret_cast<void*>(s_baseAddress + 0x7311B8);
    int64_t result = -1;
    if (pCreate && datum != 0xFFFFFFFF) {
        result = SafeCallObjectCreate(pCreate, &params);
        NOBLE_INFO("SharedTagCache: Simulation object_create_from_params returned 0x{:X}", result);
    }

    // Fallback: If direct creation failed or returned -1, try Forge spawn routine
    if (result < 0) {
        void* pSpawn = reinterpret_cast<void*>(s_baseAddress + 0x692C5C);
        if (pSpawn && datum != 0xFFFFFFFF) {
            result = SafeCallForgeSpawn(pSpawn, datum);
            NOBLE_INFO("SharedTagCache: Simulation forge_spawn_object returned 0x{:X}", result);
        }
    }

    const bool ok = (result >= 0);
    s_lastSpawnStatus.store(ok ? 1 : -1);
    return ok;
}

bool SharedTagCache::SpawnObject(uint32_t datumIndex, const std::string& tagPath) {
    QueueSpawn(datumIndex, tagPath);
    return true;
}

} // namespace noble
