#pragma once

#include <imgui.h>

namespace noble {

// One look for every Noble screen (front-end menu, tabs, pause menu), modelled on Halo: Reach's
// own menus: dark slate panels, square edges, ice-blue highlight, upper-case labels, and the
// partner's "glass" idea of a soft panel with a thin bright border. Fonts are Windows' own
// Bahnschrift / Segoe UI (loaded from the system font folder, nothing shipped).
namespace NobleStyle {

// Colours and metrics; loads fonts into the current ImGui context. Call once, right after
// ImGui::CreateContext().
void Apply();

ImVec4 Accent();        // ice blue
ImVec4 AccentDim();
ImVec4 TextMuted();

ImFont* TitleFont();    // large heading font (nullptr -> default font)
ImFont* HeroFont();     // very large screen titles ("MAIN MENU"), 56 px (nullptr -> default font)
ImFont* SmallFont();    // 15 px for hints and footnotes (nullptr -> default font)

// Section heading in the title font with a thin accent rule under it.
void Header(const char* text);
void SectionGap();

// Highlighted action buttons (PLAY, HOST, JOIN) and destructive ones (QUIT, STOP).
bool PrimaryButton(const char* label, const ImVec2& size = ImVec2(0, 0));
bool DangerButton(const char* label, const ImVec2& size = ImVec2(0, 0));

} // namespace NobleStyle
} // namespace noble
