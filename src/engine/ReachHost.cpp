#include "ReachHost.hpp"
#include "ui/LoadScreen.hpp"
#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"
#include "ui/PauseMenu.hpp"
#include <windows.h>
#include <shellapi.h>
#include <intrin.h>
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
#include <cstdlib>
#include <cwchar>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace noble {

void* ReachHost::s_vtable[160] = { nullptr };
void* ReachHost::s_vtable2[160] = { nullptr };
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

static inline bool SafeReadInt32(uintptr_t addr, int* outVal) {
    if (addr <= 0x10000 || !outVal) return false;
    __try {
        *outVal = *reinterpret_cast<const int*>(addr);
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

// Secondary interface (returned by slot 10, stored at host[1]) stubs: one per index so the log
// shows exactly which method the engine calls. RR's VTABLE2 has 160 entries; a shorter table made
// calls past index 63 jump to address 0.
template<size_t Slot>
struct SlotDispatcher2 {
    static uint64_t __fastcall Stub(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
        (void)a4;
        static std::atomic<bool> s_firstCall{false};
        if (!s_firstCall.exchange(true)) {
            NOBLE_INFO("ReachHost: iface2 slot {:3d} first call (rcx=0x{:X}, a1=0x{:X}, a2=0x{:X}, a3=0x{:X})",
                Slot, reinterpret_cast<uintptr_t>(thisPtr), a1, a2, a3);
        }
        return 0;
    }
};

// iface2 slot 147 (0x93): RR's host::hit2 returns a pointer to a static 16-byte SESSION_GUID
// (all zero) for this index and 0 for every other index. Returning 0 here made the engine read
// the session id from address 0 while loading a map (crash at haloreach+0x7924D).
alignas(16) static const uint8_t s_sessionGuid[16] = { 0 };

static uint64_t __fastcall Iface2Slot147_SessionGuid(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost: iface2 slot 147 (SessionGuid) first call (a1=0x{:X}, a2=0x{:X}, a3=0x{:X})", a1, a2, a3);
    }
    return reinterpret_cast<uint64_t>(s_sessionGuid);
}

template<size_t... Is>
static void InitVTable2Helper(void** vtable, std::index_sequence<Is...>) {
    ((vtable[Is] = reinterpret_cast<void*>(&SlotDispatcher2<Is>::Stub)), ...);
}

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

static void ExecuteGammaPass(IDXGISwapChain* swapChain);
static void ResetGammaPass();

// Slot 1: Frame tick / Render loop update callback (host_probe case 1)
static uint64_t __fastcall HostSlot1_FrameTick(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a2; (void)a3; (void)a4;
    // haloreach.dll invokes Slot 1 every frame before Present. a1 = the swap chain it is about to
    // present (RR host::hit case 1). Our passes draw here, on the engine thread, because the engine
    // owns the immediate context.
    if (a1 > 0x10000) {
        IDXGISwapChain* swapChain = reinterpret_cast<IDXGISwapChain*>(a1);
        ExecuteGammaPass(swapChain);   // brightness/gamma first
        PauseMenu::Render(swapChain);  // pause menu on top, unaffected by gamma
    }
    static std::atomic<uint64_t> s_frameCount{0};
    uint64_t frames = ++s_frameCount;
    if (frames == 1 || (frames % 300) == 0) {
        // Engine "external launch" state machine (haloreach+0xC1A114): 0..10 while launching, 11 = launched.
        // State 10 waits for the game session to be ready.
        int launchState = -1;
        if (HMODULE reach = GetModuleHandleW(L"haloreach.dll")) {
            SafeReadInt32(reinterpret_cast<uintptr_t>(reach) + 0xC1A114, &launchState);
        }
        NOBLE_INFO("ReachHost [Slot 1 - FrameTick]: Frame {} pumped from haloreach.dll (LoadProgress: {:.1f}%, launch state {})",
            frames, LoadScreen::Get().GetLoadProgress() * 100.0f, launchState);
    }
    return 0;
}

// Slot 2: SwapChain / Buffer resize callback (host_probe case 2)
static uint64_t __fastcall HostSlot2_ResizeBuffers(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    ResetGammaPass();
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 2 - ResizeBuffers]: Engine requested swap chain buffer refresh");
    }
    return 0;
}

// Slot 4: Engine logging stream (host_probe case 4)
// Slot 4 (RR host::hit case 4): a1 = reason code, a2 = const char* message.
// The engine reports session/launch events here, e.g. reason 4 with
// "external_launch_individual_state_timeout" / "external_launch_overall_timeout".
static uint64_t __fastcall HostSlot4_LogMessage(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    char buf[512] = { 0 };
    if (a2 > 0x10000) {
        SafeCopyAnsiString(buf, sizeof(buf), reinterpret_cast<const char*>(a2));
    }
    NOBLE_WARN("ReachHost [Slot 4 - EngineEvent]: reason={} message='{}'", a1, buf);
    return 0;
}

// Slot 10: Secondary host interface (host_probe case 10 returns SECOND, the iface2 object
// that host::new_host stores at host[1]). Returning 1 here would hand the engine a bad pointer.
static uint64_t __fastcall HostSlot10_Heartbeat(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 10 - SecondInterface]: returning secondary host interface");
    }
    return reinterpret_cast<uint64_t>(ReachHost::Get().GetSecondInterface());
}

