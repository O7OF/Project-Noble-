#include "common/Logger.hpp"
#include "engine/MapGameTypes.hpp"
#include "common/MapNames.hpp"
#include "engine/PadLayouts.hpp"
#include "engine/InputTuning.hpp"
#include <io.h>
#include <fcntl.h>
#include "common/Types.hpp"
#include "common/Version.hpp"
#include "host/SteamLocator.hpp"
#include "host/SteamWorkshop.hpp"
#include "host/OwnershipVerifier.hpp"
#include "host/Window.hpp"
#include "host/D3D11Renderer.hpp"
#include "host/Loader.hpp"
#include "host/EngineLimits.hpp"
#include "hooks/HookManager.hpp"
#include "hooks/SigScanner.hpp"
#include "vfs/VFS.hpp"
#include "engine/BlamEngine.hpp"
#include "engine/CameraFov.hpp"
#include "engine/CameraThirdPerson.hpp"
#include "engine/ReachArmory.hpp"
#include "engine/ElitePack.hpp"
#include "engine/SharedTagCache.hpp"
#include "engine/DevCheats.hpp"
#include "engine/MvarDump.hpp"
#include "engine/MemoryDiff.hpp"
#include "mvar/MapVariant.hpp"
#include "mvar/MapVariantWriter.hpp"
#include "mvar/MapVariantLibrary.hpp"
#include "engine/SessionVariant.hpp"
#include "engine/VariantFields.hpp"
#include "ui/UIManager.hpp"
#include "ui/ForgeMenu.hpp"
#include "ui/FrontEndMenu.hpp"
#include "ui/LoadScreen.hpp"
#include "network/SessionManager.hpp"
#include "network/MasterServerClient.hpp"
#include "network/AuthClient.hpp"
#include "network/SocialClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/UPnP.hpp"
#include "common/Settings.hpp"
#include "common/Updater.hpp"
#include "common/Relaunch.hpp"
#include "ui/PauseMenu.hpp"
#include "ui/ModCatalog.hpp"
#include "ui/UiArt.hpp"
#include "audio/ReachAudio.hpp"
#include "audio/ProcessAudio.hpp"
#include "audio/VoiceChat.hpp"
#include "ui/SocialOverlay.hpp"
#include "hooks/LobbyPatches.hpp"
#include "hooks/ForgePatches.hpp"
#include "hooks/EnginePatches.hpp"
#include <nlohmann/json.hpp>

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>
#include <csignal>
#include <cstdlib>

namespace fs = std::filesystem;

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

// Log and dump file names: the dev build keeps its own pair, so a dev session never truncates a player's log.
#ifdef NOBLE_DEV_BUILD
static constexpr const wchar_t* kCrashDumpFileName = L"noble-dev-crash.dmp";
static constexpr const char* kLogFileName = "noble-dev.log";
#else
static constexpr const wchar_t* kCrashDumpFileName = L"noble-crash.dmp";
static constexpr const char* kLogFileName = "noble-client.log";
#endif

static void WriteCrashDump(EXCEPTION_POINTERS* info);

static LONG WINAPI NobleCrashHandler(EXCEPTION_POINTERS* pExceptionInfo) {
    if (pExceptionInfo && pExceptionInfo->ExceptionRecord) {
        uintptr_t faultAddr = reinterpret_cast<uintptr_t>(pExceptionInfo->ExceptionRecord->ExceptionAddress);
        HMODULE hMod = nullptr;
        WCHAR modPath[MAX_PATH] = L"Unknown Module";
        uintptr_t rva = 0;
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCWSTR>(faultAddr), &hMod)) {
            GetModuleFileNameW(hMod, modPath, MAX_PATH);
            rva = faultAddr - reinterpret_cast<uintptr_t>(hMod);
        }

        spdlog::error("FATAL EXCEPTION CAUGHT: Code 0x{:08X} at Address 0x{:X} (+0x{:X}) (Module: {})",
            pExceptionInfo->ExceptionRecord->ExceptionCode,
            faultAddr,
            rva,
            std::filesystem::path(modPath).string());

        // Capture call stack
        void* stackFrames[32] = { nullptr };
        USHORT frameCount = CaptureStackBackTrace(0, 32, stackFrames, nullptr);
        for (USHORT i = 0; i < frameCount; ++i) {
            uintptr_t frameAddr = reinterpret_cast<uintptr_t>(stackFrames[i]);
            HMODULE frameMod = nullptr;
            WCHAR frameModPath[MAX_PATH] = L"Unknown";
            uintptr_t frameRva = 0;
            if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                   reinterpret_cast<LPCWSTR>(frameAddr), &frameMod)) {
                GetModuleFileNameW(frameMod, frameModPath, MAX_PATH);
                frameRva = frameAddr - reinterpret_cast<uintptr_t>(frameMod);
            }
            spdlog::error(" Frame {:02d}: 0x{:X} (+0x{:X}) [{}]", i, frameAddr, frameRva, std::filesystem::path(frameModPath).filename().string());
        }

        WriteCrashDump(pExceptionInfo);
    }
    spdlog::default_logger()->flush();
    return EXCEPTION_CONTINUE_SEARCH;
}

// Minidump next to the program: "noble-crash.dmp" for the release build, "noble-dev-crash.dmp" for the dev
// build (its own log, its own dump, so a dev crash never overwrites a player's report).
static void WriteCrashDump(EXCEPTION_POINTERS* info) {
    // Next to the executable, never the working directory: a player (or a test) can start the game from
    // anywhere, and a dump that lands somewhere else is a dump nobody finds.
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::filesystem::path dumpPath = std::filesystem::path(exePath).parent_path() / kCrashDumpFileName;
    const wchar_t* dumpName = dumpPath.c_str();
    HANDLE hDumpFile = CreateFileW(dumpName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hDumpFile == INVALID_HANDLE_VALUE) {
        spdlog::error("Crash minidump could not be created ({})", std::filesystem::path(dumpName).string());
        return;
    }
    MINIDUMP_EXCEPTION_INFORMATION mei{};
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = info;
    mei.ClientPointers = FALSE;
    const MINIDUMP_TYPE type = static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithIndirectlyReferencedMemory |
                                                          MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);
    const BOOL ok = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, type,
                                      info ? &mei : nullptr, nullptr, nullptr);
    CloseHandle(hDumpFile);
    spdlog::error("Crash minidump written to {} ({})", std::filesystem::path(dumpName).string(), ok ? "ok" : "MiniDumpWriteDump failed");
}

// A C++ exception that escapes (or an explicit std::terminate) never reaches NobleCrashHandler: the CRT
// answers it with a fail-fast in ucrtbase (0xC0000409, FAST_FAIL 7), which leaves a bare "noble-client.exe
// stopped working" in Windows and nothing in our log. This logs the exception's what() and a stack with
// module RVAs (resolvable through noble-client.map) before the CRT kills the process.
static void NobleTerminateHandler() {
    spdlog::error("UNHANDLED C++ EXCEPTION: the process is being terminated");
    if (std::exception_ptr current = std::current_exception()) {
        try {
            std::rethrow_exception(current);
        } catch (const std::exception& e) {
            spdlog::error("  what(): {}", e.what());
        } catch (...) {
            spdlog::error("  what(): (not a std::exception)");
        }
    } else {
        spdlog::error("  std::terminate() was called without a live exception");
    }
    void* frames[24] = {};
    const USHORT count = CaptureStackBackTrace(1, static_cast<DWORD>(std::size(frames)), frames, nullptr);
    for (USHORT i = 0; i < count; ++i) {
        const uintptr_t addr = reinterpret_cast<uintptr_t>(frames[i]);
        HMODULE mod = nullptr;
        WCHAR path[MAX_PATH] = L"Unknown";
        uintptr_t rva = 0;
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCWSTR>(addr), &mod)) {
            GetModuleFileNameW(mod, path, MAX_PATH);
            rva = addr - reinterpret_cast<uintptr_t>(mod);
        }
        spdlog::error("  at {:02d}: 0x{:X} (+0x{:X}) [{}]", i, addr, rva, std::filesystem::path(path).filename().string());
    }
    // The CRT turns an escaping C++ exception into a fail-fast in ucrtbase, so the SEH filter above never runs:
    // write the minidump here, before TerminateProcess, or this class of crash would only leave a log.
    WriteCrashDump(nullptr);
    spdlog::default_logger()->flush();
    TerminateProcess(GetCurrentProcess(), 3);
}

// std::set_terminate is kept per thread by the MSVC runtime, so the handler above only covers the thread that
// installed it: an exception that escapes a worker thread (the Armory scan, the match threads) or a direct
// abort() went straight to the CRT fail-fast and left nothing in the log (Event Viewer: ucrtbase.dll,
// 0xC0000409, FAST_FAIL 7). abort() raises SIGABRT first, and the SIGABRT handler is process-wide, so route it
// through the same logging; a terminate-driven abort still has its exception in flight for what().
static void NobleAbortSignalHandler(int) {
    spdlog::error("abort() / SIGABRT raised on thread {}", GetCurrentThreadId());
    NobleTerminateHandler();
}

// The C runtime's own error paths (an invalid argument to a CRT function, a pure virtual call) also end in a
// silent fail-fast unless a handler is installed. Log them like any other fatal error: what, where, a stack.
static std::string NarrowForLog(const wchar_t* w) {
    std::string out;
    if (!w) return "?";
    for (; *w; ++w) out.push_back(static_cast<char>(*w & 0x7F));
    return out;
}

static void NobleInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file,
                                         unsigned int line, uintptr_t) {
    spdlog::error("FATAL C RUNTIME ERROR: invalid parameter in {} ({}:{}) expression '{}'", NarrowForLog(function),
                  NarrowForLog(file), line, NarrowForLog(expression));
    NobleTerminateHandler();
}

static void NoblePureCallHandler() {
    spdlog::error("FATAL C RUNTIME ERROR: pure virtual function called");
    NobleTerminateHandler();
}

// "Did the last run end badly?" marker. Every process writes running\<pid>.flag beside the exe once it has
// started and deletes it on a clean exit. A flag whose process is gone means that run was killed or crashed
// without finishing, whatever the way it died (a fail-fast, a driver reset, Task Manager), so the next start
// says so in the log and points at where Windows keeps its own record.
static std::filesystem::path RunFlagDir() {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    return std::filesystem::path(exePath).parent_path() / L"running";
}

static std::filesystem::path OwnRunFlag() {
    return RunFlagDir() / (std::to_wstring(GetCurrentProcessId()) + L".flag");
}

static void ReportStaleRunFlagsAndCreateOwn() {
    std::error_code ec;
    const std::filesystem::path dir = RunFlagDir();
    std::filesystem::create_directories(dir, ec);
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec) || entry.path().extension() != L".flag") continue;
        DWORD pid = 0;
        try {
            pid = static_cast<DWORD>(std::stoul(entry.path().stem().wstring()));
        } catch (...) {
            continue;
        }
        if (pid == 0 || pid == GetCurrentProcessId()) continue;
        bool alive = false;
        if (HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
            DWORD code = 0;
            alive = GetExitCodeProcess(process, &code) && code == STILL_ACTIVE;
            CloseHandle(process);
        }
        if (alive) continue;   // another Noble process that is still running (the menu while a match runs)
        NOBLE_WARN("An earlier Noble process (pid {}) did not exit cleanly. Look for noble-*crash.dmp beside the exe, "
                   "and in Windows Event Viewer > Windows Logs > Application (Application Error / Windows Error "
                   "Reporting).", pid);
        std::filesystem::remove(entry.path(), ec);
    }
    std::ofstream(OwnRunFlag()) << "running\n";
}

static void RemoveOwnRunFlag() {
    std::error_code ec;
    std::filesystem::remove(OwnRunFlag(), ec);
}

// Per-monitor DPI awareness (v2). Without it Windows virtualises every size Noble reads: a 4K display at 200 %
// scaling reports 1920x1080 from GetSystemMetrics/GetMonitorInfo, so borderless and full screen rendered at 1080p
// and Windows stretched that up to the display. Must run before any window exists. The linker runs with
// /MANIFEST:NO, so this is set in code rather than in a manifest. SetProcessDpiAwarenessContext is Windows 10
// 1703+, looked up at run time; older systems fall back to system DPI awareness.
static void EnableDpiAwareness() {
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        using SetContextFn = BOOL(WINAPI*)(HANDLE);
        if (auto setContext = reinterpret_cast<SetContextFn>(GetProcAddress(user32, "SetProcessDpiAwarenessContext"))) {
            // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 == (DPI_AWARENESS_CONTEXT)-4
            if (setContext(reinterpret_cast<HANDLE>(static_cast<intptr_t>(-4)))) return;
        }
    }
    SetProcessDPIAware();
}

