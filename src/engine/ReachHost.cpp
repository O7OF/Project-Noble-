#include "ReachHost.hpp"
#include "ui/LoadScreen.hpp"
#include "common/Logger.hpp"
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

// Slot 1: Frame tick / Render loop update callback (host_probe case 1)
static uint64_t __fastcall HostSlot1_FrameTick(void* thisPtr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    (void)thisPtr; (void)a1; (void)a2; (void)a3; (void)a4;
    // haloreach.dll invokes Slot 1 every frame to tick armory, loadscreen, looks, and frame timer
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
    (void)thisPtr; (void)a3; (void)a4;
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
                std::ifstream pFile(std::filesystem::path(path), std::ios::binary);
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
            NOBLE_INFO("ReachHost [Slot 34]: using built-in default key bindings");
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

static void InputRelease(HWND hwnd) {
    if (!s_inputCaptured) return;
    ClipCursor(nullptr);
    if (hwnd && s_savedClassCursor) {
        SetClassLongPtrW(hwnd, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(s_savedClassCursor));
    }
    s_inputCaptured = false;
    NOBLE_INFO("ReachHost [Slot 36 - Input]: window lost focus, mouse released");
}

static void FillLocalInput(uint8_t* st) {
    *reinterpret_cast<uint32_t*>(st + 0x000) = 1;
    *reinterpret_cast<float*>(st + 0x118) = 1.0f;

    static HWND s_lastWindow = nullptr;
    HWND hwnd = InputFocusedWindow();
    if (!hwnd) {
        InputRelease(s_lastWindow);
        memset(s_keyDownSince, 0, sizeof(s_keyDownSince));
        return;
    }
    s_lastWindow = hwnd;

    RECT rc{};
    GetClientRect(hwnd, &rc);
    POINT tl{rc.left, rc.top}, br{rc.right, rc.bottom};
    ClientToScreen(hwnd, &tl);
    ClientToScreen(hwnd, &br);
    RECT clip{tl.x, tl.y, br.x, br.y};
    POINT center{(tl.x + br.x) / 2, (tl.y + br.y) / 2};

    bool firstCapturedFrame = false;
    if (!s_inputCaptured) {
        s_savedClassCursor = reinterpret_cast<HCURSOR>(GetClassLongPtrW(hwnd, GCLP_HCURSOR));
        SetClassLongPtrW(hwnd, GCLP_HCURSOR, 0); // hide the arrow over our window
        s_inputCaptured = true;
        firstCapturedFrame = true;
        NOBLE_INFO("ReachHost [Slot 36 - Input]: window focused, mouse captured (scale {})", InputMouseScale());
    }
    ClipCursor(&clip);

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

    // Mouse look: delta from the window centre, then recentre.
    POINT pos{};
    float dx = 0.0f, dy = 0.0f;
    if (GetCursorPos(&pos) && !firstCapturedFrame) {
        dx = static_cast<float>(pos.x - center.x);
        dy = static_cast<float>(pos.y - center.y);
    }
    SetCursorPos(center.x, center.y);
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
    return 1; // RR's text::prepare always returns 1 (glyph pre-rendering not implemented yet)
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

} // namespace noble
