#include "PauseMenu.hpp"
#include "NobleStyle.hpp"
#include "SettingsPanel.hpp"
#include "MenuSounds.hpp"
#include "UiArt.hpp"
#include "NobleUi.hpp"
#include "audio/ReachAudio.hpp"
#include "career/Career.hpp"
#include "engine/FrameCapture.hpp"
#include "host/Window.hpp"
#include "common/Logger.hpp"
#include "common/Relaunch.hpp"
#include "common/Settings.hpp"

#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <mutex>
#include <string>

namespace noble {

namespace {

std::atomic<bool> s_open{false};
std::atomic<bool> s_gameOver{false};
std::atomic<bool> s_showSettings{false};   // the SETTINGS page opened from the pause menu
std::mutex s_renderMutex;                          // Render (engine thread) vs HoldFrame (main thread)
std::atomic<IDXGISwapChain*> s_swapChain{nullptr}; // last swap chain the engine presented
std::atomic<ULONGLONG> s_lastEngineFrame{0};       // GetTickCount64 of the last slot-1 frame
std::atomic<ULONGLONG> s_gameOverAt{0};
std::mutex s_mapMutex;
std::string s_loadingMap;
std::string s_loadingMapName;

struct MapName { const char* scenario; const char* name; };

// Display names for the loading screen (same names as the front-end menu).
const MapName kMapNames[] = {
    { "50_panopticon", "Boardwalk" },   { "70_boneyard", "Boneyard" },        { "45_launch_station", "Countdown" },
    { "30_settlement", "Powerhouse" },  { "52_ivory_tower", "Reflection" },   { "35_island", "Spire" },
    { "20_sword_slayer", "Sword Base" }, { "45_aftship", "Zealot" },          { "dlc_slayer", "Anchor 9" },
    { "dlc_invasion", "Breakpoint" },   { "dlc_medium", "Tempest" },          { "condemned", "Condemned" },
    { "trainingpreserve", "Highlands" }, { "cex_beaver_creek", "Battle Canyon" }, { "cex_damnation", "Penance" },
    { "cex_timberland", "Ridgeline" },  { "cex_prisoner", "Solitary" },       { "cex_hangemhigh", "High Noon" },
    { "cex_headlong", "Breakneck" },    { "forge_halo", "Forge World" },
    { "m05", "Noble Actual" },          { "m10", "Winter Contingency" },     { "m20", "ONI: Sword Base" },
    { "m30", "Nightfall" },             { "m35", "Tip of the Spear" },        { "m45", "Long Night of Solace" },
    { "m50", "Exodus" },                { "m52", "New Alexandria" },          { "m60", "The Package" },
    { "m70", "The Pillar of Autumn" },  { "m70_bonus", "Lone Wolf" },
    { "ff10_prototype", "Overlook" },  { "ff20_courtyard", "Courtyard" },    { "ff30_waterfront", "Waterfront" },
    { "ff45_corvette", "Corvette" },    { "ff50_park", "Beachhead" },          { "ff60_airview", "Outpost" },
    { "ff60_icecave", "Glacier" },      { "ff70_holdout", "Holdout" },        { "ff_unearthed", "Unearthed" },
    { "cex_ff_halo", "Installation 04" },
};

std::string DisplayName(const std::string& scenario) {
    for (const MapName& m : kMapNames) {
        if (scenario == m.scenario) return m.name;
    }
    return scenario;
}

std::string Upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Starts a new Noble (empty args = front-end menu) and closes this one.
void RelaunchAndClose(HWND hwnd, const std::wstring& args, const char* what) {
    NOBLE_INFO("PauseMenu: {} requested", what);
    Relaunch::Start(args);
    PostMessageW(hwnd, WM_CLOSE, 0, 0);
}

ImVec2 CenteredWindow(float width) {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);
    return ImVec2(-1.0f, 38.0f);
}

constexpr ImGuiWindowFlags kPanelFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;

// The SETTINGS page is the same panel the front-end menu shows, reused inside the pause menu so the
// player can change settings during a match (FOV, VSync, frame cap, mouse, ...). It lives for the
// life of the process because it keeps the edited copy and the key-rebind state between frames.
SettingsPanel& PauseSettings() {
    static SettingsPanel panel;
    return panel;
}

void DrawSettingsPage(HWND hwnd);

} // namespace

bool PauseMenu::IsOpen() { return s_open.load(); }

void PauseMenu::Toggle() {
    if (s_gameOver.load()) return;  // the post-game panel replaces the pause menu
    // Esc on the SETTINGS page goes back to the pause menu instead of resuming. While a key rebind
    // is waiting for a key, Esc belongs to the rebind (SettingsPanel::PollRebind cancels it).
    if (s_open.load() && s_showSettings.load()) {
        if (!PauseSettings().IsCapturingKey()) {
            s_showSettings.store(false);
            MenuSounds::Back();
            NOBLE_INFO("PauseMenu: settings closed");
        }
        return;
    }
    const bool nowOpen = !s_open.load();
    if (nowOpen) Window::TakeLeftClicks();  // forget clicks made in game (shots) before the menu opened
    s_open.store(nowOpen);
    NOBLE_INFO("PauseMenu: {}", nowOpen ? "opened" : "closed");
}

