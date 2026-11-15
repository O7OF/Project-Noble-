#include "CareerPanel.hpp"
#include "InputPrompts.hpp"
#include "MenuSounds.hpp"
#include "NobleStyle.hpp"
#include "NobleUi.hpp"
#include "UiArt.hpp"
#include "UiPad.hpp"
#include "career/Career.hpp"
#include "common/MapNames.hpp"

#include <imgui.h>
#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <string>
#include <vector>

namespace noble {

namespace {

float U(float px) { return px * NobleUi::Scale(); }

// 1234567 -> "1,234,567"
std::string Grouped(long long v) {
    std::string digits = std::to_string(v < 0 ? -v : v);
    std::string out;
    for (size_t i = 0; i < digits.size(); ++i) {
        if (i && (digits.size() - i) % 3 == 0) out.push_back(',');
        out.push_back(digits[i]);
    }
    return v < 0 ? "-" + out : out;
}

std::string HoursMinutes(double minutes) {
    const long long m = static_cast<long long>(minutes + 0.5);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%lld h %02lld min", m / 60, m % 60);
    return buf;
}

const char* DifficultyName(int d) {
    static const char* const kNames[4] = { "Easy", "Normal", "Heroic", "Legendary" };
    return d >= 0 && d < 4 ? kNames[d] : "-";
}

ImU32 DifficultyColour(int d) {
    switch (d) {
    case 0: return IM_COL32(110, 210, 150, 255); // Easy
    case 1: return IM_COL32(143, 216, 255, 255); // Normal
    case 2: return IM_COL32(140, 150, 245, 255); // Heroic
    case 3: return IM_COL32(250, 196, 78, 255);  // Legendary
    default: return IM_COL32(170, 185, 200, 255);
    }
}

void DrawTacticalBar(ImDrawList* dl, ImVec2 p0, ImVec2 p1, float fraction, ImU32 fillCol) {
    const float r = std::max(1.5f, (p1.y - p0.y) * 0.45f);
    dl->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 22), r);
    dl->AddRect(p0, p1, IM_COL32(143, 216, 255, 45), r);
    const float f = std::clamp(fraction, 0.0f, 1.0f);
    if (f > 0.0f) {
        const float fw = std::max(r * 2.0f, (p1.x - p0.x) * f);
        dl->AddRectFilled(p0, ImVec2(std::min(p1.x, p0.x + fw), p1.y), fillCol, r);
    }
}

void DrawKpiTile(ImDrawList* dl, ImVec2 p0, ImVec2 p1, const char* label, const char* bigValue,
                 const char* subLabel, ImU32 accentCol, bool focused, bool hovered) {
    using namespace NobleUi;
    const float s = Scale();
    dl->AddRectFilled(p0, p1,
                      focused ? IM_COL32(22, 36, 52, 232)
                              : (hovered ? IM_COL32(18, 28, 42, 225) : IM_COL32(12, 20, 30, 215)),
                      3.0f * s);
    dl->AddRectFilled(p0, ImVec2(p0.x + 3.5f * s, p1.y), accentCol);
    DrawTacticalFrame(dl, p0, p1, s, focused, false, hovered, true);

    ImFont* small = NobleStyle::SmallFont();
    ImFont* title = NobleStyle::TitleFont();
    const float pad = 14.0f * s;
    Text(dl, small, 13.0f * s, ImVec2(p0.x + pad, p0.y + 10.0f * s), IM_COL32(143, 200, 236, 225), label);
    Text(dl, title, 25.0f * s, ImVec2(p0.x + pad, p0.y + 28.0f * s), kText, bigValue);
    if (subLabel && *subLabel) {
        Text(dl, small, 13.0f * s, ImVec2(p0.x + pad, p1.y - 22.0f * s), kTextDim, subLabel,
             (p1.x - p0.x) - pad * 1.8f);
    }
}

const char* const kModeChip[4] = { "CAMPAIGN", "FIREFIGHT", "MULTIPLAYER", "ANY MODE" };
const ImU32 kModeColour[4] = { IM_COL32(120, 190, 255, 255), IM_COL32(255, 150, 90, 255),
                               IM_COL32(150, 220, 120, 255), IM_COL32(200, 200, 210, 255) };
const ImU32 kDoneColour = IM_COL32(132, 226, 146, 255);

