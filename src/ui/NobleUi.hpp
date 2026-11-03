#pragma once

#include <imgui.h>

namespace noble {

// Shared building blocks for Noble's menus (the notes Ã‚Â§3): classic layout
// pieces drawn in Reach's look (steel-blue panels, ice-blue focus, amber only for "unsaved").
// Everything draws with ImGui draw lists inside the current window; sizes scale with Scale().
namespace NobleUi {

// Colours.
constexpr ImU32 kText = IM_COL32(236, 240, 244, 255);
constexpr ImU32 kTextDim = IM_COL32(160, 172, 184, 255);
constexpr ImU32 kTextOff = IM_COL32(105, 114, 124, 255);
constexpr ImU32 kAccent = IM_COL32(143, 216, 255, 255);
constexpr ImU32 kAmber = IM_COL32(255, 186, 74, 255);
constexpr ImU32 kRule = IM_COL32(255, 255, 255, 40);

// UI scale for the current window (the body font is 18 px at scale 1).
float Scale();

// Small letter-spaced caps followed by a thin rule across `width`.
void SectionHeader(const char* text, float width);

// Focused / hovered row background: solid 3 px left edge and a gradient fill (focus), faint fill (hover).
void RowBackground(ImDrawList* dl, ImVec2 min, ImVec2 max, bool focused, bool hovered);

// Small amber dot (changed but not applied).
void ChangedDot(ImDrawList* dl, ImVec2 center, float scale);

// Value widgets drawn into a rectangle (the row's value column). Interaction is done by the caller
// (SettingsPanel) so keyboard, pad and mouse share one path; these only draw.
//   Slider: fill bar with value text and an optional default tick (t and defaultT in 0..1, defaultT < 0 = none).
void DrawSlider(ImDrawList* dl, ImVec2 min, ImVec2 max, float t, float defaultT, const char* valueText, bool active, bool disabled,
                float textWidth = 0.0f);
//   Cycle: Ã¢â‚¬Â¹ value Ã¢â‚¬Âº with position dots under the value; arrows grey out at the ends when !wraps.
void DrawCycle(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* value, int index, int count, bool wraps,
               bool hoverLeft, bool hoverRight, bool disabled);
//   Toggle pill ON / OFF.
void DrawToggle(ImDrawList* dl, ImVec2 min, ImVec2 max, bool on, bool hovered, bool disabled);
//   Button-like box with centred text (key bindings, actions).
void DrawBox(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* text, bool hovered, bool highlight, bool disabled);

// Text helpers (font may be nullptr = current font).
float TextWidth(ImFont* font, float px, const char* text);
void Text(ImDrawList* dl, ImFont* font, float px, ImVec2 pos, ImU32 col, const char* text, float wrapWidth = 0.0f);

// Wrapped paragraph in the current window (advances the cursor).
void Paragraph(const char* text, ImU32 col = kTextDim);

} // namespace NobleUi
} // namespace noble