void PauseMenu::Close() {
    s_showSettings.store(false);
    if (s_open.exchange(false)) {
        NOBLE_INFO("PauseMenu: closed");
    }
}

void PauseMenu::ShowGameOver() {
    if (!s_gameOver.exchange(true)) {
        s_open.store(false);
        s_showSettings.store(false);
        s_gameOverAt.store(GetTickCount64());
        Window::TakeLeftClicks();
        NOBLE_INFO("PauseMenu: game over, showing post-game panel");
    }
}

bool PauseMenu::IsGameOver() { return s_gameOver.load(); }

bool PauseMenu::WantsPointer() { return s_open.load() || s_gameOver.load(); }

void PauseMenu::SetLoadingMap(const std::string& scenario) {
    std::lock_guard<std::mutex> lock(s_mapMutex);
    s_loadingMap = scenario;
    s_loadingMapName = Upper(DisplayName(scenario));
}

void PauseMenu::DrawMenu(HWND hwnd) {
    ImGuiIO& io = ImGui::GetIO();

    // Dim the game behind the menu.
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 140));

    if (s_showSettings.load()) {
        DrawSettingsPage(hwnd);
        return;
    }

    const ImVec2 button = CenteredWindow(340.0f);
    if (ImGui::Begin("##NoblePauseMenu", nullptr, kPanelFlags)) {
        NobleStyle::Header("PAUSED");
        ImGui::Spacing();

        if (NobleStyle::PrimaryButton("RESUME", button)) {
            Close();
        }
        ImGui::Spacing();
        if (ImGui::Button("SETTINGS", button)) {
            MenuSounds::Click();
            Window::TakeLeftClicks();
            s_showSettings.store(true);
            NOBLE_INFO("PauseMenu: settings opened from the pause menu");
        }
        ImGui::Spacing();
        if (ImGui::Button("QUIT TO MENU", button)) {
            // The engine can't switch maps once started (RR also runs menu and match as separate
            // processes), so start a fresh Noble with no -map, which opens the front-end menu.
            Close();
            RelaunchAndClose(hwnd, L"", "quit to menu");
        }
        ImGui::Spacing();
        if (NobleStyle::DangerButton("QUIT", button)) {
            NOBLE_INFO("PauseMenu: quit requested");
            Close();
            // The main thread owns shutdown: WM_CLOSE ends its message loop and runs the normal exit path.
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
        }
        ImGui::Spacing();
        ImGui::TextDisabled("Esc to resume");
    }
    ImGui::End();
}

// The front-end SETTINGS panel, reused while paused so settings can be changed during a match.
// Every value that Noble can reach live (FOV, VSync, frame cap, mouse, audio, key bindings) is
// applied on APPLY; the panels' own help lines list the few that need a restart.
namespace {

void DrawSettingsPage(HWND hwnd) {
    (void)hwnd;
    ImGuiIO& io = ImGui::GetIO();
    // Full screen (small margin): the three-column settings need the room.
    const float width = io.DisplaySize.x - 32.0f;
    const float height = io.DisplaySize.y - 32.0f;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("##NoblePauseSettings", nullptr, flags)) {
        NobleStyle::Header("SETTINGS");
        if (ImGui::Button("< BACK", ImVec2(110, 0))) {
            s_showSettings.store(false);
            MenuSounds::Back();
            NOBLE_INFO("PauseMenu: settings closed");
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Changes apply immediately, also in this match. Esc goes back to the pause menu.");
        ImGui::Separator();
        PauseSettings().Render();
    }
    ImGui::End();
}

} // namespace

// ---------------------------------------------------------------- post-game "exit experience"
// Reach-style staged reveal (spec 4.8): scoreboard, then each cR line counting up (Totals, or the slot
// machine for a random bonus), the total, challenge / commendation progress bars, and the rank bar
// filling up to the new total (RankUp and the new icon on a promotion). Enter, Space or a click on an
// empty spot skips to the end. PLAY AGAIN / RETURN TO MENU / QUIT work at any time.
namespace {

struct Reveal {
    ULONGLONG start = 0;       // s_gameOverAt the reveal belongs to
    bool skipped = false;
    int linesSounded = 0;
    bool completionSounded = false;
    bool totalSounded = false;
    bool rankUpSounded = false;
    bool keyWasDown = true;    // ignore a key still held from the game
};
Reveal s_reveal;

constexpr float kFirstLine = 0.8f, kLineStep = 0.5f, kLineCount = 0.35f, kSlotRoll = 0.9f;

float Ease(float x) {
    x = std::clamp(x, 0.0f, 1.0f);
    return 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x);
}

bool IsSlotLine(const Career::AwardLine& l) { return l.label == "Random bonus" || l.label == "Jackpot!"; }

void Sfx(ReachAudio::Sfx sfx, float volume = 1.0f) {
    if (Settings::Get().Snapshot().menuSounds) ReachAudio::PlaySfx(sfx, volume);
}

std::string Grouped(long long v) {
    std::string digits = std::to_string(v < 0 ? -v : v), out;
    for (size_t i = 0; i < digits.size(); ++i) {
        if (i && (digits.size() - i) % 3 == 0) out.push_back(',');
        out.push_back(digits[i]);
    }
    return v < 0 ? "-" + out : out;
}

