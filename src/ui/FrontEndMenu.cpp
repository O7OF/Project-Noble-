#include "FrontEndMenu.hpp"
#include "UIManager.hpp"
#include "NobleStyle.hpp"
#include "SettingsPanel.hpp"
#include "MenuSounds.hpp"
#include "UiArt.hpp"
#include "ServerBrowser.hpp"
#include "ArmorCustomizer.hpp"
#include "WorkshopManager.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "common/Version.hpp"
#include "engine/FrameCapture.hpp"
#include "host/Window.hpp"
#include "host/D3D11Renderer.hpp"

#include <windows.h>
#include <shellapi.h>
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwctype>
#include <string>
#include <vector>

namespace noble {

namespace {

// ---------------------------------------------------------------------------------------------
// Content
// ---------------------------------------------------------------------------------------------
struct MenuMap {
    const char* scenario;   // passed to BlamEngine::LaunchMap
    const char* name;       // shown in the list
    const char* blurb;      // one line under the preview
};

// Same maps as BlamEngine's multiplayer table (launch indices 190-209).
const MenuMap kMaps[] = {
    { "50_panopticon",     "Boardwalk",     "New Alexandria waterfront." },
    { "70_boneyard",       "Boneyard",      "A ship-breaking yard on the coast." },
    { "45_launch_station", "Countdown",     "Inside a Sabre launch facility." },
    { "30_settlement",     "Powerhouse",    "A hydroelectric plant in the hills." },
    { "52_ivory_tower",    "Reflection",    "A penthouse high above the city." },
    { "35_island",         "Spire",         "A Covenant spire on a rocky island." },
    { "20_sword_slayer",   "Sword Base",    "The atrium of ONI's Sword Base." },
    { "45_aftship",        "Zealot",        "Aboard a Covenant corvette." },
    { "dlc_slayer",        "Anchor 9",      "A space station in orbit over Reach." },
    { "dlc_invasion",      "Breakpoint",    "An ONI research site under attack." },
    { "dlc_medium",        "Tempest",       "A Forerunner coastline." },
    { "condemned",         "Condemned",     "An orbital station breaking apart." },
    { "trainingpreserve",  "Highlands",     "A training preserve in the hills." },
    { "cex_beaver_creek",  "Battle Canyon", "A remake of a canyon classic." },
    { "cex_damnation",     "Penance",       "A remake of a Covenant classic." },
    { "cex_timberland",    "Ridgeline",     "A remake of a forest classic." },
    { "cex_prisoner",      "Solitary",      "A remake of a prison classic." },
    { "cex_hangemhigh",    "High Noon",     "A remake of a tomb classic." },
    { "cex_headlong",      "Breakneck",     "A remake of a city classic." },
    { "forge_halo",        "Forge World",   "The largest canvas Reach has." },
};
constexpr int kDefaultMap = 6; // Sword Base: the map tested most

// Campaign missions (launch index 178-189).
const MenuMap kMissions[] = {
    { "m05",       "Noble Actual",          "Arrive on Reach and meet Noble Team. (Cinematic)" },
    { "m10",       "Winter Contingency",    "A relay station on the frontier has gone silent." },
    { "m20",       "ONI: Sword Base",       "The Covenant are attacking a vital ONI base." },
    { "m30",       "Nightfall",             "Scout behind enemy lines under cover of dark." },
    { "m35",       "Tip of the Spear",      "Two armies collide. Take out the spire." },
    { "m45",       "Long Night of Solace",  "Take the fight into orbit." },
    { "m50",       "Exodus",                "Get civilians out of an occupied city." },
    { "m52",       "New Alexandria",        "Air support over a city under siege." },
    { "m60",       "The Package",           "Back to Sword Base for one last job." },
    { "m70",       "The Pillar of Autumn",  "Deliver the package to the ship." },
    { "m70_bonus", "Lone Wolf",             "Hold out for as long as you can." },
};
const char* const kDifficulties[] = { "EASY", "NORMAL", "HEROIC", "LEGENDARY" };
const float kDifficultyMultiplier[] = { 0.75f, 1.0f, 1.25f, 1.5f };

// Firefight maps (launch index 210-219, as BlamEngine::ResolveMapId maps them).
const MenuMap kFirefightMaps[] = {
    { "ff50_park",       "Beachhead",       "" },
    { "ff45_corvette",   "Corvette",        "" },
    { "ff20_courtyard",  "Courtyard",       "" },
    { "ff60_icecave",    "Glacier",         "" },
    { "ff70_holdout",    "Holdout",         "" },
    { "ff60_airview",    "Outpost",         "" },
    { "ff10_prototype",  "Overlook",        "" },
    { "ff30_waterfront", "Waterfront",      "" },
    { "ff_unearthed",    "Unearthed",       "" },
    { "cex_ff_halo",     "Installation 04", "" },
};

// Reach's campaign skulls. `bit` is MCC's skull number (launch block +0x48), `icon` the name in
// MCC's skulls texture pack, `multiplier` the campaign-scoring bonus.
struct Skull {
    const char* name;
    int bit;
    const char* icon;
    const char* description;
    float multiplier;
    bool primary;
};
const Skull kSkulls[] = {
    { "Iron",         18, "IRON",         "Co-op: dying sends everyone back to the last checkpoint. Solo: death restarts the mission.", 1.2f, true },
    { "Black Eye",     3, "BLACKEYE",     "Your shields only recharge when you melee enemies.",                                          1.1f, true },
    { "Tough Luck",   36, "TOUGHLUCK",    "Enemies dodge grenades and dive out of danger far more often.",                               1.1f, true },
    { "Catch",         7, "CATCH",        "Enemies throw many more grenades.",                                                           1.1f, true },
    { "Cloud",        13, "CLOUD",        "Your motion tracker is turned off.",                                                         1.1f, true },
    { "Famine",       11, "FAMINE",       "Weapons dropped by enemies carry half the usual ammunition.",                                 1.2f, true },
    { "Thunderstorm", 34, "THUNDERSTORM", "Enemies are promoted to their higher ranks.",                                                 1.2f, true },
    { "Tilt",         35, "TILT",         "Enemy strengths and weaknesses are exaggerated.",                                             1.2f, true },
    { "Mythic",       23, "MYTHIC",       "Enemies have much more health.",                                                              1.2f, true },
    { "Blind",         4, "BLIND",        "Your HUD and first-person weapon are hidden.",                                                1.0f, false },
    { "Cowbell",       8, "COWBELL",      "Explosions throw things much farther.",                                                       1.0f, false },
    { "Grunt Birthday Party", 16, "GRUNTBIRTHDAYPARTY", "Headshots on Grunts end in a burst of confetti.",                              1.0f, false },
    { "IWHBYD",       19, "IWHBYD",       "Rare combat dialogue becomes much more common.",                                              1.0f, false },
};
constexpr int kSkullCount = static_cast<int>(sizeof(kSkulls) / sizeof(kSkulls[0]));

template <typename T, size_t N>
constexpr int Count(const T (&)[N]) { return static_cast<int>(N); }

// ---------------------------------------------------------------------------------------------
// Game variants (MCC's game_variants folder)
// ---------------------------------------------------------------------------------------------
struct MenuVariant {
    std::string label;
    std::filesystem::path path;
};

struct VariantName {
    const char* stem;   // lower-case file stem
    const char* label;
};
const VariantName kVariantNames[] = {
    { "slayer_054", "Slayer" }, { "slayer_pro_054", "Slayer Pro" }, { "slayer_classic_054", "Classic Slayer" },
    { "slayer_covy_054", "Elite Slayer" }, { "swat_054", "SWAT" }, { "ctf_054", "Capture the Flag" },
    { "ctf_1flag_054", "One Flag CTF" }, { "ctf_multiteam_054", "Multi Team CTF" }, { "ctf_neutralflag_054", "Neutral Flag" },
    { "assault_054", "Assault" }, { "assault_one_bomb_054", "One Bomb" }, { "assault_neutral_bomb_054", "Neutral Bomb" },
    { "headhunter_054", "Headhunter" }, { "headhunter_pro_054", "Headhunter Pro" }, { "infection_054", "Infection" },
    { "infection_safehavens_054", "Safe Havens" }, { "juggernaut_054", "Juggernaut" }, { "koth_054", "King of the Hill" },
    { "koth_crazyking_054", "Crazy King" }, { "oddball_054", "Oddball" }, { "race_054", "Race" }, { "rally_054", "Rally" },
    { "skeeball_default_054", "Skeeball" }, { "stockpile_054", "Stockpile" }, { "territories_054", "Territories" },
    { "territories-3plot_054", "3 Plots" }, { "territories-landgrab_054", "Land Grab" }, { "3nvasion_054", "Invasion" },
    { "3nvasion_boneyard_054", "Invasion: Boneyard" }, { "3nvasion_spire_054", "Invasion: Spire" },
    { "3nvasion_dlc_054", "Invasion: Breakpoint" }, { "2nvasion_slayer_054", "Invasion Slayer" },
    { "hr_4v4_team_3plots_dmr_ar_500points_tu", "Team 3 Plots (MCC)" },
    { "hr_4v4_team_assault_dmr_ar_3points_tu", "Team Assault (MCC)" },
    { "hr_4v4_team_crazyking_dmr_ar_150points_tu", "Team Crazy King (MCC)" },
    { "hr_4v4_team_headhunter_dmr_ar_50points_tu", "Team Headhunter (MCC)" },
    { "hr_4v4_team_multiflag_dmr_ar_3points_tu", "Team Multi Flag (MCC)" },
    { "hr_4v4_team_oddball_dmr_ar_150points_tu", "Team Oddball (MCC)" },
    { "hr_4v4_team_onebomb_dmr_ar_3points_tu", "Team One Bomb (MCC)" },
    { "hr_4v4_team_oneflag_dmr_ar_4rounds_tu", "Team One Flag (MCC)" },
    { "hr_4v4_team_slayer_dmr_ar_50points_tu", "Team Slayer (MCC)" },
    { "hr_4v4_team_stockpile_dmr_ar_10points_tu", "Team Stockpile (MCC)" },
    { "hr_4v4_team_territories_dmr_ar_4rounds_3min_tu", "Team Territories (MCC)" },
    { "hr_ffa_escalationslayer_mixed_tu", "Escalation Slayer (MCC)" },
    { "hr_ffa_escalationslayer_mixed_vanilla", "Escalation Slayer (original)" },
    { "hr_ffa_juggernaut_ar_mag_150points_tu", "Juggernaut (MCC)" },
    { "hr_ff_classic_1set", "Classic Firefight" }, { "hr_ff_generatordefense_1set", "Generator Defense" },
    { "hr_ff_gruntpocalypse_1round", "Gruntpocalypse" }, { "hr_ff_rocketfight_1set", "Rocketfight" },
    { "hr_ff_scoreattack_1round", "Score Attack" }, { "hr_ff_standard_1set", "Firefight" },
    { "hr_ff_versus_2turns", "Firefight Versus" },
};
constexpr const wchar_t* kForgeVariant = L"00_basic_editing_054.bin";

std::wstring Lower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return s;
}

std::string VariantLabel(const std::wstring& stem) {
    const std::wstring lower = Lower(stem);
    for (const VariantName& v : kVariantNames) {
        std::wstring w(v.stem, v.stem + strlen(v.stem));
        if (w == lower) return v.label;
    }
    // Unknown file (e.g. one the player added): "my_cool_game_054" -> "My Cool Game".
    std::string out;
    bool up = true;
    std::wstring s = stem;
    if (s.size() > 4 && Lower(s.substr(s.size() - 4)) == L"_054") s.resize(s.size() - 4);
    for (wchar_t c : s) {
        if (c == L'_' || c == L'-') { out.push_back(' '); up = true; continue; }
        char ch = c < 0x80 ? static_cast<char>(c) : '?';
        out.push_back(up ? static_cast<char>(::toupper(static_cast<unsigned char>(ch))) : ch);
        up = false;
    }
    return out;
}

std::vector<MenuVariant> ScanVariants(const std::filesystem::path& reachDir, bool firefight) {
    std::vector<MenuVariant> list;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(reachDir / "game_variants", ec)) {
        if (!entry.is_regular_file(ec) || Lower(entry.path().extension().wstring()) != L".bin") continue;
        const std::wstring lower = Lower(entry.path().filename().wstring());
        if (lower == Lower(kForgeVariant)) continue;           // Forge has its own menu entry
        const bool isFirefightVariant = lower.rfind(L"hr_ff_", 0) == 0;  // only valid on Firefight maps
        if (isFirefightVariant != firefight) continue;
        list.push_back({ VariantLabel(entry.path().stem().wstring()), entry.path() });
    }
    std::sort(list.begin(), list.end(), [](const MenuVariant& a, const MenuVariant& b) { return a.label < b.label; });
    return list;
}

