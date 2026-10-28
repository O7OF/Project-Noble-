#include "CareerPanel.hpp"
#include "NobleStyle.hpp"
#include "MenuSounds.hpp"
#include "UiArt.hpp"
#include "NobleUi.hpp"
#include "career/Career.hpp"

#include <imgui.h>
#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <string>

namespace noble {

namespace {

const ImVec4 kLabel = ImVec4(0.42f, 0.74f, 0.93f, 1.0f);
const ImVec4 kSection = ImVec4(0.80f, 0.86f, 0.90f, 1.0f);
const ImVec4 kDone = ImVec4(0.55f, 0.85f, 0.55f, 1.0f);

void Section(const char* text) {
    ImGui::Dummy(ImVec2(0, 6));
    ImGui::TextColored(kSection, "%s", text);
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(p, ImVec2(p.x + std::min(760.0f, ImGui::GetContentRegionAvail().x), p.y),
                                        IM_COL32(255, 255, 255, 50));
    ImGui::Dummy(ImVec2(0, 4));
}

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

void Stat(const char* label, const std::string& value) {
    ImGui::TextColored(kLabel, "%s", label);
    ImGui::SameLine(260.0f);
    ImGui::TextUnformatted(value.c_str());
}

const char* DifficultyName(int d) {
    static const char* const kNames[4] = { "Easy", "Normal", "Heroic", "Legendary" };
    return d >= 0 && d < 4 ? kNames[d] : "-";
}

} // namespace

void CareerPanel::Render() {
    static const char* const kTabs[] = { "SERVICE RECORD", "CHALLENGES", "COMMENDATIONS", "RANKS" };
    ImGui::Dummy(ImVec2(0, 2));
    for (int i = 0; i < 4; ++i) {
        if (i) ImGui::SameLine(0, 26.0f);
        const bool sel = m_tab == i;
        ImGui::PushStyleColor(ImGuiCol_Text, sel ? ImVec4(1, 1, 1, 1) : kLabel);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.06f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.10f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        if (ImGui::Button(kTabs[i])) {
            if (m_tab != i) MenuSounds::Tab(i > m_tab ? 1 : -1);
            m_tab = i;
        }
        ImGui::PopStyleColor(5);
        if (sel) {
            const ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddLine(ImVec2(a.x + 6, b.y), ImVec2(b.x - 6, b.y), IM_COL32(143, 216, 255, 255), 2.0f);
        }
    }
    if (!ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_Q, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false)) { m_tab = (m_tab + 3) % 4; MenuSounds::Tab(-1); }
        if (ImGui::IsKeyPressed(ImGuiKey_E, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false)) { m_tab = (m_tab + 1) % 4; MenuSounds::Tab(1); }
    }
    ImGui::Separator();
    ImGui::BeginChild("##careerBody", ImVec2(0, 0), ImGuiChildFlags_None);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.27f, 0.62f, 0.86f, 1.0f));   // progress bars in Noble blue
    switch (m_tab) {
    case 0: RenderRecord(); break;
    case 1: RenderChallenges(); break;
    case 2: RenderCommendations(); break;
    default: RenderRanks(); break;
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();
}

