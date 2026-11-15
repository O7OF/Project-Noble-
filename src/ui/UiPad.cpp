#include "UiPad.hpp"

#include "NobleUi.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "host/Window.hpp"

#include <imgui.h>
#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace noble {
namespace UiPad {

namespace {

struct PadState {
    uint32_t packet = 0;
    uint16_t buttons = 0;
    uint8_t lt = 0, rt = 0;
    int16_t lx = 0, ly = 0, rx = 0, ry = 0;
};
using GetStateFn = DWORD(WINAPI*)(DWORD, PadState*);

GetStateFn XInput() {
    static GetStateFn s_fn = [] {
        HMODULE m = LoadLibraryW(L"xinput1_4.dll");
        if (!m) m = LoadLibraryW(L"xinput9_1_0.dll");
        return m ? reinterpret_cast<GetStateFn>(GetProcAddress(m, "XInputGetState")) : nullptr;
    }();
    return s_fn;
}

bool ReadPhysical(PadState& out) {
    const GetStateFn get = XInput();
    if (!get) return false;
    // A missing pad makes XInputGetState slow: while none is connected, look again only every 2 s.
    static int s_slot = -1;
    static ULONGLONG s_nextScan = 0;
    if (s_slot >= 0 && get(static_cast<DWORD>(s_slot), &out) == ERROR_SUCCESS) return true;
    s_slot = -1;
    const ULONGLONG now = GetTickCount64();
    if (now < s_nextScan) return false;
    s_nextScan = now + 2000;
    for (DWORD i = 0; i < 4; ++i) {
        if (get(i, &out) == ERROR_SUCCESS) {
            s_slot = static_cast<int>(i);
            return true;
        }
    }
    return false;
}

// Only the focused window listens, so a second Noble (or a game in front) doesn't move this menu.
// In container window mode the foreground window belongs to the launcher process, which
// Window::IsForeground() counts as ours.
bool ProcessInFront() {
    if (Window::IsForeground(Window::ActiveHwnd())) return true;
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId();
}

constexpr uint16_t kUp = 0x0001, kDown = 0x0002, kLeft = 0x0004, kRight = 0x0008, kStart = 0x0010, kBack = 0x0020;
constexpr uint16_t kL3 = 0x0040, kR3 = 0x0080, kLB = 0x0100, kRB = 0x0200, kA = 0x1000, kB = 0x2000, kX = 0x4000, kY = 0x8000;

void ApplyToken(const std::string& t, PadState& p) {
    if (t == "a") p.buttons = kA;
    else if (t == "b") p.buttons = kB;
    else if (t == "x") p.buttons = kX;
    else if (t == "y") p.buttons = kY;
    else if (t == "lb") p.buttons = kLB;
    else if (t == "rb") p.buttons = kRB;
    else if (t == "up") p.buttons = kUp;
    else if (t == "down") p.buttons = kDown;
    else if (t == "left") p.buttons = kLeft;
    else if (t == "right") p.buttons = kRight;
    else if (t == "start") p.buttons = kStart;
    else if (t == "back") p.buttons = kBack;
    else if (t == "lt") p.lt = 255;
    else if (t == "rt") p.rt = 255;
    else if (t == "lsup") p.ly = 32767;
    else if (t == "lsdown") p.ly = -32768;
    else if (t == "lsleft") p.lx = -32768;
    else if (t == "lsright") p.lx = 32767;
    else if (t == "rsup") p.ry = 32767;
    else if (t == "rsdown") p.ry = -32768;
    else if (t == "rsleft") p.rx = -32768;
    else if (t == "rsright") p.rx = 32767;
}

// The -pad-script steps; empty without the option.
const std::vector<std::string>& Script() {
    static const std::vector<std::string> s_steps = [] {
        std::vector<std::string> steps;
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (_wcsicmp(argv[i], L"-pad-script") != 0) continue;
            std::string current;
            for (const wchar_t* c = argv[i + 1]; *c; ++c) {
                if (*c == L',') {
                    steps.push_back(current);
                    current.clear();
                } else {
                    current += static_cast<char>(*c);
                }
            }
            if (!current.empty()) steps.push_back(current);
        }
        if (argv) LocalFree(argv);
        return steps;
    }();
    return s_steps;
}

// True while the script is in charge of the pad; fills `out` with this moment's press.
bool ScriptState(PadState& out) {
    const std::vector<std::string>& steps = Script();
    if (steps.empty()) return false;
    constexpr ULONGLONG kLeadIn = 3000, kStep = 500, kHeld = 150;
    static const ULONGLONG s_start = GetTickCount64();
    const ULONGLONG t = GetTickCount64() - s_start;
    out = PadState{};
    if (t < kLeadIn) return true;
    const size_t index = static_cast<size_t>((t - kLeadIn) / kStep);
    if (index >= steps.size()) return true;
    static size_t s_logged = static_cast<size_t>(-1);
    if (s_logged != index) {
        s_logged = index;
        NOBLE_INFO("UiPad: -pad-script step {} = {}", index, steps[index]);
    }
    // Every step is seen for at least one frame, even if a slow frame (a page loading its art) outlasts the press.
    static size_t s_pressed = static_cast<size_t>(-1);
    if ((t - kLeadIn) % kStep < kHeld || s_pressed != index) ApplyToken(steps[index], out);
    s_pressed = index;
    return true;
}

} // namespace