int FindVariant(const std::vector<MenuVariant>& list, const wchar_t* file) {
    for (size_t i = 0; i < list.size(); ++i) {
        if (Lower(list[i].path.filename().wstring()) == Lower(file)) return static_cast<int>(i);
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------
// Drawing helpers (Reach's menu look: dark scene, big faint title, left-aligned upper-case list
// with a soft highlight bar, description under the list, button hints at the bottom).
// ---------------------------------------------------------------------------------------------
struct Ui {
    ImDrawList* dl = nullptr;
    ImVec2 size;   // screen size in pixels
    float s = 1.0f; // scale (1.0 at 720p)
};

const ImU32 kWhite = IM_COL32(236, 240, 244, 255);
const ImU32 kDim = IM_COL32(160, 170, 180, 255);
const ImU32 kDisabled = IM_COL32(110, 118, 126, 200);
const ImU32 kAccent = IM_COL32(143, 216, 255, 255);

ImFont* Font(ImFont* f) { return f ? f : ImGui::GetFont(); }

float TextWidth(ImFont* font, float px, const char* text) {
    return Font(font)->CalcTextSizeA(px, FLT_MAX, 0.0f, text).x;
}

// `wrapRight`: screen x where lines wrap (0 = no wrapping).
void Text(const Ui& ui, ImFont* font, float px, ImVec2 pos, ImU32 col, const char* text, float wrapRight = 0.0f) {
    ui.dl->AddText(Font(font), px * ui.s, pos, col, text, nullptr, wrapRight > pos.x ? wrapRight - pos.x : 0.0f);
}

std::string Upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}

void DrawBackdrop(const Ui& ui) {
    const ImVec2 o(0, 0);
    ui.dl->AddRectFilled(o, ui.size, IM_COL32(8, 12, 18, 255));
    UiArt::DrawCover(ui.dl, UiArt::MenuBackground(), o, ui.size, IM_COL32(255, 255, 255, 255));
    // Darken the left half (where the menu is) and the bottom, like Reach's menus.
    ui.dl->AddRectFilledMultiColor(o, ImVec2(ui.size.x * 0.45f, ui.size.y),
                                   IM_COL32(0, 0, 0, 205), IM_COL32(0, 0, 0, 120), IM_COL32(0, 0, 0, 120), IM_COL32(0, 0, 0, 205));
    ui.dl->AddRectFilledMultiColor(ImVec2(ui.size.x * 0.45f, 0), ImVec2(ui.size.x * 0.80f, ui.size.y),
                                   IM_COL32(0, 0, 0, 120), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 120));
    ui.dl->AddRectFilledMultiColor(ImVec2(0, ui.size.y * 0.72f), ui.size,
                                   IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 170), IM_COL32(0, 0, 0, 170));
}