// Section heading drawn straight into a draw list (letter-spaced caps + a rule).
void Heading(ImDrawList* dl, ImVec2 pos, float width, float s, const char* text) {
    ImFont* f = NobleStyle::SmallFont();
    const float px = 15.0f * s;
    float x = pos.x;
    char ch[2] = { 0, 0 };
    for (const char* c = text; *c; ++c) {
        ch[0] = *c;
        NobleUi::Text(dl, f, px, ImVec2(x, pos.y), IM_COL32(150, 190, 220, 255), ch);
        x += NobleUi::TextWidth(f, px, ch) + 1.8f * s;
    }
    if (x + 10.0f * s < pos.x + width)
        dl->AddLine(ImVec2(x + 10.0f * s, pos.y + px * 0.55f), ImVec2(pos.x + width, pos.y + px * 0.55f), NobleUi::kRule, 1.0f);
}

} // namespace

void PauseMenu::DrawGameOver(HWND hwnd) {
    using namespace NobleUi;
    ImGuiIO& io = ImGui::GetIO();
    const float W = io.DisplaySize.x, H = io.DisplaySize.y;
    const float s = std::clamp(H / 1080.0f, 0.7f, 2.0f);
    const Career::Award a = Career::LastAward();
    const auto& ranks = Career::Ranks();

    std::string mapName;
    {
        std::lock_guard<std::mutex> lock(s_mapMutex);
        mapName = s_loadingMapName;
    }

    // Reveal clock (restarts for every new results screen).
    const ULONGLONG start = s_gameOverAt.load();
    if (s_reveal.start != start) {
        s_reveal = Reveal{};
        s_reveal.start = start;
    }
    const int lineCount = a.valid ? static_cast<int>(a.lines.size()) : 0;
    const float tTotal = kFirstLine + lineCount * kLineStep + 0.2f;
    const float tRank = tTotal + 0.9f;
    const float tEnd = tRank + 1.6f;
    float t = static_cast<float>(GetTickCount64() - start) / 1000.0f;

    // Skip: Enter / Space (fresh press, window in front) or a click away from the buttons.
    bool skipKey = false;
    if (GetForegroundWindow() == hwnd) {
        const bool down = (GetAsyncKeyState(VK_RETURN) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000);
        skipKey = down && !s_reveal.keyWasDown;
        s_reveal.keyWasDown = down;
    }
    if (skipKey && t < tEnd) s_reveal.skipped = true;
    if (s_reveal.skipped) t = tEnd + 1.0f;

    if (!s_reveal.completionSounded) {
        s_reveal.completionSounded = true;
        if (!s_reveal.skipped) Sfx(ReachAudio::Sfx::GameCompletion);
    }

    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    bg->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(W, H), IM_COL32(6, 12, 20, 235), IM_COL32(6, 12, 20, 235),
                                IM_COL32(2, 4, 8, 245), IM_COL32(2, 4, 8, 245));

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(W, H));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (!ImGui::Begin("##NobleGameOver", nullptr, flags)) {
        ImGui::End();
        return;
    }
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* hero = NobleStyle::HeroFont();
    ImFont* title = NobleStyle::TitleFont();
    ImFont* smallFont = NobleStyle::SmallFont();
    ImFont* body = ImGui::GetFont();
    const float margin = 64.0f * s;

    // ---- Header: GAME OVER, map, mode, time, difficulty.
    {
        const float d = 15.0f * s;
        const ImVec2 c(margin + d, 70.0f * s);
        dl->AddQuad(ImVec2(c.x, c.y - d), ImVec2(c.x + d, c.y), ImVec2(c.x, c.y + d), ImVec2(c.x - d, c.y), kAccent, 2.0f * s);
        dl->AddQuad(ImVec2(c.x, c.y - d * 0.45f), ImVec2(c.x + d * 0.45f, c.y), ImVec2(c.x, c.y + d * 0.45f), ImVec2(c.x - d * 0.45f, c.y), kAccent, 2.0f * s);
        Text(dl, hero, 56.0f * s, ImVec2(margin + 2.6f * d, 40.0f * s), kText, "GAME OVER");
        std::string sub = Upper(mapName.empty() ? a.map : mapName);
        if (a.valid) {
            char extra[96];
            std::snprintf(extra, sizeof(extra), "%s%s  %.0f MIN", sub.empty() ? "" : "  -  ", Upper(a.type).c_str(), a.minutes);
            sub += extra;
        }
        float x = margin + 2.6f * d;
        const float y = 104.0f * s;
        Text(dl, title, 24.0f * s, ImVec2(x, y), kTextDim, sub.c_str());
        x += TextWidth(title, 24.0f * s, sub.c_str()) + 20.0f * s;
        if (a.valid && a.difficulty >= 0) {
            static const char* const kDiff[4] = { "EASY", "NORMAL", "HEROIC", "LEGENDARY" };
            const int dIdx = std::clamp(a.difficulty, 0, 3);
            const UiTexture icon = UiArt::DifficultyIcon(dIdx);
            if (icon) {
                UiArt::DrawContain(dl, icon, ImVec2(x, y - 2.0f * s), ImVec2(x + 30.0f * s, y + 28.0f * s));
                x += 36.0f * s;
            }
            Text(dl, title, 24.0f * s, ImVec2(x, y), kTextDim, kDiff[dIdx]);
        }
    }

    const float top = 170.0f * s;
    const float bottom = H - 120.0f * s;
    const float split = W * 0.58f;
    const float leftW = split - margin - 24.0f * s;
    const float rightX = split + 24.0f * s, rightW = W - margin - rightX;

    // ---- Left: scoreboard.
    float y = top;
    Heading(dl, ImVec2(margin, y), leftW, s, "SCOREBOARD");
    y += 30.0f * s;
    if (a.valid && !a.players.empty()) {
        const float colW = 74.0f * s;
        const char* const heads[5] = { "SCORE", "K", "D", "A", "MEDALS" };
        const float px = 15.0f * s;
        for (int c = 0; c < 5; ++c) {
            const float cx = margin + leftW - (5 - c) * colW;
            Text(dl, smallFont, px, ImVec2(cx + colW - TextWidth(smallFont, px, heads[c]) - 8.0f * s, y), kTextDim, heads[c]);
        }
        Text(dl, smallFont, px, ImVec2(margin + 12.0f * s, y), kTextDim, "PLAYER");
        y += 24.0f * s;
        const float rowH = 38.0f * s;
        const float appear = std::clamp(t / 0.4f, 0.0f, 1.0f);
        const int alpha = static_cast<int>(255 * appear);
        int shown = 0;
        for (const Career::ScoreRow& r : a.players) {
            if (y + rowH > top + (bottom - top) * 0.52f) break;
            const ImVec2 r0(margin, y), r1(margin + leftW, y + rowH - 4.0f * s);
            dl->AddRectFilled(r0, r1, r.local ? IM_COL32(40, 80, 115, static_cast<int>(160 * appear)) : IM_COL32(255, 255, 255, static_cast<int>(12 * appear)), 2.0f * s);
            if (r.local) dl->AddRectFilled(r0, ImVec2(r0.x + 3.0f * s, r1.y), kAccent);
            float nx = margin + 12.0f * s;
            if (r.local) {
                const UiTexture icon = UiArt::RankIcon(a.rankAfter);
                if (icon) {
                    UiArt::DrawContain(dl, icon, ImVec2(nx, y + 3.0f * s), ImVec2(nx + 28.0f * s, y + rowH - 7.0f * s), IM_COL32(255, 255, 255, alpha));
                    nx += 36.0f * s;
                }
            }
            const float ty = y + (rowH - 4.0f * s) * 0.5f - 10.0f * s;
            const ImU32 nameCol = r.local ? IM_COL32(143, 216, 255, alpha) : IM_COL32(236, 240, 244, alpha);
            Text(dl, body, 18.0f * s, ImVec2(nx, ty), nameCol, r.name.c_str());
            const long long vals[5] = { r.score, r.kills, r.deaths, r.assists, r.medals };
            for (int c = 0; c < 5; ++c) {
                const std::string v = Grouped(vals[c]);
                const float cx = margin + leftW - (5 - c) * colW;
                Text(dl, body, 18.0f * s, ImVec2(cx + colW - TextWidth(body, 18.0f * s, v.c_str()) - 8.0f * s, ty), IM_COL32(236, 240, 244, alpha), v.c_str());
            }
            y += rowH;
            ++shown;
        }
        if (shown < static_cast<int>(a.players.size())) {
            char more[48];
            std::snprintf(more, sizeof(more), "+%d more", static_cast<int>(a.players.size()) - shown);
            Text(dl, smallFont, 15.0f * s, ImVec2(margin + 12.0f * s, y), kTextDim, more);
            y += 22.0f * s;
        }
    } else {
        Text(dl, body, 18.0f * s, ImVec2(margin, y), kTextDim, "No results from this game.");
        y += 30.0f * s;
    }

    // ---- Left: challenge and commendation progress made in this game.
    y += 18.0f * s;
    Heading(dl, ImVec2(margin, y), leftW, s, "PROGRESS");
    y += 30.0f * s;
    if (!a.valid || a.progress.empty()) {
        Text(dl, body, 18.0f * s, ImVec2(margin, y), kTextDim, "No challenge or commendation progress this game.");
    } else {
        static const char* const kKind[3] = { "DAILY", "WEEKLY", "COMMENDATION" };
        const float rowH = 56.0f * s;
        int i = 0;
        for (const Career::ProgressLine& p : a.progress) {
            if (y + rowH > bottom - 8.0f * s) {
                char more[64];
                std::snprintf(more, sizeof(more), "+%d more in the Service Record", static_cast<int>(a.progress.size()) - i);
                Text(dl, smallFont, 15.0f * s, ImVec2(margin, y), kTextDim, more);
                break;
            }
            const float k = Ease((t - (0.6f + i * 0.25f)) / 0.9f);
            const ImVec2 r0(margin, y), r1(margin + leftW, y + rowH - 6.0f * s);
            const bool done = p.completed && k >= 1.0f;
            const ImU32 green = IM_COL32(140, 217, 140, 255);
            dl->AddRectFilled(r0, r1, done ? IM_COL32(30, 52, 36, 200) : IM_COL32(22, 30, 40, 200), 3.0f * s);
            float x = margin + 12.0f * s;
            if (p.kind == 2) {
                const UiTexture art = UiArt::ReachArt("commendations", p.art);
                if (art) {
                    const float bh = rowH - 16.0f * s, bw = bh * (287.0f / 90.0f);
                    UiArt::DrawContain(dl, art, ImVec2(x, y + 5.0f * s), ImVec2(x + bw, y + 5.0f * s + bh));
                    x += bw + 12.0f * s;
                }
            }
            const float chipPx = 13.0f * s;
            const float chipW = TextWidth(smallFont, chipPx, kKind[p.kind]) + 10.0f * s;
            dl->AddRect(ImVec2(x, y + 8.0f * s), ImVec2(x + chipW, y + 12.0f * s + chipPx), kTextDim, 2.0f * s);
            Text(dl, smallFont, chipPx, ImVec2(x + 5.0f * s, y + 10.0f * s), kTextDim, kKind[p.kind]);
            Text(dl, body, 18.0f * s, ImVec2(x + chipW + 10.0f * s, y + 6.0f * s), done ? green : kText, p.title.c_str());
            // Bar from the old value to the new one (old part blue-grey, this game's part amber).
            const float denom = static_cast<float>(std::max<int64_t>(1, p.target));
            const float fBefore = std::clamp(static_cast<float>(p.before) / denom, 0.0f, 1.0f);
            const float fAfter = std::clamp(static_cast<float>(p.after) / denom, 0.0f, 1.0f);
            const float f = fBefore + (fAfter - fBefore) * k;
            std::string label;
            if (done) label = p.kind == 2 ? Upper(Career::CommendationTierName(p.tier)) + " REACHED" : "COMPLETE";
            else label = Grouped(static_cast<long long>(p.before + (p.after - p.before) * k)) + " / " + Grouped(p.target);
            const float lw = TextWidth(smallFont, 15.0f * s, label.c_str());
            const float bx0 = x, bx1 = r1.x - lw - 20.0f * s, by = y + 36.0f * s;
            dl->AddRectFilled(ImVec2(bx0, by), ImVec2(bx1, by + 5.0f * s), IM_COL32(255, 255, 255, 26), 2.0f * s);
            dl->AddRectFilled(ImVec2(bx0, by), ImVec2(bx0 + (bx1 - bx0) * fBefore, by + 5.0f * s), IM_COL32(110, 150, 180, 255), 2.0f * s);
            if (f > fBefore)
                dl->AddRectFilled(ImVec2(bx0 + (bx1 - bx0) * fBefore, by), ImVec2(bx0 + (bx1 - bx0) * f, by + 5.0f * s), done ? green : kAmber, 2.0f * s);
            Text(dl, smallFont, 15.0f * s, ImVec2(r1.x - lw - 10.0f * s, by - 6.0f * s), done ? green : kText, label.c_str());
            y += rowH;
            ++i;
        }
    }

    // ---- Right: credits, counting up line by line.
    float ry = top;
    Heading(dl, ImVec2(rightX, ry), rightW, s, "CREDITS");
    ry += 34.0f * s;
    if (a.valid) {
        for (int i = 0; i < lineCount; ++i) {
            const Career::AwardLine& l = a.lines[i];
            const float lt = kFirstLine + i * kLineStep;
            if (t < lt) break;
            const bool slot = IsSlotLine(l);
            if (s_reveal.linesSounded <= i && !s_reveal.skipped) {
                s_reveal.linesSounded = i + 1;
                if (slot) Sfx(ReachAudio::Sfx::SlotMachine);
                else if (l.credits > 0) Sfx(ReachAudio::Sfx::Totals, 0.7f);
            }
            const float k = std::clamp((t - lt) / (slot ? kSlotRoll : kLineCount), 0.0f, 1.0f);
            long long shownValue = static_cast<long long>(l.credits * Ease(k));
            if (slot && k < 1.0f) {   // slot machine: rolling digits until it lands
                shownValue = (static_cast<long long>(t * 37.0f) % 10) * 1000 + (static_cast<long long>(t * 91.0f) % 10) * 100 +
                             (static_cast<long long>(t * 53.0f) % 10) * 10;
            }
            const int fade = static_cast<int>(255 * std::clamp((t - lt) / 0.2f, 0.0f, 1.0f));
            Text(dl, body, 19.0f * s, ImVec2(rightX, ry), IM_COL32(160, 172, 184, fade), l.label.c_str());
            const std::string v = "+" + Grouped(shownValue) + " cR";
            Text(dl, body, 19.0f * s, ImVec2(rightX + rightW - TextWidth(body, 19.0f * s, v.c_str()), ry),
                 slot ? IM_COL32(255, 214, 110, 255) : kAmber, v.c_str());
            ry += 30.0f * s;
            if (ry > top + (bottom - top) * 0.48f && i + 1 < lineCount) {
                int64_t rest = 0;
                for (int j = i + 1; j < lineCount; ++j) rest += a.lines[j].credits;
                const std::string more = std::to_string(lineCount - i - 1) + " more  +" + Grouped(rest) + " cR";
                if (t >= kFirstLine + (i + 1) * kLineStep) Text(dl, smallFont, 15.0f * s, ImVec2(rightX, ry), kTextDim, more.c_str());
                ry += 24.0f * s;
                break;
            }
        }
        // Total.
        ry += 8.0f * s;
        dl->AddLine(ImVec2(rightX, ry), ImVec2(rightX + rightW, ry), kRule, 1.0f);
        ry += 12.0f * s;
        if (t >= tTotal) {
            if (!s_reveal.totalSounded && !s_reveal.skipped) {
                s_reveal.totalSounded = true;
                Sfx(ReachAudio::Sfx::Totals);
            }
            const float k = Ease((t - tTotal) / 0.8f);
            Text(dl, title, 26.0f * s, ImVec2(rightX, ry), kAccent, "CREDITS EARNED");
            const std::string v = "+" + Grouped(static_cast<long long>(a.total * k)) + " cR";
            Text(dl, title, 30.0f * s, ImVec2(rightX + rightW - TextWidth(title, 30.0f * s, v.c_str()), ry - 2.0f * s), kAmber, v.c_str());
        }
        ry += 54.0f * s;

        // ---- Right: rank. The bar fills from the old total to the new one.
        Heading(dl, ImVec2(rightX, ry), rightW, s, "RANK");
        ry += 30.0f * s;
        const float k = t < tRank ? 0.0f : Ease((t - tRank) / 1.4f);
        const int64_t shownCredits = a.creditsBefore + static_cast<int64_t>((a.creditsAfter - a.creditsBefore) * k);
        const int shownRank = std::clamp(Career::RankIndex(shownCredits), 0, static_cast<int>(ranks.size()) - 1);
        const bool promoted = shownRank > a.rankBefore;
        if (promoted && !s_reveal.rankUpSounded) {
            s_reveal.rankUpSounded = true;
            Sfx(ReachAudio::Sfx::RankUp);
        }
        const float iconSize = 120.0f * s;
        if (promoted) {
            const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(ImGui::GetTime()) * 4.0f);
            dl->AddCircleFilled(ImVec2(rightX + iconSize * 0.5f, ry + iconSize * 0.5f), iconSize * 0.62f,
                                IM_COL32(255, 186, 74, static_cast<int>(30 + 40 * pulse)));
        }
        const UiTexture icon = UiArt::RankIcon(shownRank);
        if (icon) UiArt::DrawContain(dl, icon, ImVec2(rightX, ry), ImVec2(rightX + iconSize, ry + iconSize));
        else dl->AddRect(ImVec2(rightX, ry), ImVec2(rightX + iconSize, ry + iconSize), IM_COL32(255, 255, 255, 50));
        const float tx = rightX + iconSize + 22.0f * s;
        if (promoted) Text(dl, title, 22.0f * s, ImVec2(tx, ry + 4.0f * s), kAmber, "PROMOTED");
        Text(dl, title, 30.0f * s, ImVec2(tx, ry + (promoted ? 30.0f : 14.0f) * s), kText, ranks[shownRank].name);
        const float barY = ry + iconSize - 34.0f * s;
        if (shownRank + 1 < static_cast<int>(ranks.size())) {
            const int64_t lo = ranks[shownRank].credits, hi = ranks[shownRank + 1].credits;
            const float f = std::clamp(static_cast<float>(shownCredits - lo) / static_cast<float>(std::max<int64_t>(1, hi - lo)), 0.0f, 1.0f);
            const float bx1 = rightX + rightW;
            dl->AddRectFilled(ImVec2(tx, barY), ImVec2(bx1, barY + 8.0f * s), IM_COL32(255, 255, 255, 26), 3.0f * s);
            dl->AddRectFilled(ImVec2(tx, barY), ImVec2(tx + (bx1 - tx) * f, barY + 8.0f * s), kAccent, 3.0f * s);
            const std::string v = Grouped(shownCredits) + " / " + Grouped(hi) + " cR";
            Text(dl, smallFont, 15.0f * s, ImVec2(tx, barY + 14.0f * s), kTextDim, v.c_str());
            const std::string next = std::string("Next: ") + ranks[shownRank + 1].name;
            Text(dl, smallFont, 15.0f * s, ImVec2(bx1 - TextWidth(smallFont, 15.0f * s, next.c_str()), barY + 14.0f * s), kTextDim, next.c_str());
        } else {
            Text(dl, smallFont, 15.0f * s, ImVec2(tx, barY + 14.0f * s), kTextDim, "Highest rank reached");
        }
    }

    // ---- Buttons (usable at any time) and the skip hint.
    const float bh = 52.0f * s, bw = 260.0f * s, gap = 16.0f * s;
    const float by = H - 88.0f * s;
    float bx = W - margin - bw;
    bool overButton = false;
    ImGui::SetCursorScreenPos(ImVec2(bx, by));
    if (NobleStyle::DangerButton("QUIT", ImVec2(bw, bh))) {
        NOBLE_INFO("PauseMenu: quit requested from post-game panel");
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
    }
    overButton |= ImGui::IsItemHovered();
    bx -= bw + gap;
    ImGui::SetCursorScreenPos(ImVec2(bx, by));
    if (ImGui::Button("RETURN TO MENU", ImVec2(bw, bh))) RelaunchAndClose(hwnd, L"", "return to menu");
    overButton |= ImGui::IsItemHovered();
    const std::wstring again = Relaunch::CurrentGameArgs();
    if (!again.empty()) {
        bx -= bw + gap;
        ImGui::SetCursorScreenPos(ImVec2(bx, by));
        if (NobleStyle::PrimaryButton("PLAY AGAIN", ImVec2(bw, bh))) RelaunchAndClose(hwnd, again, "play again");
        overButton |= ImGui::IsItemHovered();
    }
    if (t < tEnd) {
        Text(dl, smallFont, 16.0f * s, ImVec2(margin, by + bh * 0.5f - 9.0f * s), kTextDim, "Enter, Space or click: skip");
        if (ImGui::IsMouseClicked(0) && !overButton) s_reveal.skipped = true;
    }
    ImGui::End();
}

