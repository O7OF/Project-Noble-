#include "MemberTrace.hpp"

#if defined(NOBLE_DEV_BUILD)
#include "common/Logger.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <shellapi.h>
#include <thread>
#endif

namespace noble::membertrace {

#if defined(NOBLE_DEV_BUILD)
namespace {

// Layout of one session membership object (see docs/limits-players-audit.md).
constexpr uintptr_t kSessionsRva = 0x2B07418;
constexpr uintptr_t kSessionSize = 0x61668;
constexpr int kSessions = 4;
struct Region { uint32_t begin, end; const char* name; };
// Only the slot records themselves (and the peer table); scalar fields between them (masks, counts, ids at
// +0x1738..+0x173F, +0x2240..+0x226F, +0x3948..+0x394F, +0x4450..+0x446B) do not move with the slot count, so
// accesses to them are not recorded.
constexpr Region kRegions[] = {
    { 0x0060, 0x1738, "peersA" },    // 17 x 0x158
    { 0x1740, 0x2240, "playersA" },  // 16 x 0xB0
    { 0x2270, 0x3948, "peersB" },    // 17 x 0x158
    { 0x3950, 0x4450, "playersB" },  // 16 x 0xB0
    { 0x446C, 0x4538, "peerTable" }, // 17 x 0xC (C12-L (c))
};

// C12-L (d): accesses already reviewed (tools/limits/gen_member_known.py). Any other engine instruction that
// touches the slot arrays is a path nobody has checked for slots >= 16 yet: logged once as MEMBER-NEW.
constexpr uint32_t kKnown[] = {
#include "MemberKnown.inc"
};
bool Known(uint32_t rva) { return std::binary_search(std::begin(kKnown), std::end(kKnown), rva); }
constexpr uint32_t kTraceEnd = 0x4538;   // everything traced sits in [object, object + 0x4538)

uintptr_t s_engine = 0, s_engineEnd = 0;
uintptr_t s_pageBegin[kSessions], s_pageEnd[kSessions];
std::mutex s_mutex;
struct Key { uint32_t rva; uint16_t region; uint16_t write; bool operator<(const Key& o) const { return rva != o.rva ? rva < o.rva : region != o.region ? region < o.region : write < o.write; } };
struct Hit { uint64_t count = 0; uint32_t minOffset = UINT32_MAX, maxOffset = 0; bool reported = false; };
std::map<Key, Hit> s_hits;
thread_local int t_rearm = -1;   // session index whose pages must be re-guarded after the single step
std::atomic<uint64_t> s_total{ 0 };

void Guard(int k) {
    DWORD old = 0;
    VirtualProtect(reinterpret_cast<void*>(s_pageBegin[k]), s_pageEnd[k] - s_pageBegin[k], PAGE_READWRITE | PAGE_GUARD, &old);
}

LONG CALLBACK Handler(EXCEPTION_POINTERS* info) {
    const DWORD code = info->ExceptionRecord->ExceptionCode;
    if (code == STATUS_GUARD_PAGE_VIOLATION) {
        const uintptr_t addr = info->ExceptionRecord->ExceptionInformation[1];
        for (int k = 0; k < kSessions; ++k) {
            if (addr < s_pageBegin[k] || addr >= s_pageEnd[k]) continue;
            const uintptr_t object = s_engine + kSessionsRva + static_cast<uintptr_t>(k) * kSessionSize;
            const uintptr_t rip = info->ContextRecord->Rip;
            if (addr >= object && addr < object + kTraceEnd && rip >= s_engine && rip < s_engineEnd) {
                const auto offset = static_cast<uint32_t>(addr - object);
                uint16_t region = 0;
                for (uint16_t r = 0; r < static_cast<uint16_t>(std::size(kRegions)); ++r)
                    if (offset >= kRegions[r].begin && offset < kRegions[r].end) region = static_cast<uint16_t>(r + 1);
                if (region) {
                    std::lock_guard<std::mutex> lock(s_mutex);
                    Hit& h = s_hits[{ static_cast<uint32_t>(rip - s_engine), region,
                        static_cast<uint16_t>(info->ExceptionRecord->ExceptionInformation[0]) }];
                    ++h.count;
                    if (offset < h.minOffset) h.minOffset = offset;
                    if (offset > h.maxOffset) h.maxOffset = offset;
                }
                ++s_total;
            }
            t_rearm = k;                               // the guard is gone for this access: step, then re-arm
            info->ContextRecord->EFlags |= 0x100;      // trap flag
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (code == STATUS_SINGLE_STEP && t_rearm >= 0) {
        Guard(t_rearm);
        t_rearm = -1;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

const char* Kind(uint16_t write) { return write == 1 ? "W" : write == 8 ? "X" : "R"; }

// Every 5 s: each instruction not in kKnown, once, as soon as it is seen (a stalled trace still leaves it in the log).
void ReportNew() {
    std::lock_guard<std::mutex> lock(s_mutex);
    for (auto& [key, hit] : s_hits) {
        if (hit.reported || Known(key.rva)) continue;
        hit.reported = true;
        NOBLE_WARN("MEMBER-NEW|0x{:X}|{}|{}|{}|0x{:X}-0x{:X}", key.rva, kRegions[key.region - 1].name, Kind(key.write),
            hit.count, hit.minOffset, hit.maxOffset);
    }
}

void Dump() {
    std::lock_guard<std::mutex> lock(s_mutex);
    std::set<uint32_t> fresh;
    for (const auto& [key, hit] : s_hits) if (!Known(key.rva)) fresh.insert(key.rva);
    NOBLE_INFO("MemberTrace: {} accesses, {} distinct instructions, {} not reviewed yet (NEW)", s_total.load(),
        s_hits.size(), fresh.size());
    for (const auto& [key, hit] : s_hits) {
        NOBLE_INFO("MEMBER|0x{:X}|{}|{}|{}|0x{:X}-0x{:X}|{}", key.rva, kRegions[key.region - 1].name, Kind(key.write),
            hit.count, hit.minOffset, hit.maxOffset, Known(key.rva) ? "known" : "NEW");
    }
}

bool Requested() {
    wchar_t env[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_TRACE_MEMBERS", env, 8) > 0 && env[0] == L'1') return true;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool on = false;
    for (int i = 1; argv && i < argc; ++i) if (_wcsicmp(argv[i], L"-trace-members") == 0) on = true;
    if (argv) LocalFree(argv);
    if (on) SetEnvironmentVariableW(L"NOBLE_TRACE_MEMBERS", L"1");   // carried into the relaunched match process
    return on;
}

} // namespace
#endif

void Install(HMODULE engine) {
#if defined(NOBLE_DEV_BUILD)
    if (!engine || !Requested()) return;
    s_engine = reinterpret_cast<uintptr_t>(engine);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(s_engine);
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(s_engine + dos->e_lfanew);
    s_engineEnd = s_engine + nt->OptionalHeader.SizeOfImage;
    for (int k = 0; k < kSessions; ++k) {
        const uintptr_t object = s_engine + kSessionsRva + static_cast<uintptr_t>(k) * kSessionSize;
        s_pageBegin[k] = object & ~static_cast<uintptr_t>(0xFFF);
        s_pageEnd[k] = (object + kTraceEnd + 0xFFF) & ~static_cast<uintptr_t>(0xFFF);
    }
    AddVectoredExceptionHandler(1, &Handler);
    for (int k = 0; k < kSessions; ++k) Guard(k);
    std::thread([] {
        for (int tick = 1;; ++tick) {
            Sleep(5000);
            ReportNew();
            if (tick % 6 == 0) Dump();
        }
    }).detach();
    NOBLE_WARN("MemberTrace: tracing the player/peer slots and the peer table of the 4 network sessions (slow; dev "
               "only). MEMBER-NEW| as soon as an unreviewed instruction is seen, all MEMBER| lines every 30 s; {} known",
               std::size(kKnown));
#else
    (void)engine;
#endif
}

} // namespace noble::membertrace
