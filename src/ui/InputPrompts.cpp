#include "InputPrompts.hpp"

#include <cfloat>
#include <cmath>
#include <string>

namespace noble {
namespace InputPrompts {

namespace {

Device s_device = Device::KeyboardMouse;

const ImU32 kKeyBorder = IM_COL32(200, 210, 220, 200);
const ImU32 kWhite = IM_COL32(255, 255, 255, 255);

float TextW(ImFont* font, float size, const char* text) {
    return font->CalcTextSizeA(size, FLT_MAX, 0.0f, text).x;
}

// Keyboard key: outlined box with the key name.
float Key(ImDrawList* dl, ImFont* font, ImVec2 pos, float h, const char* label) {
    const float size = h * 0.68f;
    const float w = TextW(font, size, label) + h * 0.55f;
    dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), kKeyBorder, h * 0.14f, 0, 1.2f);
    dl->AddText(font, size, ImVec2(pos.x + h * 0.275f, pos.y + (h - size) * 0.5f), kWhite, label);
    return w;
}

// Face button: filled circle in the Xbox colour with its letter.
float Face(ImDrawList* dl, ImFont* font, ImVec2 pos, float h, const char* letter, ImU32 colour) {
    const float r = h * 0.5f;
    const ImVec2 c(pos.x + r, pos.y + r);
    dl->AddCircleFilled(c, r, colour, 24);
    dl->AddCircle(c, r, IM_COL32(0, 0, 0, 90), 24, 1.0f);
    const float size = h * 0.72f;
    const float tw = TextW(font, size, letter);
    dl->AddText(font, size, ImVec2(c.x - tw * 0.5f, c.y - size * 0.5f), kWhite, letter);
    return h;
}

// Bumper: dark pill with "LB" / "RB".
float Bumper(ImDrawList* dl, ImFont* font, ImVec2 pos, float h, const char* label) {
    const float size = h * 0.6f;
    const float w = TextW(font, size, label) + h * 0.8f;
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(40, 46, 54, 240), h * 0.5f);
    dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(200, 210, 220, 160), h * 0.5f, 0, 1.0f);
    dl->AddText(font, size, ImVec2(pos.x + h * 0.4f, pos.y + (h - size) * 0.5f), kWhite, label);
    return w;
}

// D-pad with the left and right arms lit.
float DPadLeftRight(ImDrawList* dl, ImVec2 pos, float h) {
    const float t = h / 3.0f;
    const ImU32 dim = IM_COL32(90, 98, 108, 255), lit = kWhite;
    dl->AddRectFilled(ImVec2(pos.x + t, pos.y), ImVec2(pos.x + 2 * t, pos.y + h), dim, t * 0.25f);       // vertical arm
    dl->AddRectFilled(ImVec2(pos.x, pos.y + t), ImVec2(pos.x + h, pos.y + 2 * t), dim, t * 0.25f);       // horizontal arm
    dl->AddRectFilled(ImVec2(pos.x, pos.y + t), ImVec2(pos.x + t, pos.y + 2 * t), lit, t * 0.25f);       // left
    dl->AddRectFilled(ImVec2(pos.x + 2 * t, pos.y + t), ImVec2(pos.x + h, pos.y + 2 * t), lit, t * 0.25f); // right
    return h;
}

bool AnyKeyboardOrMouse() {
    const ImGuiIO& io = ImGui::GetIO();
    if (io.MouseDelta.x * io.MouseDelta.x + io.MouseDelta.y * io.MouseDelta.y > 4.0f) return true;
    if (io.MouseWheel != 0.0f) return true;
    for (int b = 0; b < ImGuiMouseButton_COUNT; ++b) if (ImGui::IsMouseClicked(b)) return true;
    for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_GamepadStart; ++k) {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(k), false)) return true;
    }
    return false;
}

bool AnyGamepad() {
    for (int k = ImGuiKey_GamepadStart; k <= ImGuiKey_GamepadRStickDown; ++k) {
        // The Win32 backend marks sticks/triggers down past a small dead zone.
        if (ImGui::IsKeyDown(static_cast<ImGuiKey>(k))) return true;
    }
    return false;
}

} // namespace

void Update() {
    if (s_device == Device::KeyboardMouse && AnyGamepad()) s_device = Device::Gamepad;
    else if (s_device == Device::Gamepad && AnyKeyboardOrMouse()) s_device = Device::KeyboardMouse;
}

Device Current() { return s_device; }

