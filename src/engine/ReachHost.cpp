#include "ReachHost.hpp"
#include "common/Logger.hpp"
#include <atomic>
#include <array>
#include <utility>
#include <cwctype>
#include <unordered_map>
#include <mutex>

namespace noble {

void* ReachHost::s_vtable[160] = { nullptr };
bool ReachHost::s_vtableInitialized = false;

template<size_t Slot>
struct SlotDispatcher {
    static uint64_t __fastcall Stub(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
        (void)a4;
        static std::atomic<bool> s_firstCall{false};
        if (!s_firstCall.exchange(true)) {
            NOBLE_INFO("ReachHost: slot {:3d} first call (rcx=0x{:X}, a1=0x{:X}, a2=0x{:X}, a3=0x{:X})",
                Slot, reinterpret_cast<uintptr_t>(thisPtr), a1, a2, a3);
        }
        return 0;
    }
};

// Slot 0: QueryInterface handler
static uint64_t __fastcall HostSlot0_QueryInterface(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 0 - QueryInterface]: riid=0x{:X}, outPtr=0x{:X}", a1, a2);
    }
    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), sizeof(void*))) {
        *reinterpret_cast<void**>(a2) = thisPtr;
        return 0; // S_OK
    }
    return 0;
}

// Slot 1: Frame tick / Render loop update callback (host_probe case 1)
static uint64_t __fastcall HostSlot1_FrameTick(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// Slot 2: SwapChain / Buffer resize callback (host_probe case 2)
static uint64_t __fastcall HostSlot2_ResizeBuffers(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 2 - ResizeBuffers]: Engine requested swap chain buffer refresh");
    }
    return 0;
}

// Slot 4: Engine logging stream (host_probe case 4)
static uint64_t __fastcall HostSlot4_LogMessage(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a2; (void)a3; (void)a4;
    if (a1 > 0x10000 && !IsBadReadPtr(reinterpret_cast<const void*>(a1), 1)) {
        const char* msg = reinterpret_cast<const char*>(a1);
        if (msg && msg[0] != '\0') {
            NOBLE_INFO("[haloreach.dll] {}", msg);
        }
    }
    return 0;
}

// Slot 10: Second tick / heartbeat (host_probe case 10)
static uint64_t __fastcall HostSlot10_Heartbeat(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 1;
}

// Slot 18: Asset lookup (host_probe case 0x12)
static uint64_t __fastcall HostSlot18_AssetLookup(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// Slot 23: Load progress handler
static uint64_t __fastcall HostSlot23_LoadProgress(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a2; (void)a3; (void)a4;
    float progress = 0.0f;
    if (a1 <= 100) {
        progress = static_cast<float>(a1) / 100.0f;
    } else {
        memcpy(&progress, &a1, sizeof(float));
    }
    NOBLE_INFO("ReachHost [Slot 23 - LoadProgress]: progress={:.1f}% (raw=0x{:X})", progress * 100.0f, a1);
    return 0;
}

// Slot 32: Display / Graphics Settings & Resource Pool Atom Generator handler (host_probe case 0x20)
static uint8_t s_hostAtomBuffer[512 * 0x88] = { 0 };

static uint64_t __fastcall HostSlot32_DisplaySettings(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 32]: a1=0x{:X}, a2=0x{:X}, a3=0x{:X}", a1, a2, a3);
    }

    // Check if this is an atom generator buffer allocation request from c_tag_resource_prediction_atom_generator (a2 = 0x88)
    if (a2 == 0x88 || (a2 > 0 && a3 > 0 && a1 <= 0x10000)) {
        size_t totalBytes = static_cast<size_t>(a2) * static_cast<size_t>(a3);
        NOBLE_INFO("ReachHost [Slot 32 - Atom Generator]: Supplying {} bytes for prediction atoms (elemSize=0x{:X}, count=0x{:X})",
            totalBytes, a2, a3);
        if (totalBytes <= sizeof(s_hostAtomBuffer)) {
            memset(s_hostAtomBuffer, 0, totalBytes);
            return reinterpret_cast<uint64_t>(s_hostAtomBuffer);
        } else {
            void* dynamicBuf = calloc(1, totalBytes);
            return reinterpret_cast<uint64_t>(dynamicBuf);
        }
    }

    // Otherwise treat as DisplaySettings query
    if (a1 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a1), 0x20)) {
        struct DisplayConfig {
            uint32_t width;        // +0x00: 1280
            uint32_t height;       // +0x04: 720
            uint32_t refreshRate;  // +0x08: 60
            uint32_t unknown;      // +0x0C: 0
            uint32_t adapterIndex; // +0x10: 0
            uint8_t  windowMode;   // +0x14: 0 (windowed)
            uint8_t  displayMode;  // +0x15: 0 (windowed)
            uint16_t pad;          // +0x16: 0
            int32_t  field18;      // +0x18: -1
            int32_t  field1C;      // +0x1C: -1
        };
        auto* cfg = reinterpret_cast<DisplayConfig*>(a1);
        cfg->width = 1280;
        cfg->height = 720;
        cfg->refreshRate = 60;
        cfg->adapterIndex = 0;
        cfg->windowMode = 0;
        cfg->displayMode = 0;
    }
    return 1; // Return true to signal successful configuration query
}