// Slot 18: Asset lookup (host_probe case 0x12)
static uint64_t __fastcall HostSlot18_AssetLookup(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// Slot 23: Load progress handler (host-probe slot23 at 0x1400822a0)
static std::atomic<uint32_t> s_loadPhases{0};
static std::atomic<float> s_loadFraction{0.0f};

static uint64_t __fastcall HostSlot23_LoadProgress(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    void* caller = _ReturnAddress();
    
    // a1 is phase index or integer percentage, a2 is optional progress float
    uint32_t phaseIndex = static_cast<uint32_t>(a1 & 0xFFFFFFFF);
    if (phaseIndex < 32) {
        s_loadPhases.fetch_or(1u << (phaseIndex & 0x1F));
    }

    float fraction = 0.0f;
    if (a1 <= 100) {
        fraction = static_cast<float>(a1) / 100.0f;
    } else {
        memcpy(&fraction, &a1, sizeof(float));
    }

    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;

    s_loadFraction.store(fraction);
    LoadScreen::Get().SetLoadProgress(fraction);

    static float s_lastLoggedFraction = -1.0f;
    if (std::abs(fraction - s_lastLoggedFraction) >= 0.01f || fraction >= 1.0f) {
        s_lastLoggedFraction = fraction;
        NOBLE_INFO("ReachHost [Slot 23 - LoadProgress]: progress={:.1f}% (raw=0x{:X}, phases=0x{:08X}) [Caller: 0x{:X}]",
            fraction * 100.0f, a1, s_loadPhases.load(), reinterpret_cast<uintptr_t>(caller));
    }
    return 0;
}

// Slot 32: Display settings fill (host_probe case 0x20)
// Matches Reach Renaissance host::hit case 0x20: a1 = engine-owned buffer, a2 = its size (0x88 seen).
// RR zeroes the buffer, writes {width, height} as two u32 at +0x00, quality bytes at +0x08..+0x17
// (01 02 02 02 02 02 01 01 | 00 01 01 01 00 02 00 00), and returns 1.
// The previous version treated this as an allocation request and returned its own buffer, leaving
// the engine's settings struct uninitialised.
static uint64_t __fastcall HostSlot32_DisplaySettings(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 32 - DisplaySettings]: buffer=0x{:X} size=0x{:X}", a1, a2);
    }
    if (a1 <= 0x10000 || a2 < 0x18) {
        return 0;
    }

    const size_t size = static_cast<size_t>(a2);
    if (!SafeZeroMemory(reinterpret_cast<void*>(a1), size)) {
        return 0;
    }

    uint8_t header[0x18] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // width, height (filled below)
        0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01,   // quality settings (RR defaults)
        0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x00
    };
    const uint32_t width = 1280;   // host window client size (Window is created at 1280x720)
    const uint32_t height = 720;
    memcpy(&header[0], &width, sizeof(width));
    memcpy(&header[4], &height, sizeof(height));
    SafeCopyMemory(reinterpret_cast<void*>(a1), header, sizeof(header));
    return 1;
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
//
// The engine takes its keyboard/mouse bindings from this profile. Without them every key
// reaches the engine (slot 36) but is bound to nothing; only mouse look works.
// Binding table: 66 entries of 0x18 bytes from +0x42C, each { u32 action, u32 VK, 16 zero bytes },
// actions 0..65. The keys below are MCC's default PC layout (actions 62..65 are unbound).
static constexpr size_t kBindingTableOffset = 0x42C;
static constexpr size_t kBindingEntrySize = 0x18;
static const uint8_t kDefaultKeyBindings[66] = {
    /*  0 */ VK_SPACE, '2',        'E',        'R',        '1',        'Q',        '4',        'F',
    /*  8 */ VK_LBUTTON, VK_LMENU, VK_RBUTTON, 'Z',        'X',        'C',        'C',        'Q',
    /* 16 */ 'W',        'S',      'A',        'D',        VK_TAB,     VK_LCONTROL, VK_SPACE, VK_LSHIFT,
    /* 24 */ VK_RBUTTON, 'R',      'F',        VK_LBUTTON, VK_LSHIFT,  VK_LCONTROL, VK_DELETE, '1',
    /* 32 */ '2',        VK_OEM_3, VK_RBUTTON, VK_NUMPAD4, 'X',        'Z',        'C',        VK_RBUTTON,
    /* 40 */ VK_RIGHT,   VK_LEFT,  VK_RETURN,  'E',        VK_SPACE,   VK_LSHIFT,  VK_NUMPAD5, 'R',
    /* 48 */ 'F',        VK_RBUTTON, VK_RBUTTON, 'E',      'G',        'C',        'X',        'F',
    /* 56 */ '3',        'G',      'L',        VK_UP,      VK_DOWN,    'P',        0,          0,
    /* 64 */ 0,          0,
};

static void WriteDefaultKeyBindings(uint8_t* profile) {
    for (uint32_t action = 0; action < 66; ++action) {
        uint8_t* entry = profile + kBindingTableOffset + action * kBindingEntrySize;
        const uint32_t vk = kDefaultKeyBindings[action];
        memcpy(entry + 0, &action, sizeof(action));
        memcpy(entry + 4, &vk, sizeof(vk));
    }
}

// Audio volumes are profile settings too (RR live::audio::set_volume): f32 at +0xA5C (game/master),
// +0xA60 (music) and +0xA64 (the third volume). A blank profile leaves them at 0.0, i.e. silence.
static constexpr size_t kVolumeGameOffset = 0xA5C;
static constexpr size_t kVolumeMusicOffset = 0xA60;
static constexpr size_t kVolumeOtherOffset = 0xA64;

static void WriteDefaultVolumes(uint8_t* profile) {
    const float full = 1.0f;
    memcpy(profile + kVolumeGameOffset, &full, sizeof(full));
    memcpy(profile + kVolumeMusicOffset, &full, sizeof(full));
    memcpy(profile + kVolumeOtherOffset, &full, sizeof(full));
}

// Look / controller settings block (+0x1B4..+0x1DF). Without it zoomed look sensitivity is 0 and
// the view freezes while scoped (walking still works). RR's live::save_pad_look writes into this
// block (+0x1B5, +0x1B7 as u8, +0x1B8 as u32/f32). Values are the stock settings: sensitivity
// bytes, two dead-zone floats (0.13, 0.12), two 1.0 multipliers, then on/off option bytes.
static constexpr size_t kLookSettingsOffset = 0x1B4;
static const uint8_t kDefaultLookSettings[0x2C] = {
    0x00, 0x12, 0x12, 0x07,                         // +0x1B4 sensitivity bytes
    0xB8, 0x1E, 0x05, 0x3E,                         // +0x1B8 f32 0.13
    0x8F, 0xC2, 0xF5, 0x3D,                         // +0x1BC f32 0.12
    0x00, 0x00, 0x80, 0x3F,                         // +0x1C0 f32 1.0
    0x00, 0x00, 0x80, 0x3F,                         // +0x1C4 f32 1.0
    0x09, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, // +0x1C8 option bytes
    0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, // +0x1D0
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, // +0x1D8
};

static void WriteDefaultLookSettings(uint8_t* profile) {
    memcpy(profile + kLookSettingsOffset, kDefaultLookSettings, sizeof(kDefaultLookSettings));
}

// "-profile <path>" (or "--profile <path>") on the command line, else empty.
static std::wstring ProfileArgPath() {
    std::wstring result;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        for (int i = 1; i + 1 < argc; ++i) {
            if (wcscmp(argv[i], L"-profile") == 0 || wcscmp(argv[i], L"--profile") == 0) {
                result = argv[i + 1];
            }
        }
        LocalFree(argv);
    }
    return result;
}

static uint8_t s_hostProfileBuffer[0x10000] = { 0 };
static bool s_hostProfileInitialized = false;

