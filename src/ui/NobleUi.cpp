#include "NobleUi.hpp"
#include "NobleStyle.hpp"

#include <algorithm>
#include <cfloat>
#include <cstring>

namespace noble {
namespace NobleUi {

namespace {
ImFont* Font(ImFont* f) { return f ? f : ImGui::GetFont(); }
}

float Scale() {
    return std::max(0.75f, ImGui::GetFontSize() / 18.0f);
}

float TextWidth(ImFont* font, float px, const char* text) {
    return Font(font)->CalcTextSizeA(px, FLT_MAX, 0.0f, text).x;
}

void Text(ImDrawList* dl, ImFont* font, float px, ImVec2 pos, ImU32 col, const char* text, float wrapWidth) {
    dl->AddText(Font(font), px, pos, col, text, nullptr, wrapWidth);
}

void Paragraph(const char* text, ImU32 col) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(col));
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

void SectionHeader(const char* text, float width) {
    const float s = Scale();
    ImGui::Dummy(ImVec2(0, 8.0f * s));
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    // Letter-spaced caps in the small font.
    ImFont* f = Font(NobleStyle::SmallFont());
    const float px = 14.0f * s;
    float x = p.x;
    char ch[2] = { 0, 0 };
    for (const char* c = text; *c; ++c) {
        ch[0] = *c;
        dl->AddText(f, px, ImVec2(x, p.y), IM_COL32(150, 190, 220, 255), ch);
        x += f->CalcTextSizeA(px, FLT_MAX, 0.0f, ch).x + 1.6f * s;
    }
    const float ruleY = p.y + px * 0.5f + 1.0f;
    if (x + 10.0f * s < p.x + width) dl->AddLine(ImVec2(x + 10.0f * s, ruleY), ImVec2(p.x + width, ruleY), kRule, 1.0f);
    ImGui::Dummy(ImVec2(width, px + 6.0f * s));
}

void RowBackground(ImDrawList* dl, ImVec2 min, ImVec2 max, bool focused, bool hovered) {
    const float s = Scale();
    if (focused) {
        dl->AddRectFilledMultiColor(min, max, IM_COL32(60, 120, 170, 130), IM_COL32(60, 120, 170, 18),
                                    IM_COL32(60, 120, 170, 18), IM_COL32(60, 120, 170, 130));
        dl->AddRectFilled(min, ImVec2(min.x + 3.0f * s, max.y), kAccent);
    } else if (hovered) {
        dl->AddRectFilled(min, max, IM_COL32(255, 255, 255, 14));
    }
}

void ChangedDot(ImDrawList* dl, ImVec2 center, float scale) {
    dl->AddCircleFilled(center, 3.2f * scale, kAmber);
}

void DrawSlider(ImDrawList* dl, ImVec2 min, ImVec2 max, float t, float defaultT, const char* valueText, bool active, bool disabled,
                float textWidth) {
    const float s = Scale();
    t = std::clamp(t, 0.0f, 1.0f);
    const float textW = textWidth > 0.0f ? textWidth : 86.0f * s;
    const float barH = 6.0f * s;
    const float cy = (min.y + max.y) * 0.5f;
    const ImVec2 b0(min.x, cy - barH * 0.5f), b1(max.x - textW, cy + barH * 0.5f);
    dl->AddRectFilled(b0, b1, IM_COL32(255, 255, 255, disabled ? 14 : 28));
    const float fx = b0.x + (b1.x - b0.x) * t;
    dl->AddRectFilled(b0, ImVec2(fx, b1.y), disabled ? IM_COL32(120, 130, 140, 140) : (active ? kAccent : IM_COL32(110, 180, 225, 255)));
    if (defaultT >= 0.0f) {
        const float dx = b0.x + (b1.x - b0.x) * std::clamp(defaultT, 0.0f, 1.0f);
        dl->AddLine(ImVec2(dx, b0.y - 4.0f * s), ImVec2(dx, b1.y + 4.0f * s), IM_COL32(255, 255, 255, 120), 1.0f);
    }
    if (!disabled) dl->AddCircleFilled(ImVec2(fx, cy), (active ? 7.0f : 5.5f) * s, kText);
    const float tw = TextWidth(nullptr, 17.0f * s, valueText);
    Text(dl, nullptr, 17.0f * s, ImVec2(max.x - tw, cy - 9.0f * s), disabled ? kTextOff : kText, valueText);
}

void DrawCycle(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* value, int index, int count, bool wraps,
               bool hoverLeft, bool hoverRight, bool disabled) {
    const float s = Scale();
    const float cy = (min.y + max.y) * 0.5f - (count > 1 ? 3.0f * s : 0.0f);
    const float aw = 22.0f * s, ah = 6.0f * s;
    const bool leftOn = !disabled && (wraps || index > 0);
    const bool rightOn = !disabled && (wraps || index < count - 1);
    const ImU32 lc = !leftOn ? IM_COL32(255, 255, 255, 40) : (hoverLeft ? kText : kTextDim);
    const ImU32 rc = !rightOn ? IM_COL32(255, 255, 255, 40) : (hoverRight ? kText : kTextDim);
    dl->AddTriangleFilled(ImVec2(min.x + aw * 0.65f, cy - ah), ImVec2(min.x + aw * 0.65f, cy + ah), ImVec2(min.x + aw * 0.3f, cy), lc);
    dl->AddTriangleFilled(ImVec2(max.x - aw * 0.65f, cy - ah), ImVec2(max.x - aw * 0.65f, cy + ah), ImVec2(max.x - aw * 0.3f, cy), rc);
    const float tw = TextWidth(nullptr, 17.0f * s, value);
    Text(dl, nullptr, 17.0f * s, ImVec2((min.x + max.x - tw) * 0.5f, cy - 9.0f * s), disabled ? kTextOff : kText, value);
    if (count > 1 && count <= 24) {
        const float gap = 7.0f * s;
        const float total = gap * (count - 1);
        const float x0 = (min.x + max.x) * 0.5f - total * 0.5f;
        const float dy = max.y - 4.0f * s;
        for (int i = 0; i < count; ++i) {
            dl->AddCircleFilled(ImVec2(x0 + i * gap, dy), (i == index ? 2.2f : 1.6f) * s,
                                i == index ? (disabled ? kTextOff : kAccent) : IM_COL32(255, 255, 255, 60));
        }
    }
}

void DrawToggle(ImDrawList* dl, ImVec2 min, ImVec2 max, bool on, bool hovered, bool disabled) {
    const float s = Scale();
    const float h = 22.0f * s, w = 48.0f * s;
    const float cy = (min.y + max.y) * 0.5f;
    const ImVec2 p0(max.x - w, cy - h * 0.5f), p1(max.x, cy + h * 0.5f);
    const ImU32 bg = disabled ? IM_COL32(255, 255, 255, 20)
                   : on ? IM_COL32(52, 112, 156, hovered ? 255 : 230) : IM_COL32(40, 48, 58, hovered ? 255 : 230);
    dl->AddRectFilled(p0, p1, bg, h * 0.5f);
    dl->AddRect(p0, p1, IM_COL32(170, 220, 255, disabled ? 40 : (on ? 160 : 70)), h * 0.5f, 0, 1.0f);
    const float r = h * 0.5f - 3.0f * s;
    const ImVec2 knob(on ? p1.x - h * 0.5f : p0.x + h * 0.5f, cy);
    dl->AddCircleFilled(knob, r, disabled ? kTextOff : kText);
    const char* label = on ? "ON" : "OFF";
    const float tw = TextWidth(nullptr, 17.0f * s, label);
    Text(dl, nullptr, 17.0f * s, ImVec2(p0.x - tw - 10.0f * s, cy - 9.0f * s), disabled ? kTextOff : (on ? kText : kTextDim), label);
}

void DrawBox(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* text, bool hovered, bool highlight, bool disabled) {
    const float s = Scale();
    dl->AddRectFilled(min, max, disabled ? IM_COL32(255, 255, 255, 10) : (hovered ? IM_COL32(60, 74, 88, 240) : IM_COL32(26, 34, 44, 230)));
    dl->AddRect(min, max, highlight ? kAccent : IM_COL32(170, 220, 255, hovered ? 160 : 60), 0.0f, 0, 1.0f);
    const float tw = TextWidth(nullptr, 17.0f * s, text);
    Text(dl, nullptr, 17.0f * s, ImVec2((min.x + max.x - tw) * 0.5f, (min.y + max.y) * 0.5f - 9.0f * s),
         disabled ? kTextOff : (highlight ? kAccent : kText), text);
}

} // namespace NobleUi
} // namespace noble