void PauseMenu::DrawLoading() {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float w = io.DisplaySize.x;
    const float h = io.DisplaySize.y;

    // Reach-style loading screen: a full-screen scene (the player's own art, Noble's art or the
    // mission's scene from the MCC install), map name bottom-left, a moving bar (the engine's
    // progress callback doesn't report real progress, so the bar only shows that it's working).
    std::string mapName, scenario;
    {
        std::lock_guard<std::mutex> lock(s_mapMutex);
        mapName = s_loadingMapName;
        scenario = s_loadingMap;
    }
    dl->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(w, h),
                                IM_COL32(10, 16, 24, 255), IM_COL32(10, 16, 24, 255),
                                IM_COL32(2, 4, 8, 255), IM_COL32(2, 4, 8, 255));
    const UiTexture scene = UiArt::LoadingBackground(scenario);
    if (scene) {
        UiArt::DrawCover(dl, scene, ImVec2(0, 0), ImVec2(w, h));
        dl->AddRectFilledMultiColor(ImVec2(0, h * 0.55f), ImVec2(w, h), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
                                    IM_COL32(0, 0, 0, 215), IM_COL32(0, 0, 0, 215));
    }
    // Multiplayer and Firefight have no per-map scene: show the map's preview as well.
    const bool campaign = scenario.size() >= 3 && scenario[0] == 'm' && scenario[1] >= '0' && scenario[1] <= '9';
    if (!campaign) {
        const UiTexture preview = UiArt::MapPreview(scenario, true);
        if (preview) {
            const float pw = w * 0.30f, ph = pw * 292.0f / 668.0f;
            const ImVec2 p0(w * 0.93f - pw, h * 0.80f - ph), p1(w * 0.93f, h * 0.80f);
            dl->AddRectFilled(ImVec2(p0.x - 4, p0.y - 4), ImVec2(p1.x + 4, p1.y + 4), IM_COL32(0, 0, 0, 160));
            UiArt::DrawCover(dl, preview, p0, p1);
            dl->AddRect(p0, p1, IM_COL32(170, 200, 220, 140));
        }
    }

    const ImVec4 accent = NobleStyle::Accent();
    const ImU32 accentCol = ImGui::ColorConvertFloat4ToU32(accent);
    const ImU32 mutedCol = ImGui::ColorConvertFloat4ToU32(NobleStyle::TextMuted());

    const float left = w * 0.07f;
    const float base = h * 0.80f;

    ImFont* title = NobleStyle::TitleFont();
    ImFont* font = ImGui::GetFont();
    const float titleSize = title ? title->FontSize * 1.4f : ImGui::GetFontSize() * 2.0f;
    dl->AddText(font, ImGui::GetFontSize(), ImVec2(left, base - titleSize - 26.0f), mutedCol, "LOADING");
    if (!mapName.empty()) {
        dl->AddText(title ? title : font, titleSize, ImVec2(left, base - titleSize - 4.0f), IM_COL32(235, 240, 245, 255), mapName.c_str());
    }

    const float barW = w * 0.30f;
    const float barH = 4.0f;
    const ImVec2 b0(left, base + 8.0f);
    const ImVec2 b1(left + barW, base + 8.0f + barH);
    dl->AddRectFilled(b0, b1, IM_COL32(255, 255, 255, 30));
    const float t = static_cast<float>(ImGui::GetTime());
    const float seg = barW * 0.25f;
    const float pos = std::fmod(t * 0.6f, 1.0f) * (barW + seg) - seg;
    const float x0 = b0.x + (pos < 0.0f ? 0.0f : pos);
    const float x1 = b0.x + ((pos + seg) > barW ? barW : (pos + seg));
    if (x1 > x0) dl->AddRectFilled(ImVec2(x0, b0.y), ImVec2(x1, b1.y), accentCol);
}