static uint64_t __fastcall HostSlot34_ProfileSettings(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    static std::atomic<bool> s_logged{false};
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 34 - ProfileSettings]: Providing Spartan armor & player loadout data");
    }

    if (!s_hostProfileInitialized) {
        memset(s_hostProfileBuffer, 0, sizeof(s_hostProfileBuffer));
        bool loaded = false;
        try {
            // Optional: "-profile <file>" loads a captured profile, like RR's --profile.
            // RR host::load_profile copies at most 0xC00 bytes of it.
            const std::wstring path = ProfileArgPath();
            if (!path.empty()) {
                std::filesystem::path profilePath(path);
                if (profilePath.is_relative()) {
                    // The working directory is moved to data\sandbox during start-up, so resolve
                    // relative paths against the folder noble-client.exe is in.
                    wchar_t exe[MAX_PATH] = {};
                    if (GetModuleFileNameW(nullptr, exe, MAX_PATH)) {
                        profilePath = std::filesystem::path(exe).parent_path() / profilePath;
                    }
                }
                std::ifstream pFile(profilePath, std::ios::binary);
                if (pFile) {
                    pFile.read(reinterpret_cast<char*>(s_hostProfileBuffer), 0xC00);
                    loaded = pFile.gcount() > 0;
                    NOBLE_INFO("ReachHost [Slot 34]: loaded profile from -profile ({} bytes)", pFile.gcount());
                } else {
                    NOBLE_WARN("ReachHost [Slot 34]: could not open -profile file; using built-in key bindings");
                }
            }
        } catch (...) {
            NOBLE_WARN("ReachHost [Slot 34]: failed to read -profile file; using built-in key bindings");
        }
        if (!loaded) {
            WriteDefaultKeyBindings(s_hostProfileBuffer);
            WriteDefaultVolumes(s_hostProfileBuffer);
            WriteDefaultLookSettings(s_hostProfileBuffer);
            NOBLE_INFO("ReachHost [Slot 34]: using built-in default key bindings, volumes and look settings");
        }
        // RR host::hit case 0x22: the profile is a zeroed 64 KB block with only the 4-character
        // service tag (UTF-16) written at +0x1AC. The old defaults here (+0x00 = 1, armour colours
        // at +0x18/+0x1C) are not what RR sends, so they are no longer written.
        static const wchar_t kServiceTag[4] = { L'N', L'O', L'B', L'L' };
        memcpy(s_hostProfileBuffer + 0x1AC, kServiceTag, sizeof(kServiceTag));
        s_hostProfileInitialized = true;
    }

    // RR only returns the pointer; it never writes into a1/a2 (the old code copied 0x100 bytes
    // into whichever argument looked like a pointer).
    return reinterpret_cast<uint64_t>(s_hostProfileBuffer);
}

// Slot 36: local-user input poll (RR host::hit case 0x24). Called every engine tick.
// a1 = user index, a2 = 0x130-byte input state the host fills. Layout (from RR's handler):
//   +0x000 u32      1 = a local user is present
//   +0x004 u8[256]  key state indexed by VK code: 0 = up, 0x80 = pressed (< 500 ms),
//                   0x81..0xFF = held (repeat counter, as RR computes it)
//   +0x104 f32 x2   mouse look delta (x, y), already scaled
//   +0x114 i32      mouse wheel pulse
//   +0x118 f32      1.0
//   +0x11C u32      mouse buttons: L=1, M=2, R=4, X1=8, X2=0x10
// Index 1 is the split-screen second user: zeroed, as RR does.
static ULONGLONG s_keyDownSince[256] = {};
static bool s_inputCaptured = false;
static HCURSOR s_savedClassCursor = nullptr;
static std::atomic<bool> s_slot36Logged{false};

static float InputMouseScale() {
    static float s_scale = [] {
        float scale = 0.0008f; // RR's default before its sensitivity multiplier
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv) {
            for (int i = 1; i + 1 < argc; ++i) {
                if (wcscmp(argv[i], L"-mouse-scale") == 0 || wcscmp(argv[i], L"--mouse-scale") == 0) {
                    float v = static_cast<float>(_wtof(argv[i + 1]));
                    if (v > 0.0f) scale = v;
                }
            }
            LocalFree(argv);
        }
        return scale;
    }();
    return s_scale;
}

// Returns our window if it is the foreground window, else nullptr.
static HWND InputFocusedWindow() {
    HWND fg = GetForegroundWindow();
    if (!fg) return nullptr;
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    return (pid == GetCurrentProcessId()) ? fg : nullptr;
}

// UI cursor (menus such as the class/loadout picker). As in RR's uicursor module, haloreach+0x2D2018
// runs only while a Reach screen that uses the mouse pointer is up; we hook it and record when it
// last ran. While it ran within the last 250 ms the pointer is "free": not clipped, not re-centred,
// the normal arrow is shown, and no mouse-look delta is sent. The engine reads the pointer position
// itself (GetCursorPos/ScreenToClient).
static constexpr uintptr_t kUiScreenRva = 0x2D2018;
using UiScreenFn = uint64_t(__fastcall*)(uint64_t, uint64_t, uint64_t, uint64_t);
static UiScreenFn s_origUiScreen = nullptr;
static std::atomic<ULONGLONG> s_uiScreenLastTick{0};

static uint64_t __fastcall UiScreenHook(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    s_uiScreenLastTick.store(GetTickCount64(), std::memory_order_relaxed);
    return s_origUiScreen(a1, a2, a3, a4);
}

static void EnsureUiCursorHook() {
    static bool s_tried = false;
    if (s_tried) return;
    s_tried = true;
    HMODULE reach = GetModuleHandleW(L"haloreach.dll");
    if (!reach) return;
    uint8_t* target = reinterpret_cast<uint8_t*>(reach) + kUiScreenRva;
    // Same check RR makes: the function must start with "mov [rsp+18h], rbx" for this build.
    static const uint8_t kPrologue[5] = { 0x48, 0x89, 0x5C, 0x24, 0x18 };
    uint8_t current[5] = {};
    if (!SafeCopyMemory(current, target, sizeof(current)) || memcmp(current, kPrologue, sizeof(kPrologue)) != 0) {
        NOBLE_WARN("ReachHost [UI cursor]: haloreach+0x{:X} does not match the expected build; menu cursor disabled", kUiScreenRva);
        return;
    }
    if (HookManager::InstallHook(target, reinterpret_cast<void*>(&UiScreenHook),
                                 reinterpret_cast<void**>(&s_origUiScreen), "ui_cursor_screen")) {
        NOBLE_INFO("ReachHost [UI cursor]: screen hook installed");
    }
}

static bool UiCursorActive() {
    const ULONGLONG last = s_uiScreenLastTick.load(std::memory_order_relaxed);
    return last != 0 && GetTickCount64() - last < 250;
}

static void InputRelease(HWND hwnd, const char* reason) {
    if (!s_inputCaptured) return;
    ClipCursor(nullptr);
    if (hwnd) {
        HCURSOR arrow = s_savedClassCursor ? s_savedClassCursor : LoadCursorW(nullptr, IDC_ARROW);
        SetClassLongPtrW(hwnd, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(arrow));
    }
    s_inputCaptured = false;
    NOBLE_INFO("ReachHost [Slot 36 - Input]: {}, mouse released", reason);
}