// "◇ TITLE" with a faint vertical side label (as on Reach's Skulls/Controller screens).
void DrawTitle(const Ui& ui, ImVec2 pos, const char* title, const char* sideLabel = nullptr) {
    const float d = 15.0f * ui.s;
    const ImVec2 c(pos.x + d, pos.y + 28.0f * ui.s);
    ui.dl->AddQuad(ImVec2(c.x, c.y - d), ImVec2(c.x + d, c.y), ImVec2(c.x, c.y + d), ImVec2(c.x - d, c.y), kAccent, 2.0f * ui.s);
    ui.dl->AddQuad(ImVec2(c.x, c.y - d * 0.45f), ImVec2(c.x + d * 0.45f, c.y), ImVec2(c.x, c.y + d * 0.45f),
                   ImVec2(c.x - d * 0.45f, c.y), IM_COL32(143, 216, 255, 140), 1.5f * ui.s);
    Text(ui, NobleStyle::HeroFont(), 50.0f, ImVec2(pos.x + d * 2.6f, pos.y), IM_COL32(220, 226, 232, 215), title);
    if (sideLabel) {
        // Vertical label: draw letters one under another, small and faint.
        float y = pos.y + 70.0f * ui.s;
        for (const char* p = sideLabel; *p; ++p) {
            char ch[2] = { *p, 0 };
            Text(ui, NobleStyle::TitleFont(), 26.0f, ImVec2(pos.x + 2.0f * ui.s, y), IM_COL32(140, 150, 160, 70), ch);
            y += 24.0f * ui.s;
        }
    }
}

struct ListItem {
    std::string label;
    bool enabled = true;
};

// Reach-style vertical list. Returns the index activated this frame (click / Enter), else -1.
// `selected` follows mouse hover and Up/Down when `keyboard` is true.
int DrawList(const Ui& ui, const char* id, ImVec2 pos, float width, const std::vector<ListItem>& items, int& selected,
             bool keyboard, float rowHeight = 30.0f, int maxRows = 0) {
    const float rh = rowHeight * ui.s;
    int activated = -1;
    const int n = static_cast<int>(items.size());
    if (n == 0) return -1;
    selected = std::clamp(selected, 0, n - 1);

    if (keyboard) {
        auto step = [&](int dir) {
            for (int k = 0; k < n; ++k) {
                selected = (selected + dir + n) % n;
                if (items[selected].enabled) break;
            }
            MenuSounds::Tab();
        };
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown)) step(1);
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp)) step(-1);
        if ((ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false) ||
             ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false)) && items[selected].enabled) {
            activated = selected;
        }
    }

    // Scroll so the selection stays visible.
    int first = 0;
    if (maxRows > 0 && n > maxRows) first = std::clamp(selected - maxRows / 2, 0, n - maxRows);
    const int last = maxRows > 0 ? std::min(n, first + maxRows) : n;

    for (int i = first; i < last; ++i) {
        const ImVec2 a(pos.x, pos.y + (i - first) * rh);
        const ImVec2 b(a.x + width, a.y + rh);
        ImGui::SetCursorScreenPos(a);
        ImGui::PushID(id);
        ImGui::PushID(i);
        ImGui::InvisibleButton("##row", ImVec2(width, rh));
        const bool hovered = ImGui::IsItemHovered();
        const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        ImGui::PopID();
        ImGui::PopID();
        if (hovered && items[i].enabled && selected != i && ImGui::GetIO().MouseDelta.x * ImGui::GetIO().MouseDelta.x +
                                                            ImGui::GetIO().MouseDelta.y * ImGui::GetIO().MouseDelta.y > 0.0f) {
            selected = i;
            MenuSounds::Hover(ImGui::GetID(id) + i);
        }
        if (clicked && items[i].enabled) {
            selected = i;
            activated = i;
        }
        if (i == selected) {
            ui.dl->AddRectFilledMultiColor(a, b, IM_COL32(255, 255, 255, 46), IM_COL32(255, 255, 255, 4),
                                           IM_COL32(255, 255, 255, 4), IM_COL32(255, 255, 255, 46));
            ui.dl->AddRectFilled(a, ImVec2(a.x + 2.0f * ui.s, b.y), kWhite);
        }
        const ImU32 col = !items[i].enabled ? kDisabled : (i == selected ? kWhite : kDim);
        Text(ui, NobleStyle::TitleFont(), 20.0f, ImVec2(a.x + 9.0f * ui.s, a.y + (rh - 20.0f * ui.s) * 0.5f), col,
             Upper(items[i].label).c_str());
    }
    if (activated >= 0) MenuSounds::Click();
    return activated;
}

