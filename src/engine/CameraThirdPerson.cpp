#include "CameraThirdPerson.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"
#include "network/NetworkTransport.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>

namespace noble {

namespace {

// haloreach+0x48A910: per-player camera-mode check. Verified prologue (MCC build 2025.08.16):
//   48 89 5C 24 08   mov qword ptr [rsp+8], rbx
//   48 89 74 24 10   mov qword ptr [rsp+10h], rsi
// Reach Renaissance hooks the same function (5-byte patch window) and answers it from its own camera state.
constexpr uintptr_t kCameraModeCheckRva = 0x48A910;
// haloreach+0x1DCADC: the camera follow/update Reach Renaissance also hooks (7-byte prologue window there). We call the
// original and then scale the follow distance field it filled in, which is how RR tunes third-person distance.
constexpr uintptr_t kCameraFollowRva = 0x1DCADC;
constexpr uint8_t kExpectedPrologue[9] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24};

using CameraModeCheckFn = bool(__fastcall*)(int32_t playerIndex);
CameraModeCheckFn s_original = nullptr;

std::atomic<bool> s_enabled{false};
std::atomic<int> s_distancePercent{100};   // 100 = engine default; >100 pulls the camera further back
std::atomic<int> s_followLogCount{0};
std::atomic<int> s_mode{0};   // experiment: 0 = answer true, 1 = answer false, 2 = original(...) | enabled
std::atomic<bool> s_f5WasDown{false};
std::atomic<int> s_logCount{0};

// The follow function takes (camera object, int, float x3 vector, void*, void* follow state). We forward everything
// untouched so the ABI cannot drift, then nudge the distance the original wrote at [state + 0x28].
using CameraFollowFn = void(__fastcall*)(void*, int32_t, float, float, float, void*, void*);
CameraFollowFn s_originalFollow = nullptr;

void __fastcall Hook_CameraFollow(void* a1, int32_t a2, float f0, float f1, float f2, void* a4, void* a5) {
    if (s_originalFollow) s_originalFollow(a1, a2, f0, f1, f2, a4, a5);
    const int percent = s_distancePercent.load();
    if (!s_enabled.load() || percent == 100 || !a5) return;
    auto* distance = reinterpret_cast<float*>(static_cast<uint8_t*>(a5) + 0x28);
    *distance *= static_cast<float>(percent) / 100.0f;
    if (s_followLogCount.fetch_add(1) < 3) {
        NOBLE_INFO("CameraThirdPerson: follow distance scaled {}% -> {:.3f}", percent, *distance);
    }
}

bool __fastcall Hook_CameraModeCheck(int32_t playerIndex) {
    const bool orig = s_original ? s_original(playerIndex) : false;
    if (!s_enabled.load()) return orig;
    // Host/mode gate: online guests do not get the camera unless the session host runs it too (Tyler's rule). Offline and
    // the host always may.
    const bool online = noble::NetworkTransport::Get().IsActive();
    const bool allowed = !online || noble::NetworkTransport::Get().IsHost();
    if (!allowed) {
        static std::atomic<int> s_deniedLog{0};
        if (s_deniedLog.fetch_add(1) < 1) NOBLE_INFO("CameraThirdPerson: third person is host-only; guest keeps first person");
        return orig;
    }
    const int mode = s_mode.load();
    if (s_logCount.fetch_add(1) < 5) {
        NOBLE_INFO("CameraThirdPerson: camera check fired (player {}, mode {}, original {})", playerIndex, mode, orig);
    }
    if (mode == 1) return false;              // experiment: maybe true means "first person"
    if (mode == 2) return orig || true;       // same as true, kept for completeness
    return true;
}

} // namespace

bool CameraThirdPerson::Install(HMODULE haloreach) {
    const uintptr_t base = reinterpret_cast<uintptr_t>(haloreach);
    if (!base) return false;
    void* target = reinterpret_cast<void*>(base + kCameraModeCheckRva);
    if (std::memcmp(target, kExpectedPrologue, sizeof(kExpectedPrologue)) != 0) {
        NOBLE_WARN("CameraThirdPerson: camera check at haloreach+0x{:X} does not match the expected prologue; third "
                   "person disabled", kCameraModeCheckRva);
        return false;
    }
    if (!HookManager::InstallHook(target, reinterpret_cast<void*>(&Hook_CameraModeCheck),
                                  reinterpret_cast<void**>(&s_original),
                                  "camera_mode_check")) {
        NOBLE_WARN("CameraThirdPerson: could not install the camera-mode detour");
        return false;
    }
    NOBLE_INFO("CameraThirdPerson: third-person camera check hooked at haloreach+0x{:X}", kCameraModeCheckRva);

    void* followTarget = reinterpret_cast<void*>(base + kCameraFollowRva);
    constexpr uint8_t kFollowPrologue[8] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x18, 0x55};
    if (std::memcmp(followTarget, kFollowPrologue, sizeof(kFollowPrologue)) == 0) {
        if (HookManager::InstallHook(followTarget, reinterpret_cast<void*>(&Hook_CameraFollow),
                                     reinterpret_cast<void**>(&s_originalFollow), "camera_follow")) {
            NOBLE_INFO("CameraThirdPerson: follow hook installed at haloreach+0x{:X}", kCameraFollowRva);
        } else {
            NOBLE_WARN("CameraThirdPerson: could not install the follow hook; distance tuning disabled");
        }
    } else {
        NOBLE_WARN("CameraThirdPerson: follow function at haloreach+0x{:X} does not match the expected prologue; "
                   "distance tuning disabled", kCameraFollowRva);
    }
    return true;
}

void CameraThirdPerson::SetDistancePercent(int percent) {
    s_distancePercent.store(std::clamp(percent, 40, 400));
    NOBLE_INFO("CameraThirdPerson: follow distance {}%", s_distancePercent.load());
}

void CameraThirdPerson::SetMode(int mode) {
    s_mode.store(mode);
    NOBLE_INFO("CameraThirdPerson: experiment mode {}", mode);
}

void CameraThirdPerson::SetEnabled(bool enabled) {
    const bool before = s_enabled.exchange(enabled);
    if (before != enabled) NOBLE_INFO("CameraThirdPerson: third person {}", enabled ? "on" : "off");
}

bool CameraThirdPerson::IsEnabled() { return s_enabled.load(); }

void CameraThirdPerson::Toggle() { SetEnabled(!s_enabled.load()); }

void CameraThirdPerson::PollHotkey(bool f5Down) {
    const bool was = s_f5WasDown.exchange(f5Down);
    if (f5Down && !was) Toggle();
}

} // namespace noble