void ChallengeCard(const Career::Challenge& c, float width, bool focused, bool& outHovered, bool& outClicked) {
    using namespace NobleUi;
    const float s = Scale();
    const float pad = 14.0f * s;
    ImFont* body = ImGui::GetFont();
    ImFont* small = NobleStyle::SmallFont();
    const float bodyPx = 18.0f * s, smallPx = 14.0f * s;
    const float textW = width - pad * 2.0f;
    ImFont* descFont = small ? small : body;
    const float descH = std::max(
        descFont->CalcTextSizeA(smallPx + 1.0f * s, FLT_MAX, textW, c.description.c_str()).y,
        descFont->CalcTextSizeA(smallPx + 1.0f * s, FLT_MAX, 0.0f, "A\nA").y);
    const float height = pad + smallPx + 8.0f * s + bodyPx + 4.0f * s + descH + 12.0f * s + 10.0f * s + pad;

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 p1(p0.x + width, p0.y + height);
    outClicked = ImGui::InvisibleButton("##chg_card", ImVec2(width, height));
    outHovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const int mode = std::clamp(c.mode, 0, 3);

    ImU32 bg = c.done ? IM_COL32(20, 42, 28, 222) : IM_COL32(14, 22, 32, 218);
    if (focused || outHovered) {
        bg = c.done ? IM_COL32(28, 56, 38, 236) : IM_COL32(24, 38, 54, 236);
    }
    dl->AddRectFilled(p0, p1, bg, 3.0f * s);
    dl->AddRectFilled(p0, ImVec2(p0.x + 3.5f * s, p1.y), c.done ? kDoneColour : kModeColour[mode]);
    DrawTacticalFrame(dl, p0, p1, s, focused, c.done, outHovered, true);

    float y = p0.y + pad;
    const float chipW = TextWidth(small, smallPx, kModeChip[mode]) + 14.0f * s;
    dl->AddRectFilled(ImVec2(p0.x + pad, y - 2.0f * s), ImVec2(p0.x + pad + chipW, y + smallPx + 3.0f * s),
                      IM_COL32(10, 16, 24, 180), 2.0f * s);
    dl->AddRect(ImVec2(p0.x + pad, y - 2.0f * s), ImVec2(p0.x + pad + chipW, y + smallPx + 3.0f * s),
                kModeColour[mode], 2.0f * s, 0, 1.0f);
    Text(dl, small, smallPx, ImVec2(p0.x + pad + 7.0f * s, y), kModeColour[mode], kModeChip[mode]);

    char reward[48];
    std::snprintf(reward, sizeof(reward), "+%s cR", Grouped(c.reward).c_str());
    const float rw = TextWidth(body, bodyPx, reward);
    Text(dl, body, bodyPx, ImVec2(p1.x - pad - rw, y - 2.0f * s), c.done ? kDoneColour : kAmber, reward);
    y += smallPx + 8.0f * s;

    Text(dl, body, bodyPx, ImVec2(p0.x + pad, y), c.done ? kDoneColour : kText, c.title.c_str());
    y += bodyPx + 4.0f * s;
    Text(dl, small, smallPx + 1.0f * s, ImVec2(p0.x + pad, y), kTextDim, c.description.c_str(), textW);
    y += descH + 12.0f * s;

    const float f = c.target > 0
                        ? std::clamp(static_cast<float>(c.progress) / static_cast<float>(c.target), 0.0f, 1.0f)
                        : 0.0f;
    char label[64];
    if (c.done) std::snprintf(label, sizeof(label), "COMPLETE");
    else std::snprintf(label, sizeof(label), "%s / %s", Grouped(c.progress).c_str(), Grouped(c.target).c_str());
    const float lw = TextWidth(small, smallPx, label);
    const float barX1 = p1.x - pad - lw - (c.done ? 26.0f : 12.0f) * s;
    const float barY = y + 1.0f * s, barH = 7.0f * s;
    DrawTacticalBar(dl, ImVec2(p0.x + pad, barY), ImVec2(barX1, barY + barH), f,
                    c.done ? kDoneColour : kAccent);

    const float ly = barY + barH * 0.5f - smallPx * 0.5f - 1.0f * s;
    if (c.done) {
        const float cx = p1.x - pad - lw - 16.0f * s, cy = barY + barH * 0.5f;
        dl->AddLine(ImVec2(cx, cy), ImVec2(cx + 4.0f * s, cy + 4.0f * s), kDoneColour, 2.0f * s);
        dl->AddLine(ImVec2(cx + 4.0f * s, cy + 4.0f * s), ImVec2(cx + 11.0f * s, cy - 5.0f * s), kDoneColour, 2.0f * s);
    }
    Text(dl, small, smallPx, ImVec2(p1.x - pad - lw, ly), c.done ? kDoneColour : kText, label);
}

ImU32 TierTint(int tier, bool tracked) {
    if (!tracked) return IM_COL32(150, 150, 150, 110);
    switch (tier) {
    case 0: return IM_COL32(170, 170, 170, 190);
    case 2: return IM_COL32(236, 168, 112, 255);
    case 3: return IM_COL32(226, 234, 246, 255);
    case 4: return IM_COL32(255, 214, 110, 255);
    case 5:
    case 6: return IM_COL32(120, 118, 128, 255);
    default: return IM_COL32_WHITE;
    }
}

ImU32 TierTextColour(int tier) {
    switch (tier) {
    case 1: return IM_COL32(200, 206, 214, 255);
    case 2: return IM_COL32(236, 168, 112, 255);
    case 3: return IM_COL32(226, 234, 246, 255);
    case 4: return IM_COL32(255, 214, 110, 255);
    case 5: return IM_COL32(170, 160, 200, 255);
    case 6: return IM_COL32(255, 160, 70, 255);
    default: return IM_COL32(150, 160, 170, 255);
    }
}

void CommendationRow(const Career::Commendation& c, float width, bool focused, bool& outHovered, bool& outClicked) {
    using namespace NobleUi;
    const float s = Scale();
    const float bannerH = 60.0f * s, bannerW = bannerH * (287.0f / 90.0f);
    const float height = bannerH + 28.0f * s;
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 p1(p0.x + width, p0.y + height);
    outClicked = ImGui::InvisibleButton("##comm_card", ImVec2(width, height));
    outHovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImU32 bg = IM_COL32(14, 22, 32, 212);
    if (focused || outHovered) bg = IM_COL32(24, 38, 54, 235);
    dl->AddRectFilled(p0, p1, bg, 3.0f * s);
    DrawTacticalFrame(dl, p0, p1, s, focused, c.tier >= 6, outHovered, true);

    const ImVec2 b0(p0.x + 8.0f * s, p0.y + (height - bannerH) * 0.5f);
    const ImVec2 b1(b0.x + bannerW, b0.y + bannerH);
    const UiTexture art = UiArt::ReachArt("commendations", c.art);
    if (art) UiArt::DrawContain(dl, art, b0, b1, TierTint(c.tier, c.tracked));
    else dl->AddRect(b0, b1, IM_COL32(255, 255, 255, 50));
    if (c.tier >= 6) {
        dl->AddRect(ImVec2(b0.x - 1, b0.y - 1), ImVec2(b1.x + 1, b1.y + 1), IM_COL32(255, 150, 60, 230),
                    0.0f, 0, 2.0f * s);
    }

    ImFont* body = ImGui::GetFont();
    ImFont* small = NobleStyle::SmallFont();
    const float x = b1.x + 14.0f * s, right = p1.x - 14.0f * s;
    const float bodyPx = 18.0f * s, smallPx = 14.0f * s;
    Text(dl, body, bodyPx, ImVec2(x, p0.y + 9.0f * s), c.tracked ? kText : kTextDim, c.title.c_str());

    const char* tierName = c.tier > 0 ? Career::CommendationTierName(c.tier) : "";
    char reward[48] = "";
    if (c.nextReward > 0) std::snprintf(reward, sizeof(reward), "+%s cR", Grouped(c.nextReward).c_str());
    const float rw = TextWidth(small, smallPx, reward);
    Text(dl, small, smallPx, ImVec2(right - rw, p0.y + 11.0f * s), kAmber, reward);
    const float tw = TextWidth(small, smallPx, tierName);
    Text(dl, small, smallPx, ImVec2(right - rw - (rw > 0 ? 12.0f * s : 0.0f) - tw, p0.y + 11.0f * s),
         TierTextColour(c.tier), tierName);
    Text(dl, small, smallPx, ImVec2(x, p0.y + 9.0f * s + bodyPx + 3.0f * s), kTextDim, c.description.c_str(),
         right - x);

    const float barY = p1.y - 17.0f * s, barH = 6.0f * s;
    if (!c.tracked) {
        const char* note = "Not tracked yet";
        Text(dl, small, smallPx, ImVec2(right - TextWidth(small, smallPx, note), barY - 6.0f * s), kTextOff, note);
        return;
    }
    char label[64];
    float f = 1.0f;
    if (c.nextTarget > 0) {
        std::snprintf(label, sizeof(label), "%s / %s", Grouped(c.value).c_str(), Grouped(c.nextTarget).c_str());
        f = static_cast<float>(c.value) / static_cast<float>(std::max<long long>(1, c.nextTarget));
    } else {
        std::snprintf(label, sizeof(label), "%s  MAX", Grouped(c.value).c_str());
    }
    const float lw = TextWidth(small, smallPx, label);
    const float barX1 = right - lw - 12.0f * s;
    DrawTacticalBar(dl, ImVec2(x, barY), ImVec2(barX1, barY + barH), f,
                    c.tier >= 6 ? IM_COL32(255, 150, 60, 255) : kAccent);
    Text(dl, small, smallPx, ImVec2(right - lw, barY + barH * 0.5f - smallPx * 0.5f - 1.0f * s), kText, label);
}

} // namespace

