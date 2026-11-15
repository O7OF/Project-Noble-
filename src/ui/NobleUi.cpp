#include "NobleUi.hpp"
#include "NobleStyle.hpp"
#include "common/Settings.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>

namespace noble {
namespace NobleUi {

namespace {
ImFont *Font(ImFont *f) { return f ? f : ImGui::GetFont(); }
} // namespace

float Scale() {
  // Scale with the overlay's back-buffer height, like FrontEndMenu's ui.s.
  // Deriving this from the font's logical size left every panel (Settings,
  // Career, Server Browser) at 1.0 while the rest of the menu rendered at 1.8x
  // on a 2880x1620 back buffer, so their text looked half-size.
  const float s = ImGui::GetIO().DisplaySize.y / 900.0f;
  // No low ceiling: a 4K back buffer needs 2.4x. The old 2.2 cap (2.0 in
  // FrontEndMenu) left the menu undersized and not reaching the screen edges
  // once a 4K desktop rendered the menu at 3840x2160. The player's UI SCALE
  // setting (75 - 150 %) goes on top of the screen-height scale.
  return std::clamp(s, 0.85f, 4.0f) * Settings::UiScale();
}

float TextWidth(ImFont *font, float px, const char *text) {
  return Font(font)->CalcTextSizeA(px, FLT_MAX, 0.0f, text).x;
}

void Text(ImDrawList *dl, ImFont *font, float px, ImVec2 pos, ImU32 col,
          const char *text, float wrapWidth) {
  dl->AddText(Font(font), px, pos, col, text, nullptr, wrapWidth);
}

void Paragraph(const char *text, ImU32 col) {
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(col));
  ImGui::PushTextWrapPos(0.0f);
  ImGui::TextUnformatted(text);
  ImGui::PopTextWrapPos();
  ImGui::PopStyleColor();
}

void SectionHeader(const char *text, float width) {
  const float s = Scale();
  ImGui::Dummy(ImVec2(0, 8.0f * s));
  const ImVec2 p = ImGui::GetCursorScreenPos();
  ImDrawList *dl = ImGui::GetWindowDrawList();
  // Letter-spaced caps in the small font.
  ImFont *f = Font(NobleStyle::SmallFont());
  const float px = 14.0f * s;
  float x = p.x;
  char ch[2] = {0, 0};
  for (const char *c = text; *c; ++c) {
    ch[0] = *c;
    dl->AddText(f, px, ImVec2(x, p.y), IM_COL32(150, 190, 220, 255), ch);
    x += f->CalcTextSizeA(px, FLT_MAX, 0.0f, ch).x + 1.6f * s;
  }
  const float ruleY = p.y + px * 0.5f + 1.0f;
  if (x + 10.0f * s < p.x + width)
    dl->AddLine(ImVec2(x + 10.0f * s, ruleY), ImVec2(p.x + width, ruleY), kRule,
                1.0f);
  ImGui::Dummy(ImVec2(width, px + 6.0f * s));
}

void RowBackground(ImDrawList *dl, ImVec2 min, ImVec2 max, bool focused,
                   bool hovered) {
  const float s = Scale();
  const float r = 5.0f * s;
  if (focused) {
    dl->AddRectFilled(min, max, IM_COL32(36, 84, 128, 125), r);
    dl->AddRect(min, max, IM_COL32(143, 216, 255, 130), r, 0, 1.0f * s);
    dl->AddRectFilled(ImVec2(min.x + 1.0f * s, min.y + 3.0f * s),
                      ImVec2(min.x + 4.5f * s, max.y - 3.0f * s), kAccent,
                      2.0f * s);
  } else if (hovered) {
    dl->AddRectFilled(min, max, IM_COL32(143, 216, 255, 20), r);
    dl->AddRect(min, max, IM_COL32(143, 216, 255, 45), r, 0, 1.0f);
  }
}

void ChangedDot(ImDrawList *dl, ImVec2 center, float scale) {
  dl->AddCircleFilled(center, 3.2f * scale, kAmber);
}

void DrawSlider(ImDrawList *dl, ImVec2 min, ImVec2 max, float t, float defaultT,
                const char *valueText, bool active, bool disabled,
                float textWidth) {
  const float s = Scale();
  t = std::clamp(t, 0.0f, 1.0f);
  const float textW = textWidth > 0.0f ? textWidth : 86.0f * s;
  const float barH = 7.0f * s;
  const float r = barH * 0.5f;
  const float cy = (min.y + max.y) * 0.5f;
  const ImVec2 b0(min.x, cy - barH * 0.5f), b1(max.x - textW, cy + barH * 0.5f);
  dl->AddRectFilled(b0, b1, IM_COL32(255, 255, 255, disabled ? 16 : 30), r);
  const float fx = b0.x + (b1.x - b0.x) * t;
  if (fx > b0.x + 1.0f) {
    dl->AddRectFilled(b0, ImVec2(fx, b1.y),
                      disabled
                          ? IM_COL32(120, 130, 140, 140)
                          : (active ? kAccent : IM_COL32(110, 180, 225, 255)),
                      r);
  }
  if (defaultT >= 0.0f) {
    const float dx = b0.x + (b1.x - b0.x) * std::clamp(defaultT, 0.0f, 1.0f);
    dl->AddLine(ImVec2(dx, b0.y - 4.0f * s), ImVec2(dx, b1.y + 4.0f * s),
                IM_COL32(255, 255, 255, 120), 1.0f);
  }
  if (!disabled)
    dl->AddCircleFilled(ImVec2(fx, cy), (active ? 7.5f : 5.8f) * s, kText);
  const float tw = TextWidth(nullptr, 17.0f * s, valueText);
  Text(dl, nullptr, 17.0f * s, ImVec2(max.x - tw, cy - 9.0f * s),
       disabled ? kTextOff : kText, valueText);
}

void DrawCycle(ImDrawList *dl, ImVec2 min, ImVec2 max, const char *value,
               int index, int count, bool wraps, bool hoverLeft,
               bool hoverRight, bool disabled) {
  const float s = Scale();
  const float cy = (min.y + max.y) * 0.5f - (count > 1 ? 3.0f * s : 0.0f);
  const float aw = 22.0f * s, ah = 6.0f * s;
  const bool leftOn = !disabled && (wraps || index > 0);
  const bool rightOn = !disabled && (wraps || index < count - 1);
  const ImU32 lc =
      !leftOn ? IM_COL32(255, 255, 255, 40) : (hoverLeft ? kText : kTextDim);
  const ImU32 rc =
      !rightOn ? IM_COL32(255, 255, 255, 40) : (hoverRight ? kText : kTextDim);
  dl->AddTriangleFilled(ImVec2(min.x + aw * 0.65f, cy - ah),
                        ImVec2(min.x + aw * 0.65f, cy + ah),
                        ImVec2(min.x + aw * 0.3f, cy), lc);
  dl->AddTriangleFilled(ImVec2(max.x - aw * 0.65f, cy - ah),
                        ImVec2(max.x - aw * 0.65f, cy + ah),
                        ImVec2(max.x - aw * 0.3f, cy), rc);
  const float tw = TextWidth(nullptr, 17.0f * s, value);
  Text(dl, nullptr, 17.0f * s,
       ImVec2((min.x + max.x - tw) * 0.5f, cy - 9.0f * s),
       disabled ? kTextOff : kText, value);
  if (count > 1 && count <= 24) {
    const float gap = 7.0f * s;
    const float total = gap * (count - 1);
    const float x0 = (min.x + max.x) * 0.5f - total * 0.5f;
    const float dy = max.y - 4.0f * s;
    for (int i = 0; i < count; ++i) {
      dl->AddCircleFilled(ImVec2(x0 + i * gap, dy),
                          (i == index ? 2.2f : 1.6f) * s,
                          i == index ? (disabled ? kTextOff : kAccent)
                                     : IM_COL32(255, 255, 255, 60));
    }
  }
}

void DrawToggle(ImDrawList *dl, ImVec2 min, ImVec2 max, bool on, bool hovered,
                bool disabled) {
  const float s = Scale();
  const float h = 22.0f * s, w = 48.0f * s;
  const float cy = (min.y + max.y) * 0.5f;
  const ImVec2 p0(max.x - w, cy - h * 0.5f), p1(max.x, cy + h * 0.5f);
  const ImU32 bg = disabled ? IM_COL32(255, 255, 255, 20)
                   : on     ? IM_COL32(52, 112, 156, hovered ? 255 : 230)
                            : IM_COL32(40, 48, 58, hovered ? 255 : 230);
  dl->AddRectFilled(p0, p1, bg, h * 0.5f);
  dl->AddRect(p0, p1, IM_COL32(170, 220, 255, disabled ? 40 : (on ? 160 : 70)),
              h * 0.5f, 0, 1.0f);
  const float r = h * 0.5f - 3.0f * s;
  const ImVec2 knob(on ? p1.x - h * 0.5f : p0.x + h * 0.5f, cy);
  dl->AddCircleFilled(knob, r, disabled ? kTextOff : kText);
  const char *label = on ? "ON" : "OFF";
  const float tw = TextWidth(nullptr, 17.0f * s, label);
  Text(dl, nullptr, 17.0f * s, ImVec2(p0.x - tw - 10.0f * s, cy - 9.0f * s),
       disabled ? kTextOff : (on ? kText : kTextDim), label);
}

void DrawBox(ImDrawList *dl, ImVec2 min, ImVec2 max, const char *text,
             bool hovered, bool highlight, bool disabled) {
  const float s = Scale();
  const float r = 5.0f * s;
  dl->AddRectFilled(min, max,
                    disabled ? IM_COL32(255, 255, 255, 10)
                             : (hovered ? IM_COL32(42, 68, 96, 240)
                                        : IM_COL32(20, 32, 46, 232)),
                    r);
  dl->AddRect(min, max,
              highlight ? kAccent : IM_COL32(143, 216, 255, hovered ? 190 : 75),
              r, 0, highlight ? 1.5f * s : 1.0f);
  const float tw = TextWidth(nullptr, 17.0f * s, text);
  Text(dl, nullptr, 17.0f * s,
       ImVec2((min.x + max.x - tw) * 0.5f, (min.y + max.y) * 0.5f - 9.0f * s),
       disabled ? kTextOff : (highlight ? kAccent : kText), text);
}

void DrawFocusHighlight(ImDrawList *dl, ImVec2 min, ImVec2 max, float scale,
                        bool active) {
  if (!active)
    return;
  const float s = scale > 0.0f ? scale : Scale();
  const float pulse =
      0.85f + 0.15f * static_cast<float>(std::sin(ImGui::GetTime() * 5.0));
  const ImU32 border =
      IM_COL32(143, 216, 255, static_cast<int>(240.0f * pulse));
  const ImU32 glow = IM_COL32(100, 190, 255, static_cast<int>(48.0f * pulse));
  const float r = 6.0f * s;
  dl->AddRectFilled(min, max, glow, r);
  dl->AddRect(min, max, border, r, 0, 2.0f * s);
  dl->AddRectFilled(ImVec2(min.x + 1.0f * s, min.y + 4.0f * s),
                    ImVec2(min.x + 5.0f * s, max.y - 4.0f * s), kAccent,
                    2.0f * s);
}

void DrawTacticalFrame(ImDrawList *dl, ImVec2 min, ImVec2 max, float scale,
                       bool focused, bool selected, bool hovered,
                       bool warmFocus) {
  if (!dl)
    return;
  const float s = scale > 0.0f ? scale : Scale();
  const float pulse =
      focused ? (0.86f +
                 0.14f * static_cast<float>(std::sin(ImGui::GetTime() * 5.0)))
              : 1.0f;
  const ImU32 focusCol =
      warmFocus ? IM_COL32(255, 168, 62, static_cast<int>(250.0f * pulse))
                : IM_COL32(143, 216, 255, static_cast<int>(250.0f * pulse));
  const ImU32 border = focused    ? focusCol
                       : selected ? IM_COL32(143, 216, 255, 215)
                       : hovered  ? IM_COL32(160, 210, 240, 155)
                                  : IM_COL32(95, 135, 168, 92);
  const float r = 6.0f * s;
  const float thickness = focused ? 2.0f * s : (selected ? 1.5f * s : 1.0f);
  if (focused) {
    const ImU32 outerGlow =
        warmFocus ? IM_COL32(255, 155, 45, 48) : IM_COL32(100, 190, 255, 48);
    dl->AddRect(ImVec2(min.x - 2.0f * s, min.y - 2.0f * s),
                ImVec2(max.x + 2.0f * s, max.y + 2.0f * s), outerGlow,
                r + 2.0f * s, 0, 3.0f * s);
  }
  dl->AddRect(min, max, border, r, 0, thickness);

  // Subtle L-shaped tactical corner brackets inside the card.
  const float inset = 6.0f * s;
  const float leg = 9.0f * s;
  const ImU32 tickCol =
      focused
          ? focusCol
          : (selected ? kAccent : IM_COL32(190, 215, 235, hovered ? 190 : 120));
  const float tw = focused ? 1.8f * s : 1.2f * s;
  const float x0 = min.x + inset, y0 = min.y + inset, x1 = max.x - inset,
              y1 = max.y - inset;
  if (x1 - x0 > leg * 2.5f && y1 - y0 > leg * 2.5f) {
    dl->AddLine(ImVec2(x0, y0), ImVec2(x0 + leg, y0), tickCol, tw);
    dl->AddLine(ImVec2(x0, y0), ImVec2(x0, y0 + leg), tickCol, tw);
    dl->AddLine(ImVec2(x1, y0), ImVec2(x1 - leg, y0), tickCol, tw);
    dl->AddLine(ImVec2(x1, y0), ImVec2(x1, y0 + leg), tickCol, tw);
    dl->AddLine(ImVec2(x0, y1), ImVec2(x0 + leg, y1), tickCol, tw);
    dl->AddLine(ImVec2(x0, y1), ImVec2(x0, y1 - leg), tickCol, tw);
    dl->AddLine(ImVec2(x1, y1), ImVec2(x1 - leg, y1), tickCol, tw);
    dl->AddLine(ImVec2(x1, y1), ImVec2(x1, y1 - leg), tickCol, tw);
  }
}

bool DrawTile(ImDrawList *dl, const char *id, ImVec2 min, ImVec2 max,
              const char *title, const char *subtitle, bool focused,
              bool selected, bool enabled, const char *badge) {
  const float s = Scale();
  const float r = 6.0f * s;
  const ImVec2 size(max.x - min.x, max.y - min.y);
  ImGui::SetCursorScreenPos(min);
  ImGui::PushID(id);
  const bool clicked = ImGui::InvisibleButton("##tile", size) && enabled;
  const bool hovered = ImGui::IsItemHovered();
  ImGui::PopID();

  const ImU32 bg = !enabled                ? IM_COL32(14, 18, 24, 150)
                   : (focused || selected) ? IM_COL32(30, 68, 105, 225)
                   : hovered               ? IM_COL32(24, 48, 72, 212)
                                           : IM_COL32(14, 24, 38, 198);
  dl->AddRectFilled(min, max, bg, r);
  if (focused) {
    DrawFocusHighlight(dl, min, max, s, true);
  } else if (selected) {
    dl->AddRect(min, max, IM_COL32(143, 216, 255, 200), r, 0, 1.5f * s);
    dl->AddRectFilled(ImVec2(min.x + 1.0f * s, min.y + 4.0f * s),
                      ImVec2(min.x + 4.0f * s, max.y - 4.0f * s),
                      IM_COL32(143, 216, 255, 215), 2.0f * s);
  } else {
    dl->AddRect(min, max, IM_COL32(143, 216, 255, hovered ? 135 : 52), r, 0,
                1.0f);
  }

  const float padX = 14.0f * s;
  const float titlePx = 19.0f * s;
  const float subPx = 13.5f * s;
  const bool hasSub = subtitle && subtitle[0] != '\0';
  const float titleY =
      hasSub ? (min.y + 8.0f * s) : ((min.y + max.y - titlePx) * 0.5f);
  const ImU32 titleCol =
      !enabled ? kTextOff
               : (focused || selected ? kText : IM_COL32(226, 236, 244, 255));
  Text(dl, NobleStyle::TitleFont(), titlePx, ImVec2(min.x + padX, titleY),
       titleCol, title);

  if (hasSub) {
    Text(dl, NobleStyle::SmallFont(), subPx,
         ImVec2(min.x + padX, titleY + titlePx + 2.0f * s),
         enabled ? kTextDim : kTextOff, subtitle,
         (max.x - min.x) - padX * 2.0f);
  }
  if (badge && badge[0] != '\0') {
    const float bw =
        TextWidth(NobleStyle::SmallFont(), 13.0f * s, badge) + 14.0f * s;
    const float bh = 20.0f * s;
    const ImVec2 b0(max.x - bw - 10.0f * s, min.y + 8.0f * s);
    const ImVec2 b1(b0.x + bw, b0.y + bh);
    dl->AddRectFilled(b0, b1, IM_COL32(143, 216, 255, focused ? 75 : 40),
                      4.0f * s);
    dl->AddRect(b0, b1, IM_COL32(143, 216, 255, 165), 4.0f * s, 0, 1.0f);
    Text(dl, NobleStyle::SmallFont(), 13.0f * s,
         ImVec2(b0.x + 7.0f * s, b0.y + 3.0f * s), kAccent, badge);
  }
  return clicked;
}

void DrawInteractiveScrollbar(ImDrawList *dl, const char *id, ImVec2 trackMin,
                              ImVec2 trackMax, float &scrollY, float maxScroll,
                              float visibleExtent, float totalExtent,
                              float scale) {
  if (!dl || maxScroll <= 0.0f || visibleExtent <= 0.0f ||
      totalExtent <= visibleExtent || trackMax.y <= trackMin.y ||
      trackMax.x <= trackMin.x) {
    return;
  }
  const float s = scale > 0.0f ? scale : Scale();
  scrollY = std::clamp(scrollY, 0.0f, maxScroll);
  const float trackW = trackMax.x - trackMin.x;
  const float trackH = trackMax.y - trackMin.y;
  const float ratio = std::clamp(visibleExtent / totalExtent, 0.08f, 1.0f);
  const float thumbH = std::max(28.0f * s, trackH * ratio);
  const float travel = std::max(1.0f, trackH - thumbH);
  const float frac = scrollY / maxScroll;
  const float thumbY = trackMin.y + travel * frac;

  const float hitPad = std::max(6.0f * s, 10.0f * s - trackW * 0.5f);
  const ImVec2 hitMin(trackMin.x - hitPad, trackMin.y);
  const ImVec2 hitSize(trackW + hitPad * 2.0f, trackH);
  const ImVec2 savedCursor = ImGui::GetCursorScreenPos();

  ImGui::PushID(id);
  ImGui::SetCursorScreenPos(hitMin);
  ImGui::InvisibleButton("##scrollbar", hitSize);
  const bool hovered = ImGui::IsItemHovered();
  const bool active = ImGui::IsItemActive();

  static ImGuiID s_dragId = 0;
  static float s_dragOffsetY = 0.0f;
  const ImGuiID widgetId = ImGui::GetItemID();
  if (ImGui::IsItemActivated()) {
    s_dragId = widgetId;
    const float mouseY = ImGui::GetIO().MousePos.y;
    if (mouseY >= thumbY && mouseY <= thumbY + thumbH) {
      s_dragOffsetY = mouseY - thumbY;
    } else {
      s_dragOffsetY = thumbH * 0.5f;
      const float jumpFrac = std::clamp(
          (mouseY - trackMin.y - s_dragOffsetY) / travel, 0.0f, 1.0f);
      scrollY = jumpFrac * maxScroll;
    }
  }
  if (active && s_dragId == widgetId &&
      ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
    const float mouseY = ImGui::GetIO().MousePos.y;
    const float dragFrac =
        std::clamp((mouseY - s_dragOffsetY - trackMin.y) / travel, 0.0f, 1.0f);
    scrollY = dragFrac * maxScroll;
  }
  if (!active && s_dragId == widgetId) {
    s_dragId = 0;
  }
  if (hovered || active) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
  }
  ImGui::PopID();
  ImGui::SetCursorScreenPos(savedCursor);
  ImGui::Dummy(ImVec2(0.0f, 0.0f));