static void FillLocalInput(uint8_t* st) {
    *reinterpret_cast<uint32_t*>(st + 0x000) = 1;
    *reinterpret_cast<float*>(st + 0x118) = 1.0f;

    static HWND s_lastWindow = nullptr;
    HWND hwnd = InputFocusedWindow();
    if (!hwnd) {
        InputRelease(s_lastWindow, "window lost focus");
        memset(s_keyDownSince, 0, sizeof(s_keyDownSince));
        return;
    }
    s_lastWindow = hwnd;

    const bool uiPointer = UiCursorActive();

    // Esc opens/closes Noble's pause menu, except while a Reach menu owns the pointer (there Esc
    // goes to the engine, e.g. to back out of a screen).
    static bool s_escWasDown = false;
    const bool escDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    if (escDown && !s_escWasDown && !uiPointer) {
        PauseMenu::Toggle();
    }
    s_escWasDown = escDown;

    // While the pause menu is open the game gets no keys or mouse, and the pointer is free.
    static bool s_holdMouseUntilRelease = false;
    if (PauseMenu::IsOpen()) {
        InputRelease(hwnd, "pause menu open");
        memset(s_keyDownSince, 0, sizeof(s_keyDownSince));
        s_holdMouseUntilRelease = true;
        return;
    }
    // After the menu closes, ignore the mouse buttons until they are released, so the click on
    // RESUME doesn't fire a shot.
    if (s_holdMouseUntilRelease) {
        const bool anyButton = (GetAsyncKeyState(VK_LBUTTON) | GetAsyncKeyState(VK_RBUTTON) | GetAsyncKeyState(VK_MBUTTON)) & 0x8000;
        if (!anyButton) s_holdMouseUntilRelease = false;
    }

    bool firstCapturedFrame = false;
    POINT center{};
    if (uiPointer) {
        // A menu is up: free the pointer and show the arrow (the window proc shows the class cursor).
        InputRelease(hwnd, "menu with pointer is up");
    } else {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        POINT tl{rc.left, rc.top}, br{rc.right, rc.bottom};
        ClientToScreen(hwnd, &tl);
        ClientToScreen(hwnd, &br);
        RECT clip{tl.x, tl.y, br.x, br.y};
        center = POINT{(tl.x + br.x) / 2, (tl.y + br.y) / 2};

        if (!s_inputCaptured) {
            HCURSOR cls = reinterpret_cast<HCURSOR>(GetClassLongPtrW(hwnd, GCLP_HCURSOR));
            if (cls) s_savedClassCursor = cls;
            SetClassLongPtrW(hwnd, GCLP_HCURSOR, 0); // hide the arrow over our window
            s_inputCaptured = true;
            firstCapturedFrame = true;
            NOBLE_INFO("ReachHost [Slot 36 - Input]: mouse captured for look (scale {})", InputMouseScale());
        }
        ClipCursor(&clip);
    }

    // Keyboard and mouse buttons by VK code.
    const ULONGLONG now = GetTickCount64();
    for (int vk = 1; vk < 256; ++vk) {
        uint8_t v = 0;
        if (GetAsyncKeyState(vk) < 0) {
            if (s_keyDownSince[vk] == 0) s_keyDownSince[vk] = now;
            ULONGLONG held = now - s_keyDownSince[vk];
            if (held < 500) {
                v = 0x80;
            } else {
                ULONGLONG n = (held - 500) / 33;
                if (n > 0x7E) n = 0x7E;
                v = static_cast<uint8_t>(static_cast<int>(n) - 0x7F);
            }
        } else {
            s_keyDownSince[vk] = 0;
        }
        st[0x004 + vk] = v;
    }

    if (!uiPointer) {
        st[0x004 + VK_ESCAPE] = 0; // Esc was ours (pause menu)
    }
    if (s_holdMouseUntilRelease) {
        st[0x004 + VK_LBUTTON] = st[0x004 + VK_RBUTTON] = st[0x004 + VK_MBUTTON] = 0;
    }

    // Mouse look: delta from the window centre, then recentre (not while a menu owns the pointer).
    float dx = 0.0f, dy = 0.0f;
    if (!uiPointer) {
        POINT pos{};
        if (GetCursorPos(&pos) && !firstCapturedFrame) {
            dx = static_cast<float>(pos.x - center.x);
            dy = static_cast<float>(pos.y - center.y);
        }
        SetCursorPos(center.x, center.y);
    }
    const float scale = InputMouseScale();
    *reinterpret_cast<float*>(st + 0x104) = dx * scale;
    *reinterpret_cast<float*>(st + 0x108) = dy * scale;

    uint32_t buttons = 0;
    if (st[0x004 + VK_LBUTTON])  buttons |= 0x01;
    if (st[0x004 + VK_MBUTTON])  buttons |= 0x02;
    if (st[0x004 + VK_RBUTTON])  buttons |= 0x04;
    if (st[0x004 + VK_XBUTTON1]) buttons |= 0x08;
    if (st[0x004 + VK_XBUTTON2]) buttons |= 0x10;
    *reinterpret_cast<uint32_t*>(st + 0x11C) = buttons;
}

static uint64_t __fastcall HostSlot36_InputController(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a3; (void)a4;
    uint64_t userIndex = a1;
    uintptr_t outBuf = (a2 > 0x10000) ? a2 : 0;
    if (a1 > 0x10000) { // older calling-convention guess: a1 -> { int index; void* buffer; }
        int idx = 0;
        uintptr_t pSub = 0;
        if (SafeReadInt32(a1, &idx)) userIndex = static_cast<uint32_t>(idx);
        if (SafeReadPointer(a1 + 8, &pSub) && pSub > 0x10000) outBuf = pSub;
    }
    EnsureUiCursorHook();
    if (!s_slot36Logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 36 - Input]: first call (a1=0x{:X}, a2=0x{:X}) -> user {} buffer 0x{:X}", a1, a2, userIndex, outBuf);
    }
    if (outBuf <= 0x10000) return 1;

    alignas(16) uint8_t state[0x130] = {};
    if (static_cast<uint32_t>(userIndex) == 0) {
        FillLocalInput(state);
    }
    SafeCopyMemory(reinterpret_cast<void*>(outBuf), state, sizeof(state));
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

        static std::unordered_map<std::string, bool> s_seenPaths;
        static std::mutex s_seenMutex;
        bool isNew = false;
        {
            std::lock_guard<std::mutex> lock(s_seenMutex);
            std::string key = std::string(localA) + "->" + resolvedA;
            if (s_seenPaths.find(key) == s_seenPaths.end()) {
                s_seenPaths[key] = true;
                isNew = true;
            }
        }
        if (isNew) {
            NOBLE_INFO("ReachHost [Slot 48 - FilePathResolver]: kind={} in='{}' -> resolved='{}'", 
                kind, localA, resolvedA);
        }

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