namespace {

// The engine gives up silently during a world load (no dump, no Windows error event, no clean-shutdown log). Trace
// whoever calls ExitProcess: the return address says whether the engine decided to quit or the CRT did.
using ExitProcessFn = VOID(WINAPI*)(UINT);
ExitProcessFn s_origExitProcess = nullptr;

// The engine's own main loop (haloreach+0xC390, called from its launcher thread). When it returns, the frame pump
// stops and the client sits idle with no error - so log the verdict and how long it ran.
using EngineMainLoopFn = char(__fastcall*)(uint64_t, uint64_t, uint64_t, uint64_t);
EngineMainLoopFn s_origEngineMainLoop = nullptr;
ULONGLONG s_engineLoopStart = 0;

char __fastcall EngineMainLoopHook(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    s_engineLoopStart = GetTickCount64();
    NOBLE_WARN("engine main loop entered (haloreach+0xC390)");
    const char result = s_origEngineMainLoop ? s_origEngineMainLoop(a1, a2, a3, a4) : 0;
    NOBLE_ERROR("engine main loop returned {} after {} ms", static_cast<int>(result),
                GetTickCount64() - s_engineLoopStart);
    if (auto logger = spdlog::default_logger()) logger->flush();
    return result;
}

// The engine's loader/render thread disappears during a failed world load. Trace thread exits that come from
// engine code so we know which thread died and from which RVA.
// Timing of the three stages the engine's launch routine runs after its main loop returns
// (FUN_18000db08: 0xD564 when the loop succeeded, then 0xC8F8 and 0x3873C during teardown). Comparing a working
// map with the Firefight-canvas mod map shows which stage bails out.
// All four x64 register parameters (rcx, rdx, r8, r9) must be preserved across GetTickCount64(): 0x3873C takes
// the memory stage index in edx and is also called during map loading (haloreach+0x305B6).
using StageFn = uint64_t(__fastcall*)(uint64_t, uint64_t, uint64_t, uint64_t);
StageFn s_origStage1 = nullptr;
StageFn s_origStage2 = nullptr;
StageFn s_origStage3 = nullptr;

uint64_t __fastcall Stage1Hook(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    const ULONGLONG t0 = GetTickCount64();
    const uint64_t res = s_origStage1 ? s_origStage1(a1, a2, a3, a4) : 0;
    NOBLE_WARN("engine stage 0xD564 returned after {} ms", GetTickCount64() - t0);
    if (auto logger = spdlog::default_logger()) logger->flush();
    return res;
}
uint64_t __fastcall Stage2Hook(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    const ULONGLONG t0 = GetTickCount64();
    const uint64_t res = s_origStage2 ? s_origStage2(a1, a2, a3, a4) : 0;
    NOBLE_WARN("engine stage 0xC8F8 returned after {} ms", GetTickCount64() - t0);
    if (auto logger = spdlog::default_logger()) logger->flush();
    return res;
}
uint64_t __fastcall Stage3Hook(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    const ULONGLONG t0 = GetTickCount64();
    const uint64_t res = s_origStage3 ? s_origStage3(a1, a2, a3, a4) : 0;
    NOBLE_WARN("engine stage 0x3873C(stage={}) returned after {} ms", static_cast<int32_t>(a2), GetTickCount64() - t0);
    if (auto logger = spdlog::default_logger()) logger->flush();
    return res;
}

// The engine prints its own fatal/warning text through OutputDebugString. Capture it: a failed map load reports the
// reason there and nowhere else, so this is the only way to see why a scenario was rejected.
using OdsAFn = void(WINAPI*)(LPCSTR);
using OdsWFn = void(WINAPI*)(LPCWSTR);
OdsAFn s_origOdsA = nullptr;
OdsWFn s_origOdsW = nullptr;

void WINAPI HookOutputDebugStringA(LPCSTR text) {
    if (text && *text) NOBLE_INFO("[engine] {}", text);
    if (s_origOdsA) s_origOdsA(text);
}
void WINAPI HookOutputDebugStringW(LPCWSTR text) {
    if (text && *text) {
        // Widen-to-narrow properly: the plain std::string(range) constructor truncates each wchar to a char.
        char buffer[1024] = {};
        const int written = WideCharToMultiByte(CP_UTF8, 0, text, -1, buffer, sizeof(buffer) - 1, nullptr, nullptr);
        if (written > 0) NOBLE_INFO("[engine] {}", buffer);
    }
    if (s_origOdsW) s_origOdsW(text);
}

// SEH-guarded memory read, for probing engine structures from the debug hooks.
static bool SafeReadPtr(uintptr_t address, void* out, size_t size) {
    __try {
        std::memcpy(out, reinterpret_cast<const void*>(address), size);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

// A map built by MCC's workshop tools declares the game type of the canvas it was built on, and the engine's map
// loader (haloreach+0x141C0) rejects the load when that type differs from the type the launch asks for. The AI
// Battleground maps declare 0x7D2 (the Firefight canvas) while a Forge/Firefight launch asks for 0x2760, so the
// loader returned 0 and the launch unwound right after the map was read. The loader parses the file (haloreach+
// 0x31204) and only then compares id/type, so this hook aligns the loaded header's type with the request in
// between - that is what lets a mod map built on another canvas mount in the mode the player picked.
static std::atomic<int> s_requestedMapType{-1};
std::atomic<int> s_requestedMapId{-1};
using MapLoadFullFn = unsigned char(__fastcall*)(void*, int, int, void*);
MapLoadFullFn s_origMapLoadFull = nullptr;

unsigned char __fastcall MapLoadFullHook(void* a1, int a2, int a3, void* a4) {
    s_requestedMapType.store(a3);
    const unsigned char result = s_origMapLoadFull ? s_origMapLoadFull(a1, a2, a3, a4) : 0;
    s_requestedMapType.store(-1);
    return result;
}

using MapParseFn = char(__fastcall*)(void*, void*);
MapParseFn s_origMapParse = nullptr;

char __fastcall MapParseHook(void* a1, void* a2) {
    const char result = s_origMapParse ? s_origMapParse(a1, a2) : 0;
    if (result) {
        // Inside the loader, right after the file is parsed: read the header id/type the loader is about to compare
        // against the requested ones. Layout from FUN_1800141c0:
        //   index = *(u32*)(DAT_180c1a600 + 4 + (DAT_180afbe38 & 0xffff) * 8)
        //   header id   = *(int*)((&DAT_184e39f20)[index >> 28] + 0x0C + index * 4)
        //   header type = *(int*)((&DAT_184e39f20)[index >> 28] + 0x10 + index * 4)
        HMODULE reach = GetModuleHandleW(L"haloreach.dll");
        const uintptr_t base = reinterpret_cast<uintptr_t>(reach);
        int slot = 0;
        uintptr_t header = 0;
        uint32_t index = 0;
        uintptr_t table = 0;
        int fileId = 0;
        int fileType = 0;
        bool ok = base && SafeReadPtr(base + 0xAFBE38, &slot, sizeof(slot)) &&
                  SafeReadPtr(base + 0xC1A600, &header, sizeof(header)) && header &&
                  SafeReadPtr(header + 4 + static_cast<uint64_t>(slot & 0xffff) * 8, &index, sizeof(index)) &&
                  SafeReadPtr(base + 0x4E39F20 + static_cast<uint64_t>(index >> 28) * 8, &table, sizeof(table)) && table &&
                  SafeReadPtr(table + 0xC + static_cast<uint64_t>(index) * 4, &fileId, sizeof(fileId)) &&
                  SafeReadPtr(table + 0x10 + static_cast<uint64_t>(index) * 4, &fileType, sizeof(fileType));
        const int want = s_requestedMapType.load();
        static const bool s_mapTypePatchOff = std::getenv("NOBLE_MAPTYPE_OFF") != nullptr;
        static const bool s_mapIdPatch = std::getenv("NOBLE_MAPID_PATCH") != nullptr;
        const int wantId = s_requestedMapId.load();
        if (ok) NOBLE_INFO("main: map declares id {} type 0x{:X}; launch requested id {} type 0x{:X}", fileId, fileType,
                           wantId, want);
        static const bool s_idForce = std::getenv("NOBLE_ID_FORCE") != nullptr;
        if (ok && s_idForce && wantId == -1 && fileId == 0 && want == 0x1392) {
            // Experiment: a foreign map parsed into a campaign slot reports id 0; a stock campaign map reports 1.
            int* idField = reinterpret_cast<int*>(table + 0xC + static_cast<uint64_t>(index) * 4);
            DWORD oldIdProtect = 0;
            if (VirtualProtect(idField, sizeof(int), PAGE_READWRITE, &oldIdProtect)) {
                *idField = 1;
                VirtualProtect(idField, sizeof(int), oldIdProtect, &oldIdProtect);
                NOBLE_INFO("main: map id 0 mounted as 1 for the campaign slot");
            }
        }
        if (ok && wantId != -1 && fileId != wantId && s_mapIdPatch) {
            int* idField = reinterpret_cast<int*>(table + 0xC + static_cast<uint64_t>(index) * 4);
            DWORD oldIdProtect = 0;
            if (VirtualProtect(idField, sizeof(int), PAGE_READWRITE, &oldIdProtect)) {
                *idField = wantId;
                VirtualProtect(idField, sizeof(int), oldIdProtect, &oldIdProtect);
                NOBLE_INFO("main: map id {} mounted as the launch requested {}", fileId, wantId);
            }
        }
        if (ok && want != -1 && fileType != want && !s_mapTypePatchOff) {
            int* typeField = reinterpret_cast<int*>(table + 0x10 + static_cast<uint64_t>(index) * 4);
            DWORD oldProtect = 0;
            if (VirtualProtect(typeField, sizeof(int), PAGE_READWRITE, &oldProtect)) {
                const int previous = *typeField;
                *typeField = want;
                VirtualProtect(typeField, sizeof(int), oldProtect, &oldProtect);
                NOBLE_INFO("main: map declares game type 0x{:X}; mounted it as the launch requested 0x{:X}", previous,
                           want);
            }
        }
    }
    return result;
}

// The map state tick (haloreach+0x40A008) is where "loading_map" becomes "running". It reads a game-type integer
// from *(*(lVar3+0x47f8)+0x60ef8); logging that value on a working map and on a Firefight-canvas map says whether
// the engine treats them as different types.
using MapStateTickFn = void(__fastcall*)(void*);
MapStateTickFn s_origMapStateTick = nullptr;
std::atomic<int> s_mapStateCalls{0};

void __fastcall MapStateTickHook(void* state) {
    const int call = s_mapStateCalls.fetch_add(1);
    if ((call % 60) == 0) {
        uintptr_t lVar3 = 0;
        int gameType = -1;
        uintptr_t slot68 = 0;
        int mapState = -1;
        if (SafeReadPtr(reinterpret_cast<uintptr_t>(state) + 0x30, &lVar3, sizeof(lVar3)) && lVar3) {
            uintptr_t inner = 0;
            if (SafeReadPtr(lVar3 + 0xC0, &inner, sizeof(inner)) && inner) {
                uintptr_t table = 0;
                if (SafeReadPtr(inner + 0x47F8, &table, sizeof(table)) && table) {
                    SafeReadPtr(table + 0x60EF8, &gameType, sizeof(gameType));
                }
            }
        }
        SafeReadPtr(reinterpret_cast<uintptr_t>(state) + 0x68, &slot68, sizeof(slot68));
        if (HMODULE reach = GetModuleHandleW(L"haloreach.dll")) {
            SafeReadPtr(reinterpret_cast<uintptr_t>(reach) + 0xC1A118, &mapState, sizeof(mapState));
        }
        char line[256];
        std::snprintf(line, sizeof(line), "map state tick #%d: state %d slot68 0x%llX gameType %d", call, mapState,
                      static_cast<unsigned long long>(slot68), gameType);
        NOBLE_INFO("{}", line);
    }
    if (s_origMapStateTick) s_origMapStateTick(state);
}

// The map state machine promotes "loading_map" to "running" only when FUN_18040b4c8 returns something other than
// -1 (haloreach+0x40A008: lVar9 = FUN_18040b4c8(...); if (lVar9 != -1) DAT_180c1a118 = 2). Log its verdict.
using SessionStartFn = uint64_t(__fastcall*)(void*);
SessionStartFn s_origSessionStart = nullptr;

uint64_t __fastcall SessionStartHook(void* a1) {
    const uint64_t result = s_origSessionStart ? s_origSessionStart(a1) : 0;
    char line[160];
    std::snprintf(line, sizeof(line), "session start check (haloreach+0x40B4C8) returned 0x%llX%s",
                  static_cast<unsigned long long>(result), result == ~0ull ? " (FAILED)" : "");
    NOBLE_WARN("{}", line);
    if (auto logger = spdlog::default_logger()) logger->flush();
    return result;
}

using ExitThreadFn = VOID(WINAPI*)(DWORD);
ExitThreadFn s_origExitThread = nullptr;

VOID WINAPI ExitThreadHook(DWORD code) {
    const uintptr_t caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
    const uintptr_t engineBase = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"haloreach.dll"));
    if (engineBase && caller > engineBase && caller < engineBase + 0x4EDA000) {
        char line[256];
        std::snprintf(line, sizeof(line), "thread %lu exits with code %lu from haloreach+0x%llX",
                      static_cast<unsigned long>(GetCurrentThreadId()), static_cast<unsigned long>(code),
                      static_cast<unsigned long long>(caller - engineBase));
        NOBLE_ERROR("{}", line);
        if (auto logger = spdlog::default_logger()) logger->flush();
    }
    if (s_origExitThread) s_origExitThread(code);
}

using FreeLibraryAndExitThreadFn = VOID(WINAPI*)(HMODULE, DWORD);
FreeLibraryAndExitThreadFn s_origFreeLibExitThread = nullptr;

VOID WINAPI FreeLibraryAndExitThreadHook(HMODULE module, DWORD code) {
    const uintptr_t caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
    const uintptr_t engineBase = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"haloreach.dll"));
    char line[256];
    std::snprintf(line, sizeof(line), "FreeLibraryAndExitThread(0x%llX) thread %lu from haloreach+0x%llX",
                  static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(module)),
                  static_cast<unsigned long>(GetCurrentThreadId()),
                  static_cast<unsigned long long>(engineBase && caller > engineBase ? caller - engineBase : 0));
    NOBLE_ERROR("{}", line);
    if (auto logger = spdlog::default_logger()) logger->flush();
    if (s_origFreeLibExitThread) s_origFreeLibExitThread(module, code);
}

