#include "InputPrompts.hpp"

#include <cfloat>
#include <cmath>

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

} // namespace InputPrompts
} // namespace noble