void CareerPanel::Render() {
    using namespace NobleUi;
    const float s = Scale();

    static const char* const kTabs[] = { "SERVICE RECORD", "CHALLENGES", "COMMENDATIONS", "RANKS" };
    if (!ImGui::GetIO().WantTextInput) {
        if (UiPad::TabPrev() || ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
            m_tab = (m_tab + 3) % 4;
            MenuSounds::Tab(-1);
        }
        if (UiPad::TabNext() || ImGui::IsKeyPressed(ImGuiKey_E, false)) {
            m_tab = (m_tab + 1) % 4;
            MenuSounds::Tab(1);
        }
    }

    const float availW = ImGui::GetContentRegionAvail().x;
    const float tabH = 38.0f * s;
    const float gap = 8.0f * s;
    const float tabW = std::min(210.0f * s, (availW - gap * 3.0f) / 4.0f);
    const ImVec2 tabOrigin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    for (int i = 0; i < 4; ++i) {
        ImGui::PushID(i);
        const ImVec2 t0(tabOrigin.x + static_cast<float>(i) * (tabW + gap), tabOrigin.y);
        const ImVec2 t1(t0.x + tabW, t0.y + tabH);
        const bool sel = (m_tab == i);
        ImGui::SetCursorScreenPos(t0);
        if (DrawTile(dl, "##career_tab", t0, t1, kTabs[i], nullptr, sel, sel, true, nullptr)) {
            if (m_tab != i) MenuSounds::Tab(i > m_tab ? 1 : -1);
            m_tab = i;
        }
        ImGui::PopID();
    }

    const float ruleY = tabOrigin.y + tabH + 8.0f * s;
    dl->AddLine(ImVec2(tabOrigin.x, ruleY), ImVec2(tabOrigin.x + availW, ruleY),
                IM_COL32(143, 216, 255, 55), 1.0f);
    ImGui::SetCursorScreenPos(ImVec2(tabOrigin.x, ruleY + 8.0f * s));

    ImGui::BeginChild("##careerBody", ImVec2(0, 0), ImGuiChildFlags_None);
    // D-pad / Left Stick navigates tiles in all 4 tabs; Right Stick smoothly scrolls everywhere,
    // and LT/RT page-scrolls Record / Challenges / Ranks while switching modes on Commendations.
    UiPad::ScrollWindow(false, m_tab != 2);
    switch (m_tab) {
    case 0: RenderRecord(); break;
    case 1: RenderChallenges(); break;
    case 2: RenderCommendations(); break;
    default: RenderRanks(); break;
    }
    ImGui::EndChild();
}

