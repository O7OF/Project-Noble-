#include "PoolResize.hpp"

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

uint64_t ArrayBytes(int64_t count, uint64_t recordSize) {
    return static_cast<uint64_t>(count) * recordSize + static_cast<uint64_t>((count + 31) / 32) * 4;
}
int s_actors = 0;   // 0 = off
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
};

void* __fastcall Hook_DataArrayNew(const char* name, int32_t count, uint64_t recordSize, int32_t alignBits, void* allocator) {
    if (s_actors > 0 && name) {
        for (const Rule& rule : kRules) {
            if (std::strcmp(name, rule.name) != 0) continue;
            if (!Selected(name)) break;
            int32_t wanted = rule.size(count);
            if (wanted > count) {
                const uint64_t left = s_budget > s_spent ? s_budget - s_spent : 0;
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
                if (header) s_spent += ArrayBytes(wanted, recordSize) - ArrayBytes(count, recordSize);
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
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i < argc; ++i) {
        if (_wcsicmp(argv[i], L"-ai-pools") == 0 && i + 1 < argc) {
            s_only = Narrow(argv[i + 1]);
            SetEnvironmentVariableW(L"NOBLE_AI_POOLS", argv[i + 1]);
        } else if (_wcsicmp(argv[i], L"-pool-trace") == 0) {
            s_trace = true;
            SetEnvironmentVariableW(L"NOBLE_POOL_TRACE", L"1");
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
    return value;
}

} // namespace

void InstallResize(HMODULE engine, bool extendedProfile) {
    const auto base = reinterpret_cast<uintptr_t>(engine);
    // The bigger AI update frame is part of the engine build itself, so its stack commit is needed
    // whenever that build is loaded, with or without -ai-actors.
    const bool bigAiFrame = std::memcmp(reinterpret_cast<void*>(base + kAiUpdateRva), kAiUpdateHead, sizeof(kAiUpdateHead)) == 0;
    if (bigAiFrame && !HookManager::InstallHook(reinterpret_cast<void*>(base + kAiUpdateRva), reinterpret_cast<void*>(&Hook_AiUpdate),
            reinterpret_cast<void**>(&s_aiUpdateOriginal), "ai_update_stack")) {
        NOBLE_ERROR("AI limits: could not hook the AI update scheduler; its 12 KB frame is used without a stack commit");
    }
    s_engineBase = base;
    s_engineEnd = base + 0x20000000;
    if (extendedProfile &&
        std::memcmp(reinterpret_cast<void*>(base + kReportGsFailureRva), kReportGsFailureHead, sizeof(kReportGsFailureHead)) == 0)
        HookManager::InstallHook(reinterpret_cast<void*>(base + kReportGsFailureRva), reinterpret_cast<void*>(&Hook_GsFailure),
            reinterpret_cast<void**>(&s_gsOriginal), "gs_failure_report");

    const int actors = ReadActorSetting();
    if (actors <= 96) return;                       // not requested (or not an increase)
    if (!extendedProfile) {
        NOBLE_WARN("AI limits: -ai-actors needs ENGINE LIMITS set to Extended; ignored on Stock");
        return;
    }
    if (std::memcmp(reinterpret_cast<void*>(base + kDataArrayNewRva), kDataArrayNewHead, sizeof(kDataArrayNewHead)) != 0) {
        NOBLE_WARN("AI limits: haloreach.dll code does not match the expected build; ignored");
        return;
    }
    const auto* sizeInsn = reinterpret_cast<const uint8_t*>(base + kAiBankSizeInsnRva);
    uint32_t aiBank = 0;
    if (sizeInsn[0] == 0xBA) std::memcpy(&aiBank, sizeInsn + 1, 4);
    if (aiBank <= kStockAiBank + kSafetyMargin) {
        NOBLE_WARN("AI limits: this engine build has no extra AI memory (bank 0x{:X}); rebuild data/limits with "
                   "make_v16_particles.py. -ai-actors ignored", aiBank);
        return;
    }
    s_budget = aiBank - kStockAiBank - kSafetyMargin;
    if (!bigAiFrame || !s_aiUpdateOriginal) {
        // Without the bigger frame the scheduler overflows past 96 live actors.
        NOBLE_WARN("AI limits: this engine build lacks the 1,024-actor AI update frame; actor pool stays at 96");
        return;
    }
    s_actors = (std::min)(actors, 1024);
    ReadPoolOptions();
    if (!HookManager::InstallHook(reinterpret_cast<void*>(base + kDataArrayNewRva), reinterpret_cast<void*>(&Hook_DataArrayNew),
            reinterpret_cast<void**>(&s_original), "ai_pool_resize")) {
        NOBLE_WARN("AI limits: could not hook data_array_new; ignored");
        s_actors = 0;
        return;
    }
    NOBLE_INFO("AI limits: experimental, actor pool {} (stock 96); AI memory +{} KB; pools: {}{}", s_actors, s_budget / 1024,
        s_only.empty() ? std::string("all") : s_only, s_trace ? "; tracing every pool" : "");
}

} // namespace noble::pools
