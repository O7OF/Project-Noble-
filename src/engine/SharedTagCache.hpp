#pragma once

#include <windows.h>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace noble {

struct PaletteItem {
    std::string name;
    std::string category;
    std::string tagPath;
    uint32_t datumIndex = 0xFFFFFFFF;
    uint32_t groupMagic = 0;
    int maxCount = 16;
    float cost = 0.0f;
};

class SharedTagCache {
public:
    // Discovers and mounts the donor AI map (forge_halo_normal.map) as a persistent secondary tag heap.
    // Hooks haloreach.dll tag_get (RVA 0x81BE8) so any multiplayer map can seamlessly spawn AI bipeds & vehicles.
    static bool Initialize(HMODULE hReachDll, const std::filesystem::path& dataDir, const std::filesystem::path& workshopDir);
    static void Shutdown();

    static bool IsMounted() { return s_isMounted; }
    static const std::filesystem::path& GetDonorMapPath() { return s_donorMapPath; }
    static uint32_t GetDonorTagCount() { return s_donorTagCount; }
    static const std::vector<PaletteItem>& GetDonorPaletteItems() { return s_paletteItems; }

    // Tag resolution detour: returns pointer into mapped donor map memory if datum belongs to SharedTagCache
    static void* ResolveTag(uint32_t groupTag, uint32_t datumIndex);

    // Injects donor AI sandbox palette entries into the active match scenario tag in RAM
    static bool InjectScenarioPalette(uint8_t* activeTagTableMemory);

    // Spawns an object from the donor palette into the world
    static bool SpawnObject(uint32_t datumIndex, const std::string& tagPath);

private:
    static bool DiscoverDonorMap(const std::filesystem::path& dataDir, const std::filesystem::path& workshopDir);
    static bool MountFile(const std::filesystem::path& mapPath);
    static bool ParseDonorTags();
    static bool InstallHooks(HMODULE hReachDll);

    static bool s_isMounted;
    static bool s_hooksInstalled;
    static HMODULE s_hReachDll;
    static uintptr_t s_baseAddress;
    static std::filesystem::path s_donorMapPath;

    // Win32 Memory-Mapped File Handles
    static HANDLE s_hFile;
    static HANDLE s_hMapping;
    static uint8_t* s_mappedView;
    static size_t s_fileSize;

    // Header & Tag Index structures
    static uint64_t s_virtualBase;
    static uint32_t s_tagIndexFileOffset;
    static uint32_t s_donorTagCount;
    static uint32_t s_donorScenarioDatum;

    // Datum index -> byte offset in donor map
    static std::unordered_map<uint32_t, uint32_t> s_tagOffsets;
    static std::unordered_map<uint32_t, uint32_t> s_tagGroups;
    static std::unordered_map<std::string, uint32_t> s_tagPathToDatum;

    static std::vector<PaletteItem> s_paletteItems;

    // Original tag_get function pointer
    using TagGetFn = void* (__fastcall*)(uint32_t, uint32_t);
    static TagGetFn s_origTagGet;
    static void* __fastcall HookedTagGet(uint32_t groupTag, uint32_t datumIndex);
};

} // namespace noble