void CareerPanel::RenderRecord() {
    using namespace NobleUi;
    const float s = Scale();
    const Career::Record r = Career::GetRecord();
    const auto& ranks = Career::Ranks();
    const int rank = Career::RankIndex(r.totals.credits);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float avail = ImGui::GetContentRegionAvail().x;

    // 1. Hero Rank Banner Card
    {
        const float heroH = 124.0f * s;
        const ImVec2 h0 = ImGui::GetCursorScreenPos();
        const ImVec2 h1(h0.x + avail, h0.y + heroH);
        dl->AddRectFilled(h0, h1, IM_COL32(14, 24, 36, 225), 3.0f * s);
        dl->AddRectFilled(h0, ImVec2(h0.x + 4.0f * s, h1.y), kAccent);
        DrawTacticalFrame(dl, h0, h1, s, false, true, false, false);

        const float iconBox = 88.0f * s;
        const ImVec2 ib0(h0.x + 18.0f * s, h0.y + (heroH - iconBox) * 0.5f);
        const ImVec2 ib1(ib0.x + iconBox, ib0.y + iconBox);
        dl->AddRectFilled(ib0, ib1, IM_COL32(8, 14, 22, 210), 4.0f * s);
        dl->AddRect(ib0, ib1, IM_COL32(143, 216, 255, 90), 4.0f * s);
        if (const UiTexture icon = UiArt::RankIcon(rank)) {
            UiArt::DrawContain(dl, icon, ImVec2(ib0.x + 6.0f * s, ib0.y + 6.0f * s),
                               ImVec2(ib1.x - 6.0f * s, ib1.y - 6.0f * s));
        }

        const float cx = ib1.x + 20.0f * s;
        const float rightBoxW = avail > 760.0f * s ? 290.0f * s : 0.0f;
        const float midRight = h1.x - rightBoxW - 22.0f * s;

        char rankNum[32];
        std::snprintf(rankNum, sizeof(rankNum), "RANK %d OF %d", rank + 1, static_cast<int>(ranks.size()));
        Text(dl, NobleStyle::SmallFont(), 13.0f * s, ImVec2(cx, h0.y + 14.0f * s), kAccent, rankNum);
        Text(dl, NobleStyle::TitleFont(), 26.0f * s, ImVec2(cx, h0.y + 30.0f * s), kText, ranks[rank].name);

        const std::string crText = Grouped(r.totals.credits) + " cR  TOTAL";
        Text(dl, NobleStyle::SmallFont(), 15.0f * s, ImVec2(cx, h0.y + 62.0f * s), kAmber, crText.c_str());

        if (rank + 1 < static_cast<int>(ranks.size())) {
            const long long lo = ranks[rank].credits, hi = ranks[rank + 1].credits;
            const float prog = hi > lo ? static_cast<float>(r.totals.credits - lo) / static_cast<float>(hi - lo) : 1.0f;
            const ImVec2 b0(cx, h0.y + 90.0f * s);
            const ImVec2 b1(std::max(cx + 140.0f * s, midRight), b0.y + 10.0f * s);
            DrawTacticalBar(dl, b0, b1, prog, kAccent);

            char pctBuf[48];
            std::snprintf(pctBuf, sizeof(pctBuf), "%d%% TO NEXT RANK", static_cast<int>(std::clamp(prog, 0.0f, 1.0f) * 100.0f));
            Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImVec2(cx, b1.y + 4.0f * s), kTextDim, pctBuf);
        } else {
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(cx, h0.y + 88.0f * s), kDoneColour,
                 "MAXIMUM SERVICE RANK REACHED");
        }

        if (rightBoxW > 0.0f) {
            const ImVec2 nb0(h1.x - rightBoxW - 14.0f * s, h0.y + 16.0f * s);
            const ImVec2 nb1(h1.x - 14.0f * s, h1.y - 16.0f * s);
            dl->AddRectFilled(nb0, nb1, IM_COL32(8, 14, 22, 190), 3.0f * s);
            dl->AddRect(nb0, nb1, IM_COL32(143, 216, 255, 65), 3.0f * s);
            if (rank + 1 < static_cast<int>(ranks.size())) {
                const long long need = std::max<long long>(0, ranks[rank + 1].credits - r.totals.credits);
                const float nextSz = 56.0f * s;
                const ImVec2 ni0(nb0.x + 12.0f * s, (nb0.y + nb1.y - nextSz) * 0.5f);
                if (const UiTexture nextIcon = UiArt::RankIcon(rank + 1)) {
                    UiArt::DrawContain(dl, nextIcon, ni0, ImVec2(ni0.x + nextSz, ni0.y + nextSz));
                }
                const float tx = ni0.x + nextSz + 12.0f * s;
                Text(dl, NobleStyle::SmallFont(), 12.0f * s, ImVec2(tx, nb0.y + 12.0f * s), kAccent, "NEXT PROMOTION");
                Text(dl, NobleStyle::SmallFont(), 15.0f * s, ImVec2(tx, nb0.y + 30.0f * s), kText, ranks[rank + 1].name,
                     nb1.x - tx - 10.0f * s);
                Text(dl, NobleStyle::SmallFont(), 13.5f * s, ImVec2(tx, nb1.y - 26.0f * s), kAmber,
                     (Grouped(need) + " cR REMAINING").c_str());
            } else {
                Text(dl, NobleStyle::TitleFont(), 17.0f * s, ImVec2(nb0.x + 18.0f * s, nb0.y + 34.0f * s),
                     kAmber, "INHERITOR ACHIEVED");
            }
        }
        ImGui::Dummy(ImVec2(avail, heroH + 14.0f * s));
    }

    // Build navigable linear item layout across KPI Tiles + Campaign Cards + Recent Match Rows
    static int s_totalRecordItems = 0;
    const int prevTotal = s_totalRecordItems;
    bool navMoved = false;

    const int kpiCols = avail >= 860.0f * s ? 3 : 2;
    const int campCols = avail >= 900.0f * s ? 3 : (avail >= 560.0f * s ? 2 : 1);
    const int campCount = static_cast<int>(r.campaignBest.size());
    const int histCount = static_cast<int>(r.history.size());
    const int totalItems = 6 + campCount + histCount;

    if (prevTotal > 0) m_recordFocus = std::clamp(m_recordFocus, 0, std::max(0, totalItems - 1));
    if (totalItems > 0 && !ImGui::GetIO().WantTextInput) {
        auto stepColsFor = [&](int idx) -> int {
            if (idx < 6) return kpiCols;
            if (idx < 6 + campCount) return campCols;
            return 1;
        };
        if (UiPad::Left() && m_recordFocus > 0) {
            --m_recordFocus;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Right() && m_recordFocus + 1 < totalItems) {
            ++m_recordFocus;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Up()) {
            const int step = stepColsFor(m_recordFocus);
            if (m_recordFocus - step >= 0) m_recordFocus -= step;
            else m_recordFocus = 0;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Down()) {
            const int step = stepColsFor(m_recordFocus);
            if (m_recordFocus + step < totalItems) m_recordFocus += step;
            else m_recordFocus = totalItems - 1;
            navMoved = true;
            MenuSounds::Scroll();
        }
    }

    int itemIdx = 0;

    // 2. Lifetime Telemetry KPI Tile Grid
    SectionHeader("SERVICE TOTALS", avail);
    {
        const float gap = 10.0f * s;
        const float tileW = (avail - gap * static_cast<float>(kpiCols - 1)) / static_cast<float>(kpiCols);
        const float tileH = 88.0f * s;

        struct KpiDef {
            const char* label;
            std::string value;
            std::string sub;
            ImU32 accent;
        };
        char gamesSub[96], kdaSub[96], combatSub[96], kdVal[32];
        std::snprintf(gamesSub, sizeof(gamesSub), "CP %s  |  FF %s  |  MP %s",
                      Grouped(r.games[0]).c_str(), Grouped(r.games[1]).c_str(), Grouped(r.games[2]).c_str());
        std::snprintf(kdVal, sizeof(kdVal), "%.2f",
                      r.totals.deaths > 0 ? static_cast<double>(r.totals.kills) / r.totals.deaths
                                          : static_cast<double>(r.totals.kills));
        std::snprintf(kdaSub, sizeof(kdaSub), "%s Kills  /  %s Deaths",
                      Grouped(r.totals.kills).c_str(), Grouped(r.totals.deaths).c_str());
        std::snprintf(combatSub, sizeof(combatSub), "Deaths %s  |  Assists %s",
                      Grouped(r.totals.deaths).c_str(), Grouped(r.totals.assists).c_str());

        const KpiDef kpis[6] = {
            { "GAMES FINISHED", Grouped(r.totals.gamesPlayed), gamesSub, kAccent },
            { "TIME PLAYED", HoursMinutes(r.totals.minutesPlayed), "Total Active Time", IM_COL32(120, 210, 170, 255) },
            { "KILL / DEATH RATIO", kdVal, kdaSub, kAmber },
            { "TOTAL KILLS", Grouped(r.totals.kills), combatSub, IM_COL32(235, 110, 110, 255) },
            { "MEDALS & BEST GAME", Grouped(r.totals.medals) + " Medals",
              "Best Match: " + Grouped(r.bestMatchCredits) + " cR", IM_COL32(175, 155, 240, 255) },
            { "CHALLENGES DONE", Grouped(r.challengesCompleted), "Daily & Weekly Completed", kDoneColour }
        };

        for (int k = 0; k < 6; ++k) {
            if (k % kpiCols != 0) ImGui::SameLine(0, gap);
            ImGui::PushID(1000 + k);
            const ImVec2 t0 = ImGui::GetCursorScreenPos();
            const ImVec2 t1(t0.x + tileW, t0.y + tileH);
            if (ImGui::InvisibleButton("##kpi", ImVec2(tileW, tileH))) {
                m_recordFocus = itemIdx;
                MenuSounds::Click();
            }
            const bool hov = ImGui::IsItemHovered();
            if (hov && !navMoved) m_recordFocus = itemIdx;
            const bool foc = (itemIdx == m_recordFocus);
            DrawKpiTile(dl, t0, t1, kpis[k].label, kpis[k].value.c_str(), kpis[k].sub.c_str(),
                        kpis[k].accent, foc, hov);
            if (foc && navMoved) ImGui::SetScrollHereY(0.4f);
            ImGui::PopID();
            ++itemIdx;
        }
        ImGui::Dummy(ImVec2(0, 4.0f * s));
        Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImGui::GetCursorScreenPos(), kTextOff,
             "Note: Kills, deaths and medals come from the game's results; their exact slots are still being verified.");
        ImGui::Dummy(ImVec2(avail, 20.0f * s));
    }

    // 3. Campaign Progress Card Grid
    if (!r.campaignBest.empty()) {
        SectionHeader("CAMPAIGN COMPLETION", avail);
        const float gap = 10.0f * s;
        const float cardW = (avail - gap * static_cast<float>(campCols - 1)) / static_cast<float>(campCols);
        const float cardH = 56.0f * s;
        int col = 0;
        for (const auto& [map, difficulty] : r.campaignBest) {
            if (col > 0) ImGui::SameLine(0, gap);
            ImGui::PushID(2000 + col + itemIdx);
            const ImVec2 c0 = ImGui::GetCursorScreenPos();
            const ImVec2 c1(c0.x + cardW, c0.y + cardH);
            if (ImGui::InvisibleButton("##camp_card", ImVec2(cardW, cardH))) {
                m_recordFocus = itemIdx;
                MenuSounds::Click();
            }
            const bool hov = ImGui::IsItemHovered();
            if (hov && !navMoved) m_recordFocus = itemIdx;
            const bool foc = (itemIdx == m_recordFocus);

            const std::string name = MapNames::Display(map);
            const ImU32 diffCol = DifficultyColour(difficulty);
            dl->AddRectFilled(c0, c1, foc ? IM_COL32(22, 36, 52, 230) : IM_COL32(12, 20, 30, 212), 3.0f * s);
            dl->AddRectFilled(c0, ImVec2(c0.x + 3.5f * s, c1.y), diffCol);
            DrawTacticalFrame(dl, c0, c1, s, foc, false, hov, true);

            Text(dl, NobleStyle::TitleFont(), 17.0f * s, ImVec2(c0.x + 14.0f * s, c0.y + 8.0f * s), kText,
                 name.c_str());
            Text(dl, NobleStyle::SmallFont(), 13.0f * s, ImVec2(c0.x + 14.0f * s, c0.y + 30.0f * s), diffCol,
                 (std::string("Completed on ") + DifficultyName(difficulty)).c_str());
            if (name != map && hov) ImGui::SetTooltip("%s", map.c_str());
            if (foc && navMoved) ImGui::SetScrollHereY(0.45f);
            ImGui::PopID();
            col = (col + 1) % campCols;
            ++itemIdx;
        }
        ImGui::Dummy(ImVec2(0, 8.0f * s));
    }

    // 4. Recent Matches Tactical Card Rows
    SectionHeader("RECENT MATCHES", avail);
    if (r.history.empty()) {
        const ImVec2 e0 = ImGui::GetCursorScreenPos();
        const ImVec2 e1(e0.x + avail, e0.y + 56.0f * s);
        dl->AddRectFilled(e0, e1, IM_COL32(12, 18, 28, 175), 3.0f * s);
        DrawTacticalFrame(dl, e0, e1, s, false, false, false, false);
        Text(dl, NobleStyle::SmallFont(), 14.5f * s, ImVec2(e0.x + 16.0f * s, e0.y + 18.0f * s), kTextDim,
             "Finish a match in Campaign, Firefight, or Multiplayer to start recording your match history.");
        ImGui::Dummy(ImVec2(avail, 60.0f * s));
    } else {
        // Header strip
        {
            const ImVec2 hr = ImGui::GetCursorScreenPos();
            Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImVec2(hr.x + 14.0f * s, hr.y), kTextDim, "DATE");
            Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImVec2(hr.x + avail * 0.18f, hr.y), kTextDim, "MODE");
            Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImVec2(hr.x + avail * 0.36f, hr.y), kTextDim, "MAP");
            Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImVec2(hr.x + avail * 0.57f, hr.y), kTextDim, "TIME");
            Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImVec2(hr.x + avail * 0.67f, hr.y), kTextDim, "K / D / A");
            Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImVec2(hr.x + avail * 0.79f, hr.y), kTextDim, "SCORE");
            Text(dl, NobleStyle::SmallFont(), 12.5f * s, ImVec2(hr.x + avail * 0.89f, hr.y), kTextDim, "CREDITS");
            ImGui::Dummy(ImVec2(avail, 20.0f * s));
        }

        auto modeName = [](const std::string& type) -> const char* {
            if (type == "campaign") return "Campaign";
            if (type == "firefight") return "Firefight";
            if (type == "multiplayer") return "Multiplayer";
            if (type == "forge") return "Forge";
            return nullptr;
        };

        const float rowH = 40.0f * s;
        for (size_t hi = 0; hi < r.history.size(); ++hi) {
            const Career::MatchSummary& m = r.history[hi];
            ImGui::PushID(3000 + static_cast<int>(hi));
            const ImVec2 m0 = ImGui::GetCursorScreenPos();
            const ImVec2 m1(m0.x + avail, m0.y + rowH);
            if (ImGui::InvisibleButton("##match_row", ImVec2(avail, rowH))) {
                m_recordFocus = itemIdx;
                MenuSounds::Click();
            }
            const bool hov = ImGui::IsItemHovered();
            if (hov && !navMoved) m_recordFocus = itemIdx;
            const bool foc = (itemIdx == m_recordFocus);

            dl->AddRectFilled(m0, m1,
                              foc ? IM_COL32(22, 36, 52, 232)
                                  : (hi % 2 == 0 ? IM_COL32(12, 20, 30, 205) : IM_COL32(15, 24, 36, 205)),
                              2.0f * s);
            DrawTacticalFrame(dl, m0, m1, s, foc, false, hov, true);

            const float ty = m0.y + (rowH - 14.0f * s) * 0.5f;
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(m0.x + 14.0f * s, ty), kTextDim, m.date.c_str());

            const char* modeLabel = modeName(m.type);
            std::string modeText = modeLabel ? std::string(modeLabel) : m.type;
            if (m.difficulty >= 0 && m.type != "multiplayer") {
                modeText += std::string(" / ") + DifficultyName(m.difficulty);
            }
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(m0.x + avail * 0.18f, ty), kText, modeText.c_str());

            const std::string mapName = MapNames::Display(m.map);
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(m0.x + avail * 0.36f, ty), kText, mapName.c_str());
            if (mapName != m.map && hov) ImGui::SetTooltip("%s", m.map.c_str());

            char durBuf[24], kdaBuf[48], scrBuf[32];
            std::snprintf(durBuf, sizeof(durBuf), "%.0f min", m.minutes);
            std::snprintf(kdaBuf, sizeof(kdaBuf), "%lld / %lld / %lld",
                          static_cast<long long>(m.kills), static_cast<long long>(m.deaths),
                          static_cast<long long>(m.assists));
            std::snprintf(scrBuf, sizeof(scrBuf), "%lld", static_cast<long long>(m.score));
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(m0.x + avail * 0.57f, ty), kTextDim, durBuf);
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(m0.x + avail * 0.67f, ty), kText, kdaBuf);
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(m0.x + avail * 0.79f, ty), kText, scrBuf);
            Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(m0.x + avail * 0.89f, ty), kAmber,
                 ("+" + Grouped(m.credits) + " cR").c_str());

            if (foc && navMoved) ImGui::SetScrollHereY(0.45f);
            ImGui::PopID();
            ++itemIdx;
        }
    }

    s_totalRecordItems = itemIdx;
}