// Slot 33: Display Quality settings handler (host_probe case 0x21)
static uint64_t __fastcall HostSlot33_DisplayQuality(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a2; (void)a3; (void)a4;
    if (a1 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a1), 0x10)) {
        *reinterpret_cast<uint64_t*>(a1) = 0x3F80000000000201ULL;     // Scale = 1.0f (0x3F800000)
        *reinterpret_cast<uint64_t*>(a1 + 8) = 0x2BF800000ULL;
    }
    return 1;
}

// Slot 34: Profile & Armor settings handler (host_probe case 0x22)
static uint64_t __fastcall HostSlot34_ProfileSettings(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 34 - ProfileSettings]: Providing Spartan armor & player loadout data");
    }
    void* targetBuf = nullptr;
    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), 0x100)) {
        targetBuf = reinterpret_cast<void*>(a2);
    } else if (a1 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a1), 0x100)) {
        targetBuf = reinterpret_cast<void*>(a1);
    }

    if (targetBuf) {
        memset(targetBuf, 0, 0x100);
        *reinterpret_cast<uint32_t*>(targetBuf) = 1; // Active player profile valid
        // Permutations
        *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(targetBuf) + 0x04) = 0; // Spartan
        *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(targetBuf) + 0x18) = 0; // Primary color: Steel
        *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(targetBuf) + 0x1C) = 12; // Secondary color: Cyan
    }
    return 1;
}

// Slot 36: Input Controller Query handler (host_probe case 0x24)
static uint64_t __fastcall HostSlot36_InputController(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a3; (void)a4;
    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), 0x130)) {
        memset(reinterpret_cast<void*>(a2), 0, 0x130);
    }
    return 1;
}

