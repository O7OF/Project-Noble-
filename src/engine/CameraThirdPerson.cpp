#include "CameraThirdPerson.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

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
constexpr uint8_t kExpectedPrologue[9] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24};

using CameraModeCheckFn = bool(__fastcall*)(int32_t playerIndex);
CameraModeCheckFn s_original = nullptr;

std::atomic<bool> s_enabled{false};
std::atomic<int> s_mode{0};   // experiment: 0 = answer true, 1 = answer false, 2 = original(...) | enabled
std::atomic<bool> s_f5WasDown{false};
std::atomic<int> s_logCount{0};

bool __fastcall Hook_CameraModeCheck(int32_t playerIndex) {
    const bool orig = s_original ? s_original(playerIndex) : false;
    if (!s_enabled.load()) return orig;
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
    return true;
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