void CareerPanel::RenderChallenges() {
    using namespace NobleUi;
    const std::vector<Career::Challenge> list = Career::Challenges();
    const float s = Scale();
    const float avail = ImGui::GetContentRegionAvail().x;
    const int columns = avail >= 820.0f * s ? 2 : 1;
    const float gap = 12.0f * s;
    const float cardW = (avail - gap * static_cast<float>(columns - 1)) / static_cast<float>(columns) - 1.0f;

    const int total = static_cast<int>(list.size());
    if (total > 0) m_challengeFocus = std::clamp(m_challengeFocus, 0, total - 1);
    bool navMoved = false;
    if (total > 0 && !ImGui::GetIO().WantTextInput) {
        if (UiPad::Left() && m_challengeFocus > 0) {
            --m_challengeFocus;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Right() && m_challengeFocus + 1 < total) {
            ++m_challengeFocus;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Up() && m_challengeFocus - columns >= 0) {
            m_challengeFocus -= columns;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Down() && m_challengeFocus + columns < total) {
            m_challengeFocus += columns;
            navMoved = true;
            MenuSounds::Scroll();
        }
    }

    ImGui::Dummy(ImVec2(0, U(2)));
    Text(ImGui::GetWindowDrawList(), NobleStyle::SmallFont(), 14.0f * s, ImGui::GetCursorScreenPos(), kTextDim,
         "Halo: Reach challenges track progress across finished matches. Rewards pay out on the post-game screen.");
    ImGui::Dummy(ImVec2(avail, 22.0f * s));

    int cardIdx = 0;
    for (int weekly = 0; weekly < 2; ++weekly) {
        int done = 0, count = 0;
        for (const Career::Challenge& c : list) {
            if (c.weekly != (weekly == 1)) continue;
            ++count;
            if (c.done) ++done;
        }
        const float headerY = ImGui::GetCursorScreenPos().y;
        char head[64];
        std::snprintf(head, sizeof(head), "%s  %d / %d", weekly ? "WEEKLY CHALLENGES" : "DAILY CHALLENGES", done, count);
        const std::string reset = weekly ? Career::WeeklyResetText() : Career::DailyResetText();
        ImFont* small = NobleStyle::SmallFont();
        const float rw = TextWidth(small, 14.0f * s, reset.c_str());
        const float left = ImGui::GetCursorScreenPos().x;
        SectionHeader(head, avail - rw - 14.0f * s);
        Text(ImGui::GetWindowDrawList(), small, 14.0f * s, ImVec2(left + avail - rw, headerY + 8.0f * s), kTextDim, reset.c_str());

        int col = 0;
        for (const Career::Challenge& c : list) {
            if (c.weekly != (weekly == 1)) continue;
            ImGui::PushID(&c);
            if (col > 0) ImGui::SameLine(0, gap);
            const bool focused = (cardIdx == m_challengeFocus);
            bool hovered = false, clicked = false;
            ImGui::BeginGroup();
            ChallengeCard(c, cardW, focused, hovered, clicked);
            ImGui::EndGroup();
            if (clicked || (hovered && !navMoved)) {
                m_challengeFocus = cardIdx;
                if (clicked) MenuSounds::Click();
            }
            if (focused && navMoved) ImGui::SetScrollHereY(0.45f);
            ImGui::PopID();
            ++cardIdx;
            col = (col + 1) % columns;
            if (col == 0) ImGui::Dummy(ImVec2(0, gap - ImGui::GetStyle().ItemSpacing.y));
        }
        if (col != 0) ImGui::Dummy(ImVec2(0, gap - ImGui::GetStyle().ItemSpacing.y));
    }
}