namespace {
bool s_acceptConsumed = false;
bool s_backConsumed = false;
bool s_xConsumed = false;
bool s_yConsumed = false;
bool s_tabConsumed = false;
float s_rStickY = 0.0f;
float s_lStickY = 0.0f;
uint16_t s_buttons = 0;
bool s_was[10] = {};
bool s_needInputResync = false;
ULONGLONG s_postMatchSettleUntil = 0;

bool s_vcEnabled = false;
bool s_vcSettingsSynced = false;
bool s_vcConnected = false;
bool s_vcMenuActive = false;
bool s_vcControllingMouse = false;
bool s_vcInputFrame = false;
bool s_vcPosInit = false;
bool s_vcLmbHeld = false;
float s_vcX = 0.0f;
float s_vcY = 0.0f;
ULONGLONG s_vcToastUntil = 0;
std::string s_vcToastText;
POINT s_lastPhysCursor{-99999, -99999};
} // namespace

bool Up() { return !IsVirtualCursorActive() && (ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp)); }
bool Down() { return !IsVirtualCursorActive() && (ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown)); }
bool Left() { return !IsVirtualCursorActive() && (ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickLeft)); }
bool Right() { return !IsVirtualCursorActive() && (ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickRight)); }
bool PageUp() { return ImGui::IsKeyPressed(ImGuiKey_GamepadL2); }
bool PageDown() { return ImGui::IsKeyPressed(ImGuiKey_GamepadR2); }

bool AcceptPressed() {
    if (s_acceptConsumed || ImGui::GetIO().WantTextInput) return false;
    return ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false) ||
           ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
           ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false);
}

bool BackPressed() {
    if (s_backConsumed) return false;
    return ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false) ||
           ImGui::IsKeyPressed(ImGuiKey_Escape, false);
}

bool ActionX() {
    if (s_xConsumed || ImGui::GetIO().WantTextInput) return false;
    return ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft, false);
}

bool ActionY() {
    if (s_yConsumed || ImGui::GetIO().WantTextInput) return false;
    return ImGui::IsKeyPressed(ImGuiKey_GamepadFaceUp, false);
}

bool TabPrev() {
    if (s_tabConsumed || ImGui::GetIO().WantTextInput) return false;
    return ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false) || ImGui::IsKeyPressed(ImGuiKey_Q, false);
}