void PauseMenu::Render(IDXGISwapChain* swapChain, bool loading) {
    if (swapChain) s_swapChain.store(swapChain);
    s_lastEngineFrame.store(GetTickCount64());
    std::lock_guard<std::mutex> lock(s_renderMutex);
    RenderImpl(swapChain, loading, false);
}

bool PauseMenu::HoldFrame() {
    if (!s_gameOver.load()) return false;
    const ULONGLONG now = GetTickCount64();
    const ULONGLONG gameOverAt = s_gameOverAt.load();
    // Give the engine a moment: it keeps drawing for several seconds after the results.
    if (now - s_lastEngineFrame.load() < 1000 || now - gameOverAt < 1000) return false;
    IDXGISwapChain* swapChain = s_swapChain.load();
    if (!swapChain) return false;
    static bool s_logged = false;
    if (!s_logged) {
        s_logged = true;
        NOBLE_INFO("PauseMenu: engine stopped drawing after the match; Noble draws the post-game panel itself");
    }
    static unsigned long long s_holdFrames = 0;
    ++s_holdFrames;
    {
        std::lock_guard<std::mutex> lock(s_renderMutex);
        RenderImpl(swapChain, false, true);
        // Test runs: one screenshot of the held panel, and -quit-after keeps working (it is normally
        // checked from slot 1, which no longer runs).
        static const bool s_testCapture = wcsstr(GetCommandLineW(), L"-capture-frames") != nullptr;
        if (s_testCapture && s_holdFrames == 60) FrameCapture::SaveNow(swapChain, 900000);
        FrameCapture::OnFrame(nullptr, 0);
    }
    swapChain->Present(1, 0);
    return true;
}