using PostQuitMessageFn = VOID(WINAPI*)(int);
PostQuitMessageFn s_origPostQuitMessage = nullptr;

VOID WINAPI PostQuitMessageHook(int code) {
    const uintptr_t caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
    const uintptr_t engineBase = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"haloreach.dll"));
    const uintptr_t callerRva = (engineBase && caller > engineBase) ? caller - engineBase : 0;
    NOBLE_ERROR("PostQuitMessage({}) requested from haloreach+0x{:X} (caller 0x{:X})", code, callerRva, caller);
    if (auto logger = spdlog::default_logger()) logger->flush();
    if (s_origPostQuitMessage) s_origPostQuitMessage(code);
}

VOID WINAPI ExitProcessHook(UINT code) {
    const uintptr_t caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
    const uintptr_t engineBase = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"haloreach.dll"));
    const uintptr_t callerRva = (engineBase && caller > engineBase) ? caller - engineBase : 0;
    NOBLE_ERROR("ExitProcess({}) requested from haloreach+0x{:X} (caller 0x{:X})", code, callerRva, caller);
    if (auto logger = spdlog::default_logger()) logger->flush();
    if (s_origExitProcess) s_origExitProcess(code);
}

} // namespace

namespace {

// A small in-process unwind engine. haloreach.dll is mapped by Noble's own loader, so dbghelp has no unwind data
// for it and StackWalk64 stops at the first frame. Windows' unwinder only needs the image's .pdata and base, so we
// parse those ourselves and step the context with RtlVirtualUnwind - enough to print a real engine call chain.
struct UnwindImage {
    uint8_t* base = nullptr;
    uint32_t size = 0;
    PRUNTIME_FUNCTION pdata = nullptr;
    size_t pdataCount = 0;
};

UnwindImage FindUnwindImage(HMODULE module) {
    UnwindImage image{};
    if (!module) return image;
    auto* base = reinterpret_cast<uint8_t*>(module);
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return image;
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return image;
    const auto& entry = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
    image.base = base;
    image.size = nt->OptionalHeader.SizeOfImage;
    image.pdata = reinterpret_cast<PRUNTIME_FUNCTION>(base + entry.VirtualAddress);
    image.pdataCount = entry.Size / sizeof(RUNTIME_FUNCTION);
    return image;
}

// One image per module, parsed on first use (cached): the unwinder has to step out of ntdll and into our own code too.
UnwindImage ImageForModule(HMODULE module) {
    static std::mutex s_mutex;
    static std::unordered_map<HMODULE, UnwindImage> s_cache;
    std::lock_guard<std::mutex> lock(s_mutex);
    const auto it = s_cache.find(module);
    if (it != s_cache.end()) return it->second;
    UnwindImage image = FindUnwindImage(module);
    s_cache.emplace(module, image);
    return image;
}

UnwindImage ImageForPc(uintptr_t pc, HMODULE* outModule) {
    HMODULE module = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(pc), &module)) {
        if (outModule) *outModule = nullptr;
        return UnwindImage{};
    }
    if (outModule) *outModule = module;
    return ImageForModule(module);
}

PRUNTIME_FUNCTION LookupRuntimeFunction(const UnwindImage& image, uintptr_t pc) {
    if (!image.base || !image.pdata || pc < reinterpret_cast<uintptr_t>(image.base)) return nullptr;
    const uint32_t rva = static_cast<uint32_t>(pc - reinterpret_cast<uintptr_t>(image.base));
    size_t lo = 0, hi = image.pdataCount;
    while (lo < hi) {
        const size_t mid = (lo + hi) / 2;
        const PRUNTIME_FUNCTION rf = &image.pdata[mid];
        if (rva < rf->BeginAddress) { hi = mid; continue; }
        if (rva >= rf->EndAddress) { lo = mid + 1; continue; }
        return rf;
    }
    return nullptr;
}

// Print up to 32 frames of a suspended thread, unwinding through the manually mapped engine.
int WalkThread(HANDLE thread, CONTEXT ctx, const UnwindImage& engine) {
    UNREFERENCED_PARAMETER(thread);
    int shown = 0;
    for (int i = 0; i < 28; ++i) {
        const uintptr_t pc = static_cast<uintptr_t>(ctx.Rip);
        HMODULE mod = nullptr;
        const UnwindImage image = ImageForPc(pc, &mod);
        uintptr_t moduleBase = reinterpret_cast<uintptr_t>(mod);
        std::string name = "?";
        if (mod) {
            WCHAR path[MAX_PATH] = L"?";
            GetModuleFileNameW(mod, path, MAX_PATH);
            name = std::filesystem::path(path).filename().string();
        }
        const bool inEngine = engine.base && pc >= reinterpret_cast<uintptr_t>(engine.base) &&
                             pc < reinterpret_cast<uintptr_t>(engine.base) + engine.size;
        char line[512];
        std::snprintf(line, sizeof(line), "    %02d %s +0x%llX", i, inEngine ? "haloreach.dll" : name.c_str(),
                      static_cast<unsigned long long>(inEngine ? pc - reinterpret_cast<uintptr_t>(engine.base)
                                                               : (moduleBase ? pc - moduleBase : pc)));
        NOBLE_ERROR("{}", line);
        ++shown;
        if (!moduleBase) break;

        PRUNTIME_FUNCTION rf = LookupRuntimeFunction(image, pc);
        if (rf) {
            DWORD64 establisher = 0;
            PVOID handlerData = nullptr;
            KNONVOLATILE_CONTEXT_POINTERS pointers{};
            RtlVirtualUnwind(UNW_FLAG_NHANDLER, static_cast<DWORD64>(moduleBase), static_cast<DWORD64>(pc), rf, &ctx,
                             &handlerData, &establisher, &pointers);
        } else {
            // Leaf function: no unwind entry, so the caller is the return address on the stack.
            if (!ctx.Rsp) break;
            ctx.Rip = *reinterpret_cast<const DWORD64*>(ctx.Rsp);
            ctx.Rsp += 8;
            if (!ctx.Rip) break;
        }
    }
    return shown;
}