bool TabNext() {
    if (s_tabConsumed || ImGui::GetIO().WantTextInput) return false;
    return ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false) || ImGui::IsKeyPressed(ImGuiKey_E, false);
}

void ConsumeAccept() { s_acceptConsumed = true; }
void ConsumeBack() { s_backConsumed = true; }
void ConsumeAll() {
    s_acceptConsumed = true;
    s_backConsumed = true;
    s_xConsumed = true;
    s_yConsumed = true;
    s_tabConsumed = true;
}

void ResetAfterMatch() {
    ConsumeAll();
    s_needInputResync = true;
    s_postMatchSettleUntil = GetTickCount64() + 250;
    s_vcLmbHeld = false;
    s_vcInputFrame = false;
    s_vcControllingMouse = false;
    s_lastPhysCursor = POINT{-99999, -99999};
}

void SetVirtualCursorEnabled(bool on) {
    s_vcEnabled = on;
    s_vcSettingsSynced = true;
    if (!on) {
        if (s_vcLmbHeld && ImGui::GetCurrentContext()) {
            ImGui::GetIO().AddMouseButtonEvent(0, false);
        }
        s_vcLmbHeld = false;
        s_vcControllingMouse = false;
        s_vcInputFrame = false;
    } else if (ImGui::GetCurrentContext()) {
        const ImGuiIO& io = ImGui::GetIO();
        if (!s_vcPosInit && io.DisplaySize.x > 0.0f && io.DisplaySize.y > 0.0f) {
            s_vcX = io.DisplaySize.x * 0.5f;
            s_vcY = io.DisplaySize.y * 0.5f;
            s_vcPosInit = true;
        }
        s_vcControllingMouse = true;
    }
}

bool IsVirtualCursorEnabled() { return s_vcEnabled; }
bool IsVirtualCursorActive() { return s_vcMenuActive && s_vcEnabled && s_vcConnected; }
bool IsVirtualCursorInputFrame() { return s_vcInputFrame; }

bool HasGamepad() {
    return (ImGui::GetIO().BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
}

float AnalogScrollY(bool includeLeftStick) {
    // When Virtual Cursor controls mouse+wheel, right stick already drives ImGui MouseWheel cleanly;
    // suppress analog scroll deltas here so hovered containers do not double-scroll.
    if (IsVirtualCursorActive() && s_vcControllingMouse) return 0.0f;
    const float dt = std::min(ImGui::GetIO().DeltaTime, 0.05f);
    const float speed = 520.0f * dt;
    float delta = -s_rStickY * speed;
    if (includeLeftStick) delta -= s_lStickY * speed;
    if (delta == 0.0f) {
        if (ImGui::IsKeyDown(ImGuiKey_GamepadRStickDown)) delta += speed;
        if (ImGui::IsKeyDown(ImGuiKey_GamepadRStickUp)) delta -= speed;
        if (includeLeftStick) {
            if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyDown(ImGuiKey_GamepadLStickDown)) delta += speed;
            if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyDown(ImGuiKey_GamepadLStickUp)) delta -= speed;
        }
    }
    return delta;
}

void ScrollWindow(bool arrows, bool triggers) {
    if (ImGui::GetScrollMaxY() <= 0.0f) return;
    float delta = 0.0f;
    if (!(IsVirtualCursorActive() && s_vcControllingMouse)) {
        const float speed = ImGui::GetFontSize() * 45.0f * ImGui::GetIO().DeltaTime;
        if (ImGui::IsKeyDown(ImGuiKey_GamepadRStickDown)) delta += speed;
        if (ImGui::IsKeyDown(ImGuiKey_GamepadRStickUp)) delta -= speed;
        if (arrows) {
            if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyDown(ImGuiKey_GamepadLStickDown)) delta += speed;
            if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyDown(ImGuiKey_GamepadLStickUp)) delta -= speed;
        }
    }
    if (triggers) {
        const float page = ImGui::GetWindowHeight() * 0.8f;
        if (PageDown()) delta += page;
        if (PageUp()) delta -= page;
    }
    if (delta != 0.0f) ImGui::SetScrollY(std::clamp(ImGui::GetScrollY() + delta, 0.0f, ImGui::GetScrollMaxY()));
}

