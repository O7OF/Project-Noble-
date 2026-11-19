#include "NetBudget.hpp"

#include "common/EngineModule.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <windows.h>

#include <atomic>
#include <cstdint>
#include <cstring>

namespace noble::NetBudget {
namespace {

// Channel-manager settings block (pointer stored in the engine's stream manager). Field meaning, from the engine's
// own bandwidth-cap function: +0x154 / +0x158 = fixed total upload caps (bit/s) for the normal / good-network case;
// +0x160 / +0x164 = % of the estimated upload speed allowed in those cases; +0x138 = lowest upload estimate the engine
// assumes (bit/s; the engine itself never assumes more than 10 Mbit/s).
constexpr uintptr_t kConfigPointerRva = 0x2D67250;
constexpr size_t kAssumedUpload = 0x138;
constexpr size_t kCapNormal = 0x154;
constexpr size_t kCapGood = 0x158;
constexpr size_t kPctNormal = 0x160;
constexpr size_t kPctGood = 0x164;

constexpr int32_t kStockCapNormal = 393216;   // 384 kbit/s (unchanged since the 2010 Xbox 360 game)
constexpr int32_t kStockCapGood = 524288;     // 512 kbit/s
constexpr int32_t kOldNobleCapNormal = 4000000;  // earlier Noble builds (4 Mbit/s), still recognised
constexpr int32_t kOldNobleCapGood = 6000000;    // 6 Mbit/s
// The fixed caps no longer limit anything real: the engine's own "50 / 75 % of the measured upload" rule decides.
constexpr int32_t kNobleCapNormal = 50000000;    // 50 Mbit/s
constexpr int32_t kNobleCapGood = 75000000;      // 75 Mbit/s
constexpr int32_t kEngineMaxAssumed = 10485760;  // stock ceiling on the upload estimate (10 Mbit/s)
constexpr int32_t kNobleMaxAssumed = 104857600;  // Noble's ceiling (100 Mbit/s): fast home connections, dedicated servers

// The estimate ceiling lives in the engine's upload-estimate update as two immediates: a compare and the store that
// clamps the estimate. Both are byte-checked; nothing is written if the bytes differ (other game build).
constexpr uintptr_t kCeilingCmpRva = 0x3E9761;    // cmp eax, imm32            (3D imm32)
constexpr uintptr_t kCeilingStoreRva = 0x3E9775;  // mov dword [rip+d], imm32  (C7 05 d32 imm32)
std::atomic<int> s_ceilingState{0};               // 0 = not tried, 1 = stock, 2 = raised, -1 = unsupported build

// Stream manager: +0x2708 is the lowest total upload budget the cap function will hand out (bit/s). When the
// game cannot measure the upload speed it assumes 10 Mbit/s whatever +0x138 says, so a host that states its
// upload speed (hostUploadKbps, e.g. a dedicated server) gets its budget through this floor instead.
constexpr uintptr_t kStreamManagerRva = 0x2D649E0;
constexpr size_t kBudgetFloor = 0x2708;
int32_t s_floorWritten = -1;                      // the value Noble last wrote (-1 = none)
int32_t s_floorStock = -1;                        // the engine's own value, first seen

std::atomic<ULONGLONG> s_lastTick{0};
std::atomic<bool> s_loggedApplied{false};
std::atomic<bool> s_loggedSkipped{false};
int32_t s_stockAssumed = -1;   // the engine's own value, remembered so the setting can be turned off again

int32_t* ConfigBlock() {
    __try {
        const auto base = reinterpret_cast<uintptr_t>(noble::ReachModule());
        if (!base) return nullptr;
        return *reinterpret_cast<int32_t* const*>(base + kConfigPointerRva);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

bool WriteImm32(uint8_t* at, int32_t value) {
    DWORD old = 0;
    if (!VirtualProtect(at, 4, PAGE_EXECUTE_READWRITE, &old)) return false;
    std::memcpy(at, &value, 4);
    DWORD ignored = 0;
    VirtualProtect(at, 4, old, &ignored);
    FlushInstructionCache(GetCurrentProcess(), at, 4);
    return true;
}

// Sets the estimate ceiling to `ceiling` (stock or Noble's). Returns false if this game build doesn't match.
bool SetEstimateCeiling(int32_t ceiling) {
    __try {
        auto* base = reinterpret_cast<uint8_t*>(noble::ReachModule());
        if (!base) return false;
        uint8_t* cmp = base + kCeilingCmpRva;
        uint8_t* store = base + kCeilingStoreRva;
        int32_t cmpImm = 0, storeImm = 0;
        std::memcpy(&cmpImm, cmp + 1, 4);
        std::memcpy(&storeImm, store + 6, 4);
        const bool shapeOk = cmp[0] == 0x3D && store[0] == 0xC7 && store[1] == 0x05;
        const bool valuesOk = (cmpImm == kEngineMaxAssumed || cmpImm == kNobleMaxAssumed) && cmpImm == storeImm;
        if (!shapeOk || !valuesOk) return false;
        if (cmpImm == ceiling) return true;
        return WriteImm32(store + 6, ceiling) && WriteImm32(cmp + 1, ceiling);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

int32_t& Field(int32_t* block, size_t offset) {
    return *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(block) + offset);
}

// 0 = applied, 1 = layout not recognised, 2 = read fault
int Apply(int32_t* block, bool boost, int uploadKbps) {
    __try {
        int32_t& capNormal = Field(block, kCapNormal);
        int32_t& capGood = Field(block, kCapGood);
        const int32_t pctNormal = Field(block, kPctNormal);
        const int32_t pctGood = Field(block, kPctGood);
        int32_t& assumed = Field(block, kAssumedUpload);
        const bool capsKnown =
            (capNormal == kStockCapNormal || capNormal == kNobleCapNormal || capNormal == kOldNobleCapNormal) &&
            (capGood == kStockCapGood || capGood == kNobleCapGood || capGood == kOldNobleCapGood);
        const bool pctKnown = pctNormal > 0 && pctNormal <= 100 && pctGood > 0 && pctGood <= 100;
        if (!capsKnown || !pctKnown || assumed < 0 || assumed > kNobleMaxAssumed) return 1;

        capNormal = boost ? kNobleCapNormal : kStockCapNormal;
        capGood = boost ? kNobleCapGood : kStockCapGood;

        if (s_stockAssumed < 0) s_stockAssumed = assumed;
        int32_t wantAssumed = s_stockAssumed;
        if (boost && uploadKbps > 0) {
            const int64_t bps = static_cast<int64_t>(uploadKbps) * 1000;
            const int32_t ceiling = s_ceilingState.load() == 2 ? kNobleMaxAssumed : kEngineMaxAssumed;
            wantAssumed = static_cast<int32_t>(bps > ceiling ? ceiling : bps);
            if (wantAssumed < s_stockAssumed) wantAssumed = s_stockAssumed;
        }
        assumed = wantAssumed;

        // Budget floor from the stated upload speed: the normal-case % of it, never above Noble's cap.
        auto* floorField = reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(noble::ReachModule()) +
                                                 kStreamManagerRva + kBudgetFloor);
        if (s_floorStock < 0) s_floorStock = *floorField;   // the engine's own floor (37568 bit/s seen 2026-11-19)
        int32_t wantFloor = s_floorStock;
        if (boost && uploadKbps > 0) {
            const int64_t f = static_cast<int64_t>(uploadKbps) * 1000 * pctNormal / 100;
            const int32_t stated = static_cast<int32_t>(f > kNobleCapNormal ? kNobleCapNormal : f);
            if (stated > wantFloor) wantFloor = stated;
        }
        // Only touch the field while it still holds the engine's value or ours (never fight another writer).
        if ((*floorField == s_floorStock || *floorField == s_floorWritten) && *floorField != wantFloor) {
            *floorField = wantFloor;
            s_floorWritten = wantFloor;
        }
        return 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return 2;
    }
}

} // namespace

void Tick() {
    const ULONGLONG now = GetTickCount64();
    if (now - s_lastTick.load(std::memory_order_relaxed) < 2000) return;
    s_lastTick.store(now, std::memory_order_relaxed);

    int32_t* block = ConfigBlock();
    if (!block) return;
    const auto settings = Settings::Get().Snapshot();

    // Upload-estimate ceiling: 10 Mbit/s stock, 100 Mbit/s with the boost (follows the setting both ways).
    const int ceilingState = s_ceilingState.load();
    if (ceilingState != -1) {
        const bool wantRaised = settings.networkBudgetBoost;
        if (wantRaised && ceilingState != 2) {
            if (SetEstimateCeiling(kNobleMaxAssumed)) {
                s_ceilingState.store(2);
                NOBLE_INFO("NetBudget: upload speed limit raised from 10 to 100 Mbit/s");
            } else {
                s_ceilingState.store(-1);
                NOBLE_WARN("NetBudget: upload speed limit not changed (unexpected game build)");
            }
        } else if (!wantRaised && ceilingState == 2) {
            if (SetEstimateCeiling(kEngineMaxAssumed)) {
                s_ceilingState.store(1);
                NOBLE_INFO("NetBudget: upload speed limit back to the game's 10 Mbit/s");
            }
        }
    }
    const int result = Apply(block, settings.networkBudgetBoost, settings.hostUploadKbps);
    if (result == 0 && !s_loggedApplied.exchange(true)) {
        NOBLE_INFO("NetBudget: network upload budget {} (host caps {} / {} kbit/s{})",
                   settings.networkBudgetBoost ? "raised" : "left at stock",
                   (settings.networkBudgetBoost ? kNobleCapNormal : kStockCapNormal) / 1000,
                   (settings.networkBudgetBoost ? kNobleCapGood : kStockCapGood) / 1000,
                   settings.hostUploadKbps > 0 ? ", upload speed set in settings" : "");
    } else if (result != 0 && !s_loggedSkipped.exchange(true)) {
        NOBLE_WARN("NetBudget: game network settings not recognised; upload budget left unchanged");
    }
}

} // namespace noble::NetBudget