// Bottom-left hints: [Enter] Select   [Esc] Back ...
void DrawHints(const Ui& ui, const std::vector<std::pair<const char*, const char*>>& hints) {
    float x = ui.size.x * 0.075f;
    const float y = ui.size.y - 46.0f * ui.s;
    ui.dl->AddLine(ImVec2(x, y - 12.0f * ui.s), ImVec2(ui.size.x * 0.62f, y - 12.0f * ui.s), IM_COL32(255, 255, 255, 50), 1.0f);
    for (const auto& [key, label] : hints) {
        const float kw = TextWidth(NobleStyle::SmallFont(), 15.0f * ui.s, key) + 12.0f * ui.s;
        const ImVec2 a(x, y), b(x + kw, y + 22.0f * ui.s);
        ui.dl->AddRect(a, b, IM_COL32(200, 210, 220, 200), 3.0f * ui.s, 0, 1.2f);
        Text(ui, NobleStyle::SmallFont(), 15.0f, ImVec2(x + 6.0f * ui.s, y + 3.0f * ui.s), kWhite, key);
        x += kw + 7.0f * ui.s;
        Text(ui, NobleStyle::SmallFont(), 17.0f, ImVec2(x, y + 2.0f * ui.s), kWhite, label);
        x += TextWidth(NobleStyle::SmallFont(), 17.0f * ui.s, label) + 26.0f * ui.s;
    }
}

// "MY PARTY" box at the top right.
void DrawParty(const Ui& ui, const std::string& playerName, const char* status) {
    const float w = 330.0f * ui.s;
    const float x = ui.size.x - w - 40.0f * ui.s;
    float y = 34.0f * ui.s;
    ui.dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w * 0.80f, y + 26.0f * ui.s), IM_COL32(0, 0, 0, 200));
    Text(ui, NobleStyle::TitleFont(), 17.0f, ImVec2(x + 8.0f * ui.s, y + 3.0f * ui.s), kWhite, "MY PARTY");
    y += 26.0f * ui.s;
    const float x2 = x + 36.0f * ui.s;
    ui.dl->AddRectFilled(ImVec2(x2, y), ImVec2(x + w, y + 28.0f * ui.s), IM_COL32(20, 26, 32, 215));
    Text(ui, NobleStyle::TitleFont(), 17.0f, ImVec2(x2 + 8.0f * ui.s, y + 5.0f * ui.s), kWhite, status);
    Text(ui, NobleStyle::TitleFont(), 17.0f, ImVec2(x + w - 42.0f * ui.s, y + 5.0f * ui.s), kWhite, "1/16");
    y += 28.0f * ui.s;
    ui.dl->AddRectFilled(ImVec2(x2, y), ImVec2(x + w, y + 30.0f * ui.s), IM_COL32(52, 98, 138, 235));
    // Emblem placeholder: Noble's own simple mark (two chevrons).
    const ImVec2 e(x2 + 16.0f * ui.s, y + 15.0f * ui.s);
    ui.dl->AddCircleFilled(e, 10.0f * ui.s, IM_COL32(26, 40, 52, 255));
    ui.dl->AddTriangleFilled(ImVec2(e.x - 6 * ui.s, e.y - 2 * ui.s), ImVec2(e.x + 6 * ui.s, e.y - 2 * ui.s), ImVec2(e.x, e.y + 6 * ui.s), kAccent);
    Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(x2 + 34.0f * ui.s, y + 5.0f * ui.s), kWhite, playerName.c_str());
}

// Option row like Reach's settings: LABEL ........ ‹ VALUE ›. Returns -1/+1 when changed.
int OptionRow(const Ui& ui, const char* id, ImVec2 pos, float width, const char* label, const char* value, bool focused) {
    const float h = 32.0f * ui.s;
    int change = 0;
    const ImVec2 b(pos.x + width, pos.y + h);
    if (focused) {
        ui.dl->AddRectFilledMultiColor(pos, b, IM_COL32(60, 120, 170, 120), IM_COL32(60, 120, 170, 20),
                                       IM_COL32(60, 120, 170, 20), IM_COL32(60, 120, 170, 120));
        ui.dl->AddRectFilled(pos, ImVec2(pos.x + 2.0f * ui.s, b.y), kAccent);
    }
    Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(pos.x + 10.0f * ui.s, pos.y + 6.0f * ui.s), focused ? kWhite : kDim, label);
    const float vx = pos.x + width * 0.48f;
    const float vw = width * 0.52f;
    const float aw = 28.0f * ui.s;
    // Arrows
    ImGui::PushID(id);
    ImGui::SetCursorScreenPos(ImVec2(vx, pos.y));
    if (ImGui::InvisibleButton("##l", ImVec2(aw, h))) change = -1;
    const bool hl = ImGui::IsItemHovered();
    ImGui::SetCursorScreenPos(ImVec2(vx + vw - aw, pos.y));
    if (ImGui::InvisibleButton("##r", ImVec2(aw, h))) change = 1;
    const bool hr = ImGui::IsItemHovered();
    ImGui::PopID();
    const float cy = pos.y + h * 0.5f, ah = 6.0f * ui.s;
    ui.dl->AddTriangleFilled(ImVec2(vx + aw * 0.65f, cy - ah), ImVec2(vx + aw * 0.65f, cy + ah), ImVec2(vx + aw * 0.3f, cy), hl ? kWhite : kDim);
    const float rx = vx + vw - aw;
    ui.dl->AddTriangleFilled(ImVec2(rx + aw * 0.35f, cy - ah), ImVec2(rx + aw * 0.35f, cy + ah), ImVec2(rx + aw * 0.7f, cy), hr ? kWhite : kDim);
    const std::string v = Upper(value);
    const float tw = TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, v.c_str());
    Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(vx + (vw - tw) * 0.5f, pos.y + 6.0f * ui.s), kWhite, v.c_str());
    if (focused) {
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft)) change = -1;
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight)) change = 1;
    }
    if (change) MenuSounds::Tab();
    return change;
}

// Big action button (START GAME etc.).
bool ActionButton(const Ui& ui, const char* id, ImVec2 pos, float width, const char* label, bool primary = true) {
    const float h = 40.0f * ui.s;
    ImGui::SetCursorScreenPos(pos);
    ImGui::PushID(id);
    const bool pressed = ImGui::InvisibleButton("##b", ImVec2(width, h));
    const bool hovered = ImGui::IsItemHovered();
    ImGui::PopID();
    const ImU32 bg = primary ? (hovered ? IM_COL32(76, 150, 200, 255) : IM_COL32(52, 112, 156, 240))
                             : (hovered ? IM_COL32(60, 74, 88, 240) : IM_COL32(30, 38, 46, 230));
    ui.dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + h), bg);
    ui.dl->AddRect(pos, ImVec2(pos.x + width, pos.y + h), IM_COL32(170, 220, 255, hovered ? 220 : 90), 0.0f, 0, 1.0f);
    const float tw = TextWidth(NobleStyle::TitleFont(), 20.0f * ui.s, label);
    Text(ui, NobleStyle::TitleFont(), 20.0f, ImVec2(pos.x + (width - tw) * 0.5f, pos.y + 9.0f * ui.s), kWhite, label);
    if (hovered) MenuSounds::Hover(ImGui::GetID(id));
    if (pressed) MenuSounds::Click();
    return pressed;
}