float Draw(ImDrawList* dl, ImFont* font, ImVec2 pos, float h, Prompt prompt) {
    if (!font) font = ImGui::GetFont();
    const float gap = h * 0.2f;
    if (s_device == Device::Gamepad) {
        switch (prompt) {
        case Prompt::Accept: return Face(dl, font, pos, h, "A", IM_COL32(92, 170, 60, 255));
        case Prompt::Back: return Face(dl, font, pos, h, "B", IM_COL32(204, 52, 44, 255));
        case Prompt::Change: return DPadLeftRight(dl, pos, h);
        case Prompt::TabLeft: return Bumper(dl, font, pos, h, "LB");
        case Prompt::TabRight: return Bumper(dl, font, pos, h, "RB");
        case Prompt::Tabs: {
            const float w = Bumper(dl, font, pos, h, "LB");
            return w + gap + Bumper(dl, font, ImVec2(pos.x + w + gap, pos.y), h, "RB");
        }
        }
        return 0.0f;
    }
    switch (prompt) {
    case Prompt::Accept: return Key(dl, font, pos, h, "ENTER");
    case Prompt::Back: return Key(dl, font, pos, h, "ESC");
    case Prompt::Change: return Key(dl, font, pos, h, "LEFT / RIGHT");
    case Prompt::TabLeft: return Key(dl, font, pos, h, "Q");
    case Prompt::TabRight: return Key(dl, font, pos, h, "E");
    case Prompt::Tabs: {
        const float w = Key(dl, font, pos, h, "Q");
        return w + gap + Key(dl, font, ImVec2(pos.x + w + gap, pos.y), h, "E");
    }
    }
    return 0.0f;
}


// ------------------------------------------------------------------------------------------------
// Single button glyphs (controller settings list, bindings, any list that names a button)
// ------------------------------------------------------------------------------------------------

namespace {

ImU32 FaceColour(Button b) {
    switch (b) {
    case Button::A: return IM_COL32(92, 170, 60, 255);
    case Button::B: return IM_COL32(204, 52, 44, 255);
    case Button::X: return IM_COL32(52, 134, 214, 255);
    case Button::Y: return IM_COL32(236, 182, 38, 255);
    default: return IM_COL32(60, 66, 76, 255);
    }
}

const char* FaceLetter(Button b) {
    switch (b) {
    case Button::A: return "A";
    case Button::B: return "B";
    case Button::X: return "X";
    case Button::Y: return "Y";
    default: return "";
    }
}

// Pill with a short label: LB / RB / LT / RT.
float Pill(ImDrawList* dl, ImFont* font, ImVec2 pos, float h, const char* label) {
    const float size = h * 0.62f;
    const float w = TextW(font, size, label) + h * 0.7f;
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(42, 48, 56, 245), h * 0.32f);
    dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(200, 210, 220, 170), h * 0.32f, 0, 1.0f);
    dl->AddText(font, size, ImVec2(pos.x + h * 0.35f, pos.y + (h - size) * 0.5f), kWhite, label);
    return w;
}

// D-pad cross with one arm lit.
float Dpad(ImDrawList* dl, ImVec2 pos, float h, bool up, bool down, bool left, bool right) {
    const float t = h / 3.0f;
    const ImU32 dim = IM_COL32(90, 98, 108, 255), lit = kWhite;
    dl->AddRectFilled(ImVec2(pos.x + t, pos.y), ImVec2(pos.x + 2 * t, pos.y + h), dim, t * 0.3f);
    dl->AddRectFilled(ImVec2(pos.x, pos.y + t), ImVec2(pos.x + h, pos.y + 2 * t), dim, t * 0.3f);
    if (up) dl->AddRectFilled(ImVec2(pos.x + t, pos.y), ImVec2(pos.x + 2 * t, pos.y + t), lit, t * 0.3f);
    if (down) dl->AddRectFilled(ImVec2(pos.x + t, pos.y + 2 * t), ImVec2(pos.x + 2 * t, pos.y + h), lit, t * 0.3f);
    if (left) dl->AddRectFilled(ImVec2(pos.x, pos.y + t), ImVec2(pos.x + t, pos.y + 2 * t), lit, t * 0.3f);
    if (right) dl->AddRectFilled(ImVec2(pos.x + 2 * t, pos.y + t), ImVec2(pos.x + h, pos.y + 2 * t), lit, t * 0.3f);
    return h;
}

// Stick click: an outlined circle with the stick letter (L / R).
float Stick(ImDrawList* dl, ImFont* font, ImVec2 pos, float h, const char* label) {
    const ImVec2 c(pos.x + h * 0.5f, pos.y + h * 0.5f);
    dl->AddCircleFilled(c, h * 0.5f, IM_COL32(34, 40, 48, 245), 24);
    dl->AddCircle(c, h * 0.5f, IM_COL32(200, 210, 220, 180), 24, 1.2f);
    const float size = h * 0.62f;
    const float tw = TextW(font, size, label);
    dl->AddText(font, size, ImVec2(c.x - tw * 0.5f, c.y - size * 0.55f), kWhite, label);
    return h;
}

