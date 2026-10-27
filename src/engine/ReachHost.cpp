#include "ReachHost.hpp"
#include "common/Logger.hpp"
#include <windows.h>
#include <atomic>
#include <array>
#include <utility>
#include <cwctype>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <string>

namespace noble {

void* ReachHost::s_vtable[160] = { nullptr };
bool ReachHost::s_vtableInitialized = false;

// -----------------------------------------------------------------------------
// Low-level SEH helpers (no C++ local objects with destructors allowed here)
// -----------------------------------------------------------------------------
static inline bool SafeReadUint32(uintptr_t addr, uint32_t* outVal) {
    if (addr <= 0x10000 || !outVal) return false;
    __try {
        *outVal = *reinterpret_cast<const uint32_t*>(addr);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static inline bool SafeReadPointer(uintptr_t addr, uintptr_t* outPtr) {
    if (addr <= 0x10000 || !outPtr) return false;
    __try {
        *outPtr = *reinterpret_cast<const uintptr_t*>(addr);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static inline bool SafeWriteUint64(uintptr_t addr, uint64_t val) {
    if (addr <= 0x10000) return false;
    __try {
        *reinterpret_cast<uint64_t*>(addr) = val;
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static inline bool SafeWriteUint8(uintptr_t addr, uint8_t val) {
    if (addr <= 0x10000) return false;
    __try {
        *reinterpret_cast<uint8_t*>(addr) = val;
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static inline bool SafeZeroMemory(void* dest, size_t size) {
    if (reinterpret_cast<uintptr_t>(dest) <= 0x10000 || size == 0) return false;
    __try {
        memset(dest, 0, size);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static inline bool SafeCopyMemory(void* dest, const void* src, size_t size) {
    if (reinterpret_cast<uintptr_t>(dest) <= 0x10000 || !src || size == 0) return false;
    __try {
        memcpy(dest, src, size);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static inline bool SafeCopyWideString(wchar_t* dest, size_t maxChars, const wchar_t* src) {
    if (reinterpret_cast<uintptr_t>(dest) <= 0x10000 || !src || maxChars == 0) return false;
    __try {
        wcsncpy_s(dest, maxChars, src, _TRUNCATE);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static inline bool SafeCopyAnsiString(char* dest, size_t maxChars, const char* src) {
    if (reinterpret_cast<uintptr_t>(dest) <= 0x10000 || !src || maxChars == 0) return false;
    __try {
        strncpy_s(dest, maxChars, src, _TRUNCATE);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static inline bool SafeExtractString(uintptr_t inPtr, wchar_t* outBufW, size_t maxW, char* outBufA, size_t maxA, bool* outIsWide) {
    if (inPtr <= 0x10000) return false;
    __try {
        const char* rawBytes = reinterpret_cast<const char*>(inPtr);
        if (rawBytes[0] == 0) return false;

        // In Windows UTF-16 LE ASCII, byte 1 is always 0 (e.g. L"m" is 0x6D, 0x00).
        // In UTF-8 / ANSI ASCII, byte 1 is non-zero (e.g. "ma" is 0x6D, 0x61).
        if (rawBytes[1] == 0 && rawBytes[2] != 0 && rawBytes[3] == 0) {
            const wchar_t* wInput = reinterpret_cast<const wchar_t*>(inPtr);
            wcsncpy_s(outBufW, maxW, wInput, _TRUNCATE);
            *outIsWide = true;
            return true;
        } else {
            strncpy_s(outBufA, maxA, rawBytes, _TRUNCATE);
            *outIsWide = false;
            return true;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// -----------------------------------------------------------------------------
// Slot Dispatcher
// -----------------------------------------------------------------------------
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
    if (a2 > 0x10000) {
        SafeCopyMemory(reinterpret_cast<void*>(a2), &thisPtr, sizeof(void*));
    }
    return 0; // S_OK
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
    if (a1 > 0x10000) {
        const char* msg = reinterpret_cast<const char*>(a1);
        char buf[512] = { 0 };
        if (SafeCopyAnsiString(buf, sizeof(buf), msg) && buf[0] != '\0') {
            NOBLE_INFO("[haloreach.dll] {}", buf);
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
    uintptr_t targetPtr = a1;
    uintptr_t deref = 0;
    if (SafeReadPointer(a1, &deref) && deref > 0x10000) {
        targetPtr = deref;
    }

    if (targetPtr > 0x10000) {
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
        DisplayConfig cfg{};
        cfg.width = 1280;
        cfg.height = 720;
        cfg.refreshRate = 60;
        cfg.adapterIndex = 0;
        cfg.windowMode = 0;
        cfg.displayMode = 0;
        SafeCopyMemory(reinterpret_cast<void*>(targetPtr), &cfg, sizeof(cfg));
    }
    return 1; // Return true to signal successful configuration query
}

// Slot 33: Display Quality settings handler (host_probe case 0x21)
static uint64_t __fastcall HostSlot33_DisplayQuality(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a2; (void)a3; (void)a4;
    uintptr_t targetPtr = a1;
    uintptr_t deref = 0;
    if (SafeReadPointer(a1, &deref) && deref > 0x10000) {
        targetPtr = deref;
    }

    if (targetPtr > 0x10000) {
        uint64_t val1 = 0x3F80000000000201ULL; // Scale = 1.0f (0x3F800000)
        uint64_t val2 = 0x2BF800000ULL;
        SafeWriteUint64(targetPtr, val1);
        SafeWriteUint64(targetPtr + 8, val2);
    }
    return 1;
}

// Slot 34: Profile & Armor settings handler (host_probe case 0x22)
static uint8_t s_hostProfileBuffer[0x10000] = { 0 };
static bool s_hostProfileInitialized = false;

static uint64_t __fastcall HostSlot34_ProfileSettings(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 34 - ProfileSettings]: Providing Spartan armor & player loadout data");
    }

    if (!s_hostProfileInitialized) {
        memset(s_hostProfileBuffer, 0, sizeof(s_hostProfileBuffer));
        try {
            // Try loading authentic profile_1.bin if present in data folder
            auto& host = ReachHost::Get();
            std::filesystem::path profilePath = host.GetDataDir() / "profile_1.bin";
            std::error_code ec;
            if (std::filesystem::exists(profilePath, ec)) {
                std::ifstream pFile(profilePath, std::ios::binary);
                if (pFile) {
                    pFile.read(reinterpret_cast<char*>(s_hostProfileBuffer), sizeof(s_hostProfileBuffer));
                    NOBLE_INFO("ReachHost [Slot 34]: Ingested authentic profile_1.bin ({} bytes)", pFile.gcount());
                }
            }
        } catch (...) {
            NOBLE_WARN("ReachHost [Slot 34]: Failed to ingest profile_1.bin, using defaults");
        }
        if (*reinterpret_cast<uint32_t*>(s_hostProfileBuffer) == 0) {
            *reinterpret_cast<uint32_t*>(s_hostProfileBuffer) = 1; // Active player profile valid
            *reinterpret_cast<uint32_t*>(s_hostProfileBuffer + 0x04) = 0; // Spartan
            *reinterpret_cast<uint32_t*>(s_hostProfileBuffer + 0x18) = 0; // Primary: Steel
            *reinterpret_cast<uint32_t*>(s_hostProfileBuffer + 0x1C) = 12; // Secondary: Cyan
        }
        s_hostProfileInitialized = true;
    }

    // If caller provided a destination buffer, copy profile data into it
    uintptr_t destBuf = (a2 > 0x10000) ? a2 : ((a1 > 0x10000) ? a1 : 0);
    if (destBuf > 0x10000) {
        SafeCopyMemory(reinterpret_cast<void*>(destBuf), s_hostProfileBuffer, 0x100);
    }

    // Reach Renaissance returns the 64KB profile buffer pointer
    return reinterpret_cast<uint64_t>(s_hostProfileBuffer);
}

// Slot 36: Input Controller Query handler (host_probe case 0x24)
static uint64_t __fastcall HostSlot36_InputController(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    uintptr_t outBuf = (a2 > 0x10000) ? a2 : 0;
    if (a1 > 0x10000) {
        uintptr_t pSub = 0;
        if (SafeReadPointer(a1 + 8, &pSub) && pSub > 0x10000) {
            outBuf = pSub;
        }
    }
    if (outBuf > 0x10000) {
        SafeZeroMemory(reinterpret_cast<void*>(outBuf), 0x130);
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
    (void)thisPtr; (void)a3; (void)a4;
    try {
        uint32_t kind = 0;
        uintptr_t outBufferPtr = 0;
        size_t charCapacity = (a2 > 0 && a2 < 4096) ? static_cast<size_t>(a2) : 260;

        if (a1 > 0x10000) {
            // In host-probe: a1 is struct { uint32_t kind; uint32_t pad; wchar_t* outBuffer; }*
            uint32_t k = 0;
            uintptr_t pBuf = 0;
            if (SafeReadUint32(a1, &k)) kind = k;
            if (SafeReadPointer(a1 + 8, &pBuf) && pBuf > 0x10000) outBufferPtr = pBuf;
        } else {
            kind = static_cast<uint32_t>(a1 & 0xFFFFFFFF);
            outBufferPtr = a2;
            if (a3 > 0 && a3 < 4096) charCapacity = static_cast<size_t>(a3);
        }

        auto& host = ReachHost::Get();
        std::filesystem::path userPath = host.GetDataDir() / "userdata" / ("kind" + std::to_string(kind)) / "HaloReach";
        std::error_code ec;
        std::filesystem::create_directories(userPath, ec);

        std::filesystem::path absPath = std::filesystem::absolute(userPath, ec);
        if (ec) absPath = userPath;

        std::wstring pathW = absPath.wstring();
        SafeCopyWideString(s_userDataFolderW, MAX_PATH, pathW.c_str());

        NOBLE_INFO("ReachHost [Slot 47 - UserDataFolder]: kind={} outBuf=0x{:X} cap={} -> '{}'", 
            kind, outBufferPtr, charCapacity, userPath.string());

        if (outBufferPtr > 0x10000) {
            wchar_t* pDest = reinterpret_cast<wchar_t*>(outBufferPtr);
            size_t copyLen = (std::min)(charCapacity > 0 ? charCapacity - 1 : 259, pathW.length());
            SafeCopyMemory(pDest, pathW.c_str(), copyLen * sizeof(wchar_t));
            SafeWriteUint8(reinterpret_cast<uintptr_t>(pDest + copyLen), 0);
        }

        return 0; // Return 0 per host-probe case 0x2F
    } catch (...) {
        return 0;
    }
}

// Slot 48: File path resolver handler (host_probe case 0x30)
static thread_local wchar_t s_resolvedPathBufW[MAX_PATH];
static thread_local char s_resolvedPathBufA[MAX_PATH];

static uint64_t __fastcall HostSlot48_FilePathResolver(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a4; // haloreach.dll passes 3 arguments; a4 is uninitialized stack memory and must never be touched
    try {
        uint32_t kind = static_cast<uint32_t>(a1 & 0xFFFFFFFF);
        uintptr_t inOutPtr = 0;
        size_t capacity = (a3 > 0 && a3 <= 4096) ? static_cast<size_t>(a3) : 260;

        if (a1 > 0x10000) {
            uint32_t k = 0;
            uintptr_t pSub = 0;
            if (SafeReadUint32(a1, &k)) kind = k;
            if (SafeReadPointer(a1 + 8, &pSub) && pSub > 0x10000) {
                inOutPtr = pSub;
            } else {
                inOutPtr = a1;
            }
        } else {
            inOutPtr = a2;
        }

        if (inOutPtr <= 0x10000) return 0;

        wchar_t localW[MAX_PATH] = { 0 };
        char localA[MAX_PATH] = { 0 };
        bool isWide = false;
        if (!SafeExtractString(inOutPtr, localW, MAX_PATH, localA, MAX_PATH, &isWide)) {
            return 0;
        }

        std::wstring inStr;
        if (isWide) {
            inStr = localW;
        } else {
            int len = MultiByteToWideChar(CP_UTF8, 0, localA, -1, nullptr, 0);
            if (len > 0) {
                inStr.resize(len - 1);
                MultiByteToWideChar(CP_UTF8, 0, localA, -1, inStr.data(), len);
            }
        }

        if (inStr.empty()) return 0;

        auto& host = ReachHost::Get();
        std::error_code ec;
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

        bool isMainMenu = (inStr.find(L"mainmenu") != std::wstring::npos) ||
                          (inStr.find(L"mainmenu.map") != std::wstring::npos) ||
                          (inStr.find(L"maps\\.map") != std::wstring::npos) ||
                          (inStr.find(L"maps/.map") != std::wstring::npos) ||
                          (inStr == L"haloreach\\maps\\.map") ||
                          (inStr == L"maps\\.map") ||
                          (inStr == L".map");

        if (isMainMenu && std::filesystem::exists(checkCustomMainMenu, ec)) {
            resolved = checkCustomMainMenu;
        } else if (std::filesystem::exists(checkUser, ec)) {
            resolved = checkUser;
        } else if (std::filesystem::exists(checkUserMenuMap, ec)) {
            resolved = checkUserMenuMap;
        } else if (std::filesystem::exists(checkUserArmor, ec)) {
            resolved = checkUserArmor;
        } else if (std::filesystem::exists(checkUserMaps, ec)) {
            resolved = checkUserMaps;
        } else if (std::filesystem::exists(checkMccRelative, ec)) {
            resolved = checkMccRelative;
        } else if (std::filesystem::exists(checkRetailReach, ec)) {
            resolved = checkRetailReach;
        } else if (std::filesystem::exists(checkRetailMaps, ec)) {
            resolved = checkRetailMaps;
        } else {
            if (inStr.find(L".map") == std::wstring::npos) {
                std::filesystem::path withExt = host.GetMapsDir() / (inStr + L".map");
                if (std::filesystem::exists(withExt, ec)) {
                    resolved = withExt;
                } else {
                    resolved = checkMccRelative;
                }
            } else {
                resolved = checkMccRelative;
            }
        }

        std::filesystem::path absPath = std::filesystem::absolute(resolved, ec);
        if (ec) absPath = resolved;

        std::wstring resolvedW = absPath.wstring();
        SafeCopyWideString(s_resolvedPathBufW, MAX_PATH, resolvedW.c_str());

        std::string resolvedA = absPath.string();
        SafeCopyAnsiString(s_resolvedPathBufA, MAX_PATH, resolvedA.c_str());

        // Safely write the resolved path back into inOutPtr without touching caller stack frames
        if (isWide) {
            SafeCopyWideString(reinterpret_cast<wchar_t*>(inOutPtr), capacity, resolvedW.c_str());
        } else {
            SafeCopyAnsiString(reinterpret_cast<char*>(inOutPtr), capacity, resolvedA.c_str());
        }

        NOBLE_INFO("ReachHost [Slot 48 - FilePathResolver]: kind={} in='{}' -> resolved='{}'", 
            kind, isWide ? localA : std::string(localA), resolvedA);
        spdlog::default_logger()->flush();

        return 1;
    } catch (const std::exception& e) {
        NOBLE_ERROR("ReachHost [Slot 48]: Exception caught: {}", e.what());
        spdlog::default_logger()->flush();
        return 0;
    } catch (...) {
        NOBLE_ERROR("ReachHost [Slot 48]: Unknown exception caught!");
        spdlog::default_logger()->flush();
        return 0;
    }
}

// Slot 51: Engine capability query handler (host_probe case 0x33)
static uint64_t __fastcall HostSlot51_EngineCapability(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 51 - EngineCapability]: capability query id={}", a1);
    }
    if (a2 > 0x10000) {
        SafeWriteUint8(a2, 1);
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

    uintptr_t pXuid = 0;
    uintptr_t pTag = 0;

    if (a1 > 0x10000) {
        uintptr_t deref1 = 0;
        uintptr_t deref2 = 0;
        if (SafeReadPointer(a1, &deref1) && deref1 > 0x10000) pXuid = deref1;
        if (SafeReadPointer(a1 + 8, &deref2) && deref2 > 0x10000) pTag = deref2;
        if (!pXuid) pXuid = a1;
    }

    if (a2 > 0x10000 && !pTag) {
        pTag = a2;
    }

    if (pXuid > 0x10000) {
        SafeWriteUint64(pXuid, 0x0009000000000001ULL);
    }

    if (pTag > 0x10000) {
        const wchar_t gamerTagW[] = L"NobleSix";
        SafeCopyWideString(reinterpret_cast<wchar_t*>(pTag), 32, gamerTagW);
    }

    return 1;
}

// Slot 97: Capability pass-through (host_probe case 0x61)
static uint64_t __fastcall HostSlot97_CapabilityPass(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 1;
}

// Slot 116: Controller Mapping (host_probe case 0x74)
static uint8_t s_controllerMap[64] = { 0 };
static bool s_controllerMapInit = false;

static uint64_t __fastcall HostSlot116_ControllerMap(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a3; (void)a4;
    if (!s_controllerMapInit) {
        memset(s_controllerMap, 0xFF, sizeof(s_controllerMap));
        s_controllerMapInit = true;
    }
    if (a2 > 0x10000) {
        SafeCopyMemory(reinterpret_cast<void*>(a2), s_controllerMap, 64);
    }
    return reinterpret_cast<uint64_t>(s_controllerMap);
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
