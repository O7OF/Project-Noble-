#include "CareerPanel.hpp"
#include "NobleStyle.hpp"
#include "MenuSounds.hpp"
#include "UiArt.hpp"
#include "career/Career.hpp"

#include <imgui.h>
#include <algorithm>
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

void CareerPanel::RenderChallenges() {
    const std::vector<Career::Challenge> list = Career::Challenges();
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::TextDisabled("%s", Career::ChallengeResetText().c_str());
    ImGui::TextDisabled("Progress counts from finished games in any mode; rewards are paid on the post-game screen.");
    for (int weekly = 0; weekly < 2; ++weekly) {
        Section(weekly ? "WEEKLY CHALLENGES" : "DAILY CHALLENGES");
        for (const Career::Challenge& c : list) {
            if (c.weekly != (weekly == 1)) continue;
            ImGui::PushID(&c);
            ImGui::TextColored(c.done ? kDone : NobleStyle::Accent(), "%s", c.title.c_str());
            ImGui::SameLine(400.0f);
            ImGui::Text("+%s cR", Grouped(c.reward).c_str());
            ImGui::TextUnformatted(c.description.c_str());
            char label[64];
            if (c.done) std::snprintf(label, sizeof(label), "COMPLETE");
            else std::snprintf(label, sizeof(label), "%lld / %lld", static_cast<long long>(c.progress), static_cast<long long>(c.target));
            const float f = c.target > 0 ? static_cast<float>(c.progress) / static_cast<float>(c.target) : 0.0f;
            ImGui::ProgressBar(std::clamp(f, 0.0f, 1.0f), ImVec2(520, 0), label);
            ImGui::Dummy(ImVec2(0, 6));
            ImGui::PopID();
        }
    }
}

void CareerPanel::RenderCommendations() {
    const std::vector<Career::Commendation> list = Career::Commendations();
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::TextDisabled("Each tier (Iron, Bronze, Silver, Gold, Onyx, Max) pays credits once, on the post-game screen.");
    Section("COMMENDATIONS");
    for (const Career::Commendation& c : list) {
        ImGui::PushID(c.title.c_str());
        ImGui::TextColored(NobleStyle::Accent(), "%s", c.title.c_str());
        ImGui::SameLine(400.0f);
        ImGui::TextColored(c.tier > 0 ? kDone : NobleStyle::TextMuted(), "%s", Career::CommendationTierName(c.tier));
        char label[96];
        float f = 1.0f;
        if (c.nextTarget > 0) {
            std::snprintf(label, sizeof(label), "%s: %s / %s for %s", c.description.c_str(), Grouped(c.value).c_str(),
                          Grouped(c.nextTarget).c_str(), Career::CommendationTierName(c.tier + 1));
            f = static_cast<float>(c.value - c.prevTarget) / static_cast<float>(std::max<long long>(1, c.nextTarget - c.prevTarget));
        } else {
            std::snprintf(label, sizeof(label), "%s: %s (maxed)", c.description.c_str(), Grouped(c.value).c_str());
        }
        ImGui::ProgressBar(std::clamp(f, 0.0f, 1.0f), ImVec2(520, 0), label);
        ImGui::Dummy(ImVec2(0, 6));
        ImGui::PopID();
    }
}

} // namespace noble
