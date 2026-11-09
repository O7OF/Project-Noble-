#include "UiPad.hpp"

#include "common/Logger.hpp"
#include "host/Window.hpp"

#include <imgui.h>
#include <windows.h>
#include <shellapi.h>

#include <algorithm>
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

bool Up() { return ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp); }
bool Down() { return ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown); }
bool Left() { return ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickLeft); }
bool Right() { return ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickRight); }
bool PageUp() { return ImGui::IsKeyPressed(ImGuiKey_GamepadL2); }
bool PageDown() { return ImGui::IsKeyPressed(ImGuiKey_GamepadR2); }

void ScrollWindow(bool arrows, bool triggers) {
    if (ImGui::GetScrollMaxY() <= 0.0f) return;
    float delta = 0.0f;
    const float speed = ImGui::GetFontSize() * 45.0f * ImGui::GetIO().DeltaTime;
    if (ImGui::IsKeyDown(ImGuiKey_GamepadRStickDown)) delta += speed;
    if (ImGui::IsKeyDown(ImGuiKey_GamepadRStickUp)) delta -= speed;
    if (arrows) {
        if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyDown(ImGuiKey_GamepadLStickDown)) delta += speed;
        if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyDown(ImGuiKey_GamepadLStickUp)) delta -= speed;
    }
    if (triggers) {
        const float page = ImGui::GetWindowHeight() * 0.8f;
        if (PageDown()) delta += page;
        if (PageUp()) delta -= page;
    }
    if (delta != 0.0f) ImGui::SetScrollY(std::clamp(ImGui::GetScrollY() + delta, 0.0f, ImGui::GetScrollMaxY()));
}

void Feed(ImGuiIO& io) {
    // Every Noble screen reads the pad itself. ImGui's own pad navigation stays on only inside a popup, whose plain
    // widgets need it: anywhere else its hidden cursor could press a second button on one A (the lobby's COPY INVITE).
    if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    else io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;

    PadState p;
    bool connected = ScriptState(p);
    if (!connected) connected = ReadPhysical(p) && ProcessInFront();
    if (connected) io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    else {
        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
        p = PadState{};
    }

    struct Digital { ImGuiKey key; uint16_t bit; };
    static const Digital kDigital[] = {
        { ImGuiKey_GamepadStart, kStart }, { ImGuiKey_GamepadBack, kBack },
        { ImGuiKey_GamepadFaceDown, kA }, { ImGuiKey_GamepadFaceRight, kB }, { ImGuiKey_GamepadFaceLeft, kX }, { ImGuiKey_GamepadFaceUp, kY },
        { ImGuiKey_GamepadDpadLeft, kLeft }, { ImGuiKey_GamepadDpadRight, kRight }, { ImGuiKey_GamepadDpadUp, kUp }, { ImGuiKey_GamepadDpadDown, kDown },
        { ImGuiKey_GamepadL1, kLB }, { ImGuiKey_GamepadR1, kRB }, { ImGuiKey_GamepadL3, kL3 }, { ImGuiKey_GamepadR3, kR3 },
    };
    static uint16_t s_buttons = 0;
    for (const Digital& d : kDigital) {
        const bool down = (p.buttons & d.bit) != 0;
        if (down != ((s_buttons & d.bit) != 0)) io.AddKeyEvent(d.key, down);
    }
    s_buttons = p.buttons;

    // Sticks and triggers count as pressed past about half way, so a resting or worn stick never navigates.
    struct Analog { ImGuiKey key; int value; int threshold; bool& was; };
    static bool s_was[10] = {};
    const Analog analog[] = {
        { ImGuiKey_GamepadL2, p.lt, 128, s_was[0] }, { ImGuiKey_GamepadR2, p.rt, 128, s_was[1] },
        { ImGuiKey_GamepadLStickLeft, -p.lx, 16000, s_was[2] }, { ImGuiKey_GamepadLStickRight, p.lx, 16000, s_was[3] },
        { ImGuiKey_GamepadLStickUp, p.ly, 16000, s_was[4] }, { ImGuiKey_GamepadLStickDown, -p.ly, 16000, s_was[5] },
        { ImGuiKey_GamepadRStickLeft, -p.rx, 16000, s_was[6] }, { ImGuiKey_GamepadRStickRight, p.rx, 16000, s_was[7] },
        { ImGuiKey_GamepadRStickUp, p.ry, 16000, s_was[8] }, { ImGuiKey_GamepadRStickDown, -p.ry, 16000, s_was[9] },
    };
    for (const Analog& a : analog) {
        const bool down = a.value >= a.threshold;
        if (down != a.was) {
            io.AddKeyAnalogEvent(a.key, down, down ? 1.0f : 0.0f);
            a.was = down;
        }
    }
}

} // namespace UiPad
} // namespace noble