void Feed(ImGuiIO& io, bool menuActive) {
    s_vcMenuActive = menuActive;
    s_vcInputFrame = false;
    if (!s_vcSettingsSynced) {
        s_vcSettingsSynced = true;
        s_vcEnabled = Settings::Get().Snapshot().padVirtualCursor;
    }

    s_acceptConsumed = false;
    s_backConsumed = false;
    s_xConsumed = false;
    s_yConsumed = false;
    s_tabConsumed = false;
    bool settling = false;
    if (s_postMatchSettleUntil != 0) {
        if (GetTickCount64() < s_postMatchSettleUntil) {
            ConsumeAll();
            settling = true;
        } else {
            s_postMatchSettleUntil = 0;
        }
    }

    // Every Noble screen reads the pad itself. ImGui's own pad navigation stays on only inside a popup, whose plain
    // widgets need it: anywhere else its hidden cursor could press a second button on one A (the lobby's COPY INVITE).
    if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    else io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;

    PadState p;
    bool connected = ScriptState(p);
    if (!connected) connected = ReadPhysical(p) && ProcessInFront();
    s_vcConnected = connected;
    if (connected) io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    else {
        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
        p = PadState{};
    }

    auto normStick = [](int v) -> float {
        const float dead = 8000.0f;
        const float mag = static_cast<float>(std::abs(v));
        if (mag <= dead) return 0.0f;
        const float s = (mag - dead) / (32767.0f - dead);
        return (v < 0 ? -1.0f : 1.0f) * std::clamp(s, 0.0f, 1.0f);
    };
    s_rStickY = normStick(p.ry);
    s_lStickY = normStick(p.ly);

    struct Digital { ImGuiKey key; uint16_t bit; };
    static const Digital kDigital[] = {
        { ImGuiKey_GamepadStart, kStart }, { ImGuiKey_GamepadBack, kBack },
        { ImGuiKey_GamepadFaceDown, kA }, { ImGuiKey_GamepadFaceRight, kB }, { ImGuiKey_GamepadFaceLeft, kX }, { ImGuiKey_GamepadFaceUp, kY },
        { ImGuiKey_GamepadDpadLeft, kLeft }, { ImGuiKey_GamepadDpadRight, kRight }, { ImGuiKey_GamepadDpadUp, kUp }, { ImGuiKey_GamepadDpadDown, kDown },
        { ImGuiKey_GamepadL1, kLB }, { ImGuiKey_GamepadR1, kRB }, { ImGuiKey_GamepadL3, kL3 }, { ImGuiKey_GamepadR3, kR3 },
    };
    const bool resync = s_needInputResync;
    s_needInputResync = false;

    if (menuActive && connected && !resync && !settling && !io.WantTextInput &&
        (p.buttons & kL3) != 0 && (s_buttons & kL3) == 0) {
        const bool nextState = !s_vcEnabled;
        SetVirtualCursorEnabled(nextState);
        SettingsData d = Settings::Get().Snapshot();
        if (d.padVirtualCursor != nextState) {
            d.padVirtualCursor = nextState;
            Settings::Get().Update(d);
        }
        s_vcToastText = nextState ? "CONTROLLER CURSOR: ON  [L3 TOGGLE]"
                                  : "CONTROLLER CURSOR: OFF  [L3 TOGGLE]";
        s_vcToastUntil = GetTickCount64() + 2400;
        if (nextState && io.DisplaySize.x > 0.0f && io.DisplaySize.y > 0.0f) {
            POINT pt{};
            RECT rc{};
            HWND hwnd = Window::ActiveHwnd();
            if (hwnd && GetCursorPos(&pt) && ScreenToClient(hwnd, &pt) && GetClientRect(hwnd, &rc) &&
                rc.right > 0 && rc.bottom > 0 &&
                pt.x >= 0 && pt.x < rc.right && pt.y >= 0 && pt.y < rc.bottom) {
                s_vcX = static_cast<float>(pt.x) * (io.DisplaySize.x / static_cast<float>(rc.right));
                s_vcY = static_cast<float>(pt.y) * (io.DisplaySize.y / static_cast<float>(rc.bottom));
            } else {
                s_vcX = io.DisplaySize.x * 0.5f;
                s_vcY = io.DisplaySize.y * 0.5f;
            }
            s_vcPosInit = true;
            s_vcControllingMouse = true;
        }
    }

    if (resync) {
        // Release any stale ImGui key states and adopt current hardware state without edge-triggering.
        for (const Digital& d : kDigital) {
            io.AddKeyEvent(d.key, false);
        }
        s_buttons = p.buttons;
    } else {
        for (const Digital& d : kDigital) {
            const bool down = (p.buttons & d.bit) != 0;
            if (down != ((s_buttons & d.bit) != 0)) io.AddKeyEvent(d.key, down);
        }
        s_buttons = p.buttons;
    }

    // Virtual Cursor Mode: smooth Left Stick cursor + A click/drag + Right Stick wheel scroll.
    const bool vcEligible = menuActive && s_vcEnabled && connected && !resync;
    if (vcEligible && io.DisplaySize.x > 0.0f && io.DisplaySize.y > 0.0f) {
        POINT curPt{};
        bool physMoved = false;
        if (GetCursorPos(&curPt)) {
            if (s_lastPhysCursor.x != -99999 &&
                (std::abs(curPt.x - s_lastPhysCursor.x) + std::abs(curPt.y - s_lastPhysCursor.y) >= 6)) {
                physMoved = true;
            }
            s_lastPhysCursor = curPt;
        }

        const float nx = static_cast<float>(p.lx) / 32767.0f;
        const float ny = -static_cast<float>(p.ly) / 32767.0f;
        const float mag = std::hypot(nx, ny);
        constexpr float kCursorDeadzone = 0.20f;
        const bool stickMoved = mag > kCursorDeadzone;
        const bool aDown = !settling && !io.WantTextInput && (p.buttons & kA) != 0;
        const bool rsWheel = std::abs(s_rStickY) > 0.0f;

        if (stickMoved || aDown || rsWheel) {
            if (!s_vcControllingMouse) {
                RECT rc{};
                HWND hwnd = Window::ActiveHwnd();
                if (!s_vcPosInit && hwnd && GetClientRect(hwnd, &rc) && ScreenToClient(hwnd, &curPt) &&
                    rc.right > 0 && rc.bottom > 0 &&
                    curPt.x >= 0 && curPt.x < rc.right && curPt.y >= 0 && curPt.y < rc.bottom) {
                    s_vcX = static_cast<float>(curPt.x) * (io.DisplaySize.x / static_cast<float>(rc.right));
                    s_vcY = static_cast<float>(curPt.y) * (io.DisplaySize.y / static_cast<float>(rc.bottom));
                } else if (!s_vcPosInit) {
                    s_vcX = io.DisplaySize.x * 0.5f;
                    s_vcY = io.DisplaySize.y * 0.5f;
                }
                s_vcPosInit = true;
            }
            s_vcControllingMouse = true;
        } else if (physMoved) {
            s_vcControllingMouse = false;
            if (s_vcLmbHeld) {
                io.AddMouseButtonEvent(0, false);
                s_vcLmbHeld = false;
            }
        }

        if (s_vcControllingMouse) {
            if (!s_vcPosInit) {
                s_vcX = io.DisplaySize.x * 0.5f;
                s_vcY = io.DisplaySize.y * 0.5f;
                s_vcPosInit = true;
            }
            const float dt = std::min(io.DeltaTime > 0.0f ? io.DeltaTime : (1.0f / 60.0f), 0.05f);
            if (stickMoved) {
                const float norm = std::clamp((mag - kCursorDeadzone) / (1.0f - kCursorDeadzone), 0.0f, 1.0f);
                const float curve = std::pow(norm, 1.55f);
                const float uiScale = std::max(0.75f, io.DisplaySize.y / 900.0f);
                const float speed = 1120.0f * uiScale * curve;
                s_vcX += (nx / mag) * speed * dt;
                s_vcY += (ny / mag) * speed * dt;
            }
            s_vcX = std::clamp(s_vcX, 4.0f, std::max(4.0f, io.DisplaySize.x - 4.0f));
            s_vcY = std::clamp(s_vcY, 4.0f, std::max(4.0f, io.DisplaySize.y - 4.0f));
            io.AddMousePosEvent(s_vcX, s_vcY);

            if (aDown != s_vcLmbHeld) {
                io.AddMouseButtonEvent(0, aDown);
                s_vcLmbHeld = aDown;
            }
            if (rsWheel) {
                io.AddMouseWheelEvent(0.0f, s_rStickY * 7.5f * dt);
            }
            // In Virtual Cursor mode, A triggers a pointer click rather than digital list Accept.
            ConsumeAccept();
            s_vcInputFrame = (stickMoved || aDown || s_vcLmbHeld || rsWheel);
        }
    } else {
        if (s_vcLmbHeld) {
            io.AddMouseButtonEvent(0, false);
            s_vcLmbHeld = false;
        }
        s_vcControllingMouse = false;
    }

    // Dominant-axis cone check on cardinal stick nav: stops slightly tilted diagonal pushes from
    // firing simultaneous horizontal + vertical navigation events and jumping between panels.
    const bool vcBlocksSticks = vcEligible;
    const int lax = std::abs(static_cast<int>(p.lx));
    const int lay = std::abs(static_cast<int>(p.ly));
    const bool allowLX = !vcBlocksSticks && lax >= 18000 && lax * 100 > lay * 135;
    const bool allowLY = !vcBlocksSticks && lay >= 18000 && lay * 100 > lax * 135;
    const int rax = std::abs(static_cast<int>(p.rx));
    const int ray = std::abs(static_cast<int>(p.ry));
    const bool allowRX = !vcBlocksSticks && rax >= 18000 && rax * 100 > ray * 135;
    const bool allowRY = !vcBlocksSticks && ray >= 18000 && ray * 100 > rax * 135;

    struct Analog { ImGuiKey key; int value; int threshold; bool allowed; bool& was; };
    const Analog analog[] = {
        { ImGuiKey_GamepadL2, p.lt, 128, true, s_was[0] }, { ImGuiKey_GamepadR2, p.rt, 128, true, s_was[1] },
        { ImGuiKey_GamepadLStickLeft, -p.lx, 18000, allowLX, s_was[2] }, { ImGuiKey_GamepadLStickRight, p.lx, 18000, allowLX, s_was[3] },
        { ImGuiKey_GamepadLStickUp, p.ly, 18000, allowLY, s_was[4] }, { ImGuiKey_GamepadLStickDown, -p.ly, 18000, allowLY, s_was[5] },
        { ImGuiKey_GamepadRStickLeft, -p.rx, 18000, allowRX, s_was[6] }, { ImGuiKey_GamepadRStickRight, p.rx, 18000, allowRX, s_was[7] },
        { ImGuiKey_GamepadRStickUp, p.ry, 18000, allowRY, s_was[8] }, { ImGuiKey_GamepadRStickDown, -p.ry, 18000, allowRY, s_was[9] },
    };
    for (const Analog& a : analog) {
        const bool down = a.allowed && a.value >= a.threshold;
        if (resync) {
            io.AddKeyAnalogEvent(a.key, false, 0.0f);
            a.was = down;
        } else if (down != a.was) {
            io.AddKeyAnalogEvent(a.key, down, down ? 1.0f : 0.0f);
            a.was = down;
        }
    }
}

