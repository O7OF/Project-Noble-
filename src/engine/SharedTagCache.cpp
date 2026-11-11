#include "SharedTagCache.hpp"
#include "hooks/HookManager.hpp"
#include "hooks/SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>
#include <algorithm>
#include <fstream>
#include <set>

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

// NOBLE_TAG_MISS_TRACE=1: log the first tag lookups that find nothing, in the active map or in the donor. A map
// that fails to load usually misses a specific tag, and this names the datum and group it asked for.
static std::atomic<int> s_tagMissLogged{0};

void* __fastcall SharedTagCache::HookedTagGet([[maybe_unused]] uint32_t groupTag, uint32_t datumIndex) {
    // 1. Check active match map first
    if (s_origTagGet) {
        void* result = s_origTagGet(groupTag, datumIndex);
        if (result != nullptr) {
            return result;
        }
    }
    {
        wchar_t traceEnv[8] = {};
        static const bool traceMisses = GetEnvironmentVariableW(L"NOBLE_TAG_MISS_TRACE", traceEnv, 8) > 0 &&
                                        traceEnv[0] != L'0';
        if (traceMisses && s_tagMissLogged.fetch_add(1) < 60) {
            char line[128];
            std::snprintf(line, sizeof(line), "tag_get miss: group 0x%X datum 0x%08X", groupTag, datumIndex);
            NOBLE_WARN("{}", line);
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

static std::string s_avoidMapStem;   // scenario the running launch is about to load

void SharedTagCache::SetAvoidMap(const std::string& scenarioStem) {
    s_avoidMapStem.clear();
    for (const char c : scenarioStem) s_avoidMapStem.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    if (!s_avoidMapStem.empty()) NOBLE_INFO("SharedTagCache: donor map will avoid '{}'", s_avoidMapStem);
}

// True when this file is the scenario being launched (m20.map when ONI: Sword Base is loading).
static bool IsAvoidedDonor(const std::filesystem::path& candidate) {
    if (s_avoidMapStem.empty()) return false;
    std::string stem = candidate.stem().string();
    for (char& c : stem) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return stem == s_avoidMapStem;
}

bool SharedTagCache::DiscoverDonorMap(const std::filesystem::path& dataDir, [[maybe_unused]] const std::filesystem::path& workshopDir, const std::filesystem::path& reachDir) {
    std::vector<std::filesystem::path> searchCandidates;

    // 1. Native stock Campaign maps from local MCC installation (Zero mod download required!)
    // m20.map (Sword Base) contains Spartans, Marines, Troopers, ODSTs, Grunts, Jackals, Skirmishers, Elites, Hunters, and the Pelican!
    if (!reachDir.empty()) {
        searchCandidates.push_back(reachDir / "maps" / "m20.map");
        searchCandidates.push_back(reachDir / "maps" / "m45.map");
        searchCandidates.push_back(reachDir / "maps" / "campaign.map");
        searchCandidates.push_back(reachDir / "maps" / "shared.map");
    }

    // 2. Local mods directory in User Storage (if a custom campaign donor map is placed there)
    if (!dataDir.empty()) {
        searchCandidates.push_back(dataDir / "mods" / "maps" / "m20.map");
        searchCandidates.push_back(dataDir / "maps" / "m20.map");
    }

    for (const auto& candidate : searchCandidates) {
        std::error_code ec;
        if (IsAvoidedDonor(candidate)) {
            NOBLE_INFO("SharedTagCache: skipping donor candidate '{}' - it is the map being launched", candidate.string());
            continue;
        }
        if (std::filesystem::is_regular_file(candidate, ec)) {
            s_donorMapPath = std::filesystem::absolute(candidate, ec);
            NOBLE_INFO("SharedTagCache: Discovered donor AI map at '{}'", s_donorMapPath.string());
            return true;
        }
    }

    // 3. Fallback search: scan workshop directory for m20.map only
    if (!workshopDir.empty()) {
        std::error_code ec;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(workshopDir, ec)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename == "m20.map" && !IsAvoidedDonor(entry.path())) {
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
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (s_hFile == INVALID_HANDLE_VALUE) {
        s_hFile = CreateFileW(
            mapPath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
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

        // Pre-populate donor AI and vehicle tag paths to their exact datums
        s_tagPathToDatum["objects/characters/spartan/spartan_ai"] = 0x96102cb9;
        s_tagPathToDatum["objects/characters/marine/marine_ai"] = 0x98952f09;
        s_tagPathToDatum["objects/characters/marine/trooper_ai"] = 0x98952f09;
        s_tagPathToDatum["objects/characters/marine/odst_ai"] = 0x98ad2f21;
        s_tagPathToDatum["objects/characters/grunt/grunt_ai"] = 0x9ddc3336;
        s_tagPathToDatum["objects/characters/jackal/jackal_ai"] = 0x9f5f34b0;
        s_tagPathToDatum["objects/characters/skirmisher/skirmisher_ai"] = 0x9f6b34bc;
        s_tagPathToDatum["objects/characters/elite/elite_ai"] = 0xa24f370e;
        s_tagPathToDatum["objects/characters/hunter/hunter_ai"] = 0xa37c3828;
        s_tagPathToDatum["objects/characters/drone/drone_ai"] = 0xa48e3927;
        s_tagPathToDatum["objects/characters/elite/elite_pinky_ai"] = 0xad234074;
        s_tagPathToDatum["objects/characters/elite/elite_schism_ai"] = 0xa26c372b;
        s_tagPathToDatum["objects/characters/marine/deviant_ai"] = 0x98992f0d;
        s_tagPathToDatum["objects/characters/spartan/spartan_ret_ai"] = 0x98342ea9;

        // Flyable Base Pelican (with patched wraith_d seat)
        s_tagPathToDatum["objects/vehicles/human/pelican/pelican"] = 0x95e82c96;
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
            DWORD oldProtect = 0;
            if (VirtualProtect(&pDwords[i], sizeof(uint32_t), PAGE_READWRITE, &oldProtect)) {
                pDwords[i] = kWraithD;
                VirtualProtect(&pDwords[i], sizeof(uint32_t), oldProtect, &oldProtect);
                ++patchCount;
            }
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


// Probe for the scenario tag's data (NOBLE_SCENARIO_DUMP=3). Called after the map load, when the tag index is live.
void SharedTagCache::DumpScenarioTag(const char* when) {
    static const auto readMem = [](uintptr_t ptr, void* out, size_t size) -> bool {
        __try {
            std::memcpy(out, reinterpret_cast<const void*>(ptr), size);
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return false;
    };
    uintptr_t header = 0;
    if (!readMem(s_baseAddress + 0xC1A230, &header, sizeof(header)) || !header) {
        NOBLE_WARN("scenario dump ({}): tag index header not readable", when);
        return;
    }
    int32_t count = 0;
    uint32_t baseOffset = 0;
    if (!readMem(header + 0x430, &count, sizeof(count)) || !readMem(header + 0x434, &baseOffset, sizeof(baseOffset))) {
        NOBLE_WARN("scenario dump ({}): header fields not readable", when);
        return;
    }
    const uint32_t selector = baseOffset >> 28;
    uintptr_t tablePtr = 0;
    if (!readMem(s_baseAddress + 0x4E39F20 + selector * 8, &tablePtr, sizeof(tablePtr)) || !tablePtr) {
        NOBLE_WARN("scenario dump ({}): tag index base[{}] not readable", when, selector);
        return;
    }
    NOBLE_WARN("scenario dump ({}): header 0x{:X} count {} baseOffset 0x{:X} selector {} table 0x{:X}", when, header,
               count, baseOffset, selector, tablePtr);
    for (int32_t i = 0; i < count; ++i) {
        uint32_t fields[6] = {};
        const uintptr_t entry = tablePtr + (static_cast<uint64_t>(baseOffset) + static_cast<uint64_t>(i) * 6) * 4;
        if (!readMem(entry, fields, sizeof(fields))) continue;
        if (fields[0] != 0x726E6373 && fields[0] != 0x73636E72) continue;   // 'scnr'
        char line[256];
        std::snprintf(line, sizeof(line), "scenario entry %d: %08X %08X %08X %08X %08X %08X", i, fields[0], fields[1],
                      fields[2], fields[3], fields[4], fields[5]);
        NOBLE_WARN("scenario dump ({}): {}", when, line);
        for (int k = 0; k < 6; ++k) {
            const uint64_t v = fields[k];
            if (v < 0x10000) continue;
            uint8_t bytes[16] = {};
            if (readMem(static_cast<uintptr_t>(v), bytes, sizeof(bytes))) {
                char hex[64];
                for (int n = 0; n < 16; ++n) std::snprintf(hex + n * 3, 4, " %02X", bytes[n]);
                NOBLE_WARN("scenario dump ({}):   field[{}]=0x{:X} as pointer:{}", when, k, v, hex);
            }
        }
        break;
    }
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
    if (!activeTagTableMemory) return false;

    // TagTable helper accesses tag count at +0x3FFF8 and entries at +i*8.
    int32_t tagCount = *reinterpret_cast<const int32_t*>(activeTagTableMemory + 0x3FFF8);
    NOBLE_INFO("SharedTagCache: palette step on tag table 0x{:X}: {} tag(s)", reinterpret_cast<uintptr_t>(activeTagTableMemory), tagCount);
    if (tagCount <= 0 || tagCount > 0x8000) {
        NOBLE_WARN("SharedTagCache: palette step skipped (tag count {} out of range)", tagCount);
        return false;
    }

    NOBLE_INFO("SharedTagCache: Active map has {} tags in memory; cataloging active tags...", tagCount);

    // Diagnostic helper: reads 0x40 bytes at ptr, SEH-guarded (a file-scope function has no unwinding objects).
    static const auto kReadMemory = [](uintptr_t ptr, void* out, size_t size) -> bool {
        __try {
            std::memcpy(out, reinterpret_cast<const void*>(ptr), size);
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return false;
    };
    static const auto kReadScenario = [](uintptr_t ptr, uint8_t* out) -> bool {
        return kReadMemory(ptr, out, 0x40);
    };

    // Diagnostic (NOBLE_SCENARIO_DUMP=2): walk the engine's tag index (header at haloreach+0xC1A230, count +0x430,
    // base +0x434, 24-byte entries) and print the scenario entry's whole record, so the data pointer field can be
    // identified instead of guessed.
    {
        wchar_t dumpEnv[8] = {};
        if (GetEnvironmentVariableW(L"NOBLE_SCENARIO_DUMP", dumpEnv, 8) > 0 && dumpEnv[0] == L'2') {
            uintptr_t header = 0;
            if (!kReadMemory(s_baseAddress + 0xC1A230, &header, sizeof(header)) || !header) {
                NOBLE_WARN("tag index: header pointer not readable");
            } else {
            int32_t count = 0;
            uint32_t baseOffset = 0;
            if (!kReadMemory(header + 0x430, &count, sizeof(count)) ||
                !kReadMemory(header + 0x434, &baseOffset, sizeof(baseOffset))) {
                NOBLE_WARN("tag index: header fields not readable");
                count = 0;
            }
            NOBLE_WARN("tag index: header 0x{:X} count {} baseOffset 0x{:X}", header, count, baseOffset);
            for (int32_t i = 0; i < count && i < 40000; ++i) {
                const uintptr_t entry = header + baseOffset + static_cast<uintptr_t>(i) * 24;
                uint32_t fields[6] = {};
                if (!kReadScenario(entry, reinterpret_cast<uint8_t*>(fields))) continue;
                const uint32_t group = fields[0];
                if (group != 0x726E6373 && group != 0x73636E72) continue;   // 'scnr'
                char line[512];
                std::snprintf(line, sizeof(line),
                              "scenario index entry %d: %08X %08X %08X %08X %08X %08X", i, fields[0], fields[1],
                              fields[2], fields[3], fields[4], fields[5]);
                NOBLE_WARN("{}", line);
                break;
            }
            }
        }
    }

    // Diagnostic (NOBLE_SCENARIO_DUMP=1): find the scenario tag (group 'scnr') in the active tag table and print the
    // first bytes of its data. Comparing a multiplayer map with a Firefight-canvas mod map shows the game-engine
    // type field the engine checks when it decides whether to create a game engine.
    {
        wchar_t dumpEnv[8] = {};
        if (GetEnvironmentVariableW(L"NOBLE_SCENARIO_DUMP", dumpEnv, 8) > 0 && dumpEnv[0] != L'0') {
            for (int32_t i = 0; i < tagCount; ++i) {
                const int16_t g = *reinterpret_cast<const int16_t*>(activeTagTableMemory + i * 8);
                if (g < 0) continue;
                const uint32_t magic = *reinterpret_cast<const uint32_t*>(activeTagTableMemory + 0x3FFFC + g * 16);
                if (magic != 0x726E6373 && magic != 0x73636E72) continue;   // 'scnr' either byte order
                const uint16_t salt = *reinterpret_cast<const uint16_t*>(activeTagTableMemory + i * 8 + 2);
                const uint32_t packed = *reinterpret_cast<const uint32_t*>(activeTagTableMemory + i * 8 + 4);
                char head[160];
                std::snprintf(head, sizeof(head), "scenario tag: index %d salt 0x%X packed 0x%08X", i, salt, packed);
                NOBLE_WARN("{}", head);
                const uintptr_t bases[2] = {
                    *reinterpret_cast<const uintptr_t*>(s_baseAddress + 0xB87E90),
                    *reinterpret_cast<const uintptr_t*>(s_baseAddress + 0xB87E88),
                };
                for (int b = 0; b < 2; ++b) {
                    const uintptr_t ptr = bases[b] + (static_cast<uintptr_t>(packed) << 2);
                    uint8_t bytes[0x40] = {};
                    if (!kReadScenario(ptr, bytes)) {
                        NOBLE_WARN("scenario base[{}]=0x{:X}: read faulted", b, bases[b]);
                        continue;
                    }
                    char hex[0x40 * 3 + 8] = {};
                    for (int k = 0; k < 0x40; ++k) std::snprintf(hex + k * 3, 4, " %02X", bytes[k]);
                    char line[512];
                    std::snprintf(line, sizeof(line), "scenario base[%d]=0x%llX ptr=0x%llX:%s", b,
                                  static_cast<unsigned long long>(bases[b]),
                                  static_cast<unsigned long long>(ptr), hex);
                    NOBLE_WARN("{}", line);
                }
                break;
            }
        }
    }

    // Diagnostic: which tag groups this map actually carries, as four-character codes. It answers questions like
    // "does this map have multiplayer_object_type_list (the Forge palette's object list)?" without a tag parser.
    {
        std::set<std::string> groups;
        for (int32_t i = 0; i < tagCount; ++i) {
            const int16_t g = *reinterpret_cast<const int16_t*>(activeTagTableMemory + i * 8);
            if (g < 0) continue;
            const uint32_t magic = *reinterpret_cast<const uint32_t*>(activeTagTableMemory + 0x3FFFC + g * 16);
            char code[5] = { static_cast<char>(magic & 0xFF), static_cast<char>((magic >> 8) & 0xFF),
                             static_cast<char>((magic >> 16) & 0xFF), static_cast<char>((magic >> 24) & 0xFF), 0 };
            bool printable = true;
            for (int k = 0; k < 4; ++k) if (code[k] < 0x20 || code[k] > 0x7E) { printable = false; break; }
            if (printable) groups.insert(code);
        }
        std::string list;
        for (const std::string& g : groups) { if (!list.empty()) list += ' '; list += g; }
        NOBLE_INFO("SharedTagCache: {} tag group(s) in this map: {}", groups.size(), list);
    }

    // Map common items from the active map tag table into s_tagPathToDatum
    for (int32_t i = 0; i < tagCount; ++i) {
        int16_t group = *reinterpret_cast<const int16_t*>(activeTagTableMemory + i * 8);
        uint16_t salt = *reinterpret_cast<const uint16_t*>(activeTagTableMemory + i * 8 + 2);
        uint32_t packed = *reinterpret_cast<const uint32_t*>(activeTagTableMemory + i * 8 + 4);
        if (group < 0 || salt == 0xFFFF || packed == 0) continue;

        uint32_t datum = (static_cast<uint32_t>(salt) << 16) | static_cast<uint32_t>(i);
        uint32_t magic = *reinterpret_cast<const uint32_t*>(activeTagTableMemory + 0x3FFFC + group * 16);

        // Group-based registrations for active map tags
        if (magic == 0x76656869) { // 'vehi'
            if (s_tagPathToDatum.find("objects/vehicles/human/mongoose/mongoose") == s_tagPathToDatum.end()) {
                s_tagPathToDatum["objects/vehicles/human/mongoose/mongoose"] = datum;
            }
            if (s_tagPathToDatum.find("objects/vehicles/human/warthog/warthog") == s_tagPathToDatum.end()) {
                s_tagPathToDatum["objects/vehicles/human/warthog/warthog"] = datum;
            }
            if (s_tagPathToDatum.find("objects/vehicles/human/scorpion/scorpion") == s_tagPathToDatum.end()) {
                s_tagPathToDatum["objects/vehicles/human/scorpion/scorpion"] = datum;
            }
        } else if (magic == 0x77656170) { // 'weap'
            if (s_tagPathToDatum.find("objects/weapons/rifle/assault_rifle/assault_rifle") == s_tagPathToDatum.end()) {
                s_tagPathToDatum["objects/weapons/rifle/assault_rifle/assault_rifle"] = datum;
            }
            if (s_tagPathToDatum.find("objects/weapons/rifle/dmr/dmr") == s_tagPathToDatum.end()) {
                s_tagPathToDatum["objects/weapons/rifle/dmr/dmr"] = datum;
            }
            if (s_tagPathToDatum.find("objects/weapons/pistol/magnum/magnum") == s_tagPathToDatum.end()) {
                s_tagPathToDatum["objects/weapons/pistol/magnum/magnum"] = datum;
            }
        }
    }

    return true;
}

static std::mutex s_spawnMutex;
static noble::SharedTagCache::SpawnRequest s_pendingSpawn;
static std::atomic<int> s_lastSpawnStatus{0};
static noble::SharedTagCache::SpawnForwardFn s_spawnForwarder = nullptr;

void SharedTagCache::SetSpawnForwarder(SpawnForwardFn fn) {
    s_spawnForwarder = fn;
}

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
// Layout matching the exact 0x50 frame expected by object_create_from_params (RVA 0x7311B8)
struct alignas(16) EnginePlacementData {
    uint32_t tagDatum = 0xFFFFFFFF; // 0x00
    uint32_t variant = 0;           // 0x04
    uint32_t unk08 = 0;             // 0x08
    uint32_t unk0C = 0;             // 0x0C
    uint64_t playerContext = 0;     // 0x10
    uint16_t playerIndex = 0;       // 0x18
    uint16_t team = 0;              // 0x1A
    uint32_t unk1C = 0;             // 0x1C
    uint64_t unk20 = 0;             // 0x20
    uint32_t unk28 = 0;             // 0x28
    float posX = 0.0f;              // 0x2C
    float posY = 0.0f;              // 0x30
    float posZ = 0.0f;              // 0x34
    uint32_t unk38 = 0;             // 0x38
    float pitch = 0.0f;             // 0x3C
    float yaw = 0.0f;               // 0x40
    float roll = 0.0f;              // 0x44
    float scale = 1.0f;             // 0x48
    uint32_t flags = 0;             // 0x4C
    uint8_t pad[0x30] = {};         // 0x50+
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

    // In an active multiplayer match, only the host should execute simulation spawns.
    // If a forwarder callback is registered and accepts the request (e.g. forwarding to host),
    // skip local execution so the host spawns authoritatively and replicates over the wire.
    if (s_spawnForwarder && s_spawnForwarder(req.datumIndex, req.tagPath)) {
        s_lastSpawnStatus.store(1);
        return true;
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

    // If still not resolved, try substring match in s_tagPathToDatum
    if (datum == 0xFFFFFFFF && !req.tagPath.empty()) {
        for (const auto& [path, d] : s_tagPathToDatum) {
            if (path.find(req.tagPath) != std::string::npos || req.tagPath.find(path) != std::string::npos) {
                datum = d;
                break;
            }
        }
    }

    if (datum == 0xFFFFFFFF) {
        // Fallback default: Spartan AI datum from donor cache
        datum = 0x96102cb9;
        NOBLE_INFO("SharedTagCache: Datum unresolved for '{}', using default donor datum 0x{:X}", req.tagPath, datum);
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
                spawnPos[0] = headPos[0] + 3.0f;
                spawnPos[1] = headPos[1] + 3.0f;
                spawnPos[2] = headPos[2] + 0.5f;
            }
        }
    }

    EnginePlacementData params;
    params.tagDatum = datum;
    params.posX = spawnPos[0];
    params.posY = spawnPos[1];
    params.posZ = spawnPos[2];
    params.scale = 1.0f;

    int64_t result = -1;

    // Prefer Forge spawn routine at 0x692C5C to attach directly into player's Forge beam
    void* pSpawn = reinterpret_cast<void*>(s_baseAddress + 0x692C5C);
    if (pSpawn) {
        result = SafeCallForgeSpawn(pSpawn, datum);
        NOBLE_INFO("SharedTagCache: Simulation forge_spawn_object returned 0x{:X}", result);
    }

    // Only if forge_spawn_object failed or is not available (e.g. Campaign/Custom Games), fall back to object_create_from_params.
    // Crucial: NEVER call both back-to-back, which previously spawned a duplicate/cloned object!
    if (result < 0) {
        void* pCreate = reinterpret_cast<void*>(s_baseAddress + 0x7311B8);
        if (pCreate) {
            result = SafeCallObjectCreate(pCreate, &params);
            NOBLE_INFO("SharedTagCache: Simulation object_create_from_params fallback returned 0x{:X}", result);
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