void PauseMenu::RenderImpl(IDXGISwapChain* swapChain, bool loading, bool clear) {
    const bool gameOver = s_gameOver.load();
    const bool open = s_open.load();
    const bool showFps = Settings::Get().Snapshot().showFps;
    if (!(open || gameOver || loading || showFps) || !swapChain || !ImGui::GetCurrentContext()) return;

    DXGI_SWAP_CHAIN_DESC desc{};
    if (FAILED(swapChain->GetDesc(&desc)) || !desc.OutputWindow) return;
    const HWND hwnd = desc.OutputWindow;

    ID3D11Device* device = nullptr;
    if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&device))) || !device) return;
    ID3D11DeviceContext* context = nullptr;
    device->GetImmediateContext(&context);

    ID3D11Texture2D* backBuffer = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    if (SUCCEEDED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))) && backBuffer) {
        device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    }

    if (context && rtv) {
        D3D11_TEXTURE2D_DESC bbDesc{};
        backBuffer->GetDesc(&bbDesc);

        // Feed ImGui ourselves on this thread (the Win32 backend's message handler runs on the
        // window thread and its event queue is not thread safe).
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // the window proc shows the arrow
        io.DisplaySize = ImVec2(static_cast<float>(bbDesc.Width), static_cast<float>(bbDesc.Height));

        static LARGE_INTEGER s_freq{}, s_last{};
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        if (s_freq.QuadPart == 0) QueryPerformanceFrequency(&s_freq);
        float dt = s_last.QuadPart ? static_cast<float>(now.QuadPart - s_last.QuadPart) / static_cast<float>(s_freq.QuadPart) : 1.0f / 60.0f;
        if (dt <= 0.0f || dt > 0.25f) dt = 1.0f / 60.0f;
        s_last = now;
        io.DeltaTime = dt;

        RECT client{};
        GetClientRect(hwnd, &client);
        POINT cursor{};
        if ((open || gameOver) && GetForegroundWindow() == hwnd && GetCursorPos(&cursor) &&
            ScreenToClient(hwnd, &cursor) && client.right > 0 && client.bottom > 0) {
            // Window may be resized while the back buffer stays the same size: scale to back-buffer pixels.
            const float sx = io.DisplaySize.x / static_cast<float>(client.right);
            const float sy = io.DisplaySize.y / static_cast<float>(client.bottom);
            io.AddMousePosEvent(static_cast<float>(cursor.x) * sx, static_cast<float>(cursor.y) * sy);
            // A click shorter than a frame is seen only as a WM_LBUTTONDOWN; give ImGui a full
            // press + release for it (ImGui spreads queued events over frames).
            const bool down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            if (Window::TakeLeftClicks() > 0 && !down) {
                io.AddMouseButtonEvent(0, true);
                io.AddMouseButtonEvent(0, false);
            } else {
                io.AddMouseButtonEvent(0, down);
            }
        } else {
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            io.AddMouseButtonEvent(0, false);
        }

        ImGui_ImplDX11_NewFrame();
        ImGui::NewFrame();
        if (showFps) {
            char fpsLabel[32];
            std::snprintf(fpsLabel, sizeof(fpsLabel), "FPS %.0f", io.Framerate);
            ImDrawList* foreground = ImGui::GetForegroundDrawList();
            const ImVec2 textSize = ImGui::CalcTextSize(fpsLabel);
            const ImVec2 pos(io.DisplaySize.x - textSize.x - 24.0f, 16.0f);
            foreground->AddRectFilled(ImVec2(pos.x - 8.0f, pos.y - 5.0f),
                ImVec2(pos.x + textSize.x + 8.0f, pos.y + textSize.y + 5.0f),
                IM_COL32(10, 16, 24, 190), 5.0f);
            foreground->AddText(pos, IM_COL32(143, 216, 255, 255), fpsLabel);
        }
        if (gameOver) {
            DrawGameOver(hwnd);
        } else {
            if (loading) DrawLoading();
            if (open) DrawMenu(hwnd);
        }
        ImGui::Render();

        // The DX11 backend saves/restores most pipeline state but not the output-merger targets.
        ID3D11RenderTargetView* oldRtv = nullptr;
        ID3D11DepthStencilView* oldDsv = nullptr;
        context->OMGetRenderTargets(1, &oldRtv, &oldDsv);
        context->OMSetRenderTargets(1, &rtv, nullptr);
        if (clear) {
            const float dark[4] = { 0.016f, 0.031f, 0.055f, 1.0f };
            context->ClearRenderTargetView(rtv, dark);
        }
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        context->OMSetRenderTargets(1, &oldRtv, oldDsv);
        if (oldRtv) oldRtv->Release();
        if (oldDsv) oldDsv->Release();
    }

    if (rtv) rtv->Release();
    if (backBuffer) backBuffer->Release();
    if (context) context->Release();
    device->Release();
}

} // namespace noble
