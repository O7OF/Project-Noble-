#include "CameraThirdPerson.hpp"

#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "hooks/HookManager.hpp"
#include "network/NetworkTransport.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace noble {

namespace {

// haloreach+0x48A910: per-player camera-mode check. Verified prologue (MCC build 2025.08.16):
//   48 89 5C 24 08   mov qword ptr [rsp+8], rbx
//   48 89 74 24 10   mov qword ptr [rsp+10h], rsi
// Reach Renaissance hooks the same function (5-byte patch window) and answers it from its own camera state.
constexpr uintptr_t kCameraModeCheckRva = 0x48A910;
// haloreach+0x1DCADC (c_orbiting_camera::update) and haloreach+0x1E2028 (c_following_camera::update):
// both take (void* camera, int32_t userIndex, float dt, void* observerCommand) and write the third-person
// field of view in radians (from haloreach+0xC8640) to [observerCommand + 0x28].
constexpr uintptr_t kCameraFollowRva = 0x1DCADC;
constexpr uintptr_t kCameraFollowingUpdateRva = 0x1E2028;
constexpr uint8_t kExpectedPrologue[9] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24};

using CameraModeCheckFn = bool(__fastcall*)(int32_t playerIndex);
CameraModeCheckFn s_original = nullptr;

std::atomic<bool> s_enabled{false};
std::atomic<int> s_distancePercent{100};   // 100 = engine default; >100 pulls the camera further back
std::atomic<int> s_followLogCount{0};
std::atomic<int> s_mode{0};   // experiment: 0 = answer true, 1 = answer false, 2 = original(...) | enabled
std::atomic<bool> s_f5WasDown{false};
std::atomic<int> s_logCount{0};
std::atomic<bool> s_engineThirdPerson{false};
std::atomic<bool> s_zoomHeld{false};
std::atomic<float> s_zoomBlend{0.0f};

using CameraUpdateFn = void(__fastcall*)(void* camera, int32_t userIndex, float dt, void* observerCommand);
CameraUpdateFn s_originalFollow = nullptr;
CameraUpdateFn s_originalFollowingUpdate = nullptr;

void ApplyThirdPersonCameraTuning(float dt, void* observerCommand) {
    if (!observerCommand) return;
    const bool active = s_enabled.load() || s_engineThirdPerson.load();
    const bool wantZoom = active && s_zoomHeld.load();
    const float step = std::clamp(dt > 0.0f && dt < 0.25f ? dt : (1.0f / 60.0f), 0.001f, 0.1f) * 10.0f;
    float blend = s_zoomBlend.load();
    if (wantZoom) {
        blend = std::min(1.0f, blend + step);
    } else {
        blend = std::max(0.0f, blend - step);
    }
    s_zoomBlend.store(blend);

    if (!active) return;
    auto* fovRad = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x28);
    const int percent = s_distancePercent.load();
    if (percent != 100) {
        *fovRad *= static_cast<float>(percent) / 100.0f;
        if (s_followLogCount.fetch_add(1) < 3) {
            NOBLE_INFO("CameraThirdPerson: follow FOV scaled {}% -> {:.3f}", percent, *fovRad);
        }
    }
    if (blend > 0.001f) {
        const float scale = CameraThirdPerson::EffectiveFovScale();
        *fovRad = std::clamp(*fovRad * scale, 0.15f, 2.5f);
    }
}

void __fastcall Hook_CameraFollow(void* camera, int32_t userIndex, float dt, void* observerCommand) {
    if (s_originalFollow) s_originalFollow(camera, userIndex, dt, observerCommand);
    ApplyThirdPersonCameraTuning(dt, observerCommand);
}

void __fastcall Hook_CameraFollowingUpdate(void* camera, int32_t userIndex, float dt, void* observerCommand) {
    if (s_originalFollowingUpdate) s_originalFollowingUpdate(camera, userIndex, dt, observerCommand);
    ApplyThirdPersonCameraTuning(dt, observerCommand);
}

bool __fastcall Hook_CameraModeCheck(int32_t playerIndex) {
    const bool orig = s_original ? s_original(playerIndex) : false;
    if (playerIndex == 0) s_engineThirdPerson.store(orig);
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

    void* followingUpdateTarget = reinterpret_cast<void*>(base + kCameraFollowingUpdateRva);
    if (std::memcmp(followingUpdateTarget, kFollowPrologue, sizeof(kFollowPrologue)) == 0) {
        if (HookManager::InstallHook(followingUpdateTarget, reinterpret_cast<void*>(&Hook_CameraFollowingUpdate),
                                     reinterpret_cast<void**>(&s_originalFollowingUpdate), "camera_following_update")) {
            NOBLE_INFO("CameraThirdPerson: following-camera update hook installed at haloreach+0x{:X}", kCameraFollowingUpdateRva);
        } else {
            NOBLE_WARN("CameraThirdPerson: could not install following-camera update hook at haloreach+0x{:X}", kCameraFollowingUpdateRva);
        }
    } else {
        NOBLE_WARN("CameraThirdPerson: following-camera update at haloreach+0x{:X} does not match expected prologue", kCameraFollowingUpdateRva);
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
    if (!enabled) {
        s_zoomHeld.store(false);
        s_zoomBlend.store(0.0f);
    }
    if (before != enabled) NOBLE_INFO("CameraThirdPerson: third person {}", enabled ? "on" : "off");
}

bool CameraThirdPerson::IsEnabled() { return s_enabled.load(); }
bool CameraThirdPerson::EngineWantsThirdPerson() { return s_engineThirdPerson.load(); }

void CameraThirdPerson::Toggle() { SetEnabled(!s_enabled.load()); }

void CameraThirdPerson::PollHotkey(bool f5Down) {
    const bool was = s_f5WasDown.exchange(f5Down);
    if (f5Down && !was) Toggle();
}

void CameraThirdPerson::SetZoomHeld(bool held) {
    s_zoomHeld.store(held);
}

bool CameraThirdPerson::IsZoomHeld() {
    return s_zoomHeld.load();
}

float CameraThirdPerson::EffectiveFovScale() {
    if (!s_enabled.load() && !s_engineThirdPerson.load()) return 1.0f;
    const float blend = std::clamp(s_zoomBlend.load(), 0.0f, 1.0f);
    if (blend <= 0.001f) return 1.0f;
    const float zoomPercent = std::clamp(Settings::Get().Snapshot().thirdPersonZoom, 0.10f, 1.00f);
    // 50% zoom -> 0.65x FOV, 100% zoom -> 0.30x FOV, 10% zoom -> 0.93x FOV.
    const float targetScale = 1.0f - (zoomPercent * 0.70f);
    return std::clamp(1.0f + (targetScale - 1.0f) * blend, 0.25f, 1.0f);
}

} // namespace noble
