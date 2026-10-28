#include "CameraFov.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <atomic>
#include <shellapi.h>
#include <cmath>

namespace noble {

namespace {

// The engine-side value lives in haloreach.dll's .data, which is writable already; VirtualProtect is
// only used so a build with a read-only section does not fault (the address is fixed per build).
std::atomic<uintptr_t> s_base{0};
std::atomic<float> s_applied{0.0f};      // last value written this session
std::atomic<uint64_t> s_lastWriteMs{0};
std::atomic<uint64_t> s_lastPollMs{0};

constexpr uint64_t kPollMs = 250;       // how often the settings are re-read
constexpr uint64_t kReassertMs = 2000;  // how often the write is repeated while the game runs

float ClampDegrees(float degrees) {
    if (!(degrees >= CameraFov::kMinDegrees)) return CameraFov::kMinDegrees;  // also catches NaN
    if (degrees > CameraFov::kMaxDegrees) return CameraFov::kMaxDegrees;
    return degrees;
}

} // namespace

void CameraFov::SetEngineModule(HMODULE module) {
    s_base.store(reinterpret_cast<uintptr_t>(module));
    s_applied.store(0.0f);
    s_lastWriteMs.store(0);
}

bool CameraFov::Apply(float degrees) {
    const uintptr_t base = s_base.load();
    if (base == 0) return false;

    const float value = ClampDegrees(degrees);
    void* const address = reinterpret_cast<void*>(base + kFovRva);

    DWORD oldProtect = 0;
    if (!VirtualProtect(address, sizeof(float), PAGE_READWRITE, &oldProtect)) {
        NOBLE_WARN("CameraFov: could not make haloreach+0x{:X} writable (error {})", kFovRva, GetLastError());
        return false;
    }
    bool written = false;
    __try {
        *reinterpret_cast<float*>(address) = value;
        written = true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        written = false;
    }
    DWORD ignored = 0;
    VirtualProtect(address, sizeof(float), oldProtect, &ignored);
    if (!written) {
        NOBLE_WARN("CameraFov: write to haloreach+0x{:X} faulted", kFovRva);
        return false;
    }

    s_applied.store(value);
    s_lastWriteMs.store(GetTickCount64());
    return true;
}

bool CameraFov::Read(float& degrees) {
    const uintptr_t base = s_base.load();
    if (base == 0) return false;
    __try {
        degrees = *reinterpret_cast<const float*>(base + kFovRva);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Test option "-fov-test-at <seconds> <degrees>": that many seconds after the first engine frame,
// change the saved FOV setting the way SETTINGS > VIDEO > APPLY does, so the live path can be checked
// with -capture-frames (the player stands still, so shots before and after differ only in FOV).
// Note: like APPLY, it saves settings.json.
static void FovTestTick(uint64_t now) {
    static bool s_parsed = false, s_done = false;
    static double s_at = 0.0;
    static float s_degrees = 0.0f;
    static uint64_t s_start = 0;
    if (!s_parsed) {
        s_parsed = true;
        s_start = now;
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 2 < argc; ++i) {
            if (_wcsicmp(argv[i], L"-fov-test-at") == 0) {
                s_at = _wtof(argv[i + 1]);
                s_degrees = static_cast<float>(_wtof(argv[i + 2]));
            }
        }
        if (argv) LocalFree(argv);
        if (s_at <= 0.0) s_done = true;
    }
    if (s_done || now - s_start < static_cast<uint64_t>(s_at * 1000.0)) return;
    s_done = true;
    SettingsData d = Settings::Get().Snapshot();
    d.fieldOfView = s_degrees;
    Settings::Get().Update(d);
    NOBLE_INFO("CameraFov: -fov-test-at: setting changed to {:.0f} degrees", s_degrees);
}

void CameraFov::Tick() {
    if (s_base.load() == 0) return;

    const uint64_t now = GetTickCount64();
    FovTestTick(now);
    if (now - s_lastPollMs.load() < kPollMs) return;
    s_lastPollMs.store(now);

    const float wanted = ClampDegrees(Settings::Get().Snapshot().fieldOfView);
    const bool changed = std::fabs(wanted - s_applied.load()) > 0.01f;
    if (!changed && now - s_lastWriteMs.load() < kReassertMs) return;

    if (Apply(wanted) && changed) {
        NOBLE_INFO("CameraFov: game field of view set to {:.0f} degrees (haloreach+0x{:X})", wanted, kFovRva);
    }
}

} // namespace noble