// Walk every thread's stack. Used by the stall watchdog: the engine can stop pumping frames without crashing, and
// this says exactly which call each thread is parked in (module + RVA, no symbols needed).
void DumpAllThreadStacks() {
    NOBLE_ERROR("STALL DUMP: no engine frame for 15 s; dumping thread stacks");
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) return;
    const UnwindImage engine = ImageForModule(GetModuleHandleW(L"haloreach.dll"));
    NOBLE_ERROR("  engine .pdata entries: {}", engine.pdataCount);
    THREADENTRY32 te{};
    te.dwSize = sizeof(te);
    if (Thread32First(snap, &te)) {
        do {
            if (te.th32OwnerProcessID != GetCurrentProcessId() || te.th32ThreadID == GetCurrentThreadId()) continue;
            HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
            if (!th) continue;
            if (SuspendThread(th) == static_cast<DWORD>(-1)) { CloseHandle(th); continue; }
            CONTEXT ctx{};
            ctx.ContextFlags = CONTEXT_FULL;
            if (GetThreadContext(th, &ctx)) {
                char head[64];
                std::snprintf(head, sizeof(head), "  thread %lu:", static_cast<unsigned long>(te.th32ThreadID));
                NOBLE_ERROR("{}", head);
                WalkThread(th, ctx, engine);
            }
            ResumeThread(th);
            CloseHandle(th);
        } while (Thread32Next(snap, &te));
    }
    CloseHandle(snap);
    if (auto logger = spdlog::default_logger()) logger->flush();
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
    EnableDpiAwareness();

    // Test-only: prove the relaunch handoff leaves no old process behind.
    {
        const std::wstring cmd = pCmdLine ? pCmdLine : L"";
        if (cmd.find(L"-relaunch-test") != std::wstring::npos &&
            cmd.find(L"-relaunch-test-child") == std::wstring::npos) {
            noble::Relaunch::HandOff(L"-relaunch-test-child -quit-after 15");
            return 0;
        }
    }

    // Headless test of the map / game-type guard: no MCC, no window, no audio. `-test-map-gametypes`.
    if (std::wstring(pCmdLine ? pCmdLine : L"").find(L"-test-map-gametypes") != std::wstring::npos) {
        const int types = noble::MapGameTypes::SelfTest();
        const int names = noble::MapNames::SelfTest();
        const int layouts = noble::PadLayouts::SelfTest();
        const int tuning = noble::InputTuning::SelfTest();
        const bool variantFields = noble::VariantFields::SelfTest();
        const bool hostVariant = noble::NetworkTransport::SelfTest();
        std::printf("%s: variant fields - read/write round trip\n", variantFields ? "PASS" : "FAIL");
        std::printf("%s: host variant materialize round trip\n", hostVariant ? "PASS" : "FAIL");
        return (types == 0 && names == 0 && layouts == 0 && tuning == 0 && variantFields && hostVariant) ? 0 : 1;
    }

    // Steam Workshop subscription helper: the join prompt starts us as "-steam-subscribe <id> -steam-dll <path>",
    // we load MCC's steam_api64.dll, ask Steam for the item and exit. This runs before any window, engine or
    // logger init so it is fast and cannot be blocked by the menu.
    {
        int subArgc = 0;
        LPWSTR* subArgv = CommandLineToArgvW(GetCommandLineW(), &subArgc);
        std::string workshopId;
        std::wstring steamApiDll;
        for (int i = 1; subArgv && i < subArgc; ++i) {
            if (_wcsicmp(subArgv[i], L"-steam-subscribe") == 0 && i + 1 < subArgc) {
                for (const wchar_t* c = subArgv[++i]; c && *c; ++c) workshopId.push_back(static_cast<char>(*c));
            } else if (_wcsicmp(subArgv[i], L"-steam-dll") == 0 && i + 1 < subArgc) {
                steamApiDll = subArgv[++i];
            }
        }
        if (subArgv) LocalFree(subArgv);
        if (!workshopId.empty()) {
            return noble::SteamWorkshop::SubscribeChildMain(workshopId, fs::path(steamApiDll));
        }
    }

    SetUnhandledExceptionFilter(&NobleCrashHandler);
    std::set_terminate(&NobleTerminateHandler);
    std::signal(SIGABRT, &NobleAbortSignalHandler);
    _set_invalid_parameter_handler(&NobleInvalidParameterHandler);
    _set_purecall_handler(&NoblePureCallHandler);
    DisableProcessWindowsGhosting();

    // 1. Initialize logging
    // "-log <file>" lets a second instance (or an automated test) keep its own log instead of truncating
    // noble-client.log, which basic_file_sink does on every start.
    {
        std::string logName = kLogFileName;
        int argcPre = 0;
        LPWSTR* argvPre = CommandLineToArgvW(GetCommandLineW(), &argcPre);
        for (int i = 1; argvPre && i + 1 < argcPre; ++i) {
            if (_wcsicmp(argvPre[i], L"-log") == 0) {
                const std::wstring w(argvPre[i + 1]);
                std::string ascii;
                for (const wchar_t c : w) ascii.push_back(static_cast<char>(c & 0x7F));   // log names are ASCII
                logName = ascii;
            }
        }
        if (argvPre) LocalFree(argvPre);
        // Next to the executable: a launch from any working directory still writes its log beside the exe.
        if (logName.find(':') == std::string::npos && logName.find('\\') == std::string::npos && logName.find('/') == std::string::npos) {
            wchar_t exePath[MAX_PATH] = {};
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
            const std::filesystem::path besideExe = std::filesystem::path(exePath).parent_path() / logName;
            logName = besideExe.string();
        }
        noble::Logger::Init(logName);
    }
    NOBLE_INFO("Starting {} - {}", noble::APP_NAME, noble::VERSION_STRING);

    // "-crash-test": throw an unhandled C++ exception so the terminate handler's minidump can be verified
    // (the CRT answers an escaping exception with a fail-fast, which the SEH filter never sees).
    if (std::wstring(pCmdLine ? pCmdLine : L"").find(L"-crash-test") != std::wstring::npos) {
        NOBLE_INFO("crash-test: writing a minidump for an unhandled C++ exception");
#pragma warning(suppress : 4297)
        throw std::runtime_error("crash-test: unhandled C++ exception");
    }

    // Headless voice tests need no MCC, window or engine.
    {
        int voiceTestExit = 0;
        if (noble::RunVoiceTestIfRequested(voiceTestExit)) {
            noble::Logger::Shutdown();
            return voiceTestExit;
        }
    }

    // 2. Discover Steam MCC installation and verify Reach DLC assets
    auto reachInstall = noble::SteamLocator::LocateAndVerifyReach();
    if (!reachInstall) {
        NOBLE_ERROR("Halo: Reach installation could not be located. Aborting launch.");
        MessageBoxW(nullptr, 
            L"Could not locate Halo: Reach DLC in any Steam library.\n\n"
            L"Please verify that Halo: The Master Chief Collection with the Halo: Reach DLC is installed on Steam.",
            L"Project Noble - Launch Error", 
            MB_ICONERROR | MB_OK);
        return 1;
    }
    ReportStaleRunFlagsAndCreateOwn();

    // 3. Strict Steam Ownership Verification
    // Unowned copies are strictly prohibited from launching
    if (!noble::OwnershipVerifier::VerifySteamOwnership(*reachInstall)) {
        NOBLE_ERROR("Steam ownership verification failed. Aborting launch.");
        return 1;
    }

    // 4. Initialize MinHook hooking framework early (required by VFS and engine hooks)
    if (!noble::HookManager::Initialize()) {
        NOBLE_ERROR("Failed to initialize MinHook.");
        return 1;
    }
    // Watch the engine's file writes: a map variant save that ends up zero bytes still hands its buffer to WriteFile,
    // and that buffer is what a working save needs (see MvarDump.cpp).
    (void)noble::InstallWriteWatch();
    // Trace who ends the process: a silent engine exit during a world load leaves no dump and no log line.
    noble::HookManager::InstallHook(reinterpret_cast<void*>(&ExitProcess),
                                    reinterpret_cast<void*>(&ExitProcessHook),
                                    reinterpret_cast<void**>(&s_origExitProcess), "trace_exitprocess");
    noble::HookManager::InstallHook(reinterpret_cast<void*>(&PostQuitMessage),
                                    reinterpret_cast<void*>(&PostQuitMessageHook),
                                    reinterpret_cast<void**>(&s_origPostQuitMessage), "trace_postquit");
    noble::HookManager::InstallHook(reinterpret_cast<void*>(&ExitThread),
                                    reinterpret_cast<void*>(&ExitThreadHook),
                                    reinterpret_cast<void**>(&s_origExitThread), "trace_exitthread");
    noble::HookManager::InstallHook(reinterpret_cast<void*>(&FreeLibraryAndExitThread),
                                    reinterpret_cast<void*>(&FreeLibraryAndExitThreadHook),
                                    reinterpret_cast<void**>(&s_origFreeLibExitThread), "trace_freelibexit");

    // 5. Locate authentic data directory (resolves blocks, menumaps, armor, and userdata)
    auto FindDataDirectory = []() -> fs::path {
        wchar_t exePathBuf[MAX_PATH] = { 0 };
        if (GetModuleFileNameW(nullptr, exePathBuf, MAX_PATH)) {
            fs::path exeDir = fs::path(exePathBuf).parent_path();
            std::vector<fs::path> candidates = {
                exeDir / "data",
                exeDir / ".." / "data",
                exeDir / ".." / ".." / "data",
                exeDir / ".." / ".." / ".." / "data",
                fs::current_path() / "data"
            };
            for (const auto& cand : candidates) {
                std::error_code ec;
                // Release packages carry no loose menu map: it and its string_ids are embedded in
                // Noble.exe and VFS extracts them on first run. The small JSON/UI files identify the
                // data folder in those installs.
                if (fs::exists(cand / "menumaps" / "mainmenu_all.map", ec) ||
                    fs::exists(cand / "launch_block_1.bin", ec) ||
                    fs::exists(cand / "forge_palette.json", ec) ||
                    fs::exists(cand / "strings_en.json", ec)) {
                    return fs::absolute(cand);
                }
            }
        }
        return fs::absolute("data");
    };

    // Test hook: a second local instance can point at its own userdata so both sides have distinct profiles.
    std::string dataDirOverride;
    {
        int preArgc = 0;
        if (LPWSTR* preArgv = CommandLineToArgvW(GetCommandLineW(), &preArgc)) {
            for (int i = 1; i + 1 < preArgc; ++i) {
                if (std::wstring(preArgv[i]) == L"-data-dir") {
                    for (const wchar_t* c = preArgv[i + 1]; c && *c; ++c) {
                        dataDirOverride.push_back(static_cast<char>(*c));
                    }
                    break;
                }
            }
            LocalFree(preArgv);
        }
    }
    fs::path userStoragePath = FindDataDirectory();
    if (!dataDirOverride.empty()) {
        userStoragePath = fs::path(dataDirOverride);
        NOBLE_INFO("main: user data directory overridden to '{}'", userStoragePath.string());
    }
    NOBLE_INFO("Resolved authentic Project Noble data directory: {}", userStoragePath.string());

    // Initialize Virtual File System (VFS)
    // Directs all file I/O: Retail MCC files remain read-only; custom mods and configs route to data/
    if (!noble::VFS::Initialize(*reachInstall, userStoragePath)) {
        NOBLE_ERROR("Failed to initialize Virtual File System (VFS).");
        return 1;
    }

    // Configure Host object paths early
    noble::ReachHost::Get().SetPaths(reachInstall->reachDirectory, reachInstall->mapsDirectory, userStoragePath);
    noble::ReachArmory::Initialize(userStoragePath);

    // Player settings (data\userdata\settings.json): window size, controls, volumes, name, servers.
    noble::Settings::Get().Load();
    // Headless: move settings to or from a file.
    {
        int transferArgc = 0;
        LPWSTR* transferArgv = CommandLineToArgvW(GetCommandLineW(), &transferArgc);
        for (int i = 1; transferArgv && i + 1 < transferArgc; ++i) {
            const bool isExport = _wcsicmp(transferArgv[i], L"-settings-export") == 0;
            if (!isExport && _wcsicmp(transferArgv[i], L"-settings-import") != 0) continue;
            const bool ok = isExport ? noble::Settings::Get().ExportTo(transferArgv[i + 1]) : noble::Settings::Get().ImportFrom(transferArgv[i + 1]);
            NOBLE_INFO("Settings: {} {}", isExport ? "export" : "import", ok ? "done" : "failed");
            noble::Logger::Shutdown();
            return ok ? 0 : 1;
        }
        if (transferArgv) LocalFree(transferArgv);
    }
    noble::AuthClient::Get().Initialize();
    noble::SocialClient::Get().Initialize();
    const noble::SettingsData startupSettings = noble::Settings::Get().Snapshot();

    // Front-end menu or a direct map launch? Needed before the swap chain is created: the menu process
    // renders at 1.5x its window (menu-only supersampling) so the Armory can zoom the engine frame
    // without pixelating it. The match process is launched separately and stays at native resolution.
    const bool showMenu = noble::FrontEndMenu::ShouldShow();

    // Auto-updater: the menu checks the public release repository once per launch. Match processes do
    // not check (they are short-lived and are relaunched from the menu). "-no-update" is for tests.
    {
        wchar_t updaterExe[MAX_PATH] = {};
        if (GetModuleFileNameW(nullptr, updaterExe, MAX_PATH) > 0) {
            noble::Updater::Get().Initialize(std::filesystem::path(updaterExe).parent_path());
        }
        if (showMenu && wcsstr(GetCommandLineW(), L"-no-update") == nullptr) {
            noble::Updater::Get().StartCheck();
        }
    }

    // 6. Create Win32 host window
    HWND containerParentHwnd = nullptr;
    {
        int numArgsEarly = 0;
        LPWSTR* argvEarly = CommandLineToArgvW(GetCommandLineW(), &numArgsEarly);
        if (argvEarly) {
            for (int i = 1; i + 1 < numArgsEarly; ++i) {
                if (_wcsicmp(argvEarly[i], L"-container-hwnd") == 0) {
                    uintptr_t val = std::wcstoull(argvEarly[i + 1], nullptr, 0);
                    containerParentHwnd = reinterpret_cast<HWND>(val);
                }
            }
            LocalFree(argvEarly);
        }
    }

    auto window = std::make_unique<noble::Window>(L"Project Noble",
        static_cast<uint32_t>(startupSettings.width), static_cast<uint32_t>(startupSettings.height));
    if (containerParentHwnd && IsWindow(containerParentHwnd)) {
        window->SetContainerParent(containerParentHwnd);
        NOBLE_INFO("main: running in container window mode [Parent HWND: 0x{:X}]", reinterpret_cast<uintptr_t>(containerParentHwnd));
    }
    window->SetBorderless(startupSettings.borderless);
    window->SetExclusiveFullscreen(startupSettings.exclusiveFullscreen);
    if (!window->Create()) {
        NOBLE_ERROR("Failed to create Win32 host window.");
        return 1;
    }

    // 7. Initialize Direct3D 11 runtime environment.
    // The engine always renders at the display's native resolution (or the window's, if that is
    // larger), never at a small window size. Windows scales that image down into a window or shows it
    // 1:1 in borderless full screen, so text, HUD and gameplay stay crisp in both modes and the window
    // mode can be switched live without ever resizing the engine's swap chain. The menu process also
    // supersamples 1.5x for the enlarged Armory view. Cap at 3840x2160.
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    const HMONITOR monitor = MonitorFromWindow(window->GetHwnd(), MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfoW(monitor, &monitorInfo);
    const uint32_t monitorWidth = static_cast<uint32_t>(std::max<LONG>(0, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left));
    const uint32_t monitorHeight = static_cast<uint32_t>(std::max<LONG>(0, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top));
    constexpr uint32_t kMaxRenderWidth = 3840;
    constexpr uint32_t kMaxRenderHeight = 2160;
    uint32_t renderWidth = std::clamp(std::max(window->GetWidth(), monitorWidth), 320u, kMaxRenderWidth);
    uint32_t renderHeight = std::clamp(std::max(window->GetHeight(), monitorHeight), 240u, kMaxRenderHeight);
    // SETTINGS > GRAPHICS > RESOLUTION (borderless and full screen only; 0 x 0 = the display's native size).
    // The window still covers the display and DXGI scales the picture up to it; a lower resolution trades
    // sharpness for frame rate. Never above the display's own size, and the engine cannot change it once
    // started, so it takes effect for the next match and the next menu launch.
    if (startupSettings.borderless && startupSettings.fullscreenWidth > 0 && startupSettings.fullscreenHeight > 0) {
        const uint32_t chosenW = static_cast<uint32_t>(startupSettings.fullscreenWidth);
        const uint32_t chosenH = static_cast<uint32_t>(startupSettings.fullscreenHeight);
        if (monitorWidth == 0 || monitorHeight == 0 || (chosenW <= monitorWidth && chosenH <= monitorHeight)) {
            renderWidth = std::clamp(chosenW, 320u, kMaxRenderWidth);
            renderHeight = std::clamp(chosenH, 240u, kMaxRenderHeight);
        } else {
            NOBLE_WARN("main: resolution {}x{} is larger than the display ({}x{}); using the display's size",
                       chosenW, chosenH, monitorWidth, monitorHeight);
        }
    }
    if (showMenu) {
        const float scale = std::min(1.5f, std::min(static_cast<float>(kMaxRenderWidth) / static_cast<float>(renderWidth),
                                                     static_cast<float>(kMaxRenderHeight) / static_cast<float>(renderHeight)));
        if (scale > 1.0f) {
            renderWidth = static_cast<uint32_t>(std::lround(static_cast<float>(renderWidth) * scale));
            renderHeight = static_cast<uint32_t>(std::lround(static_cast<float>(renderHeight) * scale));
        }
        NOBLE_INFO("main: menu render resolution {}x{} (window {}x{}, display {}x{})", renderWidth, renderHeight,
                   window->GetWidth(), window->GetHeight(), monitorWidth, monitorHeight);
    } else {
        NOBLE_INFO("main: match render resolution {}x{} (window {}x{}, display {}x{})", renderWidth, renderHeight,
                   window->GetWidth(), window->GetHeight(), monitorWidth, monitorHeight);
    }
    auto renderer = std::make_unique<noble::D3D11Renderer>();
    if (!renderer->Initialize(window->GetHwnd(), renderWidth, renderHeight)) {
        NOBLE_ERROR("Failed to initialize Direct3D 11 renderer.");
        return 1;
    }

    // Initialize LoadScreen wireframe particle renderer
    noble::LoadScreen::Get().Initialize(renderer->GetDevice(), renderer->GetContext());

    // Initialize ImGui UI Manager
    noble::UIManager::Initialize(window->GetHwnd(), renderer->GetDevice(), renderer->GetContext());
    noble::ForgeMenu::Initialize(userStoragePath.wstring());
    window->SetCustomWndProc([](HWND h, UINT m, WPARAM w, LPARAM l) {
        return noble::UIManager::HandleWndProc(h, m, w, l);
    });
    {
        // Menu and loading-screen pictures: the player's own art, Noble's art, or MCC's UI packs.
        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        noble::UiArt::Init(renderer->GetDevice(), reachInstall->reachDirectory.parent_path(),
                           std::filesystem::path(exePath).parent_path());
        // Reach's menu sounds and menu music, decoded from MCC's sound bank into data\userdata\cache\audio.
        noble::ReachAudio::Init(reachInstall->reachDirectory,
                                std::filesystem::path(exePath).parent_path() / "data" / "userdata" / "cache" / "audio");
        // Menu music belongs to the front end only. In a match process, keep the player off so an apply
        // from the pause-menu settings cannot start it over the game's own audio.
        noble::ReachAudio::SetMenuMusicEnabled(showMenu);
        if (!showMenu) noble::ReachAudio::StopMusic();
        {
            const auto& s = noble::Settings::Get().Snapshot();
            noble::ProcessAudio::ApplyAll(s.volumeGame, s.volumeMusic, s.volumeOther, s.masterMute);
        }
    }

    window->Show(nCmdShow);
    noble::ReachHost::Get().SetDisplaySize(renderWidth, renderHeight);

    // LAN discovery and hosting must already run while the front-end menu is open, so a game
    // hosted from the SERVERS tab (or by someone else on the LAN) shows up in the list.
    noble::SessionManager::Get().Initialize();
    const std::string masterUrl = startupSettings.masterServerUrl;  // empty = LAN/direct only
    noble::MasterServerClient::Get().SetUrl(masterUrl);
    noble::MasterServerClient::Get().RequestOnlineCount();

    // Parse command line arguments for direct map and variant launches (e.g. from NobleLauncher or Relaunch)
    std::string launchMap = "";
    std::string launchMode = "";
    std::string customMvar = "";
    std::string customVariant = "";
    fs::path modFolder;
    fs::path mapOverride;   // modded .map served in place of the stock launchMap file
    std::string mapTitle;   // mod map's own title and pictures (loading screen / poster)
    fs::path mapImage;
    fs::path mapThumb;
    int launchDifficulty = 1;
    uint64_t launchSkulls = 0;
    bool launchHost = false;
    bool hostOnline = true;
    bool hostPublic = true;
    int hostMaxPlayers = 16;
    int expectedPeers = 0;
    int engineModeOverride = 0;
    int liveWindowTestAt = -1;   // test-only: switch to borderless after N seconds
    bool guestPostStart = false;
    bool thirdPerson = false;
    bool voiceProbe = false;
    std::string variantDumpPath;
    std::string variantPatch;
    int thirdPersonMode = 0;
    int thirdPersonDistance = 100;
    std::string engineCommand;
    int engineCommandAt = -1;
    int forgeSaveAt = -1;   // dev: run the Forge "save map variant" path N seconds into the match
    std::wstring mvarParsePath;       // dev: decode a map variant file and report it, then exit
    std::vector<int> mvarDumpTimes;   // dev: memory dumps of the loaded map variant at these seconds
    int mvarDumpEvery = 0;            // dev: dump repeatedly, this many seconds apart (find the live map buffer)
int mvarLiveDumpEvery = 0;        // dev: dump the live variant struct/state every N seconds (Forge placement diff)
int mvarPageScanEvery = 0;        // dev: diff every writable page every N seconds (find the editor's placement list)
    std::string connectAddress = "";
    std::string connectSessionId = "";
    std::string connectSessionToken = "";
    std::string relayUrl = "";

    auto WideToNarrow = [](const std::wstring& w) -> std::string {
        if (w.empty()) return "";
        int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (len <= 0) return "";
        std::string s(len - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), len, nullptr, nullptr);
        return s;
    };

    int numArgs = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &numArgs);
    if (argvW) {
        for (int i = 1; i < numArgs; ++i) {
            std::wstring arg = argvW[i];
            if (arg == L"-map" && i + 1 < numArgs) {
                launchMap = WideToNarrow(argvW[++i]);
            } else if (arg == L"-mode" && i + 1 < numArgs) {
                launchMode = WideToNarrow(argvW[++i]);
            } else if (arg == L"-mvar" && i + 1 < numArgs) {
                customMvar = WideToNarrow(argvW[++i]);
                noble::SetSessionMapVariantPath(std::filesystem::path(argvW[i]));
            } else if (arg == L"-variant" && i + 1 < numArgs) {
                customVariant = WideToNarrow(argvW[++i]);
            } else if (arg == L"-mod" && i + 1 < numArgs) {
                modFolder = fs::path(argvW[++i]);
            } else if (arg == L"-map-override" && i + 1 < numArgs) {
                mapOverride = fs::path(argvW[++i]);
            } else if (arg == L"-map-title" && i + 1 < numArgs) {
                mapTitle = WideToNarrow(argvW[++i]);
            } else if (arg == L"-map-image" && i + 1 < numArgs) {
                mapImage = fs::path(argvW[++i]);
            } else if (arg == L"-map-thumb" && i + 1 < numArgs) {
                mapThumb = fs::path(argvW[++i]);
            } else if (arg == L"-skulls" && i + 1 < numArgs) {
                launchSkulls = _wcstoui64(argvW[++i], nullptr, 0);
            } else if (arg == L"-difficulty" && i + 1 < numArgs) {
                launchDifficulty = _wtoi(argvW[++i]);
            } else if (arg == L"-host") {
                launchHost = true;
            } else if (arg == L"-online") {
                hostOnline = true;
            } else if (arg == L"-lan") {
                hostOnline = false;
            } else if (arg == L"-public") {
                hostPublic = true;
            } else if (arg == L"-private") {
                hostPublic = false;
            } else if (arg == L"-max-players" && i + 1 < numArgs) {
                hostMaxPlayers = _wtoi(argvW[++i]);
            } else if (arg == L"-wait-peers" && i + 1 < numArgs) {
                expectedPeers = _wtoi(argvW[++i]);
            } else if (arg == L"-third-person") {
                thirdPerson = true;
            } else if (arg == L"-variant-dump" && i + 1 < numArgs) {
                variantDumpPath = WideToNarrow(argvW[++i]);
            } else if (arg == L"-variant-override" && i + 1 < numArgs) {
                variantPatch = WideToNarrow(argvW[++i]);
            } else if (arg == L"-voice-probe") {
                voiceProbe = true;
            } else if (arg == L"-live-window-test" && i + 1 < numArgs) {
                liveWindowTestAt = _wtoi(argvW[++i]);
            } else if (arg == L"-third-person-distance" && i + 1 < numArgs) {
                thirdPersonDistance = _wtoi(argvW[++i]);
                thirdPerson = true;
            } else if (arg == L"-third-person-mode" && i + 1 < numArgs) {
                thirdPersonMode = _wtoi(argvW[++i]);
                thirdPerson = true;
            } else if (arg == L"-mvar-write" && i + 2 < numArgs) {
                const std::wstring in = argvW[++i];
                const std::wstring outPath = argvW[++i];
                const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(in);
                std::vector<uint8_t> bytes;
                bool hashRecomputed = false;
                std::string error;
                if (!noble::mvar::WriteMapVariant(variant, bytes, hashRecomputed, error)) {
                    NOBLE_ERROR("Mvar write: {}", error);
                    return 1;
                }
                std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
                out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
                out.close();
                if (!out) {
                    NOBLE_ERROR("Mvar write: could not write '{}'", WideToNarrow(outPath));
                    return 1;
                }
                NOBLE_INFO("Mvar write: '{}' -> '{}' ({} bytes, hash {})", WideToNarrow(in), WideToNarrow(outPath),
                           bytes.size(), hashRecomputed ? "recomputed" : "preserved");
                return 0;
            } else if (arg == L"-mvar-save" && i + 2 < numArgs) {
                const std::wstring in = argvW[++i];
                const std::wstring outPath = argvW[++i];
                const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(in);
                std::string error;
                if (!noble::mvar::SaveMapVariantJson(variant, outPath, std::string(), error)) {
                    NOBLE_ERROR("Mvar save: {}", error);
                    return 1;
                }
                return 0;
            } else if (arg == L"-mvar-import" && i + 1 < numArgs) {
                const std::wstring in = argvW[++i];
                const std::filesystem::path library = noble::ReachHost::Get().GetDataDir() / "map_variants";
                std::string detail;
                const std::filesystem::path out =
                    noble::mvar::SaveVariantToLibrary(in, noble::mvar::VariantTitle(in), library, detail);
                if (out.empty()) {
                    NOBLE_ERROR("Mvar import: {}", detail);
                    return 1;
                }
                NOBLE_INFO("Mvar import: '{}' -> '{}'", WideToNarrow(in), out.string());
                return 0;
            } else if (arg == L"-mvar-roundtrip" && i + 1 < numArgs) {
                const std::wstring target = argvW[++i];
                std::vector<std::wstring> files;
                std::error_code ec;
                if (std::filesystem::is_directory(target, ec)) {
                    for (const auto& entry : std::filesystem::directory_iterator(target, ec)) {
                        if (!entry.is_regular_file(ec)) continue;
                        if (entry.path().extension() != L".mvar") continue;
                        files.push_back(entry.path().wstring());
                    }
                    std::sort(files.begin(), files.end());
                } else {
                    files.push_back(target);
                }
                int identical = 0, different = 0, failed = 0;
                for (const std::wstring& file : files) {
                    const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(file);
                    if (!variant.valid) {
                        NOBLE_WARN("Mvar roundtrip: '{}' did not parse: {}", WideToNarrow(file), variant.error);
                        ++failed;
                        continue;
                    }
                    std::vector<uint8_t> bytes;
                    bool hashRecomputed = false;
                    std::string error;
                    if (!noble::mvar::WriteMapVariant(variant, bytes, hashRecomputed, error)) {
                        NOBLE_WARN("Mvar roundtrip: '{}' did not write: {}", WideToNarrow(file), error);
                        ++failed;
                        continue;
                    }
                    if (bytes == variant.fileBytes) {
                        ++identical;
                    } else {
                        size_t at = 0;
                        while (at < bytes.size() && at < variant.fileBytes.size() && bytes[at] == variant.fileBytes[at]) ++at;
                        NOBLE_WARN("Mvar roundtrip: '{}' differs at 0x{:X} ({} vs {} bytes)", WideToNarrow(file), at,
                                   bytes.size(), variant.fileBytes.size());
                        ++different;
                    }
                }
                NOBLE_INFO("Mvar roundtrip: {} identical, {} different, {} failed", identical, different, failed);
                return (different || failed) ? 1 : 0;
            } else if (arg == L"-mvar-parse") {
                {
                    const std::wstring path = argvW[++i];
                    const noble::mvar::MapVariant variant = noble::mvar::ReadMapVariant(path);
                    NOBLE_INFO("Mvar parse: {}", noble::mvar::Describe(variant));
                    return variant.valid ? 0 : 1;
                }
            } else if (arg == L"-mvar-dump-every") {
                mvarDumpEvery = _wtoi(argvW[++i]);
            } else if (arg == L"-mvar-live-dump" && i + 1 < numArgs) {
                mvarLiveDumpEvery = _wtoi(argvW[++i]);
                noble::LiveDumpSeconds() = mvarLiveDumpEvery;
            } else if (arg == L"-mvar-page-scan" && i + 1 < numArgs) {
                mvarPageScanEvery = _wtoi(argvW[++i]);
                noble::PageScanSeconds() = mvarPageScanEvery;
            } else if (arg == L"-mvar-dump-at") {
                const std::wstring spec = argvW[++i];
                size_t from = 0;
                while (from <= spec.size()) {
                    const size_t comma = spec.find(L',', from);
                    const std::wstring piece = spec.substr(from, comma == std::wstring::npos ? std::wstring::npos : comma - from);
                    if (!piece.empty()) mvarDumpTimes.push_back(_wtoi(piece.c_str()));
                    if (comma == std::wstring::npos) break;
                    from = comma + 1;
                }
            } else if (arg == L"-forge-save-at") {
                forgeSaveAt = _wtoi(argvW[++i]);
            } else if (arg == L"-guest-post-start") {
                guestPostStart = true;
            } else if (arg == L"-engine-cmd" && i + 1 < numArgs) {
                engineCommand = WideToNarrow(argvW[++i]);
            } else if (arg == L"-engine-cmd-at" && i + 1 < numArgs) {
                engineCommandAt = _wtoi(argvW[++i]);
            } else if (arg == L"-connect" && i + 1 < numArgs) {
                connectAddress = WideToNarrow(argvW[++i]);
            } else if (arg == L"-session-id" && i + 1 < numArgs) {
                connectSessionId = WideToNarrow(argvW[++i]); 
            } else if (arg == L"-session-token" && i + 1 < numArgs) {
                connectSessionToken = WideToNarrow(argvW[++i]);
            } else if (arg == L"-relay" && i + 1 < numArgs) {
                relayUrl = WideToNarrow(argvW[++i]);
            }
        }
        LocalFree(argvW);
    }
    // Launch the Front-End Menu + 3D Armory (mainmenu_all.map inside haloreach.dll), or the chosen map.
    if (showMenu) {
        noble::ReachArmory::SetMenuMode(true);
        noble::FrontEndMenu::Initialize(reachInstall->reachDirectory);
        launchMap = "mainmenu";
        launchMode = "mainmenu";
    } else {
        noble::ReachArmory::SetMenuMode(false);
        if (launchMap.empty()) {
            launchMap = "20_sword_slayer";
            launchMode = "multiplayer";
        }
        noble::PauseMenu::SetLoadingMap(launchMap);
        if (!mapTitle.empty() || !mapImage.empty() || !mapThumb.empty()) {
            noble::PauseMenu::SetLoadingMapInfo(mapTitle, mapImage, mapThumb);
        }
    }


    // When data\menumaps\mainmenu_all.map is present, VFS serves it as haloreach\maps\mainmenu.map
    // so the Armory pedestal scene, Spartan/Elite models, and rr_armory HaloScript run natively.
    if (showMenu) {
        if (noble::ReachArmory::HasCustomMenuMap()) {
            NOBLE_INFO("main: Armory 3D scene served from data\\menumaps\\mainmenu_all.map via VFS");
        } else {
            const fs::path pack = noble::ElitePack::FindPackMap(userStoragePath);
            if (!pack.empty()) {
                NOBLE_INFO("main: Armory models come from '{}' (loaded as an extra cache with the menu map)", pack.string());
            } else {
                NOBLE_INFO("main: no mainmenu_all.map or armour pack in '{}'", (userStoragePath / "menumaps").string());
            }
        }
    }

    if (!modFolder.empty()) {
        NOBLE_INFO("Activating mod folder '{}'", modFolder.string());
        noble::VFS::SetActiveMod(modFolder);
    }

    // A mod map built on a Firefight canvas has to be presented to the engine as a multiplayer map — that is what
    // MCC's own mod loader does. <scenario>.mapinfo carries the map's game-engine type at +0x42 (0x00 multiplayer,
    // 0x04 Firefight, 0x08 campaign) and its mode flags at +0x43 (0x40 = multiplayer). With the stock file the
    // engine refuses the launch outright ("map and game incompatible", EngineEvent reason 4); with a patched copy
    // served from user storage the map loads. Only for Forge/multiplayer launches of a mod.
    if (!modFolder.empty() && !launchMap.empty() &&
        _stricmp(launchMode.c_str(), "firefight") != 0 && _stricmp(launchMode.c_str(), "campaign") != 0) {
        std::error_code infoEc;
        const fs::path infoSrc = reachInstall->reachDirectory / "maps" / "info" / (launchMap + ".mapinfo");
        if (fs::is_regular_file(infoSrc, infoEc)) {
            std::ifstream infoIn(infoSrc, std::ios::binary);
            std::vector<uint8_t> infoBytes((std::istreambuf_iterator<char>(infoIn)), std::istreambuf_iterator<char>());
            // Firefight canvases are patched too: the engine's map loader is aligned with the launch's requested game
            // type while it loads (see MapParseHook), so a mod map built on a Firefight canvas mounts in Forge.
            if (infoBytes.size() > 0x44 && (infoBytes[0x42] != 0x00 || (infoBytes[0x43] & 0x40) == 0)) {
                const uint8_t stockType = infoBytes[0x42];
                infoBytes[0x42] = 0x00;                                              // multiplayer engine type
                infoBytes[0x43] = static_cast<uint8_t>(infoBytes[0x43] | 0x40);      // multiplayer map flag
                const fs::path infoDst = userStoragePath / "maps" / "info" / (launchMap + ".mapinfo");
                fs::create_directories(infoDst.parent_path(), infoEc);
                std::ofstream infoOut(infoDst, std::ios::binary);
                if (infoOut) {
                    infoOut.write(reinterpret_cast<const char*>(infoBytes.data()),
                                  static_cast<std::streamsize>(infoBytes.size()));
                    NOBLE_INFO("main: map info for '{}' patched to a multiplayer map (was engine type 0x{:02X}): '{}'",
                               launchMap, stockType, infoDst.string());
                }
            }
        }
    } else if (!launchMap.empty()) {
        std::error_code infoEc;
        const fs::path staleInfoDst = userStoragePath / "maps" / "info" / (launchMap + ".mapinfo");
        if (fs::is_regular_file(staleInfoDst, infoEc)) {
            fs::remove(staleInfoDst, infoEc);
            NOBLE_INFO("main: removed stale patched mapinfo override '{}'", staleInfoDst.string());
        }
    }
    if (!mapOverride.empty()) {
        NOBLE_INFO("Map override: serving '{}' as maps\\{}.map", mapOverride.string(), launchMap);
        noble::VFS::SetMapOverride(launchMap, mapOverride);
    } else {
        noble::VFS::ClearMapOverride();
    }

    // Map-info sandbox. The engine builds its map list by enumerating "haloreach\\maps\\info\\*.*"
    // RELATIVE TO THE WORKING DIRECTORY (root prefix "haloreach\\" at RVA 0x985D28, FindFirstFileW).
    // With an empty list every launch fails with "no map selected" and the menu has no backdrop.
    {
        std::error_code ec;
        const fs::path sandboxRoot = userStoragePath / "sandbox";
        const fs::path sandboxInfo = sandboxRoot / "haloreach" / "maps" / "info";
        const fs::path mccInfo = reachInstall->mapsDirectory / "info";
        fs::create_directories(sandboxInfo, ec);
        size_t copied = 0;
        for (const auto& entry : fs::directory_iterator(mccInfo, ec)) {
            if (!entry.is_regular_file(ec)) continue;
            fs::copy_file(entry.path(), sandboxInfo / entry.path().filename(), fs::copy_options::overwrite_existing, ec);
            if (!ec) ++copied;
        }
        if (copied > 0 && SetCurrentDirectoryW(sandboxRoot.c_str())) {
            NOBLE_INFO("Map-info sandbox: copied {} file(s) from '{}'; working directory is now '{}'",
                copied, mccInfo.string(), sandboxRoot.string());
        } else {
            NOBLE_ERROR("Map-info sandbox setup failed (copied {} from '{}'). The engine will report 'no map selected'.",
                copied, mccInfo.string());
        }
    }

    // Test/debug: apply a comma-separated variant patch after the selected variant is decoded.
    if (!variantPatch.empty()) {
        noble::BlamEngine::SetVariantPatch(variantPatch);
        NOBLE_INFO("main: -variant-override '{}'", variantPatch);
    }

    // Remember this launch for the post-game "PLAY AGAIN" button and the loading screen.
    if (!showMenu) {
    auto toWide = [](const std::string& s) -> std::wstring {
        if (s.empty()) return L"";
        const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        if (len <= 0) return L"";
        std::wstring w(len - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
        return w;
    };
    std::wstring args = L"-map \"" + toWide(launchMap) + L"\" -mode \"" + toWide(launchMode) + L"\"";
    args += L" -difficulty " + std::to_wstring(launchDifficulty);
    if (!customVariant.empty()) args += L" -variant \"" + toWide(customVariant) + L"\"";
    if (!customMvar.empty()) args += L" -mvar \"" + toWide(customMvar) + L"\"";
    // Keep the map-variant memory dump running after the menu relaunches into a match.
    if (mvarDumpEvery > 0) args += L" -mvar-dump-every " + std::to_wstring(mvarDumpEvery);
    if (mvarLiveDumpEvery > 0) args += L" -mvar-live-dump " + std::to_wstring(mvarLiveDumpEvery);
    if (mvarPageScanEvery > 0) args += L" -mvar-page-scan " + std::to_wstring(mvarPageScanEvery);
    if (launchSkulls != 0) args += L" -skulls " + std::to_wstring(launchSkulls);
    if (!modFolder.empty()) args += L" -mod \"" + modFolder.wstring() + L"\"";
    if (!mapOverride.empty()) args += L" -map-override \"" + mapOverride.wstring() + L"\"";
    if (!mapTitle.empty()) args += L" -map-title \"" + toWide(mapTitle) + L"\"";
    if (!mapImage.empty()) args += L" -map-image \"" + mapImage.wstring() + L"\"";
    if (!mapThumb.empty()) args += L" -map-thumb \"" + mapThumb.wstring() + L"\"";

    // In-match scoreboard overlay data: fetch the session roster once so the panel can show names, ranks,
    // emblems and teams while the scoreboard key is held.
    if (!connectSessionId.empty()) {
        std::thread([connectSessionId]() {
            noble::LobbyState st = noble::MasterServerClient::Get().FetchLobbyStateSync(connectSessionId);
            if (!st.ok || st.roster.empty()) return;
            noble::PauseMenu::SetScoreboardSession(connectSessionId);
            std::vector<noble::PauseMenu::ScoreboardEntry> rows;
            const std::string me = noble::Settings::Get().Snapshot().playerName;
            for (const auto& p : st.roster) {
                noble::PauseMenu::ScoreboardEntry e;
                e.name = p.username;
                e.tag = p.serviceTag;
                e.rank = p.rankIndex;
                e.emblem = p.emblemId;
                e.team = p.team;
                e.hostPeer = p.isHost;
                try { if (!p.machineId.empty()) e.machine = std::stoull(p.machineId, nullptr, 0); } catch (...) {}
                e.local = !me.empty() && _stricmp(p.username.c_str(), me.c_str()) == 0;
                if (e.local) noble::VoiceChat::SetLocalTeam(p.team);
                if (e.machine != 0) noble::SocialOverlay::SetPlayerName(e.machine, p.username);
                rows.push_back(e);
            }
            noble::PauseMenu::SetScoreboardRoster(std::move(rows), st.map, st.mode);
        }).detach();
    }

    if (!connectAddress.empty()) {
        args += L" -connect \"" + toWide(connectAddress) + L"\"";
        if (!connectSessionId.empty()) {
            args += L" -session-id \"" + toWide(connectSessionId) + L"\"";
            if (!connectSessionToken.empty()) {
                args += L" -session-token \"" + toWide(connectSessionToken) + L"\"";
            }
        }
    } else if (launchHost) {
        args += L" -host";
        if (hostOnline) args += L" -online";
        else args += L" -lan";
        if (hostPublic) args += L" -public";
        else args += L" -private";
        args += L" -max-players " + std::to_wstring(hostMaxPlayers);
        if (expectedPeers > 0) args += L" -wait-peers " + std::to_wstring(expectedPeers);
        if (!connectSessionId.empty()) {
            args += L" -session-id \"" + toWide(connectSessionId) + L"\"";
            if (!connectSessionToken.empty()) {
                args += L" -session-token \"" + toWide(connectSessionToken) + L"\"";
            }
        }
    }
    noble::Relaunch::SetCurrentGameArgs(args);
    noble::PauseMenu::SetLoadingMap(launchMap);
    if (!mapTitle.empty() || !mapImage.empty() || !mapThumb.empty()) {
        NOBLE_INFO("Loading screen: mod map '{}' (image '{}')", mapTitle, mapImage.string());
        noble::PauseMenu::SetLoadingMapInfo(mapTitle, mapImage, mapThumb);
        // The mod map's description: found in the mod's own JSON on a background thread, so
        // the launch does not wait for the scan.
        if (!modFolder.empty() && !mapTitle.empty()) {
            std::thread([folder = modFolder, title = mapTitle] {
                try {
                    const fs::path parent = folder.parent_path();
                    const auto mods = noble::ModCatalog::Scan(parent, parent);
                    for (const auto &mod : mods) {
                        std::error_code ec;
                        if (!fs::equivalent(mod.folder, folder, ec)) continue;
                        for (const auto *list : {&mod.campaign, &mod.firefight, &mod.multiplayer})
                            for (const auto &m : *list)
                                if (m.title == title && !m.description.empty()) {
                                    noble::PauseMenu::SetLoadingMapDescription(m.description);
                                    return;
                                }
                        return;
                    }
                } catch (...) {
                }
            }).detach();
        }
    }
    // A stock map or mission: the same one-line blurb the menu shows for it.
    if (modFolder.empty()) noble::PauseMenu::SetLoadingMapDescription(noble::FrontEndMenu::StockBlurb(launchMap));
    if (!mapImage.empty()) noble::UiArt::ImageFile(mapImage);
    else noble::UiArt::LoadingBackground(launchMap);
    if (!mapThumb.empty()) noble::UiArt::ImageFile(mapThumb);
    else noble::UiArt::MapPreview(launchMap, true);
    }

    // 8. Load and bootstrap haloreach.dll directly in process memory
    // Extended engine limits (LIMITS-HANDOFF.md): with a profile picked, Noble prepares its own patched copy of
    // Reach's DLL and loads that one; the installed file is never modified. Any mismatch (another DLL build, a
    // bad or missing patch) falls back to the stock DLL with the reason logged, so the game always starts.
    // AI ACTORS (C7d): the pool hook lives in Loader and only sees environment variables, so publish the saved
    // setting here. SetEnvironmentVariableW in this process is inherited by the relaunched match process.
    // A value already in the environment (from -ai-actors in the menu process, or inherited by the relaunched
    // match process) wins, so a command-line override is not replaced by the saved setting.
    if (startupSettings.aiActors > 0 && startupSettings.extendedLimits != 0 &&
        GetEnvironmentVariableW(L"NOBLE_AI_ACTORS", nullptr, 0) == 0) {
        const std::wstring actors = std::to_wstring(startupSettings.aiActors);
        SetEnvironmentVariableW(L"NOBLE_AI_ACTORS", actors.c_str());
    }
    // A mod may ship its own patched haloreach.dll ("modded haloreach.dll (REQUIRED)"): the AI Battleground mods
    // do, and that binary carries patches Noble does not apply (e.g. the AI-spawn jump at RVA 0x6920B5). Load it
    // when it is the same build as the installed one - Noble's hooks are RVA-based, so only that layout is known.
    fs::path modEngineDll;
    // -no-mod-dll: run the mod's map on Noble's own engine build (control for diagnosing mod-DLL-specific failures).
    const bool skipModDll = std::wstring(GetCommandLineW()).find(L"-no-mod-dll") != std::wstring::npos;
    if (skipModDll && !modFolder.empty()) NOBLE_WARN("main: -no-mod-dll, using Noble's engine even if the mod ships one");
    if (!skipModDll && !modFolder.empty()) {
        std::error_code modEc;
        std::vector<fs::path> candidates = {
            modFolder / "modded haloreach.dll (REQUIRED)" / "haloreach.dll",
            modFolder / "haloreach.dll",
        };
        for (const auto& entry : fs::directory_iterator(modFolder, modEc)) {
            if (entry.is_directory(modEc)) candidates.push_back(entry.path() / "haloreach.dll");
        }
        for (const auto& candidate : candidates) {
            if (fs::is_regular_file(candidate, modEc)) { modEngineDll = candidate; break; }
        }
        if (!modEngineDll.empty()) {
            const auto modSize = fs::file_size(modEngineDll, modEc);
            const auto stockSize = fs::file_size(reachInstall->reachDllPath, modEc);
            if (modSize == 0 || modSize != stockSize) {
                NOBLE_WARN("main: the mod's haloreach.dll ({} bytes) is not the installed build ({} bytes); using Noble's engine",
                           modSize, stockSize);
                modEngineDll.clear();
            } else {
                NOBLE_INFO("main: loading the mod's own haloreach.dll (same build, mod-patched): '{}'", modEngineDll.string());
            }
        }
    }
    const noble::limits::Profile requestedLimits = noble::limits::ProfileFromInt(startupSettings.extendedLimits);
    const noble::limits::PrepareResult engineDll =
        noble::limits::PrepareEngineDll(reachInstall->reachDllPath, requestedLimits);
    NOBLE_INFO("Engine limits: requested {}, active {}, reason {}", noble::limits::ProfileName(requestedLimits),
               noble::limits::ProfileName(engineDll.active), engineDll.reason.empty() ? "none" : engineDll.reason.c_str());
    // AI ACTORS (C7d) is applied by the pool-resize hook once the game-state exists; log what was asked for.
    if (startupSettings.aiActors > 0 && engineDll.active != noble::limits::Profile::Stock)
        NOBLE_INFO("Engine limits: AI actors {} (network id 0x{:08X})", startupSettings.aiActors,
                   noble::limits::ProfileNetworkId(engineDll.active, startupSettings.aiActors));
    auto loader = std::make_unique<noble::Loader>();
    // The second path is the installed DLL: its folder is where the engine's own dependencies (xaudio2, MCC
    // binaries) are found. Pass both even for a fallback, so the copy never becomes the dependency root.
    const fs::path engineToLoad = modEngineDll.empty() ? engineDll.dllPath : modEngineDll;
    if (!loader->LoadReachDll(engineToLoad, reachInstall->reachDllPath)) {
        NOBLE_ERROR("Failed to load haloreach.dll into process space.");
        return 1;
    }

    // Watch the engine's own main loop: if it returns, the frame pump stops and the client sits idle.
    noble::HookManager::InstallHook(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(loader->GetModuleHandle()) + 0xC390),
                                    reinterpret_cast<void*>(&EngineMainLoopHook),
                                    reinterpret_cast<void**>(&s_origEngineMainLoop), "trace_engine_main_loop");
    noble::HookManager::InstallHook(
        reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(loader->GetModuleHandle()) + 0x31204),
        reinterpret_cast<void*>(&MapParseHook), reinterpret_cast<void**>(&s_origMapParse), "trace_map_parse");
    noble::HookManager::InstallHook(
        reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(loader->GetModuleHandle()) + 0x141C0),
        reinterpret_cast<void*>(&MapLoadFullHook), reinterpret_cast<void**>(&s_origMapLoadFull), "trace_map_load");
    {
        wchar_t stateEnv[8] = {};
        if (GetEnvironmentVariableW(L"NOBLE_TRACE_MAP_STATE", stateEnv, 8) > 0 && stateEnv[0] != L'0') {
            noble::HookManager::InstallHook(
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(loader->GetModuleHandle()) + 0x40A008),
                reinterpret_cast<void*>(&MapStateTickHook), reinterpret_cast<void**>(&s_origMapStateTick),
                "trace_map_state_tick");
        }
    }
    {
        wchar_t startEnv[8] = {};
        if (GetEnvironmentVariableW(L"NOBLE_TRACE_SESSION_START", startEnv, 8) > 0 && startEnv[0] != L'0') {
            noble::HookManager::InstallHook(
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(loader->GetModuleHandle()) + 0x40B4C8),
                reinterpret_cast<void*>(&SessionStartHook),
                reinterpret_cast<void**>(&s_origSessionStart), "trace_session_start");
        }
    }
    noble::HookManager::InstallHook(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(loader->GetModuleHandle()) + 0xD564),
                                    reinterpret_cast<void*>(&Stage1Hook),
                                    reinterpret_cast<void**>(&s_origStage1), "trace_stage1");
    noble::HookManager::InstallHook(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(loader->GetModuleHandle()) + 0xC8F8),
                                    reinterpret_cast<void*>(&Stage2Hook),
                                    reinterpret_cast<void**>(&s_origStage2), "trace_stage2");
    noble::HookManager::InstallHook(reinterpret_cast<void*>(&OutputDebugStringA), reinterpret_cast<void*>(&HookOutputDebugStringA),
                                    reinterpret_cast<void**>(&s_origOdsA), "capture_engine_output");
    noble::HookManager::InstallHook(reinterpret_cast<void*>(&OutputDebugStringW), reinterpret_cast<void*>(&HookOutputDebugStringW),
                                    reinterpret_cast<void**>(&s_origOdsW), "capture_engine_output_w");
    noble::HookManager::InstallHook(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(loader->GetModuleHandle()) + 0x3873C),
                                    reinterpret_cast<void*>(&Stage3Hook),
                                    reinterpret_cast<void**>(&s_origStage3), "trace_stage3");

    // Direct all haloreach.dll asset and map file I/O through Virtual File System (VFS)
    noble::VFS::HookTargetModule(loader->GetModuleHandle());

    // The SETTINGS > VIDEO field-of-view slider writes the engine's own camera FOV global.
    noble::CameraFov::SetEngineModule(loader->GetModuleHandle());
    noble::CameraThirdPerson::Install(loader->GetModuleHandle());
    if (thirdPersonMode > 0) noble::CameraThirdPerson::SetMode(thirdPersonMode);
    if (thirdPersonDistance != 100) noble::CameraThirdPerson::SetDistancePercent(thirdPersonDistance);
    if (voiceProbe) noble::ProbeVoiceDevices();
    noble::VoiceChat::Start();   // idle until a relay session exists
    noble::SocialOverlay::Start();
    if (thirdPerson) noble::CameraThirdPerson::SetEnabled(true);

    // Apply 32/64 player lobby expansion detours
    noble::LobbyPatches::Apply(loader->GetModuleHandle());
    noble::LobbyPatches::SetCampaignCoop(!showMenu && _stricmp(launchMode.c_str(), "campaign") == 0);

    // Apply engine stability and appearance cap patches (unlocks all 200+ armor permutations in haloreach.dll)
    noble::EnginePatches::Apply(loader->GetModuleHandle());

    // Apply Forge engine patches (AI biped/vehicle spawning in Forge, object limits)
    noble::ForgePatches::Apply(loader->GetModuleHandle());

    // Initialize Shared Tag Cache for cross-map AI spawning (Approach 1)
    // Skip for front-end menu and Campaign launches: Campaign scenarios already carry native AI/vehicle tags in
    // campaign.map + <mission>.map, and mounting a donor campaign map (or falling back from m20.map to m45.map)
    // interferes with mission loading (especially ONI: Sword Base = m20).
    if (!showMenu && _stricmp(launchMode.c_str(), "campaign") != 0) {
        noble::SharedTagCache::SetAvoidMap(launchMap);
        noble::SharedTagCache::Initialize(loader->GetModuleHandle(), userStoragePath, reachInstall->workshopDirectory, reachInstall->reachDirectory);
    }

    // Wire guest-to-host spawn forwarder to prevent duplicate local spawning in multiplayer
    noble::SharedTagCache::SetSpawnForwarder([](uint32_t datumIndex, const std::string& tagPath) -> bool {
        if (noble::NetworkTransport::Get().IsActive() && !noble::NetworkTransport::Get().IsHost()) {
            NOBLE_INFO("SharedTagCache: Forwarding guest spawn request for datum 0x{:X} '{}' to match host.", datumIndex, tagPath);
            nlohmann::json j = {
                {"type", "spawn_request"},
                {"datum", datumIndex},
                {"tagPath", tagPath}
            };
            noble::NetworkTransport::Get().SendLobbyControlJson(j.dump());
            return true;
        }
        return false;
    });