void CareerPanel::RenderRanks() {
    const Career::Totals t = Career::Get();
    const auto& ranks = Career::Ranks();
    const int rank = Career::RankIndex(t.credits);
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::TextDisabled("Earned ranks are bright, the next one is framed. cR needed under each rank.");
    ImGui::Dummy(ImVec2(0, 6));
    // Cell width from the widest name / cR line so neighbours never overlap.
    float widest = 0.0f;
    for (const auto& r : ranks) {
        std::string n = r.name;
        const size_t g = n.find(" Grade ");
        if (g != std::string::npos) n = n.substr(0, g) + " G" + n.substr(g + 7);
        widest = std::max(widest, ImGui::CalcTextSize(n.c_str()).x);
        widest = std::max(widest, ImGui::CalcTextSize((Grouped(r.credits) + " cR").c_str()).x);
    }
    const float cell = std::max(118.0f, widest + 18.0f), iconSize = 64.0f;
    const int perRow = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / cell));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    for (int i = 0; i < static_cast<int>(ranks.size()); ++i) {
        if (i % perRow) ImGui::SameLine(0, 0);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(cell, cell + 10.0f));
        const bool earned = i <= rank, next = i == rank + 1, current = i == rank;
        const ImU32 tint = earned ? IM_COL32_WHITE : (next ? IM_COL32(255, 255, 255, 200) : IM_COL32(255, 255, 255, 70));
        if (current || next) {
            dl->AddRectFilled(ImVec2(p.x + 4, p.y + 2), ImVec2(p.x + cell - 4, p.y + cell + 6),
                              current ? IM_COL32(143, 216, 255, 40) : IM_COL32(255, 255, 255, 14), 3.0f);
            dl->AddRect(ImVec2(p.x + 4, p.y + 2), ImVec2(p.x + cell - 4, p.y + cell + 6),
                        current ? IM_COL32(143, 216, 255, 220) : IM_COL32(255, 255, 255, 90), 3.0f, 0, 1.2f);
        }
        const ImVec2 i0(p.x + (cell - iconSize) * 0.5f, p.y + 8.0f);
        const UiTexture icon = UiArt::RankIcon(i);
        if (icon) UiArt::DrawContain(dl, icon, i0, ImVec2(i0.x + iconSize, i0.y + iconSize), tint);
        else dl->AddRect(i0, ImVec2(i0.x + iconSize, i0.y + iconSize), IM_COL32(255, 255, 255, earned ? 120 : 40));
        // Name (shortened: "Grade 1" -> "G1") and cR, centred.
        std::string name = ranks[i].name;
        const size_t g = name.find(" Grade ");
        if (g != std::string::npos) name = name.substr(0, g) + " G" + name.substr(g + 7);
        const std::string cr = Grouped(ranks[i].credits) + " cR";
        const ImU32 textCol = earned ? IM_COL32(236, 240, 244, 255) : IM_COL32(150, 160, 170, next ? 230 : 150);
        const ImVec2 nw = ImGui::CalcTextSize(name.c_str());
        dl->AddText(ImVec2(p.x + (cell - nw.x) * 0.5f, p.y + 8.0f + iconSize + 4.0f), textCol, name.c_str());
        const ImVec2 cw = ImGui::CalcTextSize(cr.c_str());
        dl->AddText(ImVec2(p.x + (cell - cw.x) * 0.5f, p.y + 8.0f + iconSize + 4.0f + nw.y), IM_COL32(150, 170, 190, earned ? 220 : 120), cr.c_str());
    }
}