void CareerPanel::RenderCommendations() {
    using namespace NobleUi;
    const std::vector<Career::Commendation> list = Career::Commendations();
    const float s = Scale();
    static const char* const kModeTitle[3] = { "CAMPAIGN", "FIREFIGHT", "MULTIPLAYER" };
    static const ImU32 kTabModeColour[3] = { IM_COL32(150, 170, 90, 255), IM_COL32(200, 80, 60, 255),
                                             IM_COL32(80, 140, 200, 255) };

    ImGui::Dummy(ImVec2(0, U(4)));
    if (!ImGui::GetIO().WantTextInput) {
        if (UiPad::PageUp()) {
            m_commendTab = (m_commendTab + 2) % 3;
            m_commendFocus = 0;
            MenuSounds::Tab(-1);
        }
        if (UiPad::PageDown()) {
            m_commendTab = (m_commendTab + 1) % 3;
            m_commendFocus = 0;
            MenuSounds::Tab(1);
        }
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    // Draw LT pill before mode tabs
    if (InputPrompts::Current() == InputPrompts::Device::Gamepad) {
        const ImVec2 gp = ImGui::GetCursorScreenPos();
        const float gh = 22.0f * s;
        const float gw = InputPrompts::DrawButton(dl, NobleStyle::SmallFont(),
                                                  ImVec2(gp.x, gp.y + 7.0f * s), gh, InputPrompts::Button::LT);
        ImGui::Dummy(ImVec2(gw + 6.0f * s, 36.0f * s));
        ImGui::SameLine(0, 4.0f * s);
    }

    for (int mode = 0; mode < 3; ++mode) {
        int tiers = 0, count = 0;
        for (const Career::Commendation& c : list) if (c.mode == mode) { ++count; tiers += c.tier; }
        char label[64];
        std::snprintf(label, sizeof(label), "%s  %d/%d", kModeTitle[mode], tiers, count * 6);
        const float w = TextWidth(nullptr, 17.0f * s, label) + 38.0f * s, h = 36.0f * s;
        if (mode) ImGui::SameLine(0, 8.0f * s);
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImGui::PushID(mode);
        if (ImGui::InvisibleButton("##mode", ImVec2(w, h)) && m_commendTab != mode) {
            MenuSounds::Tab(mode > m_commendTab ? 1 : -1);
            m_commendTab = mode;
            m_commendFocus = 0;
        }
        const bool sel = m_commendTab == mode, hov = ImGui::IsItemHovered();
        ImGui::PopID();
        const ImVec2 p1(p0.x + w, p0.y + h);
        dl->AddRectFilled(p0, p1, sel ? IM_COL32(28, 42, 58, 245) : IM_COL32(16, 24, 34, hov ? 235 : 195), 3.0f * s);
        dl->AddRectFilled(p0, ImVec2(p0.x + 4.0f * s, p1.y), kTabModeColour[mode]);
        DrawTacticalFrame(dl, p0, p1, s, sel, sel, hov, false);
        Text(dl, nullptr, 17.0f * s, ImVec2(p0.x + 18.0f * s, p0.y + h * 0.5f - 9.0f * s),
             sel ? kText : kTextDim, label);
    }

    if (InputPrompts::Current() == InputPrompts::Device::Gamepad) {
        ImGui::SameLine(0, 8.0f * s);
        const ImVec2 gp = ImGui::GetCursorScreenPos();
        const float gh = 22.0f * s;
        const float gw = InputPrompts::DrawButton(dl, NobleStyle::SmallFont(),
                                                  ImVec2(gp.x, gp.y + 7.0f * s), gh, InputPrompts::Button::RT);
        ImGui::Dummy(ImVec2(gw + 4.0f * s, 36.0f * s));
    }

    ImGui::Dummy(ImVec2(0, U(4)));
    Text(dl, NobleStyle::SmallFont(), 13.5f * s, ImGui::GetCursorScreenPos(), kTextDim,
         "Each tier (Steel, Bronze, Silver, Gold, Onyx, Max) pays cR once on the post-game screen. Use LT / RT to switch modes.");
    ImGui::Dummy(ImVec2(0, U(22)));

    std::vector<const Career::Commendation*> filtered;
    for (const Career::Commendation& c : list) {
        if (c.mode == m_commendTab) filtered.push_back(&c);
    }

    const float avail = ImGui::GetContentRegionAvail().x;
    const int columns = avail >= 1000.0f * s ? 2 : 1;
    const int total = static_cast<int>(filtered.size());
    if (total > 0) m_commendFocus = std::clamp(m_commendFocus, 0, total - 1);
    bool navMoved = false;
    if (total > 0 && !ImGui::GetIO().WantTextInput) {
        if (UiPad::Left() && m_commendFocus > 0) {
            --m_commendFocus;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Right() && m_commendFocus + 1 < total) {
            ++m_commendFocus;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Up() && m_commendFocus - columns >= 0) {
            m_commendFocus -= columns;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Down() && m_commendFocus + columns < total) {
            m_commendFocus += columns;
            navMoved = true;
            MenuSounds::Scroll();
        }
    }

    const float gap = 10.0f * s;
    const float rowW = (avail - gap * static_cast<float>(columns - 1)) / static_cast<float>(columns) - 1.0f;
    int col = 0;
    for (int i = 0; i < total; ++i) {
        const Career::Commendation& c = *filtered[i];
        ImGui::PushID(c.art.c_str());
        if (col > 0) ImGui::SameLine(0, gap);
        const bool focused = (i == m_commendFocus);
        bool hovered = false, clicked = false;
        ImGui::BeginGroup();
        CommendationRow(c, rowW, focused, hovered, clicked);
        ImGui::EndGroup();
        if (clicked || (hovered && !navMoved)) {
            m_commendFocus = i;
            if (clicked) MenuSounds::Click();
        }
        if (focused && navMoved) ImGui::SetScrollHereY(0.45f);
        ImGui::PopID();
        col = (col + 1) % columns;
        if (col == 0) ImGui::Dummy(ImVec2(0, gap - ImGui::GetStyle().ItemSpacing.y));
    }
}

void CareerPanel::RenderRanks() {
    using namespace NobleUi;
    const float s = Scale();
    const Career::Totals t = Career::Get();
    const auto& ranks = Career::Ranks();
    const int totalRanks = static_cast<int>(ranks.size());
    const int curRank = Career::RankIndex(t.credits);
    if (m_rankFocus < 0 || m_rankFocus >= totalRanks) m_rankFocus = curRank;

    const float avail = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Compute responsive grid columns upfront so D-pad Up/Down steps exact columns
    float widest = 0.0f;
    for (const auto& r : ranks) {
        std::string n = r.name;
        const size_t g = n.find(" Grade ");
        if (g != std::string::npos) n = n.substr(0, g) + " G" + n.substr(g + 7);
        widest = std::max(widest, ImGui::CalcTextSize(n.c_str()).x);
        widest = std::max(widest, ImGui::CalcTextSize((Grouped(r.credits) + " cR").c_str()).x);
    }
    const float cell = std::max(122.0f * s, widest + 20.0f * s), iconSize = 62.0f * s;
    const float gridGap = 6.0f * s;
    const int perRow = std::max(1, static_cast<int>((avail - 16.0f * s) / (cell + gridGap)));

    bool navMoved = false;
    if (totalRanks > 0 && !ImGui::GetIO().WantTextInput) {
        if (UiPad::Left() && m_rankFocus > 0) {
            --m_rankFocus;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Right() && m_rankFocus + 1 < totalRanks) {
            ++m_rankFocus;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Up() && m_rankFocus - perRow >= 0) {
            m_rankFocus -= perRow;
            navMoved = true;
            MenuSounds::Scroll();
        } else if (UiPad::Down() && m_rankFocus + perRow < totalRanks) {
            m_rankFocus += perRow;
            navMoved = true;
            MenuSounds::Scroll();
        }
    }

    // 1. Pinned Selected Rank Inspector Card
    {
        const auto& selRank = ranks[m_rankFocus];
        const bool earned = (m_rankFocus <= curRank);
        const bool isCurrent = (m_rankFocus == curRank);
        const float inspH = 92.0f * s;
        const ImVec2 i0 = ImGui::GetCursorScreenPos();
        const ImVec2 i1(i0.x + avail, i0.y + inspH);
        dl->AddRectFilled(i0, i1, IM_COL32(14, 24, 36, 232), 3.0f * s);
        dl->AddRectFilled(i0, ImVec2(i0.x + 4.0f * s, i1.y),
                          isCurrent ? kAccent : (earned ? kDoneColour : kAmber));
        DrawTacticalFrame(dl, i0, i1, s, false, isCurrent, false, false);

        const float imgSz = 68.0f * s;
        const ImVec2 img0(i0.x + 16.0f * s, i0.y + (inspH - imgSz) * 0.5f);
        const ImVec2 img1(img0.x + imgSz, img0.y + imgSz);
        dl->AddRectFilled(img0, img1, IM_COL32(8, 14, 22, 210), 4.0f * s);
        if (const UiTexture icon = UiArt::RankIcon(m_rankFocus)) {
            UiArt::DrawContain(dl, icon, img0, img1, earned ? IM_COL32_WHITE : IM_COL32(255, 255, 255, 210));
        }

        const float tx = img1.x + 16.0f * s;
        char tierTag[48];
        std::snprintf(tierTag, sizeof(tierTag), "RANK %d OF %d", m_rankFocus + 1, totalRanks);
        Text(dl, NobleStyle::SmallFont(), 13.0f * s, ImVec2(tx, i0.y + 12.0f * s), kAccent, tierTag);
        Text(dl, NobleStyle::TitleFont(), 24.0f * s, ImVec2(tx, i0.y + 28.0f * s), kText, selRank.name);
        Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(tx, i0.y + 58.0f * s), kTextDim,
             ("Threshold: " + Grouped(selRank.credits) + " cR").c_str());

        // Right-aligned Status / Remaining cR pill
        std::string statusLbl;
        ImU32 statusCol = kDoneColour;
        if (isCurrent) {
            statusLbl = "CURRENT SERVICE RANK";
            statusCol = kAccent;
        } else if (earned) {
            statusLbl = "RANK EARNED";
            statusCol = kDoneColour;
        } else {
            statusLbl = Grouped(std::max<long long>(0, selRank.credits - t.credits)) + " cR TO UNLOCK";
            statusCol = kAmber;
        }
        const float sw = TextWidth(NobleStyle::TitleFont(), 17.0f * s, statusLbl.c_str());
        Text(dl, NobleStyle::TitleFont(), 17.0f * s,
             ImVec2(i1.x - 18.0f * s - sw, i0.y + (inspH - 18.0f * s) * 0.5f), statusCol, statusLbl.c_str());

        ImGui::Dummy(ImVec2(avail, inspH + 10.0f * s));
    }

    // 2. Scrollable 50-Rank Grid
    ImGui::BeginChild("##ranks_grid", ImVec2(0, 0), ImGuiChildFlags_None);
    UiPad::ScrollWindow(false, true);
    ImDrawList* gdl = ImGui::GetWindowDrawList();

    for (int i = 0; i < totalRanks; ++i) {
        if (i % perRow) ImGui::SameLine(0, gridGap);
        ImGui::PushID(4000 + i);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const float tileH = cell + 10.0f * s;
        const ImVec2 p1(p.x + cell, p.y + tileH);

        if (ImGui::InvisibleButton("##rank_tile", ImVec2(cell, tileH))) {
            m_rankFocus = i;
            MenuSounds::Click();
        }
        const bool hov = ImGui::IsItemHovered();
        if (hov && !navMoved) m_rankFocus = i;

        const bool earned = i <= curRank, next = i == curRank + 1, current = i == curRank;
        const bool focused = (i == m_rankFocus);
        const ImU32 tint = earned ? IM_COL32_WHITE : (next ? IM_COL32(255, 255, 255, 200) : IM_COL32(255, 255, 255, 85));

        ImU32 bg = earned ? IM_COL32(16, 26, 38, 205) : IM_COL32(10, 16, 24, 170);
        if (current) bg = IM_COL32(20, 42, 62, 232);
        if (focused || hov) bg = IM_COL32(26, 44, 64, 240);
        gdl->AddRectFilled(p, p1, bg, 3.0f * s);
        DrawTacticalFrame(gdl, p, p1, s, focused, current || next, hov, true);

        const ImVec2 i0(p.x + (cell - iconSize) * 0.5f, p.y + 8.0f * s);
        const UiTexture icon = UiArt::RankIcon(i);
        if (icon) UiArt::DrawContain(gdl, icon, i0, ImVec2(i0.x + iconSize, i0.y + iconSize), tint);
        else gdl->AddRect(i0, ImVec2(i0.x + iconSize, i0.y + iconSize), IM_COL32(255, 255, 255, earned ? 120 : 40));

        std::string name = ranks[i].name;
        const size_t g = name.find(" Grade ");
        if (g != std::string::npos) name = name.substr(0, g) + " G" + name.substr(g + 7);
        const std::string cr = Grouped(ranks[i].credits) + " cR";
        const ImU32 textCol = earned ? IM_COL32(236, 242, 248, 255) : IM_COL32(150, 162, 174, next ? 230 : 155);
        const ImVec2 nw = ImGui::CalcTextSize(name.c_str());
        gdl->AddText(ImVec2(p.x + (cell - nw.x) * 0.5f, p.y + 12.0f * s + iconSize), textCol, name.c_str());
        const ImVec2 cw = ImGui::CalcTextSize(cr.c_str());
        gdl->AddText(ImVec2(p.x + (cell - cw.x) * 0.5f, p.y + 12.0f * s + iconSize + nw.y),
                     IM_COL32(150, 175, 200, earned ? 225 : 125), cr.c_str());

        if (focused && navMoved) ImGui::SetScrollHereY(0.45f);
        ImGui::PopID();
    }
    ImGui::EndChild();
}

} // namespace noble