// -----------------------------------------------------------------------------
// Text host slots (61-67). In MCC's PC build the host owns font rendering: Reach Renaissance
// installs text::{prepare, glyph, atlas, has_icon, kerning, register_icon, metrics} at
// slots 0x3D-0x43 (host::new_host @ 0x1400828E0). Signatures below come from those functions'
// register/stack usage. This is a first, minimal implementation written for Noble:
//   61 prepare(host, const wchar_t* text, u32 style, float size, const char* face) -> 1
//   67 metrics(host, u32 style, float size, const char* face, u16* out0, u16* out1) -> 1 if font
// Glyph/atlas rendering (62/63) is not implemented yet.
// -----------------------------------------------------------------------------
// SEH-only copy helpers (no C++ objects in these frames).
static size_t SafeCopyAnsiRaw(const char* src, char* dst, size_t cap) {
    if (!src || !dst || cap == 0) return 0;
    size_t n = 0;
    __try {
        while (n + 1 < cap && src[n]) { dst[n] = src[n]; ++n; }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    dst[n] = 0;
    return n;
}

static size_t SafeCopyWideRaw(const wchar_t* src, wchar_t* dst, size_t cap) {
    if (!src || !dst || cap == 0) return 0;
    size_t n = 0;
    __try {
        while (n + 1 < cap && src[n]) { dst[n] = src[n]; ++n; }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    dst[n] = 0;
    return n;
}

static bool SafeWriteU16(uint16_t* dst, uint16_t value) {
    if (reinterpret_cast<uintptr_t>(dst) <= 0x10000) return false;
    __try {
        *dst = value;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static std::wstring WidenFace(const char* face) {
    char buf[64];
    const size_t n = SafeCopyAnsiRaw(face, buf, sizeof(buf));
    std::wstring w;
    for (size_t i = 0; i < n; ++i) w.push_back(static_cast<wchar_t>(static_cast<unsigned char>(buf[i])));
    return w;
}

static std::string NarrowForLog(const std::wstring& w) {
    std::string out;
    for (wchar_t c : w) out.push_back(c < 0x80 ? static_cast<char>(c) : '?');
    return out;
}

static std::wstring MapReachFace(const std::wstring& requested) {
    // Reach's UI fonts are not installed on Windows; RR maps TVNordCond to Bahnschrift Condensed.
    if (requested.empty()) return L"Segoe UI";
    std::wstring lower = requested;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    if (lower.find(L"tvnord") != std::wstring::npos) return L"Bahnschrift Condensed";
    return requested;
}

struct HostFontMetrics {
    uint16_t ascent = 0;
    uint16_t descent = 0;
    bool ok = false;
};

static HostFontMetrics MeasureFont(const std::wstring& face, uint32_t pointSize, float scale) {
    HostFontMetrics m;
    // RR's Atlas::font: pixels = round(size * scale * SIZE_SCALE); the engine passes size as an
    // integer (9, 13, 15, 18, 24, 30, 42 seen) and scale as a float (1.0 seen).
    int pixelHeight = static_cast<int>(static_cast<float>(pointSize) * scale + 0.5f);
    if (pixelHeight < 1) pixelHeight = 1;
    if (pixelHeight > 512) pixelHeight = 512;

    HDC dc = CreateCompatibleDC(nullptr);
    if (!dc) return m;
    for (int attempt = 0; attempt < 2 && !m.ok; ++attempt) {
        const wchar_t* name = attempt == 0 ? face.c_str() : L"Segoe UI";
        HFONT font = CreateFontW(-pixelHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, name);
        if (!font) continue;
        HGDIOBJ old = SelectObject(dc, font);
        TEXTMETRICW tm{};
        if (GetTextMetricsW(dc, &tm)) {
            m.ascent = static_cast<uint16_t>(tm.tmAscent);
            m.descent = static_cast<uint16_t>(tm.tmDescent);
            m.ok = true;
        }
        SelectObject(dc, old);
        DeleteObject(font);
    }
    DeleteDC(dc);
    return m;
}

static std::wstring SafeReadWide(const wchar_t* text, size_t maxChars) {
    wchar_t buf[128];
    if (maxChars > 127) maxChars = 127;
    SafeCopyWideRaw(text, buf, maxChars + 1);
    return std::wstring(buf);
}

static uint64_t __fastcall HostSlot61_TextPrepare(void* thisPtr, const wchar_t* text, uint32_t style, float size, const char* face) {
    (void)thisPtr;
    static std::atomic<int> s_calls{0};
    if (s_calls.fetch_add(1) < 20) {
        NOBLE_INFO("ReachHost [Slot 61 - TextPrepare]: size={} scale={:.2f} face='{}' text='{}'",
            style, size, NarrowForLog(WidenFace(face)), NarrowForLog(SafeReadWide(text, 48)));
    }
    return 1;
}

// -----------------------------------------------------------------------------
// Font Glyph and Atlas Subsystem (Slots 62-66). Matches Reach Renaissance text::{glyph, atlas, has_icon, kerning, register_icon}.
// haloreach.dll queries glyph data (Slot 62) and binds the font texture atlas SRV (Slot 63)
// to render in-game HUD numbers, weapon bullet counts, clip reserves, and scoreboard text.
// -----------------------------------------------------------------------------
#pragma pack(push, 1)
struct HostGlyphData {
    uint16_t ch;        // +0x00 character code (wchar_t)
    uint16_t atlasX;    // +0x02 X in atlas (pixels)
    uint16_t padX[3];   // +0x04 upper 48 bits of X (0)
    uint16_t atlasY;    // +0x0A Y in atlas (pixels)
    uint16_t width;     // +0x0C bounding width (gmBlackBoxX)
    uint16_t height;    // +0x0E bounding height (gmBlackBoxY)
    int16_t  originX;   // +0x10 origin.x
    int16_t  originY;   // +0x12 origin.y
    int16_t  advanceX;  // +0x14 cellIncX
    int16_t  ascent;    // +0x16 -ascent
    uint8_t  page;      // +0x18 page index (0)
    uint8_t  reserved;  // +0x19 0
};
#pragma pack(pop)
static_assert(sizeof(HostGlyphData) == 26, "HostGlyphData must be exactly 26 bytes");

class TextAtlas {
public:
    static TextAtlas& Get() {
        static TextAtlas instance;
        return instance;
    }

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_initialized && m_device == device) return;

        m_device = device;
        m_context = context;
        if (!m_device) return;

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = 1024;
        desc.Height = 1024;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        m_pixels.assign(1024 * 1024, 0);

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = m_pixels.data();
        initData.SysMemPitch = 1024 * sizeof(uint32_t);

        m_texture.Reset();
        m_srv.Reset();

        if (SUCCEEDED(m_device->CreateTexture2D(&desc, &initData, m_texture.GetAddressOf()))) {
            m_device->CreateShaderResourceView(m_texture.Get(), nullptr, m_srv.GetAddressOf());
        }

        m_initialized = true;
        NOBLE_INFO("TextAtlas: 1024x1024 dynamic font atlas texture initialized.");

        // Pre-warm essential HUD digits '0'-'9' and common symbols
        PrewarmCommonGlyphs();
    }

    ID3D11ShaderResourceView* GetSRV(uint32_t pageIndex) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (pageIndex == 0) {
            return m_srv.Get();
        }
        return nullptr;
    }

    bool GetGlyph(wchar_t ch, uint32_t style, float scale, const char* face, HostGlyphData* outGlyph) {
        if (!outGlyph) return false;

        uint32_t scaleKey = static_cast<uint32_t>(scale * 1000.0f);
        uint64_t key = (static_cast<uint64_t>(ch) << 32) | (static_cast<uint64_t>(style) << 16) | (scaleKey & 0xFFFF);

        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            *outGlyph = it->second;
            return true;
        }

        return RasterizeGlyphLocked(ch, style, scale, face, key, outGlyph);
    }

private:
    void PrewarmCommonGlyphs() {
        const wchar_t commonChars[] = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz/:.-+ /";
        const uint32_t commonStyles[] = { 9, 13, 15, 18, 24, 30, 42 };

        HostGlyphData dummy{};
        for (uint32_t s : commonStyles) {
            for (size_t i = 0; i < (sizeof(commonChars) / sizeof(wchar_t)) - 1; ++i) {
                uint64_t key = (static_cast<uint64_t>(commonChars[i]) << 32) | (static_cast<uint64_t>(s) << 16) | 1000;
                if (m_cache.find(key) == m_cache.end()) {
                    RasterizeGlyphLocked(commonChars[i], s, 1.0f, "tvnord", key, &dummy);
                }
            }
        }
        if (m_context && m_texture) {
            m_context->UpdateSubresource(m_texture.Get(), 0, nullptr, m_pixels.data(), 1024 * sizeof(uint32_t), 0);
        }
    }

    bool RasterizeGlyphLocked(wchar_t ch, uint32_t style, float scale, const char* face, uint64_t key, HostGlyphData* outGlyph) {
        std::wstring requestedFace = WidenFace(face);
        std::wstring mappedFace = MapReachFace(requestedFace);

        int pixelHeight = static_cast<int>(static_cast<float>(style) * scale + 0.5f);
        if (pixelHeight < 1) pixelHeight = 1;
        if (pixelHeight > 256) pixelHeight = 256;

        HDC hdc = CreateCompatibleDC(nullptr);
        if (!hdc) return false;

        HFONT font = CreateFontW(-pixelHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, mappedFace.c_str());
        if (!font) {
            font = CreateFontW(-pixelHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                               OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        }

        HGDIOBJ oldFont = SelectObject(hdc, font);
        TEXTMETRICW tm{};
        GetTextMetricsW(hdc, &tm);

        GLYPHMETRICS gm{};
        MAT2 mat2 = { {0,1}, {0,0}, {0,0}, {0,1} };
        DWORD bufSize = GetGlyphOutlineW(hdc, ch, GGO_GRAY8_BITMAP, &gm, 0, nullptr, &mat2);

        std::vector<uint8_t> grayBuffer;
        if (bufSize != GDI_ERROR && bufSize > 0) {
            grayBuffer.resize(bufSize);
            GetGlyphOutlineW(hdc, ch, GGO_GRAY8_BITMAP, &gm, bufSize, grayBuffer.data(), &mat2);
        } else if (bufSize == 0) {
            // Space or non-rendering glyph
            gm.gmBlackBoxX = 0;
            gm.gmBlackBoxY = 0;
        } else {
            // Fallback for symbols where GetGlyphOutlineW is unsupported
            SIZE sz{};
            GetTextExtentPoint32W(hdc, &ch, 1, &sz);
            gm.gmBlackBoxX = static_cast<UINT>(sz.cx);
            gm.gmBlackBoxY = static_cast<UINT>(sz.cy);
            gm.gmCellIncX = static_cast<short>(sz.cx);
            gm.gmptGlyphOrigin.x = 0;
            gm.gmptGlyphOrigin.y = static_cast<short>(sz.cy);
        }

        SelectObject(hdc, oldFont);
        DeleteObject(font);
        DeleteDC(hdc);

        uint32_t w = gm.gmBlackBoxX;
        uint32_t h = gm.gmBlackBoxY;

        // Shelf packing in 1024x1024 atlas with 2px padding
        if (m_cursorX + w + 2 >= 1024) {
            m_cursorX = 1;
            m_cursorY += m_rowHeight + 2;
            m_rowHeight = 0;
        }
        if (m_cursorY + h + 2 >= 1024) {
            m_cursorX = 1;
            m_cursorY = 1;
            m_rowHeight = 0;
        }

        uint32_t gx = m_cursorX;
        uint32_t gy = m_cursorY;
        m_cursorX += w + 2;
        if (h > m_rowHeight) m_rowHeight = h;

        if (w > 0 && h > 0 && !grayBuffer.empty()) {
            uint32_t pitch = (w + 3) & ~3;
            std::vector<uint32_t> subPixels(w * h, 0);

            for (uint32_t y = 0; y < h; ++y) {
                for (uint32_t x = 0; x < w; ++x) {
                    if (y * pitch + x < grayBuffer.size()) {
                        uint8_t val = grayBuffer[y * pitch + x];
                        uint8_t alpha = (val >= 64) ? 255 : static_cast<uint8_t>((val * 255) / 64);
                        uint32_t color = (static_cast<uint32_t>(alpha) << 24) | 0x00FFFFFF;
                        subPixels[y * w + x] = color;
                        m_pixels[(gy + y) * 1024 + (gx + x)] = color;
                    }
                }
            }

            if (m_context && m_texture) {
                D3D11_BOX box{};
                box.left = gx;
                box.top = gy;
                box.front = 0;
                box.right = gx + w;
                box.bottom = gy + h;
                box.back = 1;
                m_context->UpdateSubresource(m_texture.Get(), 0, &box, subPixels.data(), w * sizeof(uint32_t), 0);
            }
        }

        HostGlyphData data{};
        data.ch = static_cast<uint16_t>(ch);
        data.atlasX = static_cast<uint16_t>(gx);
        data.padX[0] = 0;
        data.padX[1] = 0;
        data.padX[2] = 0;
        data.atlasY = static_cast<uint16_t>(gy);
        data.width = static_cast<uint16_t>(w);
        data.height = static_cast<uint16_t>(h);
        data.originX = static_cast<int16_t>(gm.gmptGlyphOrigin.x);
        data.originY = static_cast<int16_t>(gm.gmptGlyphOrigin.y);
        data.advanceX = static_cast<int16_t>(gm.gmCellIncX);
        data.ascent = -static_cast<int16_t>(tm.tmAscent);
        data.page = 0;
        data.reserved = 0;

        m_cache[key] = data;
        *outGlyph = data;
        return true;
    }

    bool m_initialized = false;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

    std::vector<uint32_t> m_pixels;
    uint32_t m_cursorX = 1;
    uint32_t m_cursorY = 1;
    uint32_t m_rowHeight = 0;

    std::unordered_map<uint64_t, HostGlyphData> m_cache;
    std::mutex m_mutex;
};

// Slot 62: Text Glyph handler (host_probe case 0x3E)
static uint64_t __fastcall HostSlot62_TextGlyph(void* thisPtr, uint64_t ch, void* outGlyph, uint32_t style, float scale, const char* face) {
    (void)thisPtr;
    if (outGlyph && reinterpret_cast<uintptr_t>(outGlyph) > 0x10000) {
        if (TextAtlas::Get().GetGlyph(static_cast<wchar_t>(ch & 0xFFFF), style, scale, face, reinterpret_cast<HostGlyphData*>(outGlyph))) {
            return 1;
        }
    }
    return 0;
}

// Slot 63: Text Atlas handler (host_probe case 0x3F)
static uint64_t __fastcall HostSlot63_TextAtlas(void* thisPtr, uint32_t pageIndex) {
    (void)thisPtr;
    return reinterpret_cast<uint64_t>(TextAtlas::Get().GetSRV(pageIndex));
}

// Slot 64: Text Has Icon (host_probe case 0x40)
static uint64_t __fastcall HostSlot64_TextHasIcon(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// Slot 65: Text Kerning (host_probe case 0x41)
static uint64_t __fastcall HostSlot65_TextKerning(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// Slot 66: Text Register Icon (host_probe case 0x42)
static uint64_t __fastcall HostSlot66_TextRegisterIcon(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
}

// -----------------------------------------------------------------------------
// Gamma post-process pass (fixes dark/purple HUD icons, crosshair, radar, and shield bar)
// Reach Renaissance executes a post-process gamma pass in Host Slot 1 before Present.
// Halo Reach outputs linear HDR backbuffer; without gamma correction, HUD icons and UI
// elements appear dim, dark navy, or purplish.
// -----------------------------------------------------------------------------
class GammaPass {
public:
    static GammaPass& Get() {
        static GammaPass instance;
        return instance;
    }

    void SetExponent(float exp) {
        m_exponent = exp;
    }

    void OnResize() {
        m_initialized = false;
        m_copyTexture.Reset();
        m_copySRV.Reset();
        m_backBufferRTV.Reset();
    }

    void Execute(IDXGISwapChain* swapChain) {
        if (!swapChain) return;

        Microsoft::WRL::ComPtr<ID3D11Device> device;
        if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(device.GetAddressOf())))) return;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())))) return;

        D3D11_TEXTURE2D_DESC desc{};
        backBuffer->GetDesc(&desc);

        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.GetAddressOf());
        if (!context) return;

        // Lazy initialize TextAtlas with device and context if not already done
        TextAtlas::Get().Initialize(device.Get(), context.Get());

        if (!m_initialized || m_width != desc.Width || m_height != desc.Height) {
            if (!InitResources(device.Get(), backBuffer.Get(), desc)) {
                return;
            }
        }

        // Copy current frame backbuffer to our shader resource texture
        context->CopyResource(m_copyTexture.Get(), backBuffer.Get());

        // Save pipeline state
        ID3D11RenderTargetView* oldRTV = nullptr;
        ID3D11DepthStencilView* oldDSV = nullptr;
        context->OMGetRenderTargets(1, &oldRTV, &oldDSV);

        UINT numViewports = 1;
        D3D11_VIEWPORT oldViewport{};
        context->RSGetViewports(&numViewports, &oldViewport);

        ID3D11VertexShader* oldVS = nullptr;
        ID3D11PixelShader* oldPS = nullptr;
        ID3D11ClassInstance* oldInstances[4] = { nullptr };
        UINT numInstances = 0;
        context->VSGetShader(&oldVS, oldInstances, &numInstances);
        context->PSGetShader(&oldPS, oldInstances, &numInstances);

        ID3D11ShaderResourceView* oldSRV = nullptr;
        context->PSGetShaderResources(0, 1, &oldSRV);

        ID3D11Buffer* oldCB = nullptr;
        context->PSGetConstantBuffers(0, 1, &oldCB);

        D3D11_PRIMITIVE_TOPOLOGY oldTopology;
        context->IAGetPrimitiveTopology(&oldTopology);

        ID3D11InputLayout* oldLayout = nullptr;
        context->IAGetInputLayout(&oldLayout);

        ID3D11BlendState* oldBlend = nullptr;
        float oldBlendFactor[4] = { 0 };
        UINT oldSampleMask = 0;
        context->OMGetBlendState(&oldBlend, oldBlendFactor, &oldSampleMask);

        ID3D11DepthStencilState* oldDepthState = nullptr;
        UINT oldStencilRef = 0;
        context->OMGetDepthStencilState(&oldDepthState, &oldStencilRef);

        // Update constant buffer with gamma exponent
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            float* cbData = static_cast<float*>(mapped.pData);
            cbData[0] = m_exponent;
            cbData[1] = 0.0f;
            cbData[2] = 0.0f;
            cbData[3] = 0.0f;
            context->Unmap(m_constantBuffer.Get(), 0);
        }

        // Bind gamma pass state
        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(desc.Width);
        vp.Height = static_cast<float>(desc.Height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        context->RSSetViewports(1, &vp);

        ID3D11RenderTargetView* rtv = m_backBufferRTV.Get();
        context->OMSetRenderTargets(1, &rtv, nullptr);
        context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
        context->OMSetDepthStencilState(nullptr, 0);

        context->IASetInputLayout(nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        context->VSSetShader(m_vs.Get(), nullptr, 0);
        context->PSSetShader(m_ps.Get(), nullptr, 0);

        ID3D11Buffer* cbs[] = { m_constantBuffer.Get() };
        context->PSSetConstantBuffers(0, 1, cbs);

        ID3D11ShaderResourceView* srvs[] = { m_copySRV.Get() };
        context->PSSetShaderResources(0, 1, srvs);

        // Draw full-screen triangle
        context->Draw(3, 0);

        // Restore pipeline state
        ID3D11ShaderResourceView* nullSRV[] = { nullptr };
        context->PSSetShaderResources(0, 1, nullSRV);

        context->OMSetRenderTargets(1, &oldRTV, oldDSV);
        if (oldRTV) oldRTV->Release();
        if (oldDSV) oldDSV->Release();

        context->RSSetViewports(numViewports, &oldViewport);

        context->VSSetShader(oldVS, nullptr, 0);
        if (oldVS) oldVS->Release();
        context->PSSetShader(oldPS, nullptr, 0);
        if (oldPS) oldPS->Release();

        context->PSSetShaderResources(0, 1, &oldSRV);
        if (oldSRV) oldSRV->Release();

        context->PSSetConstantBuffers(0, 1, &oldCB);
        if (oldCB) oldCB->Release();

        context->IASetInputLayout(oldLayout);
        if (oldLayout) oldLayout->Release();

        context->IASetPrimitiveTopology(oldTopology);

        context->OMSetBlendState(oldBlend, oldBlendFactor, oldSampleMask);
        if (oldBlend) oldBlend->Release();

        context->OMSetDepthStencilState(oldDepthState, oldStencilRef);
        if (oldDepthState) oldDepthState->Release();
    }

private:
    bool InitResources(ID3D11Device* device, ID3D11Texture2D* backBuffer, const D3D11_TEXTURE2D_DESC& desc) {
        m_width = desc.Width;
        m_height = desc.Height;

        if (!m_vs) {
            static const char* s_gammaShaderCode = R"(
                Texture2D frame_copy : register(t0);
                cbuffer Gamma : register(b0) { float4 gamma; };

                float4 vs_main(uint id : SV_VertexID) : SV_Position {
                    float2 p = float2((id << 1) & 2, id & 2);
                    return float4(p * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
                }

                float4 ps_main(float4 pos : SV_Position) : SV_Target {
                    float4 c = frame_copy.Load(int3((int2)pos.xy, 0));
                    return float4(pow(saturate(c.rgb), gamma.x), c.a);
                }
            )";

            Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
            Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
            HRESULT hr = D3DCompile(s_gammaShaderCode, strlen(s_gammaShaderCode), "GammaVS", nullptr, nullptr,
                                    "vs_main", "vs_4_0", 0, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf());
            if (FAILED(hr)) {
                if (errorBlob) {
                    NOBLE_ERROR("GammaPass: VS compile failed: {}", static_cast<const char*>(errorBlob->GetBufferPointer()));
                }
                return false;
            }

            Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
            hr = D3DCompile(s_gammaShaderCode, strlen(s_gammaShaderCode), "GammaPS", nullptr, nullptr,
                            "ps_main", "ps_4_0", 0, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf());
            if (FAILED(hr)) {
                if (errorBlob) {
                    NOBLE_ERROR("GammaPass: PS compile failed: {}", static_cast<const char*>(errorBlob->GetBufferPointer()));
                }
                return false;
            }

            if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vs.GetAddressOf()))) {
                return false;
            }
            if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_ps.GetAddressOf()))) {
                return false;
            }

            D3D11_BUFFER_DESC cbDesc{};
            cbDesc.ByteWidth = 16;
            cbDesc.Usage = D3D11_USAGE_DYNAMIC;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (FAILED(device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.GetAddressOf()))) {
                return false;
            }
        }

        D3D11_TEXTURE2D_DESC copyDesc = desc;
        copyDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        copyDesc.Usage = D3D11_USAGE_DEFAULT;
        copyDesc.CPUAccessFlags = 0;
        copyDesc.MiscFlags = 0;
        m_copyTexture.Reset();
        if (FAILED(device->CreateTexture2D(&copyDesc, nullptr, m_copyTexture.GetAddressOf()))) {
            return false;
        }

        m_copySRV.Reset();
        if (FAILED(device->CreateShaderResourceView(m_copyTexture.Get(), nullptr, m_copySRV.GetAddressOf()))) {
            return false;
        }

        m_backBufferRTV.Reset();
        if (FAILED(device->CreateRenderTargetView(backBuffer, nullptr, m_backBufferRTV.GetAddressOf()))) {
            return false;
        }

        m_initialized = true;
        NOBLE_INFO("GammaPass: Initialized post-process gamma pass ({}x{}, exponent: {:.4f})", m_width, m_height, m_exponent);
        return true;
    }

    bool m_initialized = false;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    float m_exponent = 0.4545f; // Standard linear -> sRGB gamma correction (1.0 / 2.2)

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_copyTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_copySRV;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferRTV;
};