// Map/mission preview with a thin frame, name and blurb under it.
float DrawPreview(const Ui& ui, ImVec2 pos, float width, const std::string& scenario, const char* name, const char* blurb) {
    const float h = width * 292.0f / 668.0f;
    const ImVec2 b(pos.x + width, pos.y + h);
    ui.dl->AddRectFilled(pos, b, IM_COL32(14, 20, 26, 230));
    UiTexture tex = UiArt::MapPreview(scenario, true);
    if (tex) UiArt::DrawCover(ui.dl, tex, pos, b);
    ui.dl->AddRect(pos, b, IM_COL32(170, 200, 220, 120), 0.0f, 0, 1.0f);
    float y = b.y + 10.0f * ui.s;
    Text(ui, NobleStyle::TitleFont(), 26.0f, ImVec2(pos.x, y), kWhite, name);
    y += 32.0f * ui.s;
    if (blurb && *blurb) {
        Text(ui, nullptr, 17.0f, ImVec2(pos.x, y), kDim, blurb, pos.x + width);
        y += 26.0f * ui.s;
    }
    return y;
}

std::string SkullSummary(uint64_t mask) {
    int count = 0;
    for (const Skull& s : kSkulls) if (mask & (1ull << s.bit)) ++count;
    if (count == 0) return "None";
    if (count == 1) {
        for (const Skull& s : kSkulls) if (mask & (1ull << s.bit)) return s.name;
    }
    return std::to_string(count) + " on";
}

float ScoreMultiplier(int difficulty, uint64_t mask) {
    // Reach campaign scoring: difficulty multiplier plus the sum of the active skulls' multipliers.
    float m = kDifficultyMultiplier[std::clamp(difficulty, 0, 3)];
    for (const Skull& s : kSkulls) {
        if ((mask & (1ull << s.bit)) && s.multiplier > 1.0f) m += s.multiplier;
    }
    return m;
}

// ---------------------------------------------------------------------------------------------
// Screens
// ---------------------------------------------------------------------------------------------
enum class Screen { Main, Campaign, Skulls, Firefight, CustomGame, Forge, Servers, Settings, Armory, Mods };

struct MainEntry {
    const char* label;
    const char* description;
    Screen target;
    bool enabled;
    bool quit;
};
const MainEntry kMainEntries[] = {
    { "Campaign",     "Play the story of Noble Team, from Winter Contingency to Lone Wolf.", Screen::Campaign, true, false },
    { "Matchmaking",  "Online matchmaking isn't available in Noble yet.",                     Screen::Main, false, false },
    { "Firefight",    "Hold out against waves of Covenant on the map and game type you choose.", Screen::Firefight, true, false },
    { "Custom Game",  "Play any multiplayer map and game type, host a game or join one.",     Screen::CustomGame, true, false },
    { "Forge",        "Edit multiplayer maps with Forge. Experimental in Noble.",             Screen::Forge, true, false },
    { "Theater",      "Saved films aren't supported in Noble yet.",                           Screen::Main, false, false },
    { "Armory",       "Customize your Spartan.",                                              Screen::Armory, true, false },
    { "Mods",         "Workshop maps and mods.",                                              Screen::Mods, true, false },
    { "Settings",     "Controls, key bindings, audio, video and gameplay options.",          Screen::Settings, true, false },
    { "Quit",         "Close Noble.",                                                         Screen::Main, true, true },
};

struct MenuState {
    Screen screen = Screen::Main;
    int mainIndex = 0;
    int mission = 1;
    int difficulty = 1;
    int focusRow = 0;           // right-hand option rows (sub-screens)
    int map = kDefaultMap;
    int variant = 0;
    int ffMap = 0;
    int ffVariant = 0;
    int forgeMap = 19;          // Forge World
    int skull = 0;
    uint64_t skulls = 0;
};

Screen ParseScreenArg(const std::wstring& v) {
    const std::wstring s = Lower(v);
    if (s == L"campaign") return Screen::Campaign;
    if (s == L"skulls") return Screen::Skulls;
    if (s == L"firefight") return Screen::Firefight;
    if (s == L"custom") return Screen::CustomGame;
    if (s == L"forge") return Screen::Forge;
    if (s == L"servers") return Screen::Servers;
    if (s == L"settings") return Screen::Settings;
    if (s == L"armory") return Screen::Armory;
    if (s == L"mods") return Screen::Mods;
    return Screen::Main;
}

} // namespace

bool FrontEndMenu::ShouldShow() {
    bool hasMap = false;
    bool menuRequested = false;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        for (int i = 1; i + 1 < argc; ++i) {
            const std::wstring arg = argv[i];
            const std::wstring value = Lower(argv[i + 1]);
            if (arg == L"-map") {
                hasMap = true;
                if (value == L"mainmenu" || value == L"menu") menuRequested = true;
            } else if (arg == L"-mode" && (value == L"mainmenu" || value == L"menu")) {
                menuRequested = true;
            }
        }
        LocalFree(argv);
    }
    return !hasMap || menuRequested;
}

