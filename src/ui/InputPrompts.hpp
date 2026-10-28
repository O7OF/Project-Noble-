#pragma once

#include <imgui.h>

namespace noble {

// Button prompts that follow the device the player last used: keyboard/mouse keys ("ENTER", "ESC") or
// Xbox controller buttons (A, B, LB/RB, D-pad), like MCC's menus. The controller glyphs are drawn
// in code (coloured circles / pills), so no controller art is shipped or read.
namespace InputPrompts {

enum class Device { KeyboardMouse, Gamepad };

enum class Prompt {
    Accept,   // Enter / A
    Back,     // Esc / B
    Change,   // Left-Right / D-pad left-right
    Tabs,     // Q-E / LB-RB
    TabLeft,  // Q / LB
    TabRight, // E / RB
};

// Call once per ImGui frame (after NewFrame): switches the device when the other one is used.
void Update();
Device Current();

// Draws the prompt at `pos` (top-left), `height` pixels tall. Returns the width used.
float Draw(ImDrawList* dl, ImFont* font, ImVec2 pos, float height, Prompt prompt);

} // namespace InputPrompts
} // namespace noble