static void ExecuteGammaPass(IDXGISwapChain* swapChain) {
    GammaPass::Get().Execute(swapChain);
}

static void ResetGammaPass() {
    GammaPass::Get().OnResize();
}

static uint64_t __fastcall HostSlot67_TextMetrics(void* thisPtr, uint32_t style, float size, const char* face,
                                                  uint16_t* out0, uint16_t* out1) {
    (void)thisPtr;
    const std::wstring requested = WidenFace(face);
    const std::wstring mapped = MapReachFace(requested);
    const HostFontMetrics m = MeasureFont(mapped, style, size);

    static std::atomic<int> s_calls{0};
    if (s_calls.fetch_add(1) < 20) {
        NOBLE_INFO("ReachHost [Slot 67 - TextMetrics]: size={} scale={:.2f} face='{}' -> '{}' ascent={} descent={} ok={}",
            style, size, NarrowForLog(requested), NarrowForLog(mapped), m.ascent, m.descent, m.ok);
    }
    if (!m.ok) return 0;
    if (out0) SafeWriteU16(out0, m.ascent);
    if (out1) SafeWriteU16(out1, m.descent);
    return 1;
}

// Slot 88: User identity & Gamertag handler (host_probe case 0x58)
static uint64_t __fastcall HostSlot88_UserIdentity(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    // Matches Reach Renaissance host::hit case 0x58:
    //   a1 = u64* out XUID, a2 = wchar_t* out gamertag, a3 = gamertag capacity (chars), a4 = local user index.
    // Writes straight into a1/a2 (never dereferences their current contents) and returns 1.
    // Users other than 0 are reported as absent (return 0), as RR does without a second local user.
    (void)thisPtr;
    static std::atomic<bool> s_logged{false};
    const uint32_t userIndex = static_cast<uint32_t>(a4);
    if (!s_logged.exchange(true)) {
        NOBLE_INFO("ReachHost [Slot 88 - UserIdentity]: user {} -> 'NobleSix' (XUID: 0x9000000000001)", userIndex);
    }
    if (userIndex != 0) {
        return 0;
    }

    const uint64_t xuid = 0x0009000000000001ULL;
    if (a1 > 0x10000) {
        SafeWriteUint64(a1, xuid);
    }

    const wchar_t gamerTagW[] = L"NobleSix";
    const size_t tagChars = (sizeof(gamerTagW) / sizeof(wchar_t)) - 1;
    if (a2 > 0x10000 && a3 > tagChars) {
        SafeCopyWideString(reinterpret_cast<wchar_t*>(a2), static_cast<size_t>(a3), gamerTagW);
    }
    return 1;
}

