#include "GamepadUi.hpp"

#include <Windows.h>
#include <imgui.h>

#include <cstdint>

namespace noble {
namespace {

struct XInputGamepad {
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
};
struct XInputState {
    DWORD dwPacketNumber;
    XInputGamepad Gamepad;
};
using XInputGetStateFn = DWORD(WINAPI*)(DWORD, XInputState*);

XInputGetStateFn LoadGetState() {
    static XInputGetStateFn fn = nullptr;
    static bool tried = false;
    if (!tried) {
        tried = true;
        HMODULE module = LoadLibraryW(L"xinput1_4.dll");
        if (!module) module = LoadLibraryW(L"xinput9_1_0.dll");
        if (module) fn = reinterpret_cast<XInputGetStateFn>(GetProcAddress(module, "XInputGetState"));
    }
    return fn;
}

constexpr WORD kUp = 0x0001, kDown = 0x0002, kLeft = 0x0004, kRight = 0x0008;
constexpr WORD kStart = 0x0010, kBack = 0x0020;
constexpr WORD kLB = 0x0100, kRB = 0x0200;
constexpr WORD kA = 0x1000, kB = 0x2000, kX = 0x4000, kY = 0x8000;
constexpr BYTE kTriggerDown = 96;
constexpr SHORT kStickDown = 12000;  // ~0.37 of the 32767 range: comfortable for menu navigation

const ImGuiKey kAllKeys[] = {
    ImGuiKey_GamepadDpadUp,      ImGuiKey_GamepadDpadDown,     ImGuiKey_GamepadDpadLeft,
    ImGuiKey_GamepadDpadRight,   ImGuiKey_GamepadFaceUp,       ImGuiKey_GamepadFaceDown,
    ImGuiKey_GamepadFaceLeft,    ImGuiKey_GamepadFaceRight,    ImGuiKey_GamepadStart,
    ImGuiKey_GamepadBack,        ImGuiKey_GamepadL1,           ImGuiKey_GamepadR1,
    ImGuiKey_GamepadL2,          ImGuiKey_GamepadR2,           ImGuiKey_GamepadLStickUp,
    ImGuiKey_GamepadLStickDown,  ImGuiKey_GamepadLStickLeft,   ImGuiKey_GamepadLStickRight,
    ImGuiKey_GamepadRStickUp,    ImGuiKey_GamepadRStickDown,   ImGuiKey_GamepadRStickLeft,
    ImGuiKey_GamepadRStickRight,
};

void SendKey(ImGuiKey key, bool down) { ImGui::GetIO().AddKeyEvent(key, down); }

}  // namespace

void FeedGamepadToImGui() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    static bool s_hadPad = false;
    XInputState state{};
    const auto getState = LoadGetState();
    const bool connected = getState && getState(0, &state) == ERROR_SUCCESS;
    if (!connected) {
        if (s_hadPad) {
            s_hadPad = false;
            for (ImGuiKey key : kAllKeys) SendKey(key, false);
        }
        return;
    }
    s_hadPad = true;

    const WORD buttons = state.Gamepad.wButtons;
    SendKey(ImGuiKey_GamepadDpadUp, (buttons & kUp) != 0);
    SendKey(ImGuiKey_GamepadDpadDown, (buttons & kDown) != 0);
    SendKey(ImGuiKey_GamepadDpadLeft, (buttons & kLeft) != 0);
    SendKey(ImGuiKey_GamepadDpadRight, (buttons & kRight) != 0);
    SendKey(ImGuiKey_GamepadFaceDown, (buttons & kA) != 0);
    SendKey(ImGuiKey_GamepadFaceRight, (buttons & kB) != 0);
    SendKey(ImGuiKey_GamepadFaceLeft, (buttons & kX) != 0);
    SendKey(ImGuiKey_GamepadFaceUp, (buttons & kY) != 0);
    SendKey(ImGuiKey_GamepadStart, (buttons & kStart) != 0);
    SendKey(ImGuiKey_GamepadBack, (buttons & kBack) != 0);
    SendKey(ImGuiKey_GamepadL1, (buttons & kLB) != 0);
    SendKey(ImGuiKey_GamepadR1, (buttons & kRB) != 0);
    SendKey(ImGuiKey_GamepadL2, state.Gamepad.bLeftTrigger >= kTriggerDown);
    SendKey(ImGuiKey_GamepadR2, state.Gamepad.bRightTrigger >= kTriggerDown);

    // Left stick drives navigation (SettingsPanel already reads the ImGui gamepad keys); the right stick is fed too so
    // future pages can use it without another pass.
    SendKey(ImGuiKey_GamepadLStickUp, state.Gamepad.sThumbLY > kStickDown);
    SendKey(ImGuiKey_GamepadLStickDown, state.Gamepad.sThumbLY < -kStickDown);
    SendKey(ImGuiKey_GamepadLStickLeft, state.Gamepad.sThumbLX < -kStickDown);
    SendKey(ImGuiKey_GamepadLStickRight, state.Gamepad.sThumbLX > kStickDown);
    SendKey(ImGuiKey_GamepadRStickUp, state.Gamepad.sThumbRY > kStickDown);
    SendKey(ImGuiKey_GamepadRStickDown, state.Gamepad.sThumbRY < -kStickDown);
    SendKey(ImGuiKey_GamepadRStickLeft, state.Gamepad.sThumbRX < -kStickDown);
    SendKey(ImGuiKey_GamepadRStickRight, state.Gamepad.sThumbRX > kStickDown);
}

}  // namespace noble
