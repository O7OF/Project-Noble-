#include "host/SlotMask.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"
#include "host/SlotPatch.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace noble::slotmask {
namespace {

// ---- shared helpers ----------------------------------------------------------------------------------------------

// Session object layout used by these functions (from our MCC decompile): +0x5C peer-valid mask (DWORD),
// +0x60 peer records (0x158 each; record +0x08 = peer state, 8 = established), +0x4464 local peer index,
// +0x60EF8 session state.
constexpr size_t kPeerMask = 0x5C;
constexpr size_t kLocalPeer = 0x4464;
constexpr size_t kSessionState = 0x60EF8;
constexpr size_t kPeerState = 0x08;
constexpr uint32_t kPeerEstablished = 8;

uint64_t PeerValidMask(uint8_t* session) {
    uint64_t mask = *reinterpret_cast<const uint32_t*>(session + kPeerMask);
    if (const uint32_t* high = slots::PeerMaskHigh(session)) mask |= static_cast<uint64_t>(*high) << 32;
    const uint32_t slots = slots::PeerSlotCount();
    if (slots < 64) mask &= (uint64_t{1} << slots) - 1;   // the engine's loops never look past their bound
    return mask;
}

uint8_t* PeerRecordAt(uint8_t* session, uint32_t peer) {
    if (peer < 17) return session + 0x60 + static_cast<size_t>(peer) * 0x158;
    return slots::PeerRecord(session, static_cast<int32_t>(peer));
}

int Mode() {   // 0 = off, 1 = verify, 2 = use Noble's version
    wchar_t v[4] = {};
    if (GetEnvironmentVariableW(L"NOBLE_MASK64", v, 4) > 0 && v[0] == L'1') return 2;
    if (GetEnvironmentVariableW(L"NOBLE_MASK64_VERIFY", v, 4) > 0 && v[0] == L'1') return 1;
    return 0;
}

int g_mode = 0;

struct Stats {
    const char* name;
    std::atomic<uint64_t> calls{0};
    std::atomic<uint64_t> mismatches{0};
};

// ---- 1. hr+0x395728: "is any other peer still connecting?" -------------------------------------------------------
// Engine: while the session state is 4..9, walk the valid peers (bit order) and return true for the first peer that
// is not the local one and whose state is not 8 (established). Pure query, no writes.
constexpr uintptr_t kAnyPeerConnectingRva = 0x395728;
constexpr uint8_t kAnyPeerConnectingBytes[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x7C,
                                               0x24, 0x10, 0x8B, 0x81, 0xF8, 0x0E, 0x06, 0x00};
using AnyPeerConnectingFn = bool(__fastcall*)(uint8_t* session);
AnyPeerConnectingFn g_anyPeerConnectingOriginal = nullptr;
Stats g_anyPeerConnectingStats{"any peer still connecting"};

bool AnyPeerConnecting64(uint8_t* session) {
    const uint32_t state = *reinterpret_cast<const uint32_t*>(session + kSessionState);
    if (state - 4u >= 6u) return false;
    const int32_t local = *reinterpret_cast<const int32_t*>(session + kLocalPeer);
    for (uint64_t mask = PeerValidMask(session); mask; mask &= mask - 1) {
        unsigned long bit = 0;
        _BitScanForward64(&bit, mask);
        const uint32_t peer = static_cast<uint32_t>(bit);
        if (static_cast<int32_t>(peer) == local) continue;
        const uint8_t* record = PeerRecordAt(session, peer);
        if (!record) continue;
        if (*reinterpret_cast<const uint32_t*>(record + kPeerState) != kPeerEstablished) return true;
    }
    return false;
}

bool __fastcall AnyPeerConnectingDetour(uint8_t* session) {
    g_anyPeerConnectingStats.calls.fetch_add(1, std::memory_order_relaxed);
    if (g_mode == 2) return AnyPeerConnecting64(session);
    const bool engine = g_anyPeerConnectingOriginal(session);
    if (AnyPeerConnecting64(session) != engine)
        g_anyPeerConnectingStats.mismatches.fetch_add(1, std::memory_order_relaxed);
    return engine;
}

// ---- install / stats ----------------------------------------------------------------------------------------------

struct Replacement {
    const char* label;
    uintptr_t rva;
    const uint8_t* bytes;
    size_t byteCount;
    void* detour;
    void** original;
    Stats* stats;
};

const Replacement kReplacements[] = {
    {"slotmask_any_peer_connecting", kAnyPeerConnectingRva, kAnyPeerConnectingBytes, sizeof(kAnyPeerConnectingBytes),
     reinterpret_cast<void*>(&AnyPeerConnectingDetour), reinterpret_cast<void**>(&g_anyPeerConnectingOriginal),
     &g_anyPeerConnectingStats},
};

bool BytesMatch(const uint8_t* at, const uint8_t* want, size_t n) {
    __try {
        return std::memcmp(at, want, n) == 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

std::atomic<ULONGLONG> g_lastReport{0};

} // namespace

void Install(HMODULE engine) {
    g_mode = Mode();
    if (!engine || g_mode == 0) return;
    auto* base = reinterpret_cast<uint8_t*>(engine);
    for (const Replacement& r : kReplacements) {   // all or nothing
        if (!BytesMatch(base + r.rva, r.bytes, r.byteCount)) {
            NOBLE_WARN("SlotMask: engine code for '{}' does not match this build; nothing installed", r.label);
            return;
        }
    }
    int installed = 0;
    for (const Replacement& r : kReplacements) {
        if (HookManager::InstallHook(base + r.rva, r.detour, r.original, r.label)) ++installed;
    }
    NOBLE_INFO("SlotMask: {} of {} 64-player mask functions installed ({})", installed,
               static_cast<int>(sizeof(kReplacements) / sizeof(kReplacements[0])),
               g_mode == 2 ? "Noble's versions in use" : "verify mode: engine results kept, differences counted");
}

void ReportStats() {
    if (g_mode == 0) return;
    const ULONGLONG now = GetTickCount64();
    ULONGLONG last = g_lastReport.load(std::memory_order_relaxed);
    if (now - last < 30000 || !g_lastReport.compare_exchange_strong(last, now)) return;
    for (const Replacement& r : kReplacements) {
        const uint64_t calls = r.stats->calls.load(), bad = r.stats->mismatches.load();
        if (calls && g_mode == 1)
            NOBLE_INFO("SlotMask: '{}': {} calls, {} different from the engine", r.stats->name, calls, bad);
        else if (calls)
            NOBLE_INFO("SlotMask: '{}': {} calls handled by Noble's version", r.stats->name, calls);
    }
}

} // namespace noble::slotmask
