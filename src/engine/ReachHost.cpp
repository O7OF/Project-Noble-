#include "ReachHost.hpp"
#include "common/Logger.hpp"
#include <atomic>
#include <array>
#include <utility>
#include <cwctype>

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

// Slot 32: Display / Graphics Settings handler
static uint64_t __fastcall HostSlot32_DisplaySettings(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a2; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 32 - DisplaySettings]: returning 1280x720 windowed display configuration");
    }
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

// Slot 51: Engine capability query handler
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

// Slot 47: User-data folder handler
static const wchar_t s_userDataFolderW[] = L"data";
static uint64_t __fastcall HostSlot47_UserDataFolder(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 47 - UserDataFolder]: returning 'data'");
    }
    if (a1 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a1), sizeof(wchar_t))) {
        wchar_t* outBuf = reinterpret_cast<wchar_t*>(a1);
        size_t maxLen = (a2 > 0 && a2 < 2048) ? static_cast<size_t>(a2) : 260;
        wcsncpy_s(outBuf, maxLen, s_userDataFolderW, _TRUNCATE);
    }
    return reinterpret_cast<uint64_t>(s_userDataFolderW);
}

// Slot 48: File path resolver handler
static thread_local wchar_t s_resolvedPathBufW[MAX_PATH];
static uint64_t __fastcall HostSlot48_FilePathResolver(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a4;
    if (a1 <= 0x10000) {
        return 0;
    }

    std::wstring inStr;
    const wchar_t* wInput = reinterpret_cast<const wchar_t*>(a1);
    // Check if wide string
    if (wInput[0] != 0 && (wInput[1] == 0 || iswprint(wInput[1]))) {
        inStr = wInput;
    } else {
        const char* aInput = reinterpret_cast<const char*>(a1);
        if (aInput[0] != 0) {
            int len = MultiByteToWideChar(CP_UTF8, 0, aInput, -1, nullptr, 0);
            if (len > 0) {
                inStr.resize(len - 1);
                MultiByteToWideChar(CP_UTF8, 0, aInput, -1, inStr.data(), len);
            }
        }
    }

    // Resolve path
    auto& host = ReachHost::Get();
    std::filesystem::path resolved;

    // Check user data folder first (for custom/modded maps like mainmenu.map)
    std::filesystem::path checkUser = host.GetDataDir() / inStr;
    std::filesystem::path checkUserMaps = host.GetDataDir() / "maps" / inStr;
    std::filesystem::path checkRetailMaps = host.GetMapsDir() / inStr;
    std::filesystem::path checkRetail = host.GetReachDir() / inStr;

    if (std::filesystem::exists(checkUser)) {
        resolved = checkUser;
    } else if (std::filesystem::exists(checkUserMaps)) {
        resolved = checkUserMaps;
    } else if (std::filesystem::exists(checkRetailMaps)) {
        resolved = checkRetailMaps;
    } else if (std::filesystem::exists(checkRetail)) {
        resolved = checkRetail;
    } else {
        // Fallback: check if .map extension needs to be appended
        if (inStr.find(L".map") == std::wstring::npos) {
            std::filesystem::path withExt = host.GetMapsDir() / (inStr + L".map");
            if (std::filesystem::exists(withExt)) {
                resolved = withExt;
            } else {
                resolved = host.GetMapsDir() / inStr;
            }
        } else {
            resolved = host.GetMapsDir() / inStr;
        }
    }

    std::wstring resolvedW = resolved.wstring();
    wcsncpy_s(s_resolvedPathBufW, MAX_PATH, resolvedW.c_str(), _TRUNCATE);

    if (a2 > 0x10000 && !IsBadWritePtr(reinterpret_cast<void*>(a2), sizeof(wchar_t))) {
        wchar_t* outBuf = reinterpret_cast<wchar_t*>(a2);
        size_t maxLen = (a3 > 0 && a3 < MAX_PATH) ? static_cast<size_t>(a3) : MAX_PATH;
        wcsncpy_s(outBuf, maxLen, s_resolvedPathBufW, _TRUNCATE);
    }

    char inStrNarrow[MAX_PATH]{};
    WideCharToMultiByte(CP_UTF8, 0, inStr.c_str(), -1, inStrNarrow, sizeof(inStrNarrow), nullptr, nullptr);

    NOBLE_INFO("ReachHost [Slot 48 - FilePathResolver]: in='{}' -> resolved='{}'", 
        inStrNarrow, resolved.string());

    return reinterpret_cast<uint64_t>(s_resolvedPathBufW);
}

// Slot 57: String lookup handler
static const wchar_t s_emptyWideString[] = L"";
static uint64_t __fastcall HostSlot57_StringLookup(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a2; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 57 - StringLookup]: string query token=0x{:X}", a1);
    }
    return reinterpret_cast<uint64_t>(s_emptyWideString);
}

template<size_t... Is>
static void InitVTableHelper(void** vtable, std::index_sequence<Is...>) {
    ((vtable[Is] = reinterpret_cast<void*>(&SlotDispatcher<Is>::Stub)), ...);
}

void ReachHost::InitializeVTable() {
    if (s_vtableInitialized) return;

    InitVTableHelper(s_vtable, std::make_index_sequence<160>{});

    s_vtable[0]  = reinterpret_cast<void*>(&HostSlot0_QueryInterface);
    s_vtable[23] = reinterpret_cast<void*>(&HostSlot23_LoadProgress);
    s_vtable[32] = reinterpret_cast<void*>(&HostSlot32_DisplaySettings);
    s_vtable[47] = reinterpret_cast<void*>(&HostSlot47_UserDataFolder);
    s_vtable[48] = reinterpret_cast<void*>(&HostSlot48_FilePathResolver);
    s_vtable[51] = reinterpret_cast<void*>(&HostSlot51_EngineCapability);
    s_vtable[57] = reinterpret_cast<void*>(&HostSlot57_StringLookup);

    s_vtableInitialized = true;
    NOBLE_INFO("ReachHost: 160-slot host vtable initialized (slots 0, 23, 32, 47, 48, 51, 57 active).");
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