// Slot 39: Pad Rumble handler (host_probe case 0x27)
static uint64_t __fastcall HostSlot39_PadRumble(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// Slot 41 & 43: Network Packet handlers (host_probe case 0x29, 0x2B)
static uint64_t __fastcall HostSlot41_NetSend(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

static uint64_t __fastcall HostSlot43_NetRecv(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// Slot 47: User-data folder handler (host_probe case 0x2F)
static wchar_t s_userDataFolderW[MAX_PATH] = L"data";
static uint64_t __fastcall HostSlot47_UserDataFolder(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a4;
    uint32_t kind = static_cast<uint32_t>(a1 & 0xFFFFFFFF);
    NOBLE_INFO("ReachHost [Slot 47 - UserDataFolder]: kind={}", kind);

    auto& host = ReachHost::Get();
    std::filesystem::path userPath = host.GetDataDir() / "userdata" / ("kind" + std::to_string(kind)) / "HaloReach";
    std::error_code ec;
    std::filesystem::create_directories(userPath, ec);

    std::wstring pathW = userPath.wstring();
    wcsncpy_s(s_userDataFolderW, MAX_PATH, pathW.c_str(), _TRUNCATE);

    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), sizeof(wchar_t))) {
        size_t maxChars = (a3 > 0 && a3 < 2048) ? static_cast<size_t>(a3) : 260;
        wcsncpy_s(reinterpret_cast<wchar_t*>(a2), maxChars, pathW.c_str(), _TRUNCATE);
    }
    if (a1 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a1), sizeof(wchar_t))) {
        size_t maxChars = (a2 > 0 && a2 < 2048) ? static_cast<size_t>(a2) : 260;
        wcsncpy_s(reinterpret_cast<wchar_t*>(a1), maxChars, pathW.c_str(), _TRUNCATE);
    }
    return 1;
}

// Slot 48: File path resolver handler (host_probe case 0x30)
static thread_local wchar_t s_resolvedPathBufW[MAX_PATH];
static thread_local char s_resolvedPathBufA[MAX_PATH];

static uint64_t __fastcall HostSlot48_FilePathResolver(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr;
    uint32_t kind = static_cast<uint32_t>(a1 & 0xFFFFFFFF);
    uintptr_t inPtr = (a1 > 0x10000) ? a1 : a2;
    if (inPtr <= 0x10000 || IsBadReadPtr(reinterpret_cast<const void*>(inPtr), 1)) {
        return 0;
    }

    std::wstring inStr;
    const wchar_t* wInput = reinterpret_cast<const wchar_t*>(inPtr);
    if (wInput[0] != 0 && (wInput[1] == 0 || iswprint(wInput[1]))) {
        inStr = wInput;
    } else {
        const char* aInput = reinterpret_cast<const char*>(inPtr);
        if (aInput && aInput[0] != 0) {
            int len = MultiByteToWideChar(CP_UTF8, 0, aInput, -1, nullptr, 0);
            if (len > 0) {
                inStr.resize(len - 1);
                MultiByteToWideChar(CP_UTF8, 0, aInput, -1, inStr.data(), len);
            }
        }
    }

    if (inStr.empty()) return 0;

    auto& host = ReachHost::Get();
    std::filesystem::path resolved;

    std::filesystem::path checkUser = host.GetDataDir() / inStr;
    std::filesystem::path checkUserMenuMap = host.GetDataDir() / "menumaps" / inStr;
    std::filesystem::path checkCustomMainMenu = host.GetDataDir() / "menumaps" / "mainmenu_all.map";
    std::filesystem::path checkUserArmor = host.GetDataDir() / "armor" / inStr;
    std::filesystem::path checkUserMaps = host.GetDataDir() / "maps" / inStr;
    std::filesystem::path mccRoot = host.GetReachDir().has_parent_path() ? host.GetReachDir().parent_path() : host.GetReachDir();
    std::filesystem::path checkMccRelative = mccRoot / inStr;
    std::filesystem::path checkRetailReach = host.GetReachDir() / inStr;
    std::filesystem::path checkRetailMaps = host.GetMapsDir() / inStr;

    if ((inStr.find(L"mainmenu") != std::wstring::npos || inStr.find(L"mainmenu.map") != std::wstring::npos) && std::filesystem::exists(checkCustomMainMenu)) {
        resolved = checkCustomMainMenu;
    } else if (std::filesystem::exists(checkUser)) {
        resolved = checkUser;
    } else if (std::filesystem::exists(checkUserMenuMap)) {
        resolved = checkUserMenuMap;
    } else if (std::filesystem::exists(checkUserArmor)) {
        resolved = checkUserArmor;
    } else if (std::filesystem::exists(checkUserMaps)) {
        resolved = checkUserMaps;
    } else if (std::filesystem::exists(checkMccRelative)) {
        resolved = checkMccRelative;
    } else if (std::filesystem::exists(checkRetailReach)) {
        resolved = checkRetailReach;
    } else if (std::filesystem::exists(checkRetailMaps)) {
        resolved = checkRetailMaps;
    } else {
        if (inStr.find(L".map") == std::wstring::npos) {
            std::filesystem::path withExt = host.GetMapsDir() / (inStr + L".map");
            if (std::filesystem::exists(withExt)) {
                resolved = withExt;
            } else {
                resolved = checkMccRelative;
            }
        } else {
            resolved = checkMccRelative;
        }
    }

    std::wstring resolvedW = resolved.wstring();
    wcsncpy_s(s_resolvedPathBufW, MAX_PATH, resolvedW.c_str(), _TRUNCATE);

    std::string resolvedA = resolved.string();
    strncpy_s(s_resolvedPathBufA, MAX_PATH, resolvedA.c_str(), _TRUNCATE);

    if (a4 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a4), 2)) {
        size_t maxChars = (a3 > 0 && a3 < MAX_PATH) ? static_cast<size_t>(a3) : MAX_PATH;
        strncpy_s(reinterpret_cast<char*>(a4), maxChars, resolvedA.c_str(), _TRUNCATE);
    }
    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), 2)) {
        size_t maxChars = (a3 > 0 && a3 < MAX_PATH) ? static_cast<size_t>(a3) : MAX_PATH;
        strncpy_s(reinterpret_cast<char*>(a2), maxChars, resolvedA.c_str(), _TRUNCATE);
    }

    NOBLE_INFO("ReachHost [Slot 48 - FilePathResolver]: kind={} in='{}' -> resolved='{}'", 
        kind, resolvedA, resolved.string());

    return 1;
}