FrontEndChoice FrontEndMenu::Run(Window& window, D3D11Renderer& renderer, const std::filesystem::path& reachDir) {
    FrontEndChoice choice;
    const std::vector<MenuVariant> variants = ScanVariants(reachDir, false);
    const std::vector<MenuVariant> ffVariants = ScanVariants(reachDir, true);
    const std::filesystem::path forgeVariant = reachDir / "game_variants" / kForgeVariant;

    MenuState st;
    st.variant = FindVariant(variants, L"slayer_054.bin");
    st.ffVariant = FindVariant(ffVariants, L"hr_ff_standard_1set.bin");
    st.skulls = Settings::Get().Snapshot().campaignSkulls;

    // Test options: "-menu-screen <name>" opens a screen directly, "-menu-capture <frames>" saves a
    // screenshot after that many frames and closes Noble (menus can't be clicked in automated tests).
    unsigned long long captureFrame = 0;
    int startTab = 0;
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (std::wstring(argv[i]) == L"-menu-screen") st.screen = ParseScreenArg(argv[i + 1]);
            if (std::wstring(argv[i]) == L"-menu-capture") captureFrame = _wtoi64(argv[i + 1]);
            if (std::wstring(argv[i]) == L"-menu-tab") startTab = _wtoi(argv[i + 1]);
        }
        if (argv) LocalFree(argv);
    }
    NOBLE_INFO("FrontEndMenu: showing menu ({} game variants, {} Firefight variants)", variants.size(), ffVariants.size());

    // Route window messages to ImGui only while the menu is up (main thread owns ImGui here).
    window.SetCustomWndProc([](HWND h, UINT m, WPARAM w, LPARAM l) { return UIManager::HandleWndProc(h, m, w, l); });

    SettingsPanel settingsPanel;
    settingsPanel.SetTab(startTab);
    bool modsScanned = false;
    bool done = false;
    Screen lastScreen = st.screen;
    unsigned long long frame = 0;

    auto saveSkulls = [&] {
        SettingsData d = Settings::Get().Snapshot();
        d.campaignSkulls = st.skulls;
        Settings::Get().Update(d);
    };

    while (!done && window.ProcessMessages()) {
        renderer.BeginFrame();
        UIManager::BeginFrame();
        ++frame;

        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground |
                                       ImGuiWindowFlags_NoScrollWithMouse;
        ImGui::Begin("##NobleFrontEnd", nullptr, flags);
        ImGui::PopStyleVar(2);

        Ui ui;
        ui.dl = ImGui::GetWindowDrawList();
        ui.size = vp->Size;
        ui.s = std::max(0.75f, vp->Size.y / 720.0f);
        DrawBackdrop(ui);

        const float left = ui.size.x * 0.075f;
        const float top = ui.size.y * 0.09f;
        const SettingsData settings = Settings::Get().Snapshot();
        const bool back = ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false);
        auto goBack = [&](Screen to) {
            st.screen = to;
            st.focusRow = 0;
            MenuSounds::Click();
        };

        switch (st.screen) {
        case Screen::Main: {
            DrawParty(ui, settings.playerName, "In Main Menu Lobby");
            DrawTitle(ui, ImVec2(left, ui.size.y * 0.30f), "MAIN MENU");
            Text(ui, nullptr, 17.0f, ImVec2(left + 4.0f * ui.s, ui.size.y * 0.30f + 66.0f * ui.s), kDim,
                 "This party is local to this PC.");
            std::vector<ListItem> items;
            for (const MainEntry& e : kMainEntries) items.push_back({ e.label, e.enabled });
            const float listY = ui.size.y * 0.30f + 104.0f * ui.s;
            const int hit = DrawList(ui, "main", ImVec2(left, listY), 440.0f * ui.s, items, st.mainIndex, true, 27.0f);
            const float descY = listY + items.size() * 27.0f * ui.s + 10.0f * ui.s;
            Text(ui, nullptr, 17.0f, ImVec2(left + 4.0f * ui.s, descY), kDim, kMainEntries[st.mainIndex].description,
                 left + 430.0f * ui.s);
            if (hit >= 0) {
                if (kMainEntries[hit].quit) done = true;
                else st.screen = kMainEntries[hit].target;
                st.focusRow = 0;
            }
            DrawHints(ui, { { "ENTER", "Select" }, { "ESC", "Back" } });
            std::string version(VERSION_STRING);
            const size_t paren = version.find(" (");
            if (paren != std::string::npos) version.resize(paren);
            char ver[64];
            std::snprintf(ver, sizeof(ver), "PROJECT NOBLE  v%s", version.c_str());
            Text(ui, NobleStyle::SmallFont(), 14.0f,
                 ImVec2(ui.size.x - TextWidth(NobleStyle::SmallFont(), 14.0f * ui.s, ver) - 24.0f * ui.s, ui.size.y - 30.0f * ui.s),
                 IM_COL32(200, 210, 220, 110), ver);
            break;
        }

        case Screen::Campaign: {
            DrawTitle(ui, ImVec2(left, top), "CAMPAIGN", "SOLO");
            std::vector<ListItem> items;
            for (const MenuMap& m : kMissions) items.push_back({ m.name, true });
            const float listY = top + 96.0f * ui.s;
            const int hit = DrawList(ui, "missions", ImVec2(left + 34.0f * ui.s, listY), 380.0f * ui.s, items, st.mission,
                                     st.focusRow == 0, 30.0f);
            if (hit >= 0) st.focusRow = 1;

            const float px = ui.size.x * 0.47f, pw = ui.size.x * 0.44f;
            float y = DrawPreview(ui, ImVec2(px, top + 96.0f * ui.s), pw, kMissions[st.mission].scenario,
                                  kMissions[st.mission].name, kMissions[st.mission].blurb);
            y += 6.0f * ui.s;
            const int dd = OptionRow(ui, "diff", ImVec2(px, y), pw, "DIFFICULTY", kDifficulties[st.difficulty], st.focusRow == 1);
            if (dd) st.difficulty = (st.difficulty + dd + 4) % 4;
            y += 36.0f * ui.s;
            const std::string skullText = SkullSummary(st.skulls);
            if (OptionRow(ui, "skulls", ImVec2(px, y), pw, "SKULLS", skullText.c_str(), st.focusRow == 2)) {
                st.screen = Screen::Skulls;
            }
            ImGui::SetCursorScreenPos(ImVec2(px, y));
            if (ImGui::InvisibleButton("##skullrow", ImVec2(pw * 0.46f, 32.0f * ui.s))) st.screen = Screen::Skulls;
            y += 36.0f * ui.s;
            char mult[64];
            std::snprintf(mult, sizeof(mult), "Score multiplier  x%.2f", ScoreMultiplier(st.difficulty, st.skulls));
            Text(ui, nullptr, 16.0f, ImVec2(px + 10.0f * ui.s, y + 4.0f * ui.s), kDim, mult);
            y += 34.0f * ui.s;
            const bool start = ActionButton(ui, "start", ImVec2(px, y), pw, "START MISSION") ||
                               (st.focusRow == 3 && ImGui::IsKeyPressed(ImGuiKey_Enter, false));
            if (start) {
                choice.play = true;
                choice.mode = "campaign";
                choice.map = kMissions[st.mission].scenario;
                choice.difficulty = st.difficulty;
                choice.skulls = st.skulls;
                done = true;
            }
            // Focus: list -> difficulty -> skulls -> start (Tab / Right moves on, Esc goes back a step).
            if (st.focusRow > 0 && (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_Tab))) st.focusRow = std::min(3, st.focusRow + 1);
            if (st.focusRow > 0 && ImGui::IsKeyPressed(ImGuiKey_UpArrow)) st.focusRow = std::max(1, st.focusRow - 1);
            if (st.focusRow == 2 && ImGui::IsKeyPressed(ImGuiKey_Enter, false)) st.screen = Screen::Skulls;
            if (back) {
                if (st.focusRow > 0) st.focusRow = 0;
                else goBack(Screen::Main);
            }
            DrawHints(ui, { { "ENTER", "Select" }, { "ESC", "Back" }, { "LEFT / RIGHT", "Change" } });
            break;
        }

        case Screen::Skulls: {
            DrawTitle(ui, ImVec2(left, top), "SKULLS", "CAMPAIGN");
            const float gx = left + 46.0f * ui.s;
            float gy = top + 92.0f * ui.s;
            const float cell = 78.0f * ui.s, icon = 64.0f * ui.s;
            auto section = [&](const char* name, float y) {
                Text(ui, NobleStyle::TitleFont(), 20.0f, ImVec2(gx, y), kWhite, name);
                ui.dl->AddLine(ImVec2(gx, y + 26.0f * ui.s), ImVec2(gx + cell * 5.0f, y + 26.0f * ui.s), IM_COL32(255, 255, 255, 60));
            };
            // Keyboard: arrows move, Enter toggles.
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) st.skull = std::min(kSkullCount - 1, st.skull + 1);
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) st.skull = std::max(0, st.skull - 1);
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) st.skull = std::min(kSkullCount - 1, st.skull + 5);
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) st.skull = std::max(0, st.skull - 5);
            bool toggle = ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_Space, false);

            section("Primary Skulls", gy);
            gy += 36.0f * ui.s;
            int primaryIndex = 0, secondaryIndex = 0;
            float secondaryTop = gy + cell * 2.0f + 14.0f * ui.s;
            for (int i = 0; i < kSkullCount; ++i) {
                const Skull& sk = kSkulls[i];
                int idx = sk.primary ? primaryIndex++ : secondaryIndex++;
                const float baseY = sk.primary ? gy : secondaryTop + 36.0f * ui.s;
                const ImVec2 a(gx + (idx % 5) * cell, baseY + (idx / 5) * cell);
                ImGui::SetCursorScreenPos(a);
                ImGui::PushID(i);
                if (ImGui::InvisibleButton("##skull", ImVec2(icon, icon))) {
                    st.skull = i;
                    toggle = true;
                }
                if (ImGui::IsItemHovered() && ImGui::GetIO().MouseDelta.x != 0.0f) st.skull = i;
                ImGui::PopID();
                const bool on = (st.skulls & (1ull << sk.bit)) != 0;
                if (i == st.skull) ui.dl->AddCircle(ImVec2(a.x + icon * 0.5f, a.y + icon * 0.5f), icon * 0.56f, kWhite, 0, 2.0f * ui.s);
                if (on) ui.dl->AddCircleFilled(ImVec2(a.x + icon * 0.5f, a.y + icon * 0.5f), icon * 0.52f, IM_COL32(230, 160, 90, 70));
                UiTexture tex = UiArt::SkullIcon(sk.icon);
                const ImU32 tint = on ? IM_COL32(255, 205, 160, 255) : IM_COL32(255, 255, 255, 105);
                if (tex) ui.dl->AddImage(tex.id, a, ImVec2(a.x + icon, a.y + icon), ImVec2(0, 0), ImVec2(1, 1), tint);
                else Text(ui, nullptr, 14.0f, ImVec2(a.x, a.y + icon * 0.4f), on ? kWhite : kDim, sk.name, a.x + icon);
            }
            section("Secondary Skulls", secondaryTop);
            if (toggle) {
                st.skulls ^= 1ull << kSkulls[st.skull].bit;
                saveSkulls();
                MenuSounds::Click();
            }

            // Detail panel
            const Skull& sel = kSkulls[st.skull];
            const float dx = ui.size.x * 0.60f;
            float dy = top + 70.0f * ui.s;
            UiTexture big = UiArt::SkullIcon(sel.icon);
            const float bs = 170.0f * ui.s;
            if (big) ui.dl->AddImage(big.id, ImVec2(dx + 40.0f * ui.s, dy), ImVec2(dx + 40.0f * ui.s + bs, dy + bs));
            dy += bs + 18.0f * ui.s;
            Text(ui, NobleStyle::HeroFont(), 34.0f, ImVec2(dx, dy), kWhite, sel.name);
            dy += 42.0f * ui.s;
            Text(ui, nullptr, 17.0f, ImVec2(dx, dy), kDim, sel.description, dx + 300.0f * ui.s);
            dy += 74.0f * ui.s;
            const bool selOn = (st.skulls & (1ull << sel.bit)) != 0;
            Text(ui, nullptr, 17.0f, ImVec2(dx, dy), kWhite, selOn ? "Status: On" : "Status: Off");
            dy += 24.0f * ui.s;
            char mult[64];
            if (sel.multiplier > 1.0f) std::snprintf(mult, sizeof(mult), "Score bonus: +%.1f", sel.multiplier);
            else std::snprintf(mult, sizeof(mult), "No score bonus");
            Text(ui, nullptr, 17.0f, ImVec2(dx, dy), kDim, mult);

            // Bottom strip like Reach's (player + score multiplier).
            const float by = ui.size.y - 104.0f * ui.s;
            ui.dl->AddRectFilledMultiColor(ImVec2(left + 40.0f * ui.s, by), ImVec2(ui.size.x * 0.80f, by + 40.0f * ui.s),
                                           IM_COL32(40, 52, 64, 210), IM_COL32(40, 52, 64, 60), IM_COL32(40, 52, 64, 60), IM_COL32(40, 52, 64, 210));
            Text(ui, NobleStyle::TitleFont(), 24.0f, ImVec2(left + 52.0f * ui.s, by + 6.0f * ui.s), kWhite, settings.playerName.c_str());
            char total[64];
            std::snprintf(total, sizeof(total), "x%.2f", ScoreMultiplier(st.difficulty, st.skulls));
            Text(ui, NobleStyle::TitleFont(), 24.0f, ImVec2(ui.size.x * 0.80f - 90.0f * ui.s, by + 6.0f * ui.s), kWhite, total);
            if (back) goBack(Screen::Campaign);
            DrawHints(ui, { { "ENTER", "Toggle" }, { "ESC", "Back" } });
            break;
        }

        case Screen::Firefight:
        case Screen::CustomGame:
        case Screen::Forge: {
            const bool ff = st.screen == Screen::Firefight;
            const bool forge = st.screen == Screen::Forge;
            DrawTitle(ui, ImVec2(left, top), ff ? "FIREFIGHT" : forge ? "FORGE" : "CUSTOM GAME", ff ? "SURVIVAL" : forge ? "EDIT" : "MULTIPLAYER");
            const MenuMap* maps = ff ? kFirefightMaps : kMaps;
            const int mapCount = ff ? Count(kFirefightMaps) : Count(kMaps);
            int& mapIndex = ff ? st.ffMap : forge ? st.forgeMap : st.map;
            std::vector<ListItem> items;
            for (int i = 0; i < mapCount; ++i) items.push_back({ maps[i].name, true });
            const int hit = DrawList(ui, "maps", ImVec2(left + 34.0f * ui.s, top + 96.0f * ui.s), 330.0f * ui.s, items, mapIndex,
                                     st.focusRow == 0, 27.0f, 15);
            if (hit >= 0) st.focusRow = 1;

            const float px = ui.size.x * 0.47f, pw = ui.size.x * 0.44f;
            float y = DrawPreview(ui, ImVec2(px, top + 96.0f * ui.s), pw, maps[mapIndex].scenario, maps[mapIndex].name, maps[mapIndex].blurb);
            y += 6.0f * ui.s;
            int focusRows = 1;
            if (!forge) {
                const std::vector<MenuVariant>& list = ff ? ffVariants : variants;
                int& v = ff ? st.ffVariant : st.variant;
                const char* label = list.empty() ? "Default" : list[std::clamp(v, 0, static_cast<int>(list.size()) - 1)].label.c_str();
                const int dv = OptionRow(ui, "variant", ImVec2(px, y), pw, "GAME TYPE", label, st.focusRow == focusRows);
                if (dv && !list.empty()) v = (v + dv + static_cast<int>(list.size())) % static_cast<int>(list.size());
                y += 36.0f * ui.s;
                ++focusRows;
            }
            if (ff) {
                const int dd = OptionRow(ui, "ffdiff", ImVec2(px, y), pw, "DIFFICULTY", kDifficulties[st.difficulty], st.focusRow == focusRows);
                if (dd) st.difficulty = (st.difficulty + dd + 4) % 4;
                y += 36.0f * ui.s;
                ++focusRows;
            }
            if (forge && !std::filesystem::exists(forgeVariant)) {
                Text(ui, nullptr, 16.0f, ImVec2(px + 10.0f * ui.s, y), IM_COL32(255, 190, 120, 255),
                     "MCC's Forge game type (00_basic_editing_054.bin) wasn't found.");
                y += 26.0f * ui.s;
            }
            y += 10.0f * ui.s;
            const bool start = ActionButton(ui, "start", ImVec2(px, y), pw, forge ? "START FORGE" : "START GAME") ||
                               (st.focusRow == focusRows && ImGui::IsKeyPressed(ImGuiKey_Enter, false));
            y += 48.0f * ui.s;
            if (!ff && !forge && ActionButton(ui, "browse", ImVec2(px, y), pw, "HOST OR JOIN GAMES", false)) st.screen = Screen::Servers;
            if (start) {
                choice.play = true;
                choice.map = maps[mapIndex].scenario;
                if (ff) {
                    choice.mode = "firefight";
                    choice.difficulty = st.difficulty;
                    if (!ffVariants.empty()) choice.variantPath = ffVariants[st.ffVariant].path.string();
                } else {
                    choice.mode = "multiplayer";
                    if (forge) choice.variantPath = forgeVariant.string();
                    else if (!variants.empty()) choice.variantPath = variants[st.variant].path.string();
                }
                done = true;
            }
            if (st.focusRow > 0 && (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_Tab))) st.focusRow = std::min(focusRows, st.focusRow + 1);
            if (st.focusRow > 0 && ImGui::IsKeyPressed(ImGuiKey_UpArrow)) st.focusRow = std::max(1, st.focusRow - 1);
            if (back) {
                if (st.focusRow > 0) st.focusRow = 0;
                else goBack(Screen::Main);
            }
            DrawHints(ui, { { "ENTER", "Select" }, { "ESC", "Back" }, { "LEFT / RIGHT", "Change" } });
            break;
        }

        case Screen::Servers:
        case Screen::Settings:
        case Screen::Armory:
        case Screen::Mods: {
            const char* title = st.screen == Screen::Servers ? "GAMES" : st.screen == Screen::Settings ? "SETTINGS"
                              : st.screen == Screen::Armory ? "ARMORY" : "MODS";
            DrawTitle(ui, ImVec2(left, top), title);
            // The existing panels are regular ImGui widgets: put them in a framed child.
            ImGui::SetCursorScreenPos(ImVec2(left, top + 84.0f * ui.s));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.03f, 0.05f, 0.07f, 0.82f));
            ImGui::BeginChild("##panel", ImVec2(ui.size.x - left * 2.0f, ui.size.y - top - 84.0f * ui.s - 70.0f * ui.s),
                              ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
            if (st.screen == Screen::Servers) {
                UIManager::GetServerBrowser().Render();
            } else if (st.screen == Screen::Settings) {
                settingsPanel.Render();
            } else if (st.screen == Screen::Armory) {
                UIManager::GetArmorCustomizer().Render(renderer.GetDevice(), renderer.GetContext());
            } else {
                if (!modsScanned) {
                    // <steamapps>\workshop\content\976730 (MCC) and Noble's own data\mods.
                    const auto steamapps = reachDir.parent_path().parent_path().parent_path();
                    wchar_t exe[MAX_PATH] = {};
                    GetModuleFileNameW(nullptr, exe, MAX_PATH);
                    UIManager::GetWorkshopManager().RefreshMods(steamapps / "workshop" / "content" / "976730",
                                                                std::filesystem::path(exe).parent_path() / "data" / "mods");
                    modsScanned = true;
                }
                UIManager::GetWorkshopManager().Render();
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            // Esc goes back unless a text box or key rebind wants it.
            if (back && !ImGui::GetIO().WantTextInput && !settingsPanel.IsCapturingKey()) {
                goBack(st.screen == Screen::Servers ? Screen::CustomGame : Screen::Main);
            }
            DrawHints(ui, { { "ESC", "Back" } });
            break;
        }
        }

        if (st.screen != lastScreen) {
            if (st.screen != Screen::Main || lastScreen != Screen::Main) MenuSounds::Tab();
            lastScreen = st.screen;
        }

        ImGui::End();

        UIManager::EndFrame();
        if (captureFrame && frame == captureFrame) {
            FrameCapture::SaveNow(renderer.GetSwapChain(), frame);
            NOBLE_INFO("FrontEndMenu: -menu-capture done, closing");
            choice.play = false;
            done = true;
        }
        renderer.EndFrame(true);
    }

    // From here on the engine thread owns rendering; the pause menu feeds ImGui itself.
    window.SetCustomWndProc(nullptr);
    if (choice.play) {
        NOBLE_INFO("FrontEndMenu: PLAY {} {} (difficulty {}, skulls 0x{:X}, variant '{}')", choice.mode, choice.map,
                   choice.difficulty, choice.skulls, choice.variantPath);
    } else {
        NOBLE_INFO("FrontEndMenu: quit from menu");
    }
    return choice;
}

} // namespace noble
