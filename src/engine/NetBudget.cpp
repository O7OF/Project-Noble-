#include "NetBudget.hpp"

#include "common/EngineModule.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <windows.h>

#include <atomic>
#include <cstdint>

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
constexpr int32_t kNobleCapNormal = 4000000;  // 4 Mbit/s
constexpr int32_t kNobleCapGood = 6000000;    // 6 Mbit/s
constexpr int32_t kEngineMaxAssumed = 10485760;

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
        const bool capsKnown = (capNormal == kStockCapNormal || capNormal == kNobleCapNormal) &&
                               (capGood == kStockCapGood || capGood == kNobleCapGood);
        const bool pctKnown = pctNormal > 0 && pctNormal <= 100 && pctGood > 0 && pctGood <= 100;
        if (!capsKnown || !pctKnown || assumed < 0 || assumed > kEngineMaxAssumed) return 1;

        capNormal = boost ? kNobleCapNormal : kStockCapNormal;
        capGood = boost ? kNobleCapGood : kStockCapGood;

        if (s_stockAssumed < 0) s_stockAssumed = assumed;
        int32_t wantAssumed = s_stockAssumed;
        if (boost && uploadKbps > 0) {
            const int64_t bps = static_cast<int64_t>(uploadKbps) * 1000;
            wantAssumed = static_cast<int32_t>(bps > kEngineMaxAssumed ? kEngineMaxAssumed : bps);
            if (wantAssumed < s_stockAssumed) wantAssumed = s_stockAssumed;
        }
        assumed = wantAssumed;
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