  const float updatedFrac = std::clamp(scrollY / maxScroll, 0.0f, 1.0f);
  const float drawThumbY = trackMin.y + travel * updatedFrac;
  const float r = trackW * 0.5f;
  dl->AddRectFilled(trackMin, trackMax, IM_COL32(10, 18, 28, 175), r);
  dl->AddRect(trackMin, trackMax,
              IM_COL32(100, 175, 225, (hovered || active) ? 110 : 45), r, 0,
              1.0f);

  const ImVec2 tMin(trackMin.x + 1.0f, drawThumbY);
  const ImVec2 tMax(trackMax.x - 1.0f, drawThumbY + thumbH);
  const ImU32 thumbCol = active    ? IM_COL32(143, 226, 255, 255)
                         : hovered ? IM_COL32(115, 200, 248, 242)
                                   : IM_COL32(82, 165, 220, 210);
  dl->AddRectFilled(tMin, tMax, thumbCol, r);
  if ((hovered || active) && thumbH > 16.0f * s) {
    const float cx = (tMin.x + tMax.x) * 0.5f;
    const float cy = (tMin.y + tMax.y) * 0.5f;
    dl->AddLine(ImVec2(cx - trackW * 0.22f, cy - 3.0f * s),
                ImVec2(cx + trackW * 0.22f, cy - 3.0f * s),
                IM_COL32(14, 24, 36, 180), 1.4f * s);
    dl->AddLine(ImVec2(cx - trackW * 0.22f, cy + 3.0f * s),
                ImVec2(cx + trackW * 0.22f, cy + 3.0f * s),
                IM_COL32(14, 24, 36, 180), 1.4f * s);
  }
}

} // namespace NobleUi
} // namespace noble