// Slot 51: Engine capability query handler (host_probe case 0x33)
static uint64_t __fastcall HostSlot51_EngineCapability(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 51 - EngineCapability]: capability query id={}", a1);
    }
    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), sizeof(uint8_t))) {
        *reinterpret_cast<uint8_t*>(a2) = 1;
    }
    return 1; // Return true
}

// Slot 52: Engine State query (host_probe case 0x34)
static uint64_t __fastcall HostSlot52_EngineState(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 1;
}

// Slot 55: Strings Clear handler (host_probe case 0x37)
static uint64_t __fastcall HostSlot55_StringsClear(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// Slot 56: Strings Register handler (host_probe case 0x38)
static uint64_t __fastcall HostSlot56_StringsRegister(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 1;
}

// Slot 57: String lookup handler (host_probe case 0x39)
static const wchar_t s_emptyWideString[] = L"";
static uint64_t __fastcall HostSlot57_StringLookup(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a2; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 57 - StringLookup]: string query token=0x{:X}", a1);
    }
    return reinterpret_cast<uint64_t>(s_emptyWideString);
}

// Slot 88: User identity & Gamertag handler (host_probe case 0x58)
static uint64_t __fastcall HostSlot88_UserIdentity(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 88 - UserIdentity]: Supplying player identity 'NobleSix' (XUID: 0x9000000000001)");
    }
    // Set 64-bit SteamID / XUID in a1
    if (a1 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a1), sizeof(uint64_t))) {
        *reinterpret_cast<uint64_t*>(a1) = 0x0009000000000001ULL;
    }
    // Set UTF-16 gamer tag in a2
    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), 32 * sizeof(wchar_t))) {
        const wchar_t gamerTagW[] = L"NobleSix";
        wcsncpy_s(reinterpret_cast<wchar_t*>(a2), 32, gamerTagW, _TRUNCATE);
        reinterpret_cast<char*>(a2)[31] = '\0';
    }
    return 1;
}

// Slot 97: Capability pass-through (host_probe case 0x61)
static uint64_t __fastcall HostSlot97_CapabilityPass(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 1;
}