#if defined(NOBLE_DEV_BUILD)
    // Initialize Developer Cheats & Mod Menu engine hooks (Dev builds only)
    noble::DevCheats::Initialize(loader->GetModuleHandle());
#endif

    // Wire Forge palette placement handler
    noble::ForgeMenu::SetPlaceHandler([](const noble::ForgeMenu::Item& item) -> bool {
        return noble::ForgePatches::PlaceObject(item.tag, item.tagIndex);
    });

    // 9. Instantiate Blam! Engine & Data Access interfaces
    auto blamEngine = std::make_unique<noble::BlamEngine>();
    noble::EngineHostContext hostContext;
    hostContext.hWnd = window->GetHwnd();
    hostContext.pDevice = renderer->GetDevice();
    hostContext.pContext = renderer->GetContext();
    hostContext.pSwapChain = renderer->GetSwapChain();
    hostContext.width = renderWidth;
    hostContext.height = renderHeight;

    if (!blamEngine->Initialize(*reachInstall, loader->GetModuleHandle(), loader->GetExports(), hostContext)) {
        NOBLE_ERROR("Failed to initialize Blam! engine host interfaces.");
        return 1;
    }

    // Dev tool: decode a game variant with the engine and exit.
    if (!variantDumpPath.empty()) {
        const bool ok = blamEngine->DumpGameVariant(variantDumpPath);
        noble::Logger::Shutdown();
        TerminateProcess(GetCurrentProcess(), ok ? 0 : 1);
    }

    // Elite armour pack (data\armor\<name>.map + .rrreloc, local only): loaded as an extra cache with
    // the match map. When mainmenu_all.map is present, the menu map already has all Spartan and Elite
    // Armory models and scripts built in, so ElitePack is only needed in matches.
    if (!showMenu || !noble::ReachArmory::HasCustomMenuMap()) {
        noble::ElitePack::Install(loader->GetModuleHandle(), userStoragePath, launchMode);
    }

    if (!connectAddress.empty()) {
        launchHost = false; // Guest joining remote match
        NOBLE_INFO("Launching Halo: Reach Engine as Guest connecting to '{}' (Map: {}, Mode: {}, Session: '{}')",
                   connectAddress, launchMap, launchMode, connectSessionId);

        std::string tip = connectAddress;
        uint16_t tport = 11774;
        const size_t c = connectAddress.rfind(':');
        if (c != std::string::npos) {
            tip = connectAddress.substr(0, c);
            const int p = std::atoi(connectAddress.substr(c + 1).c_str());
            if (p > 0 && p <= 65535) tport = static_cast<uint16_t>(p);
        }

        // Initialize Guest NetworkTransport and register the join with the Master Server / WebSocket Relay
        // BEFORE starting haloreach.dll's engine thread so the Host's NAT hole-punching and the WebSocket
        // relay are already active when the engine sends its first join-request packet.
        noble::NetworkTransport::Get().Initialize(false, tip, tport);
        const uint64_t guestMachineId = noble::NetworkTransport::Get().GetLocalMachineId();
        const uint16_t guestPort = noble::NetworkTransport::Get().GetLocalPort();
        if (!relayUrl.empty()) {
            noble::NetworkTransport::Get().StartRelay(relayUrl, connectSessionId.empty() ? "HALOREAC" : connectSessionId, false, guestMachineId);
        } else if (!noble::MasterServerClient::Get().Enabled()) {
            NOBLE_WARN("main: [GUEST] master server not enabled (SETTINGS > ONLINE); joining by direct UDP only, no relay");
        } else if (noble::MasterServerClient::Get().Enabled()) {
            const noble::MasterJoinInfo joinInfo = noble::MasterServerClient::Get().JoinSessionSync(
                connectSessionId, guestMachineId, guestPort, tip, tport);
            // Why the relay did or did not start (direct UDP alone fails when the host is behind CGNAT).
            NOBLE_INFO("main: [GUEST] master server join for session '{}': ok={}, relay_url={}, error='{}'",
                       connectSessionId, joinInfo.ok, joinInfo.relayUrl.empty() ? "(none)" : joinInfo.relayUrl, joinInfo.error);
            if (joinInfo.ok && !joinInfo.relayUrl.empty()) {
                noble::NetworkTransport::Get().StartRelay(
                    joinInfo.relayUrl,
                    joinInfo.sessionId.empty() ? "HALOREAC" : joinInfo.sessionId,
                    false,
                    guestMachineId);
            }
        }

    if (guestPostStart) noble::BlamEngine::SetGuestPostStart(true);
    if (engineModeOverride > 0) {
        noble::BlamEngine::SetEngineModeOverride(engineModeOverride);
        NOBLE_WARN("main: -engine-mode {} (research hook - forcing launch-block +0x0C)", engineModeOverride);
    }
    blamEngine->LaunchMap(launchMap, launchMode, launchDifficulty, launchSkulls, customMvar, customVariant, connectAddress, expectedPeers);
    } else {
        if (launchHost) {
            // Bind UDP port 11774 and start Master Server advertisement + WebSocket Relay BEFORE
            // blamEngine->LaunchMap loads the map so the Host's UDP socket and relay are ready before
            // any Guest sends its first join-request packet.
            noble::NetworkTransport::Get().Initialize(true);

            noble::HostConfig cfg;
            std::string modeTitle = launchMode;
            if (!modeTitle.empty()) modeTitle[0] = static_cast<char>(toupper(static_cast<unsigned char>(modeTitle[0])));
            cfg.sessionName = startupSettings.playerName + "'s " + modeTitle;
            if (launchMode == "campaign") cfg.sessionName = startupSettings.playerName + "'s Co-op Campaign";
            else if (launchMode == "firefight") cfg.sessionName = startupSettings.playerName + "'s Firefight";
            else if (launchMode == "forge") cfg.sessionName = startupSettings.playerName + "'s Forge";

            cfg.gameMode = launchMode;
            cfg.mapName = launchMap;
            cfg.gametypeName = (launchMode == "firefight" ? "Firefight" : launchMode == "forge" ? "Forge" : "Slayer");
            cfg.maxPlayers = hostMaxPlayers;
            // The port the game socket really got: 11774 unless another program holds it (the socket is exclusive
            // since 2026-10-29 and then falls back to a free port). Advertising 11774 regardless sent guests elsewhere.
            cfg.port = noble::NetworkTransport::Get().IsActive() ? noble::NetworkTransport::Get().GetLocalPort() : 11774;
            cfg.mode = hostOnline ? noble::NetworkMode::Online_P2P : noble::NetworkMode::Local_LAN;

            if (noble::SessionManager::Get().StartHosting(cfg)) {
                NOBLE_INFO("main: Started self-hosting match '{}' on port {} (Mode: {})", cfg.sessionName, cfg.port, hostOnline ? "Online P2P" : "Local LAN");
                if (hostOnline && noble::MasterServerClient::Get().Enabled()) {
                    noble::MasterAdvert advert;
                    advert.name = cfg.sessionName;
                    advert.port = cfg.port;
                    advert.maxPlayers = cfg.maxPlayers;
                    advert.map = cfg.mapName;
                    advert.mode = cfg.gameMode;
                    advert.variant = customVariant.empty() ? cfg.gametypeName : fs::path(customVariant).filename().string();
                    advert.difficulty = launchDifficulty;
                    advert.skulls = launchSkulls;
                    advert.inProgress = true;
                    advert.hostPeerId = noble::AuthClient::Get().IsLoggedIn() ? static_cast<uint64_t>(noble::AuthClient::Get().GetUserId()) : 1ULL;
                    advert.hostMachineId = "0x484F535400000001";
                    advert.hostUsername = noble::AuthClient::Get().IsLoggedIn() ? noble::AuthClient::Get().GetUsername() : (startupSettings.playerName.empty() ? "Host" : startupSettings.playerName);
                    advert.hostIsDev = noble::AuthClient::Get().IsDeveloper();
                    advert.hostServiceTag = startupSettings.serviceTag.empty() ? "117" : startupSettings.serviceTag;
                    advert.hasPassword = false;
                    advert.isPrivate = !hostPublic;
                    advert.isMod = !modFolder.empty();
                    // The match process re-registers the host's session; carry the mod it launched with so the
                    // session's mod id matches the map (the lobby's id is not available here).
                    if (!modFolder.empty()) {
                        advert.modId = modFolder.filename().string();
                        advert.modTitle = mapTitle;
                    }
                    advert.address = noble::UPnP::Get().GetExternalIP();
                    if (!connectSessionId.empty()) {
                        advert.id = connectSessionId;
                        advert.token = connectSessionToken;
                        advert.inProgress = true;

                        std::string mUrl = noble::MasterServerClient::Get().Url();
                        if (!mUrl.empty()) {
                            std::string hostPart = mUrl;
                            if (hostPart.rfind("https://", 0) == 0) hostPart = hostPart.substr(8);
                            else if (hostPart.rfind("http://", 0) == 0) hostPart = hostPart.substr(7);
                            while (!hostPart.empty() && (hostPart.back() == '/' || hostPart.back() == ' ')) hostPart.pop_back();
                            std::string computedRelayUrl = "wss://" + hostPart + "/ws/relay2?session=" + connectSessionId + "&role=host&protocol=" + std::to_string(noble::kClientProtocol);
                            noble::NetworkTransport::Get().StartRelay(computedRelayUrl, connectSessionId, true, noble::NetworkTransport::Get().GetLocalMachineId());
                            NOBLE_INFO("main: Pre-emptively connected host to WebSocket relay: {}", computedRelayUrl);
                        }
                    }
                    noble::MasterServerClient::Get().StartAdvertising(advert);
                    NOBLE_INFO("main: Advertised hosted match to master server ({}:{}) [Session: '{}']", advert.address, advert.port, connectSessionId.empty() ? "new" : connectSessionId);
                }
            } else {
                NOBLE_WARN("main: Failed to start self-hosting match");
            }
        }

        NOBLE_INFO("Launching Halo: Reach Engine: Target Map='{}', Mode='{}'", launchMap, launchMode);
        blamEngine->LaunchMap(launchMap, launchMode, launchDifficulty, launchSkulls, customMvar, customVariant, "", expectedPeers);

        // The lobby's teams, from "-lobby-teams "1,2,4"": player index -> team (1..8). The engine deals its own
        // two teams, so each player's side is overridden in HaloScript once they are in the match
        // (player_override_team <player index> <0-based team>). Retried a few times: the first attempt can
        // land before the player unit exists, and the engine re-spawns on round start.
        std::vector<int> lobbyTeams;
        {
            int argc = 0;
            LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
            for (int i = 1; argv && i + 1 < argc; ++i) {
                if (_wcsicmp(argv[i], L"-lobby-teams") != 0) continue;
                std::wstring csv = argv[i + 1];
                size_t start = 0;
                while (start <= csv.size()) {
                    const size_t comma = csv.find(L',', start);
                    const std::wstring part = csv.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
                    if (!part.empty()) lobbyTeams.push_back(std::clamp(_wtoi(part.c_str()), 1, 8));
                    if (comma == std::wstring::npos) break;
                    start = comma + 1;
                }
            }
            if (argv) LocalFree(argv);
        }

        // Synchronous Co-Op modes (Campaign / Firefight) do not have opposing player teams, and executing
        // host-only HaloScript `player_override_team` mutates deterministic player state asymmetrically (OOS).
        if (!lobbyTeams.empty() && launchMode != "campaign" && launchMode != "firefight") {
            // The scoreboard's roster-less fallback and voice chat use this: -lobby-teams is in roster order,
            // so entry 0 is the local player of a hosted match.
            noble::PauseMenu::SetLocalTeam(lobbyTeams.front());
            noble::VoiceChat::SetLocalTeam(lobbyTeams.front());
            NOBLE_INFO("main: applying {} lobby team assignment(s) to the match (local side {})",
                       lobbyTeams.size(), lobbyTeams.front());
            // Re-issued for three minutes, once every 3 s. The engine only has a team when its session starts
            // (tens of seconds into the load) and its override is a lobby-side request, so a handful of early
            // one-shot commands all landed while the map was still loading and were dropped.
            std::thread([teams = lobbyTeams] {
                const ULONGLONG start = GetTickCount64();
                while (GetTickCount64() - start < 180000) {
                    if (noble::BlamEngine* engine = noble::BlamEngine::Active()) {
                        for (size_t i = 0; i < teams.size(); ++i) {
                            engine->ExecuteCommand("player_override_team " + std::to_string(i) + " " + std::to_string(teams[i] - 1));
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                }
            }).detach();
        }
    }

    if (!engineCommand.empty() && engineCommandAt >= 0) {
        // Research hook: dispatch one engine command after N seconds (e.g. -engine-cmd "enter_editor_mode" -engine-cmd-at 16)
        const std::string cmd = engineCommand;
        const int delayMs = engineCommandAt * 1000;
        std::thread([cmd, delayMs]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            NOBLE_WARN("main: dispatching engine command '{}' (research hook)", cmd);
            if (noble::BlamEngine::Active()) noble::BlamEngine::Active()->ExecuteCommand(cmd);
            else NOBLE_WARN("main: no active engine for command '{}'", cmd);
        }).detach();
    }

    if (mvarLiveDumpEvery > 0) {
        // The snapshot needs the engine thread's TLS state, so it runs from slot 1 rather than a worker thread.
        noble::BlamEngine::ConfigureLiveDump(noble::ReachHost::Get().GetDataDir() / "mvar_live", mvarLiveDumpEvery);
    }
    if (mvarPageScanEvery > 0) {
        noble::StartPageScan(mvarPageScanEvery, noble::ReachHost::Get().GetDataDir() / "mvar_pages");
    }

    if (mvarDumpEvery > 0) {
        std::thread([mvarDumpEvery] {
            int index = 0;
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(mvarDumpEvery));
                const std::filesystem::path out = noble::ReachHost::Get().GetDataDir() / "mvar_dump" /
                                                  ("t" + std::to_string(++index));
                const int found = noble::DumpMapVariantCandidates(out);
                NOBLE_WARN("main: -mvar-dump-every snapshot {} -> {} candidate(s)", index, found);
            }
        }).detach();
    }

    if (!mvarDumpTimes.empty()) {
        std::thread([times = mvarDumpTimes] {
            int previous = 0;
            for (const int at : times) {
                const int wait = at > previous ? at - previous : 0;
                previous = at;
                std::this_thread::sleep_for(std::chrono::seconds(wait));
                NOBLE_WARN("main: -mvar-dump-at {} - scanning memory for loaded map variants", at);
                const std::filesystem::path out =
                    noble::ReachHost::Get().GetDataDir() / "mvar_dump" / std::to_string(at);
                const int found = noble::DumpMapVariantCandidates(out);
                NOBLE_WARN("main: -mvar-dump-at {} found {} candidate(s) in '{}'", at, found, out.string());
                (void)noble::InstallWriteWatch();
            }
        }).detach();
    }

    if (forgeSaveAt >= 0) {
        std::thread([forgeSaveAt] {
            std::this_thread::sleep_for(std::chrono::seconds(forgeSaveAt));
            NOBLE_WARN("main: -forge-save-at {} - running the Forge save-map-variant path (test hook)", forgeSaveAt);
            const std::filesystem::path saved = noble::PauseMenu::SaveForgeMapVariant("Dev Test Variant");
            NOBLE_WARN("main: -forge-save-at result: '{}'", saved.empty() ? std::string("(nothing)") : saved.string());
        }).detach();
    }

    NOBLE_INFO("Authentic Halo: Reach host message loop active. Driving frames...");

    // 11. Native Win32 message pump: Drive frame rendering, wireframe dots, UI overlays, and swap chain Present
    const ULONGLONG liveWindowStart = GetTickCount64();
    bool liveWindowSwitched = false;
    // NOBLE_STALL_DUMP=1: if the engine stops pumping frames for 15 s, dump every thread's stack once.
    {
        wchar_t stallEnv[8] = {};
        if (GetEnvironmentVariableW(L"NOBLE_STALL_DUMP", stallEnv, 8) > 0 && stallEnv[0] != L'0') {
            std::thread([] {
                for (int i = 0; i < 300; ++i) {
                    Sleep(1000);
                    const uint64_t last = noble::ReachHost::Get().GetLastFrameTickMs();
                    if (last == 0) continue;
                    if (GetTickCount64() - last > 15000) {
                        DumpAllThreadStacks();
                        return;
                    }
                }
            }).detach();
        }
    }

    while (window->ProcessMessages()) {
        if (!liveWindowSwitched && liveWindowTestAt >= 0 &&
            GetTickCount64() - liveWindowStart >= static_cast<ULONGLONG>(liveWindowTestAt) * 1000ull) {
            liveWindowSwitched = true;
            window->ApplyWindowSettings(startupSettings.width, startupSettings.height, true);
            NOBLE_INFO("main: -live-window-test switched to borderless full screen without restarting");
        }

        if (noble::FrontEndMenu::IsMatchRunning()) {
            noble::FrontEndMenu::CheckMatchStatus();
            MsgWaitForMultipleObjectsEx(0, nullptr, 25, QS_ALLINPUT, MWMO_ALERTABLE);
            continue;
        }

        blamEngine->Update(0.016f);

        // haloreach.dll's engine thread presents (host slot 1). After a match it stops drawing; then the
        // post-game panel is drawn and presented from here (PauseMenu::HoldFrame, as the engine does).
        // The front-end menu likewise once the engine drops its menu map (FrontEndMenu::HoldFrame).
        if (!noble::PauseMenu::HoldFrame() && !noble::FrontEndMenu::HoldFrame()) {
            MsgWaitForMultipleObjectsEx(0, nullptr, 10, QS_ALLINPUT, MWMO_ALERTABLE);
        }
    }

    // Watchdog in case network shutdown stalls on an unresponsive router or server.
    std::thread([] {
        Sleep(5000);
        NOBLE_WARN("Shutdown took over 5 s; exiting anyway.");
        RemoveOwnRunFlag();
        noble::Logger::Shutdown();
        TerminateProcess(GetCurrentProcess(), 0);  // not ExitProcess: its DLL shutdown can hang (see below)
    }).detach();

    // 13. Clean shutdown: stop master server advertisement, UPnP/LAN discovery, and UDP/relay sockets,
    // flush logs, and terminate the process without tearing down live D3D11/ImGui or c_game_engine
    // heap objects under haloreach.dll's running engine/audio threads (fixes the ntdll access violation on exit).
    NOBLE_INFO("Initiating clean host shutdown...");
    noble::VoiceChat::Shutdown();
    noble::SocialClient::Get().Shutdown();
    noble::MasterServerClient::Get().Shutdown();
    noble::SessionManager::Get().Shutdown();
    blamEngine->Shutdown();

    NOBLE_INFO("Network and host services shut down; exiting process cleanly.");
    RemoveOwnRunFlag();
    noble::Logger::Shutdown();
    TerminateProcess(GetCurrentProcess(), 0);  // not ExitProcess: its DLL shutdown can hang
    return 0;   // not reached
}