void CareerPanel::RenderRecord() {
    const Career::Record r = Career::GetRecord();
    const auto& ranks = Career::Ranks();
    const int rank = Career::RankIndex(r.totals.credits);

    ImGui::Dummy(ImVec2(0, 4));
    // Header: big rank icon (when the player has the rank pictures), rank, cR and progress.
    const UiTexture icon = UiArt::RankIcon(rank);
    if (icon) {
        const ImVec2 p = ImGui::GetCursorScreenPos();
        UiArt::DrawContain(ImGui::GetWindowDrawList(), icon, p, ImVec2(p.x + 96.0f, p.y + 96.0f));
        ImGui::Dummy(ImVec2(96.0f, 96.0f));
        ImGui::SameLine(0, 18.0f);
    }
    ImGui::BeginGroup();
    if (ImFont* f = NobleStyle::TitleFont()) ImGui::PushFont(f);
    ImGui::TextUnformatted(ranks[rank].name);
    if (NobleStyle::TitleFont()) ImGui::PopFont();
    ImGui::TextColored(NobleStyle::Accent(), "%s cR", Grouped(r.totals.credits).c_str());
    if (rank + 1 < static_cast<int>(ranks.size())) {
        const long long lo = ranks[rank].credits, hi = ranks[rank + 1].credits;
        const float f = static_cast<float>(r.totals.credits - lo) / static_cast<float>(hi - lo);
        char label[96];
        std::snprintf(label, sizeof(label), "%s / %s cR", Grouped(r.totals.credits).c_str(), Grouped(hi).c_str());
        ImGui::ProgressBar(f, ImVec2(520, 0), label);
        const UiTexture next = UiArt::RankIcon(rank + 1);
        if (next) {
            ImGui::TextDisabled("Next:");
            ImGui::SameLine();
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const float h = ImGui::GetTextLineHeight() + 6.0f;
            UiArt::DrawContain(ImGui::GetWindowDrawList(), next, ImVec2(p.x, p.y - 3.0f), ImVec2(p.x + h, p.y - 3.0f + h));
            ImGui::Dummy(ImVec2(h, ImGui::GetTextLineHeight()));
            ImGui::SameLine();
        } else {
            ImGui::TextDisabled("Next:");
            ImGui::SameLine();
        }
        ImGui::Text("%s at %s cR (%s to go)", ranks[rank + 1].name, Grouped(hi).c_str(), Grouped(hi - r.totals.credits).c_str());
    } else {
        ImGui::TextUnformatted("Highest rank reached.");
    }
    ImGui::EndGroup();

    Section("TOTALS");
    const long long games = r.totals.gamesPlayed;
    char buf[160];
    std::snprintf(buf, sizeof(buf), "%s   (campaign %s, Firefight %s, multiplayer %s)", Grouped(games).c_str(),
                  Grouped(r.games[0]).c_str(), Grouped(r.games[1]).c_str(), Grouped(r.games[2]).c_str());
    Stat("GAMES FINISHED", buf);
    Stat("TIME PLAYED", HoursMinutes(r.totals.minutesPlayed));
    Stat("KILLS", Grouped(r.totals.kills));
    Stat("DEATHS", Grouped(r.totals.deaths));
    Stat("ASSISTS", Grouped(r.totals.assists));
    std::snprintf(buf, sizeof(buf), "%.2f", r.totals.deaths > 0 ? static_cast<double>(r.totals.kills) / r.totals.deaths
                                                                 : static_cast<double>(r.totals.kills));
    Stat("KILL / DEATH", buf);
    Stat("MEDALS", Grouped(r.totals.medals));
    Stat("BEST GAME", Grouped(r.bestMatchCredits) + " cR");
    Stat("CHALLENGES DONE", Grouped(r.challengesCompleted));
    ImGui::TextDisabled("Kills, deaths and medals come from the game's results; their exact slots are still being checked.");

    if (!r.campaignBest.empty()) {
        Section("CAMPAIGN");
        for (const auto& [map, difficulty] : r.campaignBest) {
            ImGui::TextColored(kLabel, "%s", map.c_str());
            ImGui::SameLine(260.0f);
            ImGui::Text("Completed on %s", DifficultyName(difficulty));
        }
    }

    Section("RECENT GAMES");
    if (r.history.empty()) {
        ImGui::TextDisabled("Finish a game to start your service record.");
        return;
    }
    const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("##history", 7, flags)) {
        ImGui::TableSetupColumn("DATE", ImGuiTableColumnFlags_WidthFixed, 170);
        ImGui::TableSetupColumn("MODE", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableSetupColumn("MAP", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableSetupColumn("TIME", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("K / D / A", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("SCORE", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("CREDITS", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableHeadersRow();
        for (const Career::MatchSummary& m : r.history) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(m.date.c_str());
            ImGui::TableNextColumn();
            if (m.difficulty >= 0 && m.type != "multiplayer") ImGui::Text("%s (%s)", m.type.c_str(), DifficultyName(m.difficulty));
            else ImGui::TextUnformatted(m.type.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(m.map.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%.0f min", m.minutes);
            ImGui::TableNextColumn(); ImGui::Text("%lld / %lld / %lld", static_cast<long long>(m.kills), static_cast<long long>(m.deaths), static_cast<long long>(m.assists));
            ImGui::TableNextColumn(); ImGui::Text("%lld", static_cast<long long>(m.score));
            ImGui::TableNextColumn(); ImGui::TextColored(NobleStyle::Accent(), "+%s cR", Grouped(m.credits).c_str());
        }
        ImGui::EndTable();
    }
}

namespace {

const char* const kModeChip[4] = { "CAMPAIGN", "FIREFIGHT", "MULTIPLAYER", "ANY MODE" };
const ImU32 kModeColour[4] = { IM_COL32(120, 190, 255, 255), IM_COL32(255, 150, 90, 255),
                               IM_COL32(150, 220, 120, 255), IM_COL32(200, 200, 210, 255) };
const ImU32 kDoneColour = IM_COL32(140, 217, 140, 255);

// One challenge as a card: mode chip, name, requirement, progress bar, reward.
void ChallengeCard(const Career::Challenge& c, float width) {
    using namespace NobleUi;
    const float s = Scale();
    const float pad = 12.0f * s;
    ImFont* body = ImGui::GetFont();
    ImFont* small = NobleStyle::SmallFont();
    const float bodyPx = 18.0f * s, smallPx = 14.0f * s;
    const float textW = width - pad * 2.0f;
    // At least two lines for the requirement so cards side by side line up.
    ImFont* descFont = small ? small : body;
    const float descH = std::max(descFont->CalcTextSizeA(smallPx + 1.0f * s, FLT_MAX, textW, c.description.c_str()).y,
                                 descFont->CalcTextSizeA(smallPx + 1.0f * s, FLT_MAX, 0.0f, "A\nA").y);
    const float height = pad + smallPx + 6.0f * s + bodyPx + 4.0f * s + descH + 10.0f * s + 8.0f * s + pad;

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 p1(p0.x + width, p0.y + height);
    ImGui::Dummy(ImVec2(width, height));
    const bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const int mode = std::clamp(c.mode, 0, 3);
    dl->AddRectFilled(p0, p1, c.done ? IM_COL32(30, 52, 36, 220) : IM_COL32(22, 30, 40, hovered ? 240 : 215), 4.0f * s);
    dl->AddRect(p0, p1, c.done ? IM_COL32(140, 217, 140, 150) : IM_COL32(170, 220, 255, hovered ? 110 : 45), 4.0f * s, 0, 1.0f);
    dl->AddRectFilled(p0, ImVec2(p0.x + 3.0f * s, p1.y), c.done ? kDoneColour : kModeColour[mode], 4.0f * s, ImDrawFlags_RoundCornersLeft);

    float y = p0.y + pad;
    // Mode chip (left) and reward (right).
    const float chipW = TextWidth(small, smallPx, kModeChip[mode]) + 12.0f * s;
    dl->AddRect(ImVec2(p0.x + pad, y - 2.0f * s), ImVec2(p0.x + pad + chipW, y + smallPx + 3.0f * s), kModeColour[mode], 3.0f * s, 0, 1.0f);
    Text(dl, small, smallPx, ImVec2(p0.x + pad + 6.0f * s, y), kModeColour[mode], kModeChip[mode]);
    char reward[48];
    std::snprintf(reward, sizeof(reward), "+%s cR", Grouped(c.reward).c_str());
    const float rw = TextWidth(body, bodyPx, reward);
    Text(dl, body, bodyPx, ImVec2(p1.x - pad - rw, y - 2.0f * s), c.done ? kTextDim : kAmber, reward);
    y += smallPx + 6.0f * s;

    // Name and requirement.
    Text(dl, body, bodyPx, ImVec2(p0.x + pad, y), c.done ? kDoneColour : kText, c.title.c_str());
    y += bodyPx + 4.0f * s;
    Text(dl, small, smallPx + 1.0f * s, ImVec2(p0.x + pad, y), kTextDim, c.description.c_str(), textW);
    y += descH + 10.0f * s;

    // Progress bar with "x / y" (or a check and COMPLETE).
    const float f = c.target > 0 ? std::clamp(static_cast<float>(c.progress) / static_cast<float>(c.target), 0.0f, 1.0f) : 0.0f;
    char label[64];
    if (c.done) std::snprintf(label, sizeof(label), "COMPLETE");
    else std::snprintf(label, sizeof(label), "%s / %s", Grouped(c.progress).c_str(), Grouped(c.target).c_str());
    const float lw = TextWidth(small, smallPx, label);
    const float barX1 = p1.x - pad - lw - (c.done ? 26.0f : 12.0f) * s;
    const float barY = y + 1.0f * s, barH = 6.0f * s;
    dl->AddRectFilled(ImVec2(p0.x + pad, barY), ImVec2(barX1, barY + barH), IM_COL32(255, 255, 255, 26), 3.0f * s);
    if (f > 0.0f) {
        dl->AddRectFilled(ImVec2(p0.x + pad, barY), ImVec2(p0.x + pad + (barX1 - p0.x - pad) * f, barY + barH),
                          c.done ? kDoneColour : kAccent, 3.0f * s);
    }
    const float ly = barY + barH * 0.5f - smallPx * 0.5f - 1.0f * s;
    if (c.done) {
        // Check mark before COMPLETE.
        const float cx = p1.x - pad - lw - 16.0f * s, cy = barY + barH * 0.5f;
        dl->AddLine(ImVec2(cx, cy), ImVec2(cx + 4.0f * s, cy + 4.0f * s), kDoneColour, 2.0f * s);
        dl->AddLine(ImVec2(cx + 4.0f * s, cy + 4.0f * s), ImVec2(cx + 11.0f * s, cy - 5.0f * s), kDoneColour, 2.0f * s);
    }
    Text(dl, small, smallPx, ImVec2(p1.x - pad - lw, ly), c.done ? kDoneColour : kText, label);
}

} // namespace

void CareerPanel::RenderChallenges() {
    using namespace NobleUi;
    const std::vector<Career::Challenge> list = Career::Challenges();
    const float s = Scale();
    const float avail = ImGui::GetContentRegionAvail().x;
    const int columns = avail >= 820.0f * s ? 2 : 1;
    const float gap = 12.0f * s;
    const float cardW = (avail - gap * (columns - 1)) / columns - 1.0f;
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::TextDisabled("Halo: Reach's own challenges. Progress counts from finished games; rewards are paid on the post-game screen.");
    for (int weekly = 0; weekly < 2; ++weekly) {
        int done = 0, count = 0;
        for (const Career::Challenge& c : list) {
            if (c.weekly != (weekly == 1)) continue;
            ++count;
            if (c.done) ++done;
        }
        // Header: name, done count, and the reset countdown on the right.
        const float headerY = ImGui::GetCursorScreenPos().y;
        char head[64];
        std::snprintf(head, sizeof(head), "%s  %d / %d", weekly ? "WEEKLY CHALLENGES" : "DAILY CHALLENGES", done, count);
        const std::string reset = weekly ? Career::WeeklyResetText() : Career::DailyResetText();
        ImFont* small = NobleStyle::SmallFont();
        const float rw = TextWidth(small, 14.0f * s, reset.c_str());
        const float left = ImGui::GetCursorScreenPos().x;
        SectionHeader(head, avail - rw - 14.0f * s);   // the rule stops before the countdown
        Text(ImGui::GetWindowDrawList(), small, 14.0f * s, ImVec2(left + avail - rw, headerY + 8.0f * s), kTextDim, reset.c_str());

        int col = 0;
        for (const Career::Challenge& c : list) {
            if (c.weekly != (weekly == 1)) continue;
            ImGui::PushID(&c);
            if (col > 0) ImGui::SameLine(0, gap);
            ImGui::BeginGroup();
            ChallengeCard(c, cardW);
            ImGui::EndGroup();
            ImGui::PopID();
            col = (col + 1) % columns;
            if (col == 0) ImGui::Dummy(ImVec2(0, gap - ImGui::GetStyle().ItemSpacing.y));
        }
        if (col != 0) ImGui::Dummy(ImVec2(0, gap - ImGui::GetStyle().ItemSpacing.y));
    }
}

namespace {

// Tier colours. Only the Steel art exists, so higher tiers tint it (Max is Onyx with an amber frame).
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

// One commendation like Reach's Service Record: banner, name, tier, progress bar.
void CommendationRow(const Career::Commendation& c, float width) {
    using namespace NobleUi;
    const float s = Scale();
    const float bannerH = 58.0f * s, bannerW = bannerH * (287.0f / 90.0f);
    const float height = bannerH + 12.0f * s;
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 p1(p0.x + width, p0.y + height);
    ImGui::Dummy(ImVec2(width, height));
    const bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p0, p1, IM_COL32(22, 30, 40, hovered ? 235 : 200), 3.0f * s);

    // Banner (tinted by tier).
    const ImVec2 b0(p0.x + 6.0f * s, p0.y + 6.0f * s), b1(b0.x + bannerW, b0.y + bannerH);
    const UiTexture art = UiArt::ReachArt("commendations", c.art);
    if (art) UiArt::DrawContain(dl, art, b0, b1, TierTint(c.tier, c.tracked));
    else dl->AddRect(b0, b1, IM_COL32(255, 255, 255, 50));
    if (c.tier >= 6) dl->AddRect(ImVec2(b0.x - 1, b0.y - 1), ImVec2(b1.x + 1, b1.y + 1), IM_COL32(255, 150, 60, 230), 0.0f, 0, 2.0f * s);

    // Name and tier.
    ImFont* body = ImGui::GetFont();
    ImFont* small = NobleStyle::SmallFont();
    const float x = b1.x + 14.0f * s, right = p1.x - 12.0f * s;
    const float bodyPx = 18.0f * s, smallPx = 14.0f * s;
    Text(dl, body, bodyPx, ImVec2(x, p0.y + 8.0f * s), c.tracked ? kText : kTextDim, c.title.c_str());
    const char* tierName = c.tier > 0 ? Career::CommendationTierName(c.tier) : "";
    const float tw = TextWidth(small, smallPx, tierName);
    Text(dl, small, smallPx, ImVec2(right - tw, p0.y + 10.0f * s), TierTextColour(c.tier), tierName);
    Text(dl, small, smallPx, ImVec2(x, p0.y + 8.0f * s + bodyPx + 2.0f * s), kTextDim, c.description.c_str(), right - x);

    // Progress toward the next tier.
    const float barY = p1.y - 16.0f * s, barH = 5.0f * s;
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
    const float barX1 = right - lw - 10.0f * s;
    dl->AddRectFilled(ImVec2(x, barY), ImVec2(barX1, barY + barH), IM_COL32(255, 255, 255, 26), 2.0f * s);
    dl->AddRectFilled(ImVec2(x, barY), ImVec2(x + (barX1 - x) * std::clamp(f, 0.0f, 1.0f), barY + barH),
                      c.tier >= 6 ? IM_COL32(255, 150, 60, 255) : kAccent, 2.0f * s);
    Text(dl, small, smallPx, ImVec2(right - lw, barY + barH * 0.5f - smallPx * 0.5f - 1.0f * s), kText, label);
}

} // namespace

void CareerPanel::RenderCommendations() {
    using namespace NobleUi;
    const std::vector<Career::Commendation> list = Career::Commendations();
    const float s = Scale();
    const float avail = ImGui::GetContentRegionAvail().x;
    const int columns = avail >= 1000.0f * s ? 2 : 1;
    const float gap = 10.0f * s;
    const float rowW = (avail - gap * (columns - 1)) / columns - 1.0f;
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::TextDisabled("Halo: Reach's 45 commendations. Each tier (Steel, Bronze, Silver, Gold, Onyx, Max) pays credits once, on the post-game screen.");
    ImGui::TextDisabled("Dimmed ones need stats Noble can't read from the game yet (headshots, weapons, enemy types, multikills, sprees, vehicles).");
    static const char* const kModeTitle[3] = { "CAMPAIGN", "FIREFIGHT", "MULTIPLAYER" };
    for (int mode = 0; mode < 3; ++mode) {
        int tiers = 0, count = 0;
        for (const Career::Commendation& c : list) if (c.mode == mode) { ++count; tiers += c.tier; }
        char head[64];
        std::snprintf(head, sizeof(head), "%s  %d / %d TIERS", kModeTitle[mode], tiers, count * 6);
        SectionHeader(head, avail);
        int col = 0;
        for (const Career::Commendation& c : list) {
            if (c.mode != mode) continue;
            ImGui::PushID(c.art.c_str());
            if (col > 0) ImGui::SameLine(0, gap);
            ImGui::BeginGroup();
            CommendationRow(c, rowW);
            ImGui::EndGroup();
            ImGui::PopID();
            col = (col + 1) % columns;
            if (col == 0) ImGui::Dummy(ImVec2(0, gap - ImGui::GetStyle().ItemSpacing.y));
        }
        if (col != 0) ImGui::Dummy(ImVec2(0, gap - ImGui::GetStyle().ItemSpacing.y));
    }
}

} // namespace noble