// Slot 116: Controller Mapping (host_probe case 0x74)
static uint64_t __fastcall HostSlot116_ControllerMap(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a3; (void)a4;
    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), 64)) {
        memset(reinterpret_cast<void*>(a2), 0xFF, 64);
    }
    return 1;
}

template<size_t... Is>
static void InitVTableHelper(void** vtable, std::index_sequence<Is...>) {
    ((vtable[Is] = reinterpret_cast<void*>(&SlotDispatcher<Is>::Stub)), ...);
}

void ReachHost::InitializeVTable() {
    if (s_vtableInitialized) return;

    InitVTableHelper(s_vtable, std::make_index_sequence<160>{});

    // Slots verified from Reach Renaissance (host-probe.exe) decompilation
    s_vtable[0]   = reinterpret_cast<void*>(&HostSlot0_QueryInterface);
    s_vtable[1]   = reinterpret_cast<void*>(&HostSlot1_FrameTick);
    s_vtable[2]   = reinterpret_cast<void*>(&HostSlot2_ResizeBuffers);
    s_vtable[4]   = reinterpret_cast<void*>(&HostSlot4_LogMessage);
    s_vtable[10]  = reinterpret_cast<void*>(&HostSlot10_Heartbeat);
    s_vtable[18]  = reinterpret_cast<void*>(&HostSlot18_AssetLookup);
    s_vtable[23]  = reinterpret_cast<void*>(&HostSlot23_LoadProgress);
    s_vtable[32]  = reinterpret_cast<void*>(&HostSlot32_DisplaySettings);
    s_vtable[33]  = reinterpret_cast<void*>(&HostSlot33_DisplayQuality);
    s_vtable[34]  = reinterpret_cast<void*>(&HostSlot34_ProfileSettings);
    s_vtable[36]  = reinterpret_cast<void*>(&HostSlot36_InputController);
    s_vtable[39]  = reinterpret_cast<void*>(&HostSlot39_PadRumble);
    s_vtable[41]  = reinterpret_cast<void*>(&HostSlot41_NetSend);
    s_vtable[43]  = reinterpret_cast<void*>(&HostSlot43_NetRecv);
    s_vtable[47]  = reinterpret_cast<void*>(&HostSlot47_UserDataFolder);
    s_vtable[48]  = reinterpret_cast<void*>(&HostSlot48_FilePathResolver);
    s_vtable[51]  = reinterpret_cast<void*>(&HostSlot51_EngineCapability);
    s_vtable[52]  = reinterpret_cast<void*>(&HostSlot52_EngineState);
    s_vtable[55]  = reinterpret_cast<void*>(&HostSlot55_StringsClear);
    s_vtable[56]  = reinterpret_cast<void*>(&HostSlot56_StringsRegister);
    s_vtable[57]  = reinterpret_cast<void*>(&HostSlot57_StringLookup);
    s_vtable[88]  = reinterpret_cast<void*>(&HostSlot88_UserIdentity);
    s_vtable[97]  = reinterpret_cast<void*>(&HostSlot97_CapabilityPass);
    s_vtable[116] = reinterpret_cast<void*>(&HostSlot116_ControllerMap);

    s_vtableInitialized = true;
    NOBLE_INFO("ReachHost: 160-slot host vtable initialized matching all 24 Reach Renaissance host slots.");
}

ReachHost::ReachHost() {
    InitializeVTable();
    m_vptr = s_vtable;
}

ReachHost& ReachHost::Get() {
    static ReachHost s_instance;
    return s_instance;
}

void ReachHost::SetPaths(const std::filesystem::path& reachDir,
                         const std::filesystem::path& mapsDir,
                         const std::filesystem::path& dataDir) {
    m_reachDir = reachDir;
    m_mapsDir = mapsDir;
    m_dataDir = dataDir;
}

} // namespace noble