// Round "system" glyph: two bars (Start), two squares (Back) or three bars (Menu).
float SystemGlyph(ImDrawList* dl, ImVec2 pos, float h, Button b) {
    const ImVec2 c(pos.x + h * 0.5f, pos.y + h * 0.5f);
    dl->AddCircleFilled(c, h * 0.5f, IM_COL32(200, 210, 220, 235), 24);
    const ImU32 ink = IM_COL32(20, 24, 28, 255);
    const float u = h * 0.13f;
    if (b == Button::Back) {
        dl->AddRectFilled(ImVec2(c.x - u * 1.6f, c.y - u * 1.2f), ImVec2(c.x - u * 0.2f, c.y + u * 0.2f), ink);
        dl->AddRectFilled(ImVec2(c.x + u * 0.2f, c.y + u * 0.2f), ImVec2(c.x + u * 1.6f, c.y + u * 1.6f), ink);
    } else if (b == Button::Menu) {
        for (int i = -1; i <= 1; ++i) {
            dl->AddRectFilled(ImVec2(c.x - u * 1.6f, c.y + i * u * 1.3f - u * 0.35f),
                              ImVec2(c.x + u * 1.6f, c.y + i * u * 1.3f + u * 0.35f), ink);
        }
    } else {
        dl->AddRectFilled(ImVec2(c.x - u * 1.5f, c.y - u * 0.9f), ImVec2(c.x + u * 1.5f, c.y - u * 0.1f), ink);
        dl->AddRectFilled(ImVec2(c.x - u * 1.5f, c.y + u * 0.3f), ImVec2(c.x + u * 1.5f, c.y + u * 1.1f), ink);
    }
    return h;
}

} // namespace

float ButtonWidth(ImFont* font, float height, Button button) {
    if (!font) font = ImGui::GetFont();
    const float h = height;
    switch (button) {
    case Button::LB: return TextW(font, h * 0.62f, "LB") + h * 0.7f;
    case Button::RB: return TextW(font, h * 0.62f, "RB") + h * 0.7f;
    case Button::LT: return TextW(font, h * 0.62f, "LT") + h * 0.7f;
    case Button::RT: return TextW(font, h * 0.62f, "RT") + h * 0.7f;
    default: return h;   // face buttons, d-pad and sticks are square
    }
}

float DrawButton(ImDrawList* dl, ImFont* font, ImVec2 pos, float height, Button button) {
    if (!font) font = ImGui::GetFont();
    const float h = height;
    switch (button) {
    case Button::A: case Button::B: case Button::X: case Button::Y:
        Face(dl, font, pos, h, FaceLetter(button), FaceColour(button));
        return h;
    case Button::LB: return Pill(dl, font, pos, h * 0.8f, "LB");
    case Button::RB: return Pill(dl, font, pos, h * 0.8f, "RB");
    case Button::LT: return Pill(dl, font, pos, h * 0.8f, "LT");
    case Button::RT: return Pill(dl, font, pos, h * 0.8f, "RT");
    case Button::DpadUp: return Dpad(dl, pos, h, true, false, false, false);
    case Button::DpadDown: return Dpad(dl, pos, h, false, true, false, false);
    case Button::DpadLeft: return Dpad(dl, pos, h, false, false, true, false);
    case Button::DpadRight: return Dpad(dl, pos, h, false, false, false, true);
    case Button::LStick: return Stick(dl, font, pos, h, "L");
    case Button::RStick: return Stick(dl, font, pos, h, "R");
    default: return SystemGlyph(dl, pos, h, button);
    }
}

float KeyWidth(ImFont* font, float height, const char* label) {
    if (!font) font = ImGui::GetFont();
    return TextW(font, height * 0.68f, label) + height * 0.55f;
}

float DrawKey(ImDrawList* dl, ImFont* font, ImVec2 pos, float height, const char* label) {
    if (!font) font = ImGui::GetFont();
    return Key(dl, font, pos, height, label);
}

bool ButtonFromSuffix(const char* suffix, Button& outButton) {
    if (!suffix) return false;
    const std::string text = suffix;
    struct Map { const char* key; Button button; };
    static const Map kMap[] = {
        { "A", Button::A }, { "B", Button::B }, { "X", Button::X }, { "Y", Button::Y },
        { "LB", Button::LB }, { "RB", Button::RB }, { "LT", Button::LT }, { "RT", Button::RT },
        { "D-pad", Button::DpadUp },
        { "Start", Button::Start }, { "Back", Button::Back }, { "Menu", Button::Menu },
        { "click", Button::RStick },   // "(click)": a stick click; the caller says which stick
    };
    // Exact match only: a substring test made "(LB)" resolve to B and "(Back)" resolve to B as well.
    for (const Map& m : kMap) {
        if (text == m.key) {
            outButton = m.button;
            return true;
        }
    }
    return false;
}

} // namespace InputPrompts
} // namespace noble