void DrawVirtualCursor(ImDrawList* dl) {
    if (!dl || !ImGui::GetCurrentContext()) return;
    const ImGuiIO& io = ImGui::GetIO();
    const float s = std::max(0.75f, io.DisplaySize.y / 900.0f);

    const ULONGLONG now = GetTickCount64();
    if (now < s_vcToastUntil && !s_vcToastText.empty()) {
        const float rem = static_cast<float>(s_vcToastUntil - now);
        const float alpha = std::clamp(rem / 400.0f, 0.0f, 1.0f);
        const int aBg = static_cast<int>(215.0f * alpha);
        const int aRim = static_cast<int>(220.0f * alpha);
        const int aTxt = static_cast<int>(255.0f * alpha);
        const float px = 14.0f * s;
        const float tw = NobleUi::TextWidth(nullptr, px, s_vcToastText.c_str());
        const float bw = tw + 30.0f * s;
        const float bh = 28.0f * s;
        const ImVec2 t0((io.DisplaySize.x - bw) * 0.5f, 18.0f * s);
        const ImVec2 t1(t0.x + bw, t0.y + bh);
        dl->AddRectFilled(t0, t1, IM_COL32(10, 18, 28, aBg), bh * 0.5f);
        dl->AddRect(t0, t1, s_vcEnabled ? IM_COL32(143, 216, 255, aRim) : IM_COL32(255, 186, 74, aRim), bh * 0.5f, 0, 1.4f * s);
        NobleUi::Text(dl, nullptr, px, ImVec2(t0.x + 15.0f * s, t0.y + 6.0f * s),
                      s_vcEnabled ? IM_COL32(143, 224, 255, aTxt) : IM_COL32(255, 200, 100, aTxt),
                      s_vcToastText.c_str());
    }

    if (!(s_vcMenuActive && s_vcEnabled && s_vcConnected && s_vcControllingMouse && s_vcPosInit)) return;

    const ImVec2 c(s_vcX, s_vcY);
    const float outerR = (s_vcLmbHeld ? 9.5f : 12.5f) * s;
    const float tickIn = (s_vcLmbHeld ? 4.5f : 5.5f) * s;
    const float tickOut = outerR + 4.5f * s;
    const ImU32 coreCol = s_vcLmbHeld ? IM_COL32(255, 195, 85, 255) : IM_COL32(143, 224, 255, 250);
    const ImU32 glowCol = s_vcLmbHeld ? IM_COL32(255, 180, 60, 75) : IM_COL32(100, 195, 255, 65);

    dl->AddCircleFilled(c, outerR + 2.0f * s, IM_COL32(6, 12, 20, 120), 28);
    dl->AddCircle(c, outerR + 2.5f * s, glowCol, 28, 3.0f * s);
    dl->AddCircle(c, outerR, coreCol, 28, 1.8f * s);

    const float lw = 1.8f * s;
    dl->AddLine(ImVec2(c.x - tickOut, c.y), ImVec2(c.x - tickIn, c.y), coreCol, lw);
    dl->AddLine(ImVec2(c.x + tickIn, c.y), ImVec2(c.x + tickOut, c.y), coreCol, lw);
    dl->AddLine(ImVec2(c.x, c.y - tickOut), ImVec2(c.x, c.y - tickIn), coreCol, lw);
    dl->AddLine(ImVec2(c.x, c.y + tickIn), ImVec2(c.x, c.y + tickOut), coreCol, lw);
    dl->AddCircleFilled(c, 2.5f * s, IM_COL32(255, 255, 255, 255), 16);
}

} // namespace UiPad
} // namespace noble
