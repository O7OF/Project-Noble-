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

// A single Xbox button glyph, for lists that name the button (the CONTROLLER settings page, the key
// binding list, scoreboards). Drawn in code, same look as the prompts above.
enum class Button {
    A, B, X, Y,          // coloured face buttons
    LB, RB,              // shoulder bumpers (grey pills)
    LT, RT,              // analogue triggers
    DpadUp, DpadDown, DpadLeft, DpadRight,
    LStick, RStick,      // stick clicks
    Start, Back, Menu,   // Start / Back / "menu" (hamburger)
};

// Width `DrawButton` would use for this glyph at `height` pixels, so callers can lay text out after it.
float ButtonWidth(ImFont* font, float height, Button button);
float DrawButton(ImDrawList* dl, ImFont* font, ImVec2 pos, float height, Button button);

// A keyboard key box for an arbitrary label ("Y", "X", "TAB"), for lists that show a key name.
float KeyWidth(ImFont* font, float height, const char* label);
float DrawKey(ImDrawList* dl, ImFont* font, ImVec2 pos, float height, const char* label);

// Parses a binding label suffix such as "(RT)", "(click)", "(D-pad)", "(Start)" to a Button.
// `outButton` is only written when the suffix maps to one; returns false otherwise.
bool ButtonFromSuffix(const char* suffix, Button& outButton);

} // namespace InputPrompts
} // namespace noble