// Slot 97: Capability pass-through (host_probe case 0x61)
static uint64_t __fastcall HostSlot97_CapabilityPass(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 1;
}

// Slot 116: Controller Mapping (host_probe case 0x74). Called with the player's XUID in a1; the
// other registers are leftovers (a2 was seen pointing at this function). RR's pad::controller_map
// only logs its pad layout. The old handler wrote 64 bytes of 0xFF through a2, so it now writes
// nothing and returns 0.
static uint64_t __fastcall HostSlot116_ControllerMap(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    return 0;
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
    s_vtable[61]  = reinterpret_cast<void*>(&HostSlot61_TextPrepare);
    s_vtable[62]  = reinterpret_cast<void*>(&HostSlot62_TextGlyph);
    s_vtable[63]  = reinterpret_cast<void*>(&HostSlot63_TextAtlas);
    s_vtable[64]  = reinterpret_cast<void*>(&HostSlot64_TextHasIcon);
    s_vtable[65]  = reinterpret_cast<void*>(&HostSlot65_TextKerning);
    s_vtable[66]  = reinterpret_cast<void*>(&HostSlot66_TextRegisterIcon);
    s_vtable[67]  = reinterpret_cast<void*>(&HostSlot67_TextMetrics);
    s_vtable[88]  = reinterpret_cast<void*>(&HostSlot88_UserIdentity);
    s_vtable[97]  = reinterpret_cast<void*>(&HostSlot97_CapabilityPass);
    s_vtable[116] = reinterpret_cast<void*>(&HostSlot116_ControllerMap);

    InitVTable2Helper(s_vtable2, std::make_index_sequence<160>{});
    s_vtable2[147] = reinterpret_cast<void*>(&Iface2Slot147_SessionGuid);

    s_vtableInitialized = true;
    NOBLE_INFO("ReachHost: 160-slot host vtable and secondary vtable initialized matching Reach Renaissance host.");
}

ReachHost::ReachHost() {
    InitializeVTable();
    // Link host objects matching host::new_host (0x1400828e0)
    // m_secondObject[0] = s_vtable2
    m_secondObject[0] = s_vtable2;
    // m_hostObject[0] = s_vtable, m_hostObject[1] = m_secondObject
    m_hostObject[0] = s_vtable;
    m_hostObject[1] = m_secondObject;
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
    if (!dataDir.empty()) {
        std::error_code ec;
        if (std::filesystem::exists(dataDir / "menumaps" / "mainmenu_all.map", ec) ||
            std::filesystem::exists(dataDir / "launch_block_1.bin", ec) ||
            m_dataDir.empty()) {
            m_dataDir = dataDir;
        }
    }
}

void ReachHost::SetDeviceAndContext(ID3D11Device* device, ID3D11DeviceContext* context) {
    m_device = device;
    m_context = context;
    TextAtlas::Get().Initialize(device, context);
}

} // namespace noble
