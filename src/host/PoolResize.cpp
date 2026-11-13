#include "PoolResize.hpp"
#include "SlotPatch.hpp"
#include "EngineLimits.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

#include <intrin.h>
#include <cstdio>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <shellapi.h>
#include <string>

namespace noble::pools {

namespace {

constexpr uintptr_t kDataArrayNewRva = 0x12FCC;
const uint8_t kDataArrayNewHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57 };

using DataArrayNewFn = void*(__fastcall*)(const char* name, int32_t count, uint64_t recordSize, int32_t alignBits, void* allocator);
DataArrayNewFn s_original = nullptr;

// The AI pools are carved from game-state bank 1 (stock 0x220000 bytes, ~5 KB spare). Noble's engine
// profiles enlarge it; the room added is all the resized pools may use, or they overrun bank 2.
constexpr uintptr_t kAiBankSizeInsnRva = 0x16154;   // mov edx, <bank 1 size>
constexpr uint32_t kStockAiBank = 0x220000;
constexpr uint64_t kSafetyMargin = 0x10000;
uint64_t s_budget = 0;
uint64_t s_spent = 0;

// Stack-overrun reporter. A local buffer sized for the stock actor count trips the engine's stack
// cookie and __report_gsfailure kills the process with a fail-fast that no crash handler sees, so
// log the failing function and the call stack first.
constexpr uintptr_t kReportGsFailureRva = 0x788A78;
const uint8_t kReportGsFailureHead[] = { 0x48, 0x89, 0x4C, 0x24, 0x08, 0x48, 0x83, 0xEC, 0x38, 0xB9, 0x17, 0x00, 0x00, 0x00 };
using GsFailureFn = void(__fastcall*)(uintptr_t);
GsFailureFn s_gsOriginal = nullptr;
uintptr_t s_engineBase = 0, s_engineEnd = 0;

std::string Where(uintptr_t a) {
    char buf[48];
    if (a >= s_engineBase && a < s_engineEnd) std::snprintf(buf, sizeof(buf), "haloreach+0x%llX", static_cast<unsigned long long>(a - s_engineBase));
    else std::snprintf(buf, sizeof(buf), "0x%llX", static_cast<unsigned long long>(a));
    return buf;
}

void __fastcall Hook_GsFailure(uintptr_t cookie) {
    NOBLE_ERROR("STACK OVERRUN in {} (an engine function's local buffer overflowed; stack cookie 0x{:X})",
        Where(reinterpret_cast<uintptr_t>(_ReturnAddress())), cookie);
    void* frames[24] = {};
    const USHORT n = RtlCaptureStackBackTrace(0, 24, frames, nullptr);
    for (USHORT i = 0; i < n; ++i) NOBLE_ERROR("  overrun frame {:02}: {}", i, Where(reinterpret_cast<uintptr_t>(frames[i])));
    s_gsOriginal(cookie);
}

// The engine profile grows the AI update scheduler's stack frame (0x70ABBC) from 0x568 to 0x30E8 bytes
// so its per-actor list holds 1,024 entries. The original was under one page and has no stack probe,
// so commit the pages first: a probing function's frame, released before the call, is deeper than it.
constexpr uintptr_t kAiUpdateRva = 0x70ABBC;
const uint8_t kAiUpdateHead[] = { 0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x08, 0x48, 0x89, 0x70, 0x10, 0x48, 0x89, 0x78, 0x18, 0x55,
                                  0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xA8, 0x18, 0xD0, 0xFF, 0xFF };   // lea rbp,[rax-0x2FE8]
using AiUpdateFn = uintptr_t(__fastcall*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
AiUpdateFn s_aiUpdateOriginal = nullptr;

__declspec(noinline) void CommitStack() {
    volatile uint8_t probe[0x4000];
    for (size_t i = 0; i < sizeof(probe); i += 0x1000) probe[i] = 0;
}

uintptr_t __fastcall Hook_AiUpdate(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d) {
    CommitStack();
    return s_aiUpdateOriginal(a, b, c, d);
}

// The AI spawn-candidate search (0x76E814) also grew its stack frame in the engine build: 0x900 -> 0x1100 bytes
// for a 1,024-entry scratch array (see AI_FRAME3_OPS in make_v16_particles.py). It has no __chkstk either, so
// commit the pages before its prologue runs. Only the patched prologue matches (lea rbp,[rax-0x1038]).
constexpr uintptr_t kAiSpawnSearchRva = 0x76E814;
const uint8_t kAiSpawnSearchHead[] = { 0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55,
                                       0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xA8, 0xC8, 0xEF, 0xFF, 0xFF };   // lea rbp,[rax-0x1038]
using AiSpawnSearchFn = uintptr_t(__fastcall*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
AiSpawnSearchFn s_aiSpawnSearchOriginal = nullptr;

uintptr_t __fastcall Hook_AiSpawnSearch(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d) {
    CommitStack();
    return s_aiSpawnSearchOriginal(a, b, c, d);
}

uint64_t ArrayBytes(int64_t count, uint64_t recordSize) {
    return static_cast<uint64_t>(count) * recordSize + static_cast<uint64_t>((count + 31) / 32) * 4;
}
int s_actors = 0;   // 0 = off
uint64_t s_renderBudget = 0, s_renderSpent = 0;   // added room in render bank 3 (see InstallResize)
int s_renderStates = 0;   // -render-states N: opt-in only; the game-state bank cannot take it yet (see below)
std::string s_only;  // -ai-pools a,b,c: resize only these (bisecting crashes); empty = all
bool s_trace = false;

bool Selected(const char* name) {
    if (s_only.empty()) return true;
    const std::string list = "," + s_only + ",";
    return list.find("," + std::string(name) + ",") != std::string::npos;
}

void Trace(const char* name, int32_t count, uint64_t recordSize, void* allocator, void* header) {
    if (!s_trace || !header) return;
    const auto* h = static_cast<const uint8_t*>(header);
    NOBLE_INFO("POOLNEW|{}|{}|{}|{:p}|{:p}|data={:p}|bitmap={:p}", name ? name : "?", count, recordSize, header, allocator,
        *reinterpret_cast<void* const*>(h + 0x50), *reinterpret_cast<void* const*>(h + 0x58));
}

struct Rule { const char* name; int32_t (*size)(int32_t stock); };

int32_t Scaled(int32_t stock) { return static_cast<int32_t>((static_cast<int64_t>(stock) * s_actors + 95) / 96); }
int32_t AtLeast256(int32_t stock) { return (std::max)(stock, 256); }

const Rule kRules[] = {
    { "actor", [](int32_t) { return static_cast<int32_t>(s_actors); } },
    { "prop", [](int32_t stock) { return Scaled(stock); } },              // perceived objects (256 stock)
    { "prop_ref", [](int32_t stock) { return Scaled(stock); } },          // per-actor references to them (1,024)
    { "prop_search", [](int32_t stock) { return Scaled(stock); } },       // 32
    { "command scripts", [](int32_t stock) { return AtLeast256(stock); } },     // 40
    { "object list header", [](int32_t stock) { return AtLeast256(stock); } },  // 48
    { "dynamic firing points", [](int32_t stock) { return (std::max)(stock, 64); } },  // 15
    { "swarm", [](int32_t stock) { return (std::max)(stock, 128); } },    // 32
    // Script engine pools (LIMITS-TODO C6). Capacities seen in the live pool report; big custom maps and
    // horde scripts run out of script nodes and references first.
    { "det hs thread", [](int32_t stock) { return (std::max)(stock, 1024); } },        // 320 x 1,324 B
    { "non-det hs thread", [](int32_t stock) { return (std::max)(stock, 32); } },      // 4 x 1,324 B
    { "list object reference", [](int32_t stock) { return (std::max)(stock, 512); } }, // 128 x 12 B
    // "script node" (16,280 x 24 B in the pool report) is not created through data_array_new, so the rule
    // below is inert for now; it is here for the day that constructor is found. See docs/limits-ai.md (C6).
    { "script node", [](int32_t stock) { return (std::max)(stock, 32768); } },         // 16,280 x 24 B
    // C11 runtime half: opt-in with -render-states N (needs -ai-actors too). Before the render bank grew, 16,384 crashed at
    // haloreach+0x27B01F and 8,192 at haloreach+0x6E58E during map load — the game-state bank has no room for
    // the 624-byte records beyond the stock 4,096. Noble builds now add 0x800000 to render bank 3 (room for 16,384);
    // the records are charged to that room (s_renderBudget), not the AI bank.
    { "cached object render states", [](int32_t stock) { return stock; } },   // -render-states overrides
};

static bool PatchDword(uintptr_t addr, int32_t delta, bool isAbsolute = false) {
    DWORD oldProtect = 0;
    if (!VirtualProtect(reinterpret_cast<void*>(addr), 4, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;
    int32_t val = 0;
    std::memcpy(&val, reinterpret_cast<void*>(addr), 4);
    val = isAbsolute ? delta : (val + delta);
    std::memcpy(reinterpret_cast<void*>(addr), &val, 4);
    VirtualProtect(reinterpret_cast<void*>(addr), 4, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(addr), 4);
    return true;
}

constexpr int32_t kAiFrameExtra = 1024 * 12 - 96 * 12; // 0x2B80
struct FrameOp { uintptr_t rva; int32_t delta; };
const FrameOp kAiFrameOps[] = {
    { 0x70ABD7, -kAiFrameExtra },
    { 0x70ABDE,  kAiFrameExtra },
    { 0x70ABF8,  kAiFrameExtra },
    { 0x70AFA7,  kAiFrameExtra },
    { 0x70AFB7,  kAiFrameExtra },
};

constexpr int32_t kAiFrame2Extra = 0x5800;
const FrameOp kAiFrame2Ops[] = {
    { 0x76EC0C, -kAiFrame2Extra },
    { 0x76EC11,  kAiFrame2Extra },
    { 0x76EC21,  kAiFrame2Extra },
    { 0x76EC32,  kAiFrame2Extra },
    { 0x76EC39,  kAiFrame2Extra },
    { 0x76EC5F,  kAiFrame2Extra },
    { 0x76EC72,  kAiFrame2Extra },
    { 0x76ED0D,  kAiFrame2Extra },
    { 0x76EE4B,  kAiFrame2Extra },
    { 0x76EE5B,  kAiFrame2Extra },
};
const FrameOp kAiFrame2Caps[] = {
    { 0x76EC6B, 0x200 },
    { 0x76EB83, 0x200 },
    { 0x76EA68, 0x200 },
};

constexpr int32_t kAiFrame3Extra = 0x800;
const FrameOp kAiFrame3Ops[] = {
    { 0x76E829, -kAiFrame3Extra },
    { 0x76E830,  kAiFrame3Extra },
    { 0x76E845,  kAiFrame3Extra },
    { 0x76E854,  kAiFrame3Extra },
    { 0x76E869,  kAiFrame3Extra },
    { 0x76EAB4,  kAiFrame3Extra },
    { 0x76EAC4,  kAiFrame3Extra },
};
// Global tracking for pool hooks
static bool s_hasOfflinePatches = false;

const FrameOp kAiFrame3Caps[] = {
    { 0x76E8EF, 0x200 },
    { 0x76E925, 0x200 },
};

// C12-L step 4: per-player pools follow the slot arrays to 32 - only once SlotPatch has really installed 32-player
// mode (the pools are created at game-state init, after SlotPatch::Install). Their memory comes from the engine's
// own allocator object (not the AI bank budget); data_array_new returns null when it has no room, so fall back to
// the stock 16 and log it. "simulation distributed view" is not created here (SlotPatch handles it).
const char* const kPlayerPools[] = { "players", "fire team", "simulation view data array" };

void* __fastcall Hook_DataArrayNew(const char* name, int32_t count, uint64_t recordSize, int32_t alignBits, void* allocator) {
    if (name && count == 16 && slots::Players32Active()) {
        for (const char* pool : kPlayerPools) {
            if (std::strcmp(name, pool) != 0) continue;
            int32_t made = 32;
            void* header = s_original(name, made, recordSize, alignBits, allocator);
            if (header) NOBLE_WARN("C12 players: {} 16 -> 32 ({} KB more)", name, (16 * recordSize) / 1024);
            else {
                NOBLE_ERROR("C12 players: no room to raise '{}' to 32; using 16", name);
                made = count;
                header = s_original(name, made, recordSize, alignBits, allocator);
            }
            Trace(name, made, recordSize, allocator, header);
            return header;
        }
    }

    const noble::limits::Profile profile = noble::limits::ActiveProfile();
    const bool extendedProfile = profile != noble::limits::Profile::Stock;

    if (extendedProfile && name) {
        int32_t wanted = count;
        if (std::strcmp(name, "object") == 0 && count == 2048) {
            wanted = 32767;
        } else if (s_hasOfflinePatches) {
            if (profile >= noble::limits::Profile::V16Particles) {
                if (std::strcmp(name, "c_particle_emitter_gpu") == 0 && count == 832) wanted = 6656;
                else if (std::strcmp(name, "c_particle_gpu") == 0 && count == 2048) wanted = 16384;
                else if (std::strcmp(name, "c_particle_emitter_gpu::s_row") == 0 && count == 448) wanted = 3584;
            }
            if (profile == noble::limits::Profile::Full) {
                if (std::strcmp(name, "c_light_volume_gpu") == 0 && count == 160) wanted = 65535;
                else if (std::strcmp(name, "c_light_volume_gpu::s_row") == 0 && count == 160) wanted = 65535;
                else if (std::strcmp(name, "beam") == 0 && count == 160) wanted = 65535;
                else if (std::strcmp(name, "beam_location") == 0 && count == 160) wanted = 65535;
                else if (std::strcmp(name, "beam_system") == 0 && count == 160) wanted = 65535;
            }
        }

        if (wanted > count) {
            void* header = s_original(name, wanted, recordSize, alignBits, allocator);
            if (header) {
                NOBLE_INFO("Engine limits: {} {} -> {} ({} KB)", name, count, wanted,
                    (static_cast<uint64_t>(wanted - count) * recordSize) / 1024);
                Trace(name, wanted, recordSize, allocator, header);
                return header;
            } else {
                NOBLE_WARN("Engine limits: memory allocation fallback for '{}' (wanted {}); using {}", name, wanted, count);
            }
        }
    }
    if (s_actors > 0 && name) {
        for (const Rule& rule : kRules) {
            if (std::strcmp(name, rule.name) != 0) continue;
            if (!Selected(name)) break;
            // The render-state array is created from the same game-state banks, before the AI pools, so it must
            // stay inside the AI bank budget (charging 16,384 to it crashed the engine at map load: 0xC0000005).
            // It is only raised when the actor count leaves room for both; at 1,024 actors the bank is needed
            // for the actor pools themselves.
            // Render states live in game-state bank 3 (render), not the AI bank: they are charged to the render
            // bank's own added room (s_renderBudget), read from the loaded engine at install time.
            const bool renderStates = std::strcmp(name, "cached object render states") == 0;
            if (renderStates && s_renderStates <= count) break;
            int32_t wanted = renderStates ? s_renderStates : rule.size(count);
            if (wanted > count) {
                const uint64_t budget = renderStates ? s_renderBudget : s_budget;
                const uint64_t spent = renderStates ? s_renderSpent : s_spent;
                const uint64_t left = budget > spent ? budget - spent : 0;
                if (ArrayBytes(wanted, recordSize) - ArrayBytes(count, recordSize) > left) {
                    const auto fit = static_cast<int32_t>(count + left / (recordSize + 1));
                    NOBLE_WARN("AI limits: {} {} does not fit the AI memory bank ({} KB left); using {}", name, wanted, left / 1024,
                        (std::max)(fit, count));
                    wanted = (std::max)(fit, count);
                }
            }
            if (wanted > count) {
                NOBLE_INFO("AI limits: {} {} -> {} ({} KB more game-state memory)", name, count, wanted,
                    (static_cast<uint64_t>(wanted - count) * recordSize) / 1024);
                void* header = s_original(name, wanted, recordSize, alignBits, allocator);
                if (header) (renderStates ? s_renderSpent : s_spent) += ArrayBytes(wanted, recordSize) - ArrayBytes(count, recordSize);
                if (!header) {
                    NOBLE_ERROR("AI limits: game-state memory ran out raising '{}' to {}; using {}", name, wanted, count);
                    header = s_original(name, count, recordSize, alignBits, allocator);
                }
                Trace(name, wanted, recordSize, allocator, header);
                return header;
            }
            break;
        }
    }
    void* header = s_original(name, count, recordSize, alignBits, allocator);
    Trace(name, count, recordSize, allocator, header);
    return header;
}

std::string Narrow(const wchar_t* w) {
    std::string out;
    for (; w && *w; ++w) out.push_back(static_cast<char>(*w < 128 ? *w : '?'));
    return out;
}

void ReadPoolOptions() {
    wchar_t env[256] = {};
    if (GetEnvironmentVariableW(L"NOBLE_AI_POOLS", env, 256) > 0) s_only = Narrow(env);
    if (GetEnvironmentVariableW(L"NOBLE_POOL_TRACE", env, 256) > 0) s_trace = env[0] == L'1';
    if (GetEnvironmentVariableW(L"NOBLE_RENDER_STATES", env, 256) > 0) s_renderStates = _wtoi(env);
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i < argc; ++i) {
        if (_wcsicmp(argv[i], L"-ai-pools") == 0 && i + 1 < argc) {
            s_only = Narrow(argv[i + 1]);
            SetEnvironmentVariableW(L"NOBLE_AI_POOLS", argv[i + 1]);
        } else if (_wcsicmp(argv[i], L"-pool-trace") == 0) {
            s_trace = true;
            SetEnvironmentVariableW(L"NOBLE_POOL_TRACE", L"1");
        } else if (_wcsicmp(argv[i], L"-render-states") == 0 && i + 1 < argc) {
            s_renderStates = _wtoi(argv[i + 1]);
            SetEnvironmentVariableW(L"NOBLE_RENDER_STATES", argv[i + 1]);
        }
    }
    if (argv) LocalFree(argv);
}

int ReadActorSetting() {
    int value = 0;
    wchar_t env[32] = {};
    if (GetEnvironmentVariableW(L"NOBLE_AI_ACTORS", env, 32) > 0) value = _wtoi(env);
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i + 1 < argc; ++i) {
        if (_wcsicmp(argv[i], L"-ai-actors") == 0) {
            value = _wtoi(argv[i + 1]);
            SetEnvironmentVariableW(L"NOBLE_AI_ACTORS", argv[i + 1]);   // carried into the relaunched match process
        }
    }
    if (argv) LocalFree(argv);
    // No fallback to the settings file here: the dedicated server target has no Settings. The client copies
    // SETTINGS > ENGINE > AI ACTORS into NOBLE_AI_ACTORS before loading the engine (main.cpp, step 8).
    return value;
}

} // namespace

void InstallResize(HMODULE engine, bool extendedProfile) {
    const auto base = reinterpret_cast<uintptr_t>(engine);
    s_engineBase = base;
    s_engineEnd = base + 0x20000000;

    const int actors = ReadActorSetting();
    if (!extendedProfile && actors <= 96 && !slots::Players32Requested()) {
        return;
    }

    if (std::memcmp(reinterpret_cast<void*>(base + kDataArrayNewRva), kDataArrayNewHead, sizeof(kDataArrayNewHead)) != 0) {
        NOBLE_WARN("AI limits: haloreach.dll code does not match the expected build; ignored");
        return;
    }

    // Stack probes and frame patches for AI update scheduler & spawn candidate search
    const bool bigAiFrame = std::memcmp(reinterpret_cast<void*>(base + kAiUpdateRva), kAiUpdateHead, sizeof(kAiUpdateHead)) == 0;
    if (!bigAiFrame) {
        for (const auto& op : kAiFrameOps) {
            PatchDword(base + op.rva, op.delta);
        }
    }
    if (!HookManager::InstallHook(reinterpret_cast<void*>(base + kAiUpdateRva), reinterpret_cast<void*>(&Hook_AiUpdate),
            reinterpret_cast<void**>(&s_aiUpdateOriginal), "ai_update_stack")) {
        NOBLE_WARN("AI limits: could not hook the AI update scheduler stack probe");
    }

    const bool bigSpawnFrame = std::memcmp(reinterpret_cast<void*>(base + kAiSpawnSearchRva), kAiSpawnSearchHead, sizeof(kAiSpawnSearchHead)) == 0;
    if (!bigSpawnFrame) {
        for (const auto& op : kAiFrame2Ops) PatchDword(base + op.rva, op.delta);
        for (const auto& op : kAiFrame2Caps) PatchDword(base + op.rva, op.delta);
        for (const auto& op : kAiFrame3Ops) PatchDword(base + op.rva, op.delta);
        for (const auto& op : kAiFrame3Caps) PatchDword(base + op.rva, op.delta);
    }
    if (!HookManager::InstallHook(reinterpret_cast<void*>(base + kAiSpawnSearchRva), reinterpret_cast<void*>(&Hook_AiSpawnSearch),
            reinterpret_cast<void**>(&s_aiSpawnSearchOriginal), "ai_spawn_search_stack")) {
        NOBLE_WARN("AI limits: could not hook the AI spawn search stack probe");
    }

    if (extendedProfile &&
        std::memcmp(reinterpret_cast<void*>(base + kReportGsFailureRva), kReportGsFailureHead, sizeof(kReportGsFailureHead)) == 0)
        HookManager::InstallHook(reinterpret_cast<void*>(base + kReportGsFailureRva), reinterpret_cast<void*>(&Hook_GsFailure),
            reinterpret_cast<void**>(&s_gsOriginal), "gs_failure_report");

    // Bank 1 (AI memory bank)
    const auto* sizeInsn = reinterpret_cast<const uint8_t*>(base + kAiBankSizeInsnRva);
    uint32_t aiBank = 0;
    if (sizeInsn[0] == 0xBA) std::memcpy(&aiBank, sizeInsn + 1, 4);
    if (aiBank <= kStockAiBank + kSafetyMargin) {
        NOBLE_WARN("AI limits: this engine build has stock AI memory bank (0x{:X}); actor pool stays at 96 on stock DLL", aiBank);
        s_budget = 0;
        s_actors = 0;
    } else {
        s_budget = aiBank - kStockAiBank - kSafetyMargin;
        if (actors > 96) s_actors = (std::min)(actors, 1024);
    }

    // Bank 3 (Render memory bank)
    const auto* renderInsn = reinterpret_cast<const uint8_t*>(base + 0x16199);
    uint32_t renderBank = 0;
    if (renderInsn[0] == 0xBA) std::memcpy(&renderBank, renderInsn + 1, 4);
    if (renderBank > 0x560000) {
        s_renderBudget = renderBank - 0x560000 - kSafetyMargin;
        s_hasOfflinePatches = true;
    } else {
        s_renderBudget = 0;
        s_hasOfflinePatches = false;
    }

    ReadPoolOptions();

    if (!HookManager::InstallHook(reinterpret_cast<void*>(base + kDataArrayNewRva), reinterpret_cast<void*>(&Hook_DataArrayNew),
            reinterpret_cast<void**>(&s_original), "ai_pool_resize")) {
        NOBLE_WARN("AI limits: could not hook data_array_new; pool resize disabled");
        s_actors = 0;
        return;
    }
    NOBLE_INFO("Engine limits & AI pools hook installed (profile: {}, actors: {}, AI memory: +{} KB)",
        noble::limits::ProfileName(noble::limits::ActiveProfile()),
        s_actors > 0 ? std::to_string(s_actors) : "stock (96)",
        s_budget / 1024);
}

int ActiveActors() {
    return s_actors > 0 ? s_actors : 96;
}

} // namespace noble::pools
