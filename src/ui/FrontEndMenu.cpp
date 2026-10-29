#include "FrontEndMenu.hpp"
#include "network/UPnP.hpp"
#include "network/AuthClient.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/SessionManager.hpp"
#include <format>
#include <nlohmann/json.hpp>
#include "UIManager.hpp"
#include "NobleStyle.hpp"
#include "SettingsPanel.hpp"
#include "CareerPanel.hpp"
#include "MenuSounds.hpp"
#include "InputPrompts.hpp"
#include "audio/ReachAudio.hpp"
#include "UiArt.hpp"
#include "ServerBrowser.hpp"
#include "ArmorCustomizer.hpp"
#include "WorkshopManager.hpp"
#include "ModCatalog.hpp"
#include "vfs/VFS.hpp"
#include "career/Career.hpp"
#include "common/Logger.hpp"
#include "common/Relaunch.hpp"
#include "common/Settings.hpp"
#include "common/Version.hpp"
#include "engine/FrameCapture.hpp"
#include "engine/ReachArmory.hpp"
#include "host/Window.hpp"
#include "host/D3D11Renderer.hpp"

#include <windows.h>
#include <shellapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cwctype>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <utility>

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
const char* const kDifficultyNames[] = { "Easy", "Normal", "Heroic", "Legendary" };

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
        if (lower == Lower(kForgeVariant)) continue;
        const bool isFirefightVariant = lower.rfind(L"hr_ff_", 0) == 0;
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
// Drawing helpers
// ---------------------------------------------------------------------------------------------
struct Ui {
    ImDrawList* dl = nullptr;
    ImVec2 size;
    float s = 1.0f;
};

const ImU32 kWhite = IM_COL32(236, 240, 244, 255);
const ImU32 kDim = IM_COL32(160, 170, 180, 255);
const ImU32 kDisabled = IM_COL32(110, 118, 126, 200);
const ImU32 kAccent = IM_COL32(143, 216, 255, 255);

ImFont* Font(ImFont* f) { return f ? f : ImGui::GetFont(); }

float TextWidth(ImFont* font, float px, const char* text) {
    return Font(font)->CalcTextSizeA(px, FLT_MAX, 0.0f, text).x;
}

void Text(const Ui& ui, ImFont* font, float px, ImVec2 pos, ImU32 col, const char* text, float wrapRight = 0.0f) {
    ui.dl->AddText(Font(font), px * ui.s, pos, col, text, nullptr, wrapRight > pos.x ? wrapRight - pos.x : 0.0f);
}

std::string Upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}

enum class Screen { Main, Campaign, Skulls, Firefight, CustomGame, Forge, Servers, Settings, Armory, Mods, Career, PreGameLobby };

void DrawBackdrop(const Ui& ui, Screen screen) {
    const ImVec2 o(0, 0);
    if (!ReachArmory::IsSceneReady()) {
        // Fallback 2D background only during the initial 1-2s while mainmenu_all.map loads into haloreach.dll
        ui.dl->AddRectFilled(o, ui.size, IM_COL32(8, 12, 18, 255));
        UiArt::DrawCover(ui.dl, UiArt::MenuBackground(), o, ui.size, IM_COL32(255, 255, 255, 255));
    }

    if (screen == Screen::Armory) {
        // In the Armory, keep the right side 100% clear so the live 3D Spartan/Elite in mainmenu_all.map shines
        ui.dl->AddRectFilledMultiColor(o, ImVec2(ui.size.x * 0.42f, ui.size.y),
                                       IM_COL32(4, 8, 14, 175), IM_COL32(4, 8, 14, 70),
                                       IM_COL32(4, 8, 14, 70), IM_COL32(4, 8, 14, 175));
        ui.dl->AddRectFilledMultiColor(ImVec2(ui.size.x * 0.42f, 0), ImVec2(ui.size.x * 0.56f, ui.size.y),
                                       IM_COL32(4, 8, 14, 70), IM_COL32(4, 8, 14, 0),
                                       IM_COL32(4, 8, 14, 0), IM_COL32(4, 8, 14, 70));
        ui.dl->AddRectFilledMultiColor(ImVec2(0, ui.size.y * 0.82f), ui.size,
                                       IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
                                       IM_COL32(0, 0, 0, 150), IM_COL32(0, 0, 0, 150));
    } else {
        // Darken the left half (where the menu list is) and the bottom, like Reach's native menus.
        ui.dl->AddRectFilledMultiColor(o, ImVec2(ui.size.x * 0.45f, ui.size.y),
                                       IM_COL32(0, 0, 0, 190), IM_COL32(0, 0, 0, 105),
                                       IM_COL32(0, 0, 0, 105), IM_COL32(0, 0, 0, 190));
        ui.dl->AddRectFilledMultiColor(ImVec2(ui.size.x * 0.45f, 0), ImVec2(ui.size.x * 0.78f, ui.size.y),
                                       IM_COL32(0, 0, 0, 105), IM_COL32(0, 0, 0, 0),
                                       IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 105));
        ui.dl->AddRectFilledMultiColor(ImVec2(0, ui.size.y * 0.72f), ui.size,
                                       IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
                                       IM_COL32(0, 0, 0, 165), IM_COL32(0, 0, 0, 165));
    }
}

void DrawTitle(const Ui& ui, ImVec2 pos, const char* title, const char* sideLabel = nullptr) {
    const float d = 15.0f * ui.s;
    const ImVec2 c(pos.x + d, pos.y + 28.0f * ui.s);
    ui.dl->AddQuad(ImVec2(c.x, c.y - d), ImVec2(c.x + d, c.y), ImVec2(c.x, c.y + d), ImVec2(c.x - d, c.y), kAccent, 2.0f * ui.s);
    ui.dl->AddQuad(ImVec2(c.x, c.y - d * 0.45f), ImVec2(c.x + d * 0.45f, c.y), ImVec2(c.x, c.y + d * 0.45f),
                   ImVec2(c.x - d * 0.45f, c.y), IM_COL32(143, 216, 255, 140), 1.5f * ui.s);
    Text(ui, NobleStyle::HeroFont(), 50.0f, ImVec2(pos.x + d * 2.6f, pos.y), IM_COL32(220, 226, 232, 215), title);
    if (sideLabel) {
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
    bool header = false;
    bool expanded = false;
    std::string thumb;   // scenario whose small preview is drawn before the label (empty = none)
    int number = 0;      // shown before the thumbnail when > 0
};

bool AcceptPressed();

int DrawList(const Ui& ui, const char* id, ImVec2 pos, float width, const std::vector<ListItem>& items, int& selected,
             bool keyboard, float rowHeight = 30.0f, int maxRows = 0, bool hoverSelects = true) {
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
            MenuSounds::Scroll();
        };
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown)) step(1);
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp)) step(-1);
        if (AcceptPressed() && items[selected].enabled) {
            activated = selected;
        }
    }

    static std::unordered_map<ImGuiID, int> s_first, s_lastSelected;
    const ImGuiID key = ImGui::GetID(id);
    int& first = s_first[key];
    if (maxRows > 0 && n > maxRows) {
        auto lastSel = s_lastSelected.find(key);
        if (lastSel == s_lastSelected.end() || lastSel->second != selected) {
            if (selected < first) first = selected;
            else if (selected >= first + maxRows) first = selected - maxRows + 1;
            s_lastSelected[key] = selected;
        }
        const ImVec2 area0 = pos, area1(pos.x + width + 14.0f * ui.s, pos.y + maxRows * rh);
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f && ImGui::IsMouseHoveringRect(area0, area1, false)) first -= static_cast<int>(wheel * 3.0f);
        first = std::clamp(first, 0, n - maxRows);
        const float trackX = pos.x + width + 6.0f * ui.s, trackH = maxRows * rh;
        ui.dl->AddRectFilled(ImVec2(trackX, pos.y), ImVec2(trackX + 3.0f * ui.s, pos.y + trackH), IM_COL32(255, 255, 255, 25));
        const float thumbH = std::max(18.0f * ui.s, trackH * maxRows / n);
        const float thumbY = pos.y + (trackH - thumbH) * first / static_cast<float>(n - maxRows);
        ui.dl->AddRectFilled(ImVec2(trackX, thumbY), ImVec2(trackX + 3.0f * ui.s, thumbY + thumbH), IM_COL32(143, 216, 255, 170));
    } else {
        first = 0;
    }
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
        const bool mouseMoved = ImGui::GetIO().MouseDelta.x * ImGui::GetIO().MouseDelta.x +
                                ImGui::GetIO().MouseDelta.y * ImGui::GetIO().MouseDelta.y > 0.0f;
        if (hovered && items[i].enabled && selected != i && mouseMoved) {
            if (hoverSelects) selected = i;
            MenuSounds::Hover(ImGui::GetID(id) + i);
        }
        if (hovered && !hoverSelects && i != selected && items[i].enabled) {
            ui.dl->AddRectFilled(a, b, IM_COL32(255, 255, 255, 16));
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
        if (items[i].header) {
            const float cx = a.x + 14.0f * ui.s, cy = a.y + rh * 0.5f, t = 5.0f * ui.s;
            const ImU32 ac = i == selected ? kWhite : kAccent;
            if (items[i].expanded) ui.dl->AddTriangleFilled(ImVec2(cx - t, cy - t * 0.6f), ImVec2(cx + t, cy - t * 0.6f), ImVec2(cx, cy + t * 0.8f), ac);
            else ui.dl->AddTriangleFilled(ImVec2(cx - t * 0.6f, cy - t), ImVec2(cx - t * 0.6f, cy + t), ImVec2(cx + t * 0.8f, cy), ac);
            Text(ui, NobleStyle::TitleFont(), 19.0f, ImVec2(a.x + 28.0f * ui.s, a.y + (rh - 19.0f * ui.s) * 0.5f), ac,
                 Upper(items[i].label).c_str());
            ui.dl->AddLine(ImVec2(a.x, b.y - 1.0f), ImVec2(b.x, b.y - 1.0f), IM_COL32(143, 216, 255, 60), 1.0f);
            continue;
        }
        const ImU32 col = !items[i].enabled ? kDisabled : (i == selected ? kWhite : kDim);
        float textX = a.x + (items[0].header ? 28.0f : 9.0f) * ui.s;
        if (!items[i].thumb.empty() && rh >= 34.0f * ui.s) {
            // Number, then the map's small preview (MCC's levels pack), then the name.
            if (items[i].number > 0) {
                char num[8];
                std::snprintf(num, sizeof(num), "%02d", items[i].number);
                Text(ui, NobleStyle::SmallFont(), 14.0f, ImVec2(textX, a.y + (rh - 14.0f * ui.s) * 0.5f),
                     i == selected ? kAccent : IM_COL32(120, 135, 150, 255), num);
                textX += 24.0f * ui.s;
            }
            const float th = rh - 8.0f * ui.s, tw = th * 316.0f / 160.0f;
            const ImVec2 t0(textX, a.y + 4.0f * ui.s), t1(textX + tw, a.y + 4.0f * ui.s + th);
            ui.dl->AddRectFilled(t0, t1, IM_COL32(14, 20, 26, 220));
            if (UiTexture tex = UiArt::MapPreview(items[i].thumb, false)) {
                UiArt::DrawCover(ui.dl, tex, t0, t1, items[i].enabled ? IM_COL32_WHITE : IM_COL32(255, 255, 255, 90));
            }
            ui.dl->AddRect(t0, t1, i == selected ? IM_COL32(170, 220, 255, 200) : IM_COL32(255, 255, 255, 40), 0.0f, 0, 1.0f);
            textX += tw + 10.0f * ui.s;
        }
        Text(ui, NobleStyle::TitleFont(), 20.0f, ImVec2(textX, a.y + (rh - 20.0f * ui.s) * 0.5f), col,
             Upper(items[i].label).c_str());
    }
    if (activated >= 0) MenuSounds::Click();
    return activated;
}

bool DrawSourceTabs(const Ui& ui, ImVec2 pos, int& active, int moddedCount) {
    const char* labels[2] = { "VANILLA", "MODDED" };
    int next = active;
    float x = pos.x;
    for (int i = 0; i < 2; ++i) {
        char text[32];
        if (i == 1) std::snprintf(text, sizeof(text), "%s (%d)", labels[i], moddedCount);
        else std::snprintf(text, sizeof(text), "%s", labels[i]);
        const float w = TextWidth(NobleStyle::TitleFont(), 20.0f * ui.s, text) + 24.0f * ui.s;
        const float h = 30.0f * ui.s;
        ImGui::SetCursorScreenPos(ImVec2(x, pos.y));
        ImGui::PushID("srctab");
        ImGui::PushID(i);
        if (ImGui::InvisibleButton("##t", ImVec2(w, h))) next = i;
        const bool hovered = ImGui::IsItemHovered();
        ImGui::PopID();
        ImGui::PopID();
        const bool on = active == i;
        if (on) {
            ui.dl->AddRectFilled(ImVec2(x, pos.y), ImVec2(x + w, pos.y + h), IM_COL32(60, 120, 170, 90));
            ui.dl->AddRectFilled(ImVec2(x, pos.y + h - 2.0f * ui.s), ImVec2(x + w, pos.y + h), kAccent);
        } else if (hovered) {
            ui.dl->AddRectFilled(ImVec2(x, pos.y), ImVec2(x + w, pos.y + h), IM_COL32(255, 255, 255, 18));
        }
        Text(ui, NobleStyle::TitleFont(), 20.0f, ImVec2(x + 12.0f * ui.s, pos.y + 4.0f * ui.s), on ? kWhite : kDim, text);
        x += w + 6.0f * ui.s;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Q, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false)) next = 0;
    if (ImGui::IsKeyPressed(ImGuiKey_E, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false)) next = 1;
    if (next == active) return false;
    MenuSounds::Tab(next > active ? 1 : -1);
    active = next;
    return true;
}

using InputPrompts::Prompt;

// A button hint at the bottom (ESC Back, ENTER Select, ...) clicked with the mouse. Set by DrawHints,
// read on the next frame as if the key had been pressed.
int s_hintClicked = -1;      // Prompt clicked during the last frame, -1 = none
int s_hintThisFrame = -1;    // the click being handled this frame
bool HintClicked(Prompt p) { return s_hintThisFrame == static_cast<int>(p); }

bool AcceptPressed() {
    return ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false) ||
           ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false) || HintClicked(Prompt::Accept);
}

void DrawHints(const Ui& ui, const std::vector<std::pair<Prompt, const char*>>& hints) {
    float x = ui.size.x * 0.075f;
    const float y = ui.size.y - 46.0f * ui.s;
    ui.dl->AddLine(ImVec2(x, y - 12.0f * ui.s), ImVec2(ui.size.x * 0.62f, y - 12.0f * ui.s), IM_COL32(255, 255, 255, 50), 1.0f);
    for (const auto& [prompt, label] : hints) {
        const float x0 = x;
        x += InputPrompts::Draw(ui.dl, NobleStyle::SmallFont(), ImVec2(x, y), 22.0f * ui.s, prompt) + 7.0f * ui.s;
        Text(ui, NobleStyle::SmallFont(), 17.0f, ImVec2(x, y + 2.0f * ui.s), kWhite, label);
        x += TextWidth(NobleStyle::SmallFont(), 17.0f * ui.s, label);
        // Back and Select can also be clicked (Change / Tabs only describe keys, so they are not buttons).
        if (prompt == Prompt::Back || prompt == Prompt::Accept) {
            const ImVec2 r0(x0 - 6.0f * ui.s, y - 5.0f * ui.s);
            const ImVec2 r1(x + 8.0f * ui.s, y + 27.0f * ui.s);
            if (ImGui::IsMouseHoveringRect(r0, r1, false)) {
                ui.dl->AddRectFilled(r0, r1, IM_COL32(143, 216, 255, 40), 3.0f * ui.s);
                ui.dl->AddRect(r0, r1, IM_COL32(143, 216, 255, 120), 3.0f * ui.s);
                if (ImGui::IsMouseClicked(0)) {
                    s_hintClicked = static_cast<int>(prompt);
                    MenuSounds::Click();
                }
            }
        }
        x += 26.0f * ui.s;
    }
}


static bool s_showAccountModal = false;
static int s_accountTab = 0;
static char s_accountUser[32] = "";
static char s_accountPass[32] = "";
static std::string s_accountStatusMsg = "";
static bool s_accountStatusError = false;
static bool s_showDevMenu = false;
static std::string s_devStatusMsg = "";

void DrawDevMenuModal(const Ui& ui) {
    if (!s_showDevMenu) return;

    // Strict security check: only developers can access this menu!
    if (!AuthClient::Get().IsDeveloper()) {
        s_showDevMenu = false;
        return;
    }

    ImGui::OpenPopup("NOBLE DEVELOPER CONSOLE");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.0f * ui.s, 0));

    if (ImGui::BeginPopupModal("NOBLE DEVELOPER CONSOLE", &s_showDevMenu, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "[DEV] NOBLE DEVELOPER MENU");
        ImGui::TextDisabled("Logged in as Developer: %s (User ID: %d)",
                           AuthClient::Get().GetUsername().c_str(), AuthClient::Get().GetUserId());
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4));

        // 1. UNLOCK ALL ARMORS
        if (ImGui::Button("UNLOCK ALL ARMORS", ImVec2(220, 32))) {
            Career::DevSetArmoryUnlockAll(true);
            s_devStatusMsg = "SUCCESS: All Spartan and Elite armors unlocked!";
            MenuSounds::Click();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Unlocks all armors & permutations");

        // 2. GIVE MAX CREDITS
        if (ImGui::Button("GIVE MAX CREDITS", ImVec2(220, 32))) {
            Career::DevGrantMaxCredits();
            s_devStatusMsg = "SUCCESS: Max credits granted (+999,999,999 cR)!";
            MenuSounds::Click();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Grants 999,999,999 cR spendable balance");

        // 3. GIVE MAX RANK
        if (ImGui::Button("GIVE MAX RANK", ImVec2(220, 32))) {
            Career::DevSetMaxRank();
            s_devStatusMsg = "SUCCESS: Promoted to highest rank (Inheritor)!";
            MenuSounds::Click();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Sets rank to Inheritor (20,000,000 cR)");

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4));

        // Convenience: UNLOCK ALL & MAX EVERYTHING
        if (ImGui::Button("MAX EVERYTHING", ImVec2(220, 32))) {
            Career::DevSetArmoryUnlockAll(true);
            Career::DevGrantMaxCredits();
            Career::DevSetMaxRank();
            s_devStatusMsg = "SUCCESS: All armors unlocked, Inheritor rank & 999M cR granted!";
            MenuSounds::Click();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Unlock all armors, max rank & credits");

        // Reset progress option
        if (ImGui::Button("RESET CAREER", ImVec2(220, 28))) {
            Career::DevResetProgress();
            s_devStatusMsg = "Career reset: 0 cR, Recruit rank, armors locked.";
            MenuSounds::Click();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Reset to default (testing only)");

        if (!s_devStatusMsg.empty()) {
            ImGui::Dummy(ImVec2(0, 6));
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", s_devStatusMsg.c_str());
        }

        ImGui::Dummy(ImVec2(0, 8));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4));
        if (ImGui::Button("CLOSE", ImVec2(100, 30))) {
            s_showDevMenu = false;
        }

        ImGui::EndPopup();
    }
}

void DrawAccountModal(const Ui& ui) {
    if (!s_showAccountModal) return;

    ImGui::OpenPopup("ACCOUNT LOGIN / REGISTER");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(440.0f * ui.s, 0));

    if (ImGui::BeginPopupModal("ACCOUNT LOGIN / REGISTER", &s_showAccountModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        bool isLoggedIn = AuthClient::Get().IsLoggedIn();
        if (isLoggedIn) {
            static uint64_t s_lastDevCheck = 0;
            uint64_t now = GetTickCount64();
            if (now - s_lastDevCheck > 2500) {
                s_lastDevCheck = now;
                AuthClient::Get().RefreshDeveloperStatusAsync();
            }

            const bool isDev = AuthClient::Get().IsDeveloper();
            std::string userDisplay = "Logged in as: " + AuthClient::Get().GetUsername();
            if (isDev) {
                userDisplay += " [DEV]";
            }
            ImGui::TextColored(isDev ? ImVec4(1.0f, 0.85f, 0.2f, 1.0f) : ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "%s", userDisplay.c_str());

            if (isDev) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "[DEVELOPER]");
                ImGui::Dummy(ImVec2(0, 4));
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.2f, 1.0f), "Developer Account (ID: %d)", AuthClient::Get().GetUserId());
                ImGui::TextDisabled("Developer Status: ACTIVE");
                ImGui::TextDisabled("All armors, developer matrix camos, and special privileges unlocked.");
                ImGui::Dummy(ImVec2(0, 4));
                if (ImGui::Button("OPEN DEVELOPER MENU", ImVec2(200, 30))) {
                    s_showDevMenu = true;
                }
            } else {
                ImGui::Dummy(ImVec2(0, 4));
                ImGui::TextDisabled("Standard User Account (ID: %d)", AuthClient::Get().GetUserId());
            }
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0, 10));
            if (ImGui::Button("LOG OUT", ImVec2(120, 32))) {
                AuthClient::Get().Logout();
                s_accountStatusMsg = "Logged out successfully.";
                s_accountStatusError = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("CLOSE", ImVec2(100, 32))) {
                s_showAccountModal = false;
            }
        } else {
            if (ImGui::Button("LOG IN", ImVec2(120, 28))) s_accountTab = 0;
            ImGui::SameLine();
            if (ImGui::Button("REGISTER", ImVec2(120, 28))) s_accountTab = 1;
            ImGui::Separator();

            ImGui::Text(s_accountTab == 0 ? "Log in to your Noble account:" : "Register a new Noble account:");
            ImGui::Dummy(ImVec2(0, 4));

            ImGui::Text("Username:");
            ImGui::InputText("##acc_user", s_accountUser, sizeof(s_accountUser));

            ImGui::Text("Password:");
            ImGui::InputText("##acc_pass", s_accountPass, sizeof(s_accountPass), ImGuiInputTextFlags_Password);

            if (!s_accountStatusMsg.empty()) {
                ImGui::Dummy(ImVec2(0, 4));
                ImGui::TextColored(s_accountStatusError ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                                   "%s", s_accountStatusMsg.c_str());
            }

            ImGui::Dummy(ImVec2(0, 8));
            if (s_accountTab == 0) {
                if (ImGui::Button("LOG IN##submit", ImVec2(120, 32)) || (ImGui::IsKeyPressed(ImGuiKey_Enter) && ImGui::IsWindowFocused())) {
                    auto res = AuthClient::Get().Login(s_accountUser, s_accountPass);
                    if (res.success) {
                        s_accountStatusMsg = "Login successful!";
                        s_accountStatusError = false;
                        s_showAccountModal = false;
                    } else {
                        s_accountStatusMsg = res.error;
                        s_accountStatusError = true;
                    }
                }
            } else {
                if (ImGui::Button("REGISTER##submit", ImVec2(120, 32)) || (ImGui::IsKeyPressed(ImGuiKey_Enter) && ImGui::IsWindowFocused())) {
                    auto res = AuthClient::Get().Register(s_accountUser, s_accountPass);
                    if (res.success) {
                        s_accountStatusMsg = "Registration successful! Logged in.";
                        s_accountStatusError = false;
                        s_showAccountModal = false;
                    } else {
                        s_accountStatusMsg = res.error;
                        s_accountStatusError = true;
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("CANCEL", ImVec2(100, 32))) {
                s_showAccountModal = false;
            }
        }
        ImGui::EndPopup();
    }
}

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
    const ImVec2 e(x2 + 16.0f * ui.s, y + 15.0f * ui.s);
    ui.dl->AddCircleFilled(e, 10.0f * ui.s, IM_COL32(26, 40, 52, 255));
    ui.dl->AddTriangleFilled(ImVec2(e.x - 6 * ui.s, e.y - 2 * ui.s), ImVec2(e.x + 6 * ui.s, e.y - 2 * ui.s), ImVec2(e.x, e.y + 6 * ui.s), kAccent);
    std::string partyUser = playerName;
    if (AuthClient::Get().IsDeveloper()) {
        partyUser += " [DEV]";
    }
    Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(x2 + 34.0f * ui.s, y + 5.0f * ui.s),
         AuthClient::Get().IsDeveloper() ? IM_COL32(255, 215, 0, 255) : kWhite,
         partyUser.c_str());

    // Developer badge
    if (AuthClient::Get().IsDeveloper()) {
        float nameW = TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, partyUser.c_str());
        ImVec2 badgePos(x2 + 40.0f * ui.s + nameW, y + 6.0f * ui.s);
        ui.dl->AddRectFilled(badgePos, ImVec2(badgePos.x + 46.0f * ui.s, badgePos.y + 18.0f * ui.s), IM_COL32(220, 160, 20, 240), 3.0f * ui.s);
        Text(ui, NobleStyle::SmallFont(), 11.0f, ImVec2(badgePos.x + 10.0f * ui.s, badgePos.y + 3.0f * ui.s), IM_COL32(10, 10, 10, 255), "DEV");
    }

    // Rank icon
    static int s_rank = -1;
    static ULONGLONG s_rankRead = 0;
    if (s_rank < 0 || GetTickCount64() - s_rankRead > 2000) {
        s_rank = Career::RankIndex(Career::Get().credits);
        s_rankRead = GetTickCount64();
    }
    const UiTexture rankIcon = UiArt::RankIcon(s_rank);
    if (rankIcon) {
        const float is = 26.0f * ui.s;
        UiArt::DrawContain(ui.dl, rankIcon, ImVec2(x + w - is - 8.0f * ui.s, y + 2.0f * ui.s),
                           ImVec2(x + w - 8.0f * ui.s, y + 2.0f * ui.s + is));
    }

    // Account quick button
    y += 30.0f * ui.s;
    const float accH = 22.0f * ui.s;
    const ImVec2 accP0(x2, y);
    const ImVec2 accP1(x + w, y + accH);
    ImGui::SetCursorScreenPos(accP0);
    ImGui::PushID("party_account_btn");
    bool accClicked = ImGui::InvisibleButton("##acc", ImVec2(accP1.x - accP0.x, accH));
    bool accHov = ImGui::IsItemHovered();
    ImGui::PopID();
    if (accClicked) {
        s_showAccountModal = true;
        MenuSounds::Click();
    }
    ui.dl->AddRectFilled(accP0, accP1, accHov ? IM_COL32(30, 48, 68, 230) : IM_COL32(15, 24, 34, 200));
    ui.dl->AddRect(accP0, accP1, accHov ? IM_COL32(143, 216, 255, 180) : IM_COL32(60, 90, 120, 100), 0.0f, 0, 1.0f);
    if (AuthClient::Get().IsLoggedIn()) {
        char buf[64];
        if (AuthClient::Get().IsDeveloper()) {
            std::snprintf(buf, sizeof(buf), "ONLINE [DEV]: %s", AuthClient::Get().GetUsername().c_str());
        } else {
            std::snprintf(buf, sizeof(buf), "ONLINE: %s", AuthClient::Get().GetUsername().c_str());
        }
        Text(ui, NobleStyle::SmallFont(), 12.0f, ImVec2(x2 + 8.0f * ui.s, y + 4.0f * ui.s),
             AuthClient::Get().IsDeveloper() ? IM_COL32(255, 215, 0, 255) : IM_COL32(120, 220, 160, 255), buf);
    } else {
        Text(ui, NobleStyle::SmallFont(), 12.0f, ImVec2(x2 + 8.0f * ui.s, y + 4.0f * ui.s), IM_COL32(200, 210, 220, 200), "[ LOGIN / REGISTER ]");
    }

    if (AuthClient::Get().IsDeveloper()) {
        y += 26.0f * ui.s;
        const float devH = 22.0f * ui.s;
        const ImVec2 devP0(x2, y);
        const ImVec2 devP1(x + w, y + devH);
        ImGui::SetCursorScreenPos(devP0);
        ImGui::PushID("party_dev_menu_btn");
        bool devClicked = ImGui::InvisibleButton("##devbtn", ImVec2(devP1.x - devP0.x, devH));
        bool devHov = ImGui::IsItemHovered();
        ImGui::PopID();
        if (devClicked) {
            s_showDevMenu = true;
            MenuSounds::Click();
        }
        ui.dl->AddRectFilled(devP0, devP1, devHov ? IM_COL32(90, 60, 10, 240) : IM_COL32(45, 30, 8, 220));
        ui.dl->AddRect(devP0, devP1, devHov ? IM_COL32(255, 215, 0, 240) : IM_COL32(180, 130, 20, 180), 0.0f, 0, 1.0f);
        Text(ui, NobleStyle::SmallFont(), 12.0f, ImVec2(x2 + 8.0f * ui.s, y + 4.0f * ui.s), IM_COL32(255, 215, 0, 255), "[ DEVELOPER MENU ]");
    }
}

// DIFFICULTY as four tiles with Reach's difficulty icons (from the player's Reach art, see
// UiArt::DifficultyIcon; plain named tiles when there is none). Click picks; Left/Right change it
// when the row has focus. Returns the height used.
float DifficultyTiles(const Ui& ui, const char* id, ImVec2 pos, float width, int& difficulty, bool focused) {
    const float top = 6.0f * ui.s;      // keeps the label clear of the row above
    const float labelH = 26.0f * ui.s;
    const float gap = 8.0f * ui.s;
    const float tw = (width - gap * 3.0f) / 4.0f;
    const float th = 66.0f * ui.s;
    pos.y += top;
    Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(pos.x + 10.0f * ui.s, pos.y), focused ? kWhite : kDim, "DIFFICULTY");
    if (focused) ui.dl->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 2.0f * ui.s, pos.y + labelH - 6.0f * ui.s), kAccent);
    const float ty = pos.y + labelH;
    static const char* const kNames[4] = { "EASY", "NORMAL", "HEROIC", "LEGENDARY" };
    ImGui::PushID(id);
    for (int d = 0; d < 4; ++d) {
        const ImVec2 a(pos.x + d * (tw + gap), ty);
        const ImVec2 b(a.x + tw, ty + th);
        ImGui::SetCursorScreenPos(a);
        ImGui::PushID(d);
        const bool clicked = ImGui::InvisibleButton("##tile", ImVec2(tw, th));
        const bool hovered = ImGui::IsItemHovered();
        const ImGuiID hoverId = ImGui::GetItemID();
        ImGui::PopID();
        const bool sel = d == difficulty;
        ui.dl->AddRectFilled(a, b, sel ? IM_COL32(52, 98, 138, 200) : (hovered ? IM_COL32(40, 56, 72, 200) : IM_COL32(16, 22, 30, 190)));
        ui.dl->AddRect(a, b, sel ? kAccent : IM_COL32(170, 220, 255, hovered ? 120 : 45), 0.0f, 0, sel ? 2.0f : 1.0f);
        const UiTexture icon = UiArt::DifficultyIcon(d);
        const float nameH = 20.0f * ui.s;
        if (icon) {
            UiArt::DrawContain(ui.dl, icon, ImVec2(a.x + 4.0f * ui.s, a.y + 4.0f * ui.s), ImVec2(b.x - 4.0f * ui.s, b.y - nameH),
                               sel || hovered ? IM_COL32_WHITE : IM_COL32(255, 255, 255, 170));
        }
        const float nw = TextWidth(NobleStyle::TitleFont(), 15.0f * ui.s, kNames[d]);
        Text(ui, NobleStyle::TitleFont(), 15.0f, ImVec2(a.x + (tw - nw) * 0.5f, icon ? b.y - nameH : a.y + (th - nameH) * 0.5f),
             sel ? kWhite : kDim, kNames[d]);
        if (hovered) MenuSounds::Hover(hoverId);
        if (clicked && difficulty != d) {
            difficulty = d;
            MenuSounds::Change();
        }
    }
    ImGui::PopID();
    if (focused) {
        int change = 0;
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft)) change = -1;
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight)) change = 1;
        if (change) {
            difficulty = std::clamp(difficulty + change, 0, 3);
            MenuSounds::Change();
        }
    }
    return top + labelH + th;
}

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
    if (change) MenuSounds::Change();
    return change;
}

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
        ImFont* font = Font(nullptr);
        const float blurbHeight = font->CalcTextSizeA(17.0f * ui.s, FLT_MAX, width, blurb).y;
        y += blurbHeight + 8.0f * ui.s;
    }
    return y;
}

// Spec §3 HeroBanner: the map picture with a dark gradient, an accent kicker line, the title, and the
// blurb under the picture. Returns the y below the blurb.
float HeroBanner(const Ui& ui, ImVec2 pos, float width, float height, const std::string& scenario, const std::string& kicker,
                 const char* title, const char* blurb) {
    const ImVec2 b(pos.x + width, pos.y + height);
    ui.dl->AddRectFilled(pos, b, IM_COL32(14, 20, 26, 230));
    if (UiTexture tex = UiArt::MapPreview(scenario, true)) UiArt::DrawCover(ui.dl, tex, pos, b);
    const float gy = pos.y + height * 0.45f;
    ui.dl->AddRectFilledMultiColor(ImVec2(pos.x, gy), b, IM_COL32(4, 8, 12, 0), IM_COL32(4, 8, 12, 0),
                                   IM_COL32(4, 8, 12, 235), IM_COL32(4, 8, 12, 235));
    ui.dl->AddRect(pos, b, IM_COL32(170, 200, 220, 120), 0.0f, 0, 1.0f);
    const float pad = 16.0f * ui.s;
    Text(ui, NobleStyle::TitleFont(), 34.0f, ImVec2(pos.x + pad, b.y - pad - 34.0f * ui.s), kWhite, Upper(title).c_str());
    if (!kicker.empty()) Text(ui, NobleStyle::SmallFont(), 15.0f, ImVec2(pos.x + pad, b.y - pad - 56.0f * ui.s), kAccent, kicker.c_str());
    float y = b.y + 8.0f * ui.s;
    if (blurb && *blurb) {
        Text(ui, nullptr, 17.0f, ImVec2(pos.x, y), kDim, blurb, pos.x + width);
        y += Font(nullptr)->CalcTextSizeA(17.0f * ui.s, FLT_MAX, width, blurb).y + 8.0f * ui.s;
    }
    return y;
}

// Spec §3 BigAction: large button with a title and a subtitle line. Disabled buttons are greyed and
// show their subtitle as the reason.
bool BigAction(const Ui& ui, const char* id, ImVec2 pos, float width, const char* label, const char* sub, bool primary,
               bool enabled, bool focused = false) {
    const float h = 50.0f * ui.s;
    ImGui::SetCursorScreenPos(pos);
    ImGui::PushID(id);
    const bool pressed = ImGui::InvisibleButton("##b", ImVec2(width, h)) && enabled;
    const bool hovered = ImGui::IsItemHovered() && enabled;
    ImGui::PopID();
    const ImVec2 b(pos.x + width, pos.y + h);
    const ImU32 bg = !enabled ? IM_COL32(24, 30, 36, 200)
                   : primary ? (hovered || focused ? IM_COL32(76, 150, 200, 255) : IM_COL32(52, 112, 156, 240))
                             : (hovered || focused ? IM_COL32(60, 74, 88, 240) : IM_COL32(30, 38, 46, 230));
    ui.dl->AddRectFilled(pos, b, bg);
    ui.dl->AddRect(pos, b, enabled ? IM_COL32(170, 220, 255, hovered || focused ? 220 : 90) : IM_COL32(255, 255, 255, 30), 0.0f, 0, 1.0f);
    Text(ui, NobleStyle::TitleFont(), 20.0f, ImVec2(pos.x + 14.0f * ui.s, pos.y + 5.0f * ui.s), enabled ? kWhite : kDisabled, label);
    if (sub && *sub) Text(ui, NobleStyle::SmallFont(), 14.0f, ImVec2(pos.x + 14.0f * ui.s, pos.y + 29.0f * ui.s),
                          enabled ? IM_COL32(200, 225, 245, 230) : kDisabled, sub);
    if (hovered) MenuSounds::Hover(ImGui::GetID(id));
    if (pressed) MenuSounds::Click();
    return pressed;
}

// Small icons of the skulls that are on, in a row; returns the width used.
float SkullStrip(const Ui& ui, ImVec2 pos, float size, uint64_t mask, int maxIcons);

float SkullStrip(const Ui& ui, ImVec2 pos, float size, uint64_t mask, int maxIcons) {
    float x = pos.x;
    int shown = 0;
    for (int i = 0; i < kSkullCount && shown < maxIcons; ++i) {
        if (!(mask & (1ull << kSkulls[i].bit))) continue;
        if (UiTexture icon = UiArt::SkullIcon(kSkulls[i].icon)) {
            ui.dl->AddImage(icon.id, ImVec2(x, pos.y), ImVec2(x + size, pos.y + size));
        } else {
            ui.dl->AddRect(ImVec2(x, pos.y), ImVec2(x + size, pos.y + size), IM_COL32(255, 255, 255, 80));
        }
        x += size + 3.0f * ui.s;
        ++shown;
    }
    return x - pos.x;
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
    float m = kDifficultyMultiplier[std::clamp(difficulty, 0, 3)];
    for (const Skull& s : kSkulls) {
        if ((mask & (1ull << s.bit)) && s.multiplier > 1.0f) m += s.multiplier;
    }
    return m;
}

struct MainEntry {
    const char* label;
    const char* description;
    Screen target;
    bool enabled;
    bool quit;
};
const MainEntry kMainEntries[] = {
    { "Campaign",     "Play the story of Noble Team, from Winter Contingency to Lone Wolf.", Screen::Campaign, true, false },
    { "Server Browser", "Browse and join online multiplayer, co-op and custom games.",        Screen::Servers, true, false },
    { "Firefight",    "Hold out against waves of Covenant on the map and game type you choose.", Screen::Firefight, true, false },
    { "Custom Game",  "Play any multiplayer map and game type, host a game or join one.",     Screen::CustomGame, true, false },
    { "Forge",        "Edit multiplayer maps with Forge. Experimental in Noble.",             Screen::Forge, true, false },
    { "Theater",      "Saved films aren't supported in Noble yet.",                           Screen::Main, false, false },
    { "Service Record", "Your rank, credits, challenges, commendations and recent games.",   Screen::Career, true, false },
    { "Armory",       "Customize your Spartan and Sangheili armor in Reach's 3D Armory.",     Screen::Armory, true, false },
    { "Account",      "Log in or register your account to sync data and unlock developer access.", Screen::Main, true, false },
    { "Mods",         "Workshop maps and mods.",                                              Screen::Mods, true, false },
    { "Settings",     "Controls, key bindings, audio, video and gameplay options.",          Screen::Settings, true, false },
    { "Quit",         "Close Noble.",                                                         Screen::Main, true, true },
};

struct MenuState {
    Screen screen = Screen::Main;
    int mainIndex = 0;
    int mission = 1;
    int difficulty = 1;
    int focusRow = 0;
    int map = kDefaultMap;
    int variant = 0;
    int ffMap = 0;
    int ffVariant = 0;
    int forgeMap = 19;
    int skull = 0;
    uint64_t skulls = 0;
    int campaignSource = 0;
    int ffSource = 0;
    int mpSource = 0;
    int forgeSource = 0;
    std::set<std::string> openMods;
    int campaignModded = 0;
    int ffModded = 0;
    int mpModded = 0;
    int forgeModded = 0;
    int lobbyNetMode = 1;       // 0 = Local (LAN), 1 = Online (P2P)
    int lobbyPrivacy = 0;       // 0 = Public (Master Server), 1 = Private (Direct Connect)
    bool confirmQuit = false;   // "Quit Project Noble?" box on the main menu (Esc or QUIT)
    int quitChoice = 1;         // 0 = Yes, 1 = No (No is preselected)
    bool showAccountModal = false;
    int accountTab = 0;         // 0 = Login, 1 = Register
    char accountUser[32] = "";
    char accountPass[32] = "";
    std::string accountStatusMsg = "";
    bool accountStatusError = false;

    // Pre-game lobby state
    bool isLobbyHost = false;
    std::string lobbySessionId;
    std::string lobbySessionToken;
    std::string lobbyMap = "20_sword_slayer";
    std::string lobbyMode = "multiplayer";
    std::string lobbyVariantPath;
    std::string lobbyVariantName = "Slayer";
    int lobbyDifficulty = 1;
    uint64_t lobbySkulls = 0;
    int lobbyMaxPlayers = 16;
    std::string lobbyHostEndpoint;
    std::vector<LobbyPlayer> lobbyRoster;
    bool lobbyLaunchPending = false;
    std::string lobbyLaunchMap;
    std::string lobbyLaunchMode;
    std::string lobbyLaunchVariant;
    int lobbyLaunchDiff = 1;
    uint64_t lobbyLaunchSkulls = 0;
    double lastLobbyPoll = 0.0;
};


static void SetupHostLobby(MenuState& st, const std::string& mapScenario, const std::string& mode,
                           const std::string& varPath, const std::string& varName, int diff, uint64_t skulls, int maxPlayers) {
    st.isLobbyHost = true;
    st.lobbyMap = mapScenario;
    st.lobbyMode = mode;
    st.lobbyVariantPath = varPath;
    st.lobbyVariantName = varName;
    st.lobbyDifficulty = diff;
    st.lobbySkulls = skulls;
    st.lobbyMaxPlayers = maxPlayers;
    st.lobbyLaunchPending = false;
    st.lastLobbyPoll = ImGui::GetTime();

    HostConfig cfg;
    std::string pName = AuthClient::Get().IsLoggedIn() ? AuthClient::Get().GetUsername() : Settings::Get().Snapshot().playerName;
    if (pName.empty()) pName = "Host";
    cfg.sessionName = pName + "'s Lobby";
    cfg.gameMode = st.lobbyMode;
    cfg.mapName = st.lobbyMap;
    cfg.gametypeName = st.lobbyVariantName;
    cfg.maxPlayers = st.lobbyMaxPlayers;
    cfg.port = 11774;
    cfg.mode = (st.lobbyNetMode == 1) ? NetworkMode::Online_P2P : NetworkMode::Local_LAN;
    SessionManager::Get().StartHosting(cfg);

    if (st.lobbyNetMode == 1 && st.lobbyPrivacy == 0 && MasterServerClient::Get().Enabled()) {
        MasterAdvert adv;
        adv.name = cfg.sessionName;
        adv.port = cfg.port;
        adv.maxPlayers = cfg.maxPlayers;
        adv.map = cfg.mapName;
        adv.mode = cfg.gameMode;
        adv.variant = st.lobbyVariantName;
        adv.difficulty = st.lobbyDifficulty;
        adv.skulls = st.lobbySkulls;
        adv.inProgress = false;
        adv.hostPeerId = AuthClient::Get().IsLoggedIn() ? static_cast<uint64_t>(AuthClient::Get().GetUserId()) : 1ULL;
        adv.hostMachineId = "0x484F535400000001";
        adv.hostUsername = pName;
        adv.hostIsDev = AuthClient::Get().IsDeveloper();
        adv.hostServiceTag = Settings::Get().Snapshot().serviceTag.empty() ? "117" : Settings::Get().Snapshot().serviceTag;
        MasterServerClient::Get().StartAdvertising(adv);
        st.lobbySessionId = MasterServerClient::Get().GetCurrentSessionId();
        st.lobbySessionToken = MasterServerClient::Get().GetCurrentSessionToken();
    }

    st.lobbyRoster.clear();
    LobbyPlayer me;
    me.peerId = AuthClient::Get().IsLoggedIn() ? static_cast<uint64_t>(AuthClient::Get().GetUserId()) : 1ULL;
    me.username = pName;
    me.serviceTag = Settings::Get().Snapshot().serviceTag.empty() ? "117" : Settings::Get().Snapshot().serviceTag;
    me.isHost = true;
    me.isDev = AuthClient::Get().IsDeveloper();
    me.ping = 0;
    st.lobbyRoster.push_back(me);

    st.screen = Screen::PreGameLobby;
}

void DrawLobbyControls(const Ui& ui, ImVec2 pos, [[maybe_unused]] float width, MenuState& st) {
    const float h = 32.0f * ui.s;
    const float btnW = 150.0f * ui.s;
    float x = pos.x;

    // Network Mode Toggle (Online P2P vs Local LAN)
    const bool isOnline = (st.lobbyNetMode == 1);
    const char* netLabel = isOnline ? "NETWORK: ONLINE" : "NETWORK: LOCAL";
    ImGui::SetCursorScreenPos(ImVec2(x, pos.y));
    ImGui::PushID("btn_lobby_net");
    if (ImGui::InvisibleButton("##net", ImVec2(btnW, h))) {
        st.lobbyNetMode = isOnline ? 0 : 1;
        MenuSounds::Click();
    }
    const bool netHover = ImGui::IsItemHovered();
    ImGui::PopID();
    const ImVec2 bNet(x + btnW, pos.y + h);
    ui.dl->AddRectFilled(ImVec2(x, pos.y), bNet, isOnline ? (netHover ? IM_COL32(40, 110, 170, 230) : IM_COL32(28, 80, 128, 200))
                                                           : (netHover ? IM_COL32(80, 80, 90, 230) : IM_COL32(50, 52, 60, 200)));
    ui.dl->AddRect(ImVec2(x, pos.y), bNet, netHover ? IM_COL32(180, 220, 255, 230) : IM_COL32(100, 140, 180, 120), 0.0f, 0, 1.0f);
    Text(ui, NobleStyle::SmallFont(), 13.0f, ImVec2(x + 10.0f * ui.s, pos.y + 8.0f * ui.s), kWhite, netLabel);
    x += btnW + 8.0f * ui.s;

    // Privacy Toggle (Public vs Private) - only relevant if online
    if (isOnline) {
        const bool isPublic = (st.lobbyPrivacy == 0);
        const char* privLabel = isPublic ? "LOBBY: PUBLIC" : "LOBBY: PRIVATE";
        ImGui::SetCursorScreenPos(ImVec2(x, pos.y));
        ImGui::PushID("btn_lobby_priv");
        if (ImGui::InvisibleButton("##priv", ImVec2(btnW, h))) {
            st.lobbyPrivacy = isPublic ? 1 : 0;
            MenuSounds::Click();
        }
        const bool privHover = ImGui::IsItemHovered();
        ImGui::PopID();
        const ImVec2 bPriv(x + btnW, pos.y + h);
        ui.dl->AddRectFilled(ImVec2(x, pos.y), bPriv, isPublic ? (privHover ? IM_COL32(36, 120, 90, 230) : IM_COL32(24, 90, 68, 200))
                                                               : (privHover ? IM_COL32(110, 80, 40, 230) : IM_COL32(80, 58, 28, 200)));
        ui.dl->AddRect(ImVec2(x, pos.y), bPriv, privHover ? IM_COL32(180, 255, 220, 230) : IM_COL32(100, 180, 140, 120), 0.0f, 0, 1.0f);
        Text(ui, NobleStyle::SmallFont(), 13.0f, ImVec2(x + 10.0f * ui.s, pos.y + 8.0f * ui.s), kWhite, privLabel);
        x += btnW + 12.0f * ui.s;
    }

    // Status / IP info
    char statusBuf[160];
    if (isOnline) {
        const std::string& extIp = UPnP::Get().GetExternalIP();
        const bool upnpOk = UPnP::Get().IsActive();
        if (upnpOk && extIp != "Unknown") {
            std::snprintf(statusBuf, sizeof(statusBuf), "UPnP Ready (%s:11774) · %s", extIp.c_str(), st.lobbyPrivacy == 0 ? "Broadcasting to Master" : "Direct Connect");
        } else {
            std::snprintf(statusBuf, sizeof(statusBuf), "Online P2P (Port 11774) · %s", st.lobbyPrivacy == 0 ? "Broadcasting to Master" : "Direct Connect");
        }
    } else {
        std::snprintf(statusBuf, sizeof(statusBuf), "Local LAN Match · Discoverable on subnet");
    }
    Text(ui, NobleStyle::SmallFont(), 13.0f, ImVec2(x, pos.y + 9.0f * ui.s), kDim, statusBuf);
}

struct MapEntry {
    std::string scenario, name, blurb;
    bool enabled = true;
    const ModEntry* mod = nullptr;
    bool header = false;
};

bool KnownScenario(const std::string& scenario, const MenuMap* maps, int count) {
    for (int i = 0; i < count; ++i) if (scenario == maps[i].scenario) return true;
    return false;
}

std::vector<MapEntry> ModMaps(const ModEntry& mod, const std::vector<ModMap>& listed, const MenuMap* base, int baseCount) {
    std::vector<MapEntry> out;
    for (const ModMap& m : listed) {
        const bool known = KnownScenario(m.scenario, base, baseCount);
        out.push_back({ m.scenario, m.title, known ? m.description : "A new map: Noble can only start maps that are in Reach so far.", known, &mod });
    }
    for (const std::string& file : mod.mapFiles) {
        if (!KnownScenario(file, base, baseCount)) continue;
        if (std::any_of(out.begin(), out.end(), [&](const MapEntry& e) { return e.scenario == file; })) continue;
        for (int i = 0; i < baseCount; ++i) {
            if (file == base[i].scenario) out.push_back({ file, base[i].name, base[i].blurb, true, &mod });
        }
    }
    return out;
}

enum class ModMode { Campaign, Firefight, Multiplayer };
std::string OpenKey(ModMode mode, const ModEntry& mod) { return std::to_string(static_cast<int>(mode)) + ":" + mod.id; }

std::vector<MapEntry> ModdedMaps(const std::vector<const ModEntry*>& mods, ModMode mode, const MenuMap* base, int baseCount,
                                 const std::set<std::string>& open) {
    std::vector<MapEntry> out;
    for (const ModEntry* mod : mods) {
        const std::vector<ModMap>& listed = mode == ModMode::Campaign ? mod->campaign
                                          : mode == ModMode::Firefight ? mod->firefight : mod->multiplayer;
        std::vector<MapEntry> maps = ModMaps(*mod, listed, base, baseCount);
        if (maps.empty()) continue;
        MapEntry heading;
        heading.name = mod->title + "  (" + std::to_string(maps.size()) + ")";
        heading.header = true;
        heading.mod = mod;
        std::string about = mod->description.substr(0, 320);
        if (mod->description.size() > 320) about += "...";
        heading.blurb = about;
        heading.enabled = open.count(OpenKey(mode, *mod)) != 0;
        out.push_back(heading);
        if (!heading.enabled) continue;
        for (MapEntry& m : maps) out.push_back(std::move(m));
    }
    return out;
}

void ToggleMod(std::set<std::string>& open, ModMode mode, const ModEntry& mod) {
    const std::string k = OpenKey(mode, mod);
    if (!open.erase(k)) open.insert(k);
}

void FixSelection(const std::vector<MapEntry>& list, int& selected) {
    selected = list.empty() ? 0 : std::clamp(selected, 0, static_cast<int>(list.size()) - 1);
}

std::vector<ListItem> ToItems(const std::vector<MapEntry>& list) {
    std::vector<ListItem> items;
    for (const MapEntry& m : list) {
        if (m.header) {
            items.push_back({ m.name, true, true, m.enabled });
        } else {
            ListItem it{ m.name, m.enabled, false, false };
            it.thumb = m.scenario;
            it.number = static_cast<int>(std::count_if(items.begin(), items.end(), [](const ListItem& x) { return !x.header; })) + 1;
            items.push_back(it);
        }
    }
    return items;
}

std::vector<MapEntry> BaseMaps(const MenuMap* maps, int count) {
    std::vector<MapEntry> out;
    for (int i = 0; i < count; ++i) out.push_back({ maps[i].scenario, maps[i].name, maps[i].blurb, true });
    return out;
}

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
    if (s == L"career") return Screen::Career;
    if (s == L"lobby" || s == L"pregame") return Screen::PreGameLobby;
    return Screen::Main;
}

std::wstring ToWide(const std::string& s) {
    if (s.empty()) return L"";
    const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring w(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
    return w;
}

// Persistent state for in-engine overlay mode
struct OverlayContext {
    bool initialized = false;
    std::filesystem::path reachDir;
    std::vector<MenuVariant> variants;
    std::vector<MenuVariant> ffVariants;
    std::filesystem::path forgeVariant;
    std::vector<ModEntry> mods;
    std::vector<const ModEntry*> campaignMods;
    std::vector<const ModEntry*> ffMods;
    std::vector<const ModEntry*> mpMods;
    MenuState st;
    SettingsPanel settingsPanel;
    CareerPanel careerPanel;
    bool modsScanned = false;
    unsigned long long captureFrame = 0;
    unsigned long long totalFrames = 0;
    unsigned long long sceneReadyFrames = 0;
    bool closed = false;
};

// The engine draws the menu scene and calls RenderOverlay from slot 1. MCC ships no Reach menu map,
// so about a minute after start the engine gives up on it ("_map_state_none") and stops calling
// slot 1; HoldFrame then draws the menu from the main thread so it keeps working.
std::mutex s_overlayMutex;
std::atomic<IDXGISwapChain*> s_overlaySwapChain{nullptr};
std::atomic<ULONGLONG> s_lastEngineOverlayFrame{0};

OverlayContext& GetOverlayCtx() {
    static OverlayContext s_ctx;
    return s_ctx;
}

void UpdateSceneForScreen(Screen screen) {
    switch (screen) {
    case Screen::Main:
        ReachArmory::SetTargetScene("start_menu");
        break;
    case Screen::Campaign:
    case Screen::Skulls:
        ReachArmory::SetTargetScene("campaign");
        break;
    case Screen::Firefight:
        ReachArmory::SetTargetScene("firefight");
        break;
    case Screen::CustomGame:
    case Screen::Servers:
        ReachArmory::SetTargetScene("custom_games");
        break;
    case Screen::Forge:
        ReachArmory::SetTargetScene("forge");
        break;
    case Screen::Armory:
        UIManager::GetArmorCustomizer().UpdateArmoryCamera();
        break;
    default:
        // Settings, Service Record, Mods, ...: the main menu's scene, never the Armory's.
        ReachArmory::SetTargetScene("start_menu");
        break;
    }
}

void FeedEngineThreadInputs(HWND hwnd, ImGuiIO& io) {
    RECT client{};
    GetClientRect(hwnd, &client);
    POINT cursor{};
    const bool focused = (GetForegroundWindow() == hwnd);
    if (focused && GetCursorPos(&cursor) && ScreenToClient(hwnd, &cursor) && client.right > 0 && client.bottom > 0) {
        const float sx = io.DisplaySize.x / static_cast<float>(client.right);
        const float sy = io.DisplaySize.y / static_cast<float>(client.bottom);
        io.AddMousePosEvent(static_cast<float>(cursor.x) * sx, static_cast<float>(cursor.y) * sy);

        const bool lDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        if (Window::TakeLeftClicks() > 0 && !lDown) {
            io.AddMouseButtonEvent(0, true);
            io.AddMouseButtonEvent(0, false);
        } else {
            io.AddMouseButtonEvent(0, lDown);
        }
        const bool rDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        io.AddMouseButtonEvent(1, rDown);

        const int wheel = Window::TakeWheelNotches();
        if (wheel != 0) {
            io.AddMouseWheelEvent(0.0f, static_cast<float>(wheel));
        }
    } else {
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        io.AddMouseButtonEvent(0, false);
        io.AddMouseButtonEvent(1, false);
    }

    struct KeyMap {
        int vk;
        ImGuiKey key;
        char ch;
    };
    static const KeyMap kKeys[] = {
        { VK_UP,     ImGuiKey_UpArrow,     0 },
        { VK_DOWN,   ImGuiKey_DownArrow,   0 },
        { VK_LEFT,   ImGuiKey_LeftArrow,   0 },
        { VK_RIGHT,  ImGuiKey_RightArrow,  0 },
        { VK_RETURN, ImGuiKey_Enter,       0 },
        { VK_ESCAPE, ImGuiKey_Escape,      0 },
        { VK_TAB,    ImGuiKey_Tab,         0 },
        { VK_SPACE,  ImGuiKey_Space,       ' ' },
        { VK_BACK,   ImGuiKey_Backspace,   0 },
        { VK_DELETE, ImGuiKey_Delete,      0 },
        { 'Q',       ImGuiKey_Q,           'Q' },
        { 'E',       ImGuiKey_E,           'E' },
        { 'A',       ImGuiKey_A,           'A' },
        { 'B',       ImGuiKey_B,           'B' },
        { 'C',       ImGuiKey_C,           'C' },
        { 'D',       ImGuiKey_D,           'D' },
        { 'F',       ImGuiKey_F,           'F' },
        { 'G',       ImGuiKey_G,           'G' },
        { 'H',       ImGuiKey_H,           'H' },
        { 'I',       ImGuiKey_I,           'I' },
        { 'J',       ImGuiKey_J,           'J' },
        { 'K',       ImGuiKey_K,           'K' },
        { 'L',       ImGuiKey_L,           'L' },
        { 'M',       ImGuiKey_M,           'M' },
        { 'N',       ImGuiKey_N,           'N' },
        { 'O',       ImGuiKey_O,           'O' },
        { 'P',       ImGuiKey_P,           'P' },
        { 'R',       ImGuiKey_R,           'R' },
        { 'S',       ImGuiKey_S,           'S' },
        { 'T',       ImGuiKey_T,           'T' },
        { 'U',       ImGuiKey_U,           'U' },
        { 'V',       ImGuiKey_V,           'V' },
        { 'W',       ImGuiKey_W,           'W' },
        { 'X',       ImGuiKey_X,           'X' },
        { 'Y',       ImGuiKey_Y,           'Y' },
        { 'Z',       ImGuiKey_Z,           'Z' },
        { '0',       ImGuiKey_0,           '0' },
        { '1',       ImGuiKey_1,           '1' },
        { '2',       ImGuiKey_2,           '2' },
        { '3',       ImGuiKey_3,           '3' },
        { '4',       ImGuiKey_4,           '4' },
        { '5',       ImGuiKey_5,           '5' },
        { '6',       ImGuiKey_6,           '6' },
        { '7',       ImGuiKey_7,           '7' },
        { '8',       ImGuiKey_8,           '8' },
        { '9',       ImGuiKey_9,           '9' },
    };
    static bool s_prevDown[sizeof(kKeys) / sizeof(kKeys[0])] = {};
    for (size_t i = 0; i < sizeof(kKeys) / sizeof(kKeys[0]); ++i) {
        const bool down = focused && ((GetAsyncKeyState(kKeys[i].vk) & 0x8000) != 0);
        if (down != s_prevDown[i]) {
            io.AddKeyEvent(kKeys[i].key, down);
            if (down && kKeys[i].ch != 0 && io.WantTextInput) {
                io.AddInputCharacter(static_cast<unsigned int>(kKeys[i].ch));
            }
            s_prevDown[i] = down;
        }
    }
    InputPrompts::Update();
}


// Lobby events from the WebSocket relay thread. The callbacks used to write MenuState's strings and roster on that
// thread while the menu read them on its own; two launch events 0.1 s apart (2026-10-29, joining O7OF) then killed
// the menu without a crash log (heap corruption ends the process before the crash handler runs). The relay
// thread now only fills this, and the menu takes it over at the top of the lobby screen.
struct PendingLobbyEvents {
    bool launch = false;
    std::string map, mode, variant, hostEndpoint, sessionId;
    int difficulty = 1;
    uint64_t skulls = 0;
    bool roster = false;
    std::vector<LobbyPlayer> rosterPlayers;
};
static std::mutex s_lobbyEventMutex;
static PendingLobbyEvents s_lobbyEvents;

static void TakePendingLobbyEvents(MenuState& st) {
    PendingLobbyEvents ev;
    {
        std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
        if (!s_lobbyEvents.launch && !s_lobbyEvents.roster) return;
        ev = std::move(s_lobbyEvents);
        s_lobbyEvents = PendingLobbyEvents{};
    }
    if (ev.roster) st.lobbyRoster = std::move(ev.rosterPlayers);
    if (ev.launch && !st.lobbyLaunchPending) {
        st.lobbyLaunchPending = true;
        st.lobbyLaunchMap = ev.map;
        st.lobbyLaunchMode = ev.mode;
        st.lobbyLaunchVariant = ev.variant;
        st.lobbyLaunchDiff = ev.difficulty;
        st.lobbyLaunchSkulls = ev.skulls;
        if (!ev.hostEndpoint.empty()) st.lobbyHostEndpoint = ev.hostEndpoint;
        if (!ev.sessionId.empty()) st.lobbySessionId = ev.sessionId;
    }
}

static void DrawPreGameLobby(OverlayContext& ctx, const Ui& ui, float left, float top, FrontEndChoice& choice, bool& done, bool back) {
    MenuState& st = ctx.st;
    TakePendingLobbyEvents(st);
    const float colW = ui.size.x * 0.44f;
    const float pxLeft = left + 34.0f * ui.s;
    const float pxRight = ui.size.x * 0.52f;
    const float actionsY = ui.size.y - 58.0f * ui.s - 14.0f * ui.s - 50.0f * ui.s;

    // Polling for lobby state updates
    const double now = ImGui::GetTime();
    if (now - st.lastLobbyPoll > 1.2) {
        st.lastLobbyPoll = now;
        if (st.isLobbyHost && st.lobbySessionId.empty()) {
            st.lobbySessionId = MasterServerClient::Get().GetCurrentSessionId();
            st.lobbySessionToken = MasterServerClient::Get().GetCurrentSessionToken();
        }
        if (!st.lobbySessionId.empty() && MasterServerClient::Get().Enabled()) {
            LobbyState current = MasterServerClient::Get().FetchLobbyStateSync(st.lobbySessionId);
            if (current.ok) {
                if (!st.isLobbyHost) {
                    st.lobbyMap = current.map;
                    st.lobbyMode = current.mode;
                    st.lobbyVariantName = current.variant;
                    st.lobbyDifficulty = current.difficulty;
                    st.lobbySkulls = current.skulls;
                }
                if (!current.roster.empty()) {
                    st.lobbyRoster = current.roster;
                }
                // Launch only while the host's match is running. The server keeps the last launch_data when the host
                // re-registers the same address and port for its next lobby, so "launch_data present" alone sent a
                // guest straight into a match that wasn't starting (2026-10-29 review, tested against server.js).
                if (!st.isLobbyHost && current.inProgress) {
                    st.lobbyLaunchPending = true;
                    st.lobbyLaunchMap = current.launchMap.empty() ? current.map : current.launchMap;
                    st.lobbyLaunchMode = current.launchMode.empty() ? current.mode : current.launchMode;
                    st.lobbyLaunchVariant = current.launchVariant;
                    st.lobbyLaunchDiff = current.launchDifficulty;
                    st.lobbyLaunchSkulls = current.launchSkulls;
                }
            }
        }
    }

    // If launch was signaled (via WebSocket or HTTP poll) and we are guest, launch!
    if (!st.isLobbyHost && st.lobbyLaunchPending && !done) {
        choice.play = true;
        choice.host = false;
        choice.map = st.lobbyLaunchMap.empty() ? st.lobbyMap : st.lobbyLaunchMap;
        choice.mode = st.lobbyLaunchMode.empty() ? st.lobbyMode : st.lobbyLaunchMode;
        choice.variantPath = st.lobbyLaunchVariant;
        choice.difficulty = st.lobbyLaunchDiff;
        choice.skulls = st.lobbyLaunchSkulls;
        choice.connectAddress = st.lobbyHostEndpoint;
        choice.sessionId = st.lobbySessionId;

        // The host sends its variant as a path on its own PC; use the guest's copy of the same file. Without a
        // variant the guest's launch block has no game variant (the 5% load hang from 2026-10-28).
        if (!st.lobbyLaunchVariant.empty()) {
            const std::filesystem::path want = std::filesystem::path(st.lobbyLaunchVariant).filename();
            std::filesystem::path local;
            for (const auto* list : { &ctx.variants, &ctx.ffVariants })
                for (const MenuVariant& v : *list)
                    if (local.empty() && _wcsicmp(v.path.filename().c_str(), want.c_str()) == 0) local = v.path;
            if (!local.empty()) {
                choice.variantPath = local.string();
            } else {
                NOBLE_WARN("FrontEndMenu: host's game variant '{}' isn't in this PC's variants; launching without it",
                           st.lobbyLaunchVariant);
            }
        }
        NOBLE_INFO("FrontEndMenu: Guest executing synchronized lobby launch to {}", st.lobbyHostEndpoint);
        done = true;
        return;
    }

    // Title Header
    std::string modeSubtitle = Upper(st.lobbyMode);
    if (st.lobbyMode == "custom" || st.lobbyMode == "multiplayer") modeSubtitle = "MULTIPLAYER";
    else if (st.lobbyMode == "firefight") modeSubtitle = "SURVIVAL";
    else if (st.lobbyMode == "campaign") modeSubtitle = "CO-OP CAMPAIGN";
    else if (st.lobbyMode == "forge") modeSubtitle = "FORGE EDIT";

    DrawTitle(ui, ImVec2(left, top), "PRE-GAME LOBBY", modeSubtitle.c_str());

    // ── LEFT COLUMN: Map & Match Settings ─────────────────────────────────────
    const float bannerTop = top + 96.0f * ui.s;
    const float bannerH = std::clamp(actionsY - 140.0f * ui.s - bannerTop, 110.0f * ui.s, colW * 292.0f / 668.0f);

    std::string kicker = "LOBBY Â· " + modeSubtitle;
    std::string mapDisplay = st.lobbyMap;
    for (int i = 0; i < Count(kMaps); ++i) {
        if (kMaps[i].scenario == st.lobbyMap) { mapDisplay = kMaps[i].name; break; }
    }
    for (int i = 0; i < Count(kFirefightMaps); ++i) {
        if (kFirefightMaps[i].scenario == st.lobbyMap) { mapDisplay = kFirefightMaps[i].name; break; }
    }
    for (int i = 0; i < Count(kMissions); ++i) {
        if (kMissions[i].scenario == st.lobbyMap) { mapDisplay = kMissions[i].name; break; }
    }

    float y = HeroBanner(ui, ImVec2(pxLeft, bannerTop), colW, bannerH, st.lobbyMap, kicker, mapDisplay.c_str(), "Gather players before launching into the match.");
    y += 12.0f * ui.s;

    // Map selector row (host can cycle maps)
    if (st.isLobbyHost) {
        const char* const mapLabel = mapDisplay.c_str();
        const int dv = OptionRow(ui, "lobby_map", ImVec2(pxLeft, y), colW, "SELECTED MAP", mapLabel, false);
        if (dv) {
            const MenuMap* baseMaps = (st.lobbyMode == "firefight") ? kFirefightMaps : kMaps;
            const int count = (st.lobbyMode == "firefight") ? Count(kFirefightMaps) : Count(kMaps);
            int curIdx = 0;
            for (int i = 0; i < count; ++i) {
                if (baseMaps[i].scenario == st.lobbyMap) { curIdx = i; break; }
            }
            curIdx = (curIdx + dv + count) % count;
            st.lobbyMap = baseMaps[curIdx].scenario;
            MasterServerClient::Get().UpdateLobbyAdvert(st.lobbyMap, st.lobbyMode, st.lobbyVariantName, st.lobbyDifficulty, st.lobbySkulls);
        }
    } else {
        OptionRow(ui, "lobby_map", ImVec2(pxLeft, y), colW, "SELECTED MAP", mapDisplay.c_str(), false);
    }
    y += 36.0f * ui.s;

    // Gametype / Variant row
    if (st.isLobbyHost && st.lobbyMode != "campaign" && st.lobbyMode != "forge") {
        const std::vector<MenuVariant>& varList = (st.lobbyMode == "firefight") ? ctx.ffVariants : ctx.variants;
        const char* vLabel = st.lobbyVariantName.empty() ? "Default" : st.lobbyVariantName.c_str();
        const int dv = OptionRow(ui, "lobby_var", ImVec2(pxLeft, y), colW, "GAME TYPE", vLabel, false);
        if (dv && !varList.empty()) {
            int curIdx = 0;
            for (int i = 0; i < static_cast<int>(varList.size()); ++i) {
                if (varList[i].label == st.lobbyVariantName || varList[i].path.string() == st.lobbyVariantPath) {
                    curIdx = i;
                    break;
                }
            }
            curIdx = (curIdx + dv + static_cast<int>(varList.size())) % static_cast<int>(varList.size());
            st.lobbyVariantPath = varList[curIdx].path.string();
            st.lobbyVariantName = varList[curIdx].label;
            MasterServerClient::Get().UpdateLobbyAdvert(st.lobbyMap, st.lobbyMode, st.lobbyVariantName, st.lobbyDifficulty, st.lobbySkulls);
        }
    } else {
        OptionRow(ui, "lobby_var", ImVec2(pxLeft, y), colW, "GAME TYPE", st.lobbyVariantName.c_str(), false);
    }
    y += 40.0f * ui.s;

    // Network / Invite Info Card
    const float cardH = 68.0f * ui.s;
    ui.dl->AddRectFilled(ImVec2(pxLeft, y), ImVec2(pxLeft + colW, y + cardH), IM_COL32(14, 20, 30, 200), 4.0f * ui.s);
    ui.dl->AddRect(ImVec2(pxLeft, y), ImVec2(pxLeft + colW, y + cardH), IM_COL32(60, 90, 130, 120), 4.0f * ui.s);

    std::string extIp = UPnP::Get().GetExternalIP();
    if (extIp.empty() || extIp == "Unknown") extIp = "Direct IP";
    char hostAddrBuf[128];
    if (st.isLobbyHost) {
        std::snprintf(hostAddrBuf, sizeof(hostAddrBuf), "HOST ADDRESS: %s:11774 (%s)", extIp.c_str(), st.lobbyNetMode == 1 ? "ONLINE P2P" : "LOCAL LAN");
    } else {
        std::snprintf(hostAddrBuf, sizeof(hostAddrBuf), "CONNECTED TO: %s", st.lobbyHostEndpoint.c_str());
    }
    Text(ui, NobleStyle::SmallFont(), 13.0f, ImVec2(pxLeft + 12.0f * ui.s, y + 10.0f * ui.s), IM_COL32(160, 210, 255, 230), hostAddrBuf);

    char statusSub[128];
    std::snprintf(statusSub, sizeof(statusSub), "%s Â· %s",
        st.isLobbyHost ? (st.lobbyPrivacy == 0 ? "Public Master Listing" : "Private Direct Join") : "Joined Session",
        UPnP::Get().IsActive() ? "UPnP Active" : "Direct Port");
    Text(ui, NobleStyle::SmallFont(), 12.0f, ImVec2(pxLeft + 12.0f * ui.s, y + 36.0f * ui.s), kDim, statusSub);

    // Copy Invite Button inside network card
    const float copyW = 120.0f * ui.s;
    const float copyH = 26.0f * ui.s;
    ImGui::SetCursorScreenPos(ImVec2(pxLeft + colW - copyW - 10.0f * ui.s, y + (cardH - copyH) * 0.5f));
    ImGui::PushID("btn_copy_lobby_inv");
    if (ImGui::Button("COPY INVITE", ImVec2(copyW, copyH))) {
        std::string inv = "/connect " + (st.isLobbyHost ? extIp + ":11774" : st.lobbyHostEndpoint);
        ImGui::SetClipboardText(inv.c_str());
        MenuSounds::Click();
    }
    ImGui::PopID();

    // ── RIGHT COLUMN: Player Lobby List (Roster) ──────────────────────────────
    const float rosterW = colW;
    const float rosterTop = top + 96.0f * ui.s;
    const float rosterH = actionsY - rosterTop;

    // Header bar: PLAYERS (X / 16)
    const float rHeaderH = 36.0f * ui.s;
    ui.dl->AddRectFilled(ImVec2(pxRight, rosterTop), ImVec2(pxRight + rosterW, rosterTop + rHeaderH), IM_COL32(16, 24, 38, 240), 4.0f * ui.s, ImDrawFlags_RoundCornersTop);
    ui.dl->AddRect(ImVec2(pxRight, rosterTop), ImVec2(pxRight + rosterW, rosterTop + rHeaderH), IM_COL32(60, 100, 150, 150), 4.0f * ui.s, ImDrawFlags_RoundCornersTop);

    char rosterHeaderBuf[64];
    std::snprintf(rosterHeaderBuf, sizeof(rosterHeaderBuf), "PLAYERS  (%zu / %d)", st.lobbyRoster.size(), st.lobbyMaxPlayers);
    Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(pxRight + 14.0f * ui.s, rosterTop + 9.0f * ui.s), IM_COL32(230, 240, 255, 240), rosterHeaderBuf);

    // Background container for player rows
    ui.dl->AddRectFilled(ImVec2(pxRight, rosterTop + rHeaderH), ImVec2(pxRight + rosterW, rosterTop + rosterH), IM_COL32(8, 12, 18, 210), 4.0f * ui.s, ImDrawFlags_RoundCornersBottom);
    ui.dl->AddRect(ImVec2(pxRight, rosterTop + rHeaderH), ImVec2(pxRight + rosterW, rosterTop + rosterH), IM_COL32(40, 60, 90, 120), 4.0f * ui.s, ImDrawFlags_RoundCornersBottom);

    // Player Rows
    float rowY = rosterTop + rHeaderH + 6.0f * ui.s;
    const float rowH = 44.0f * ui.s;

    if (st.lobbyRoster.empty()) {
        Text(ui, NobleStyle::SmallFont(), 14.0f, ImVec2(pxRight + 20.0f * ui.s, rowY + 12.0f * ui.s), kDim, "Waiting for players to join...");
    }

    for (size_t idx = 0; idx < st.lobbyRoster.size(); ++idx) {
        if (rowY + rowH > rosterTop + rosterH - 8.0f * ui.s) break;
        const auto& player = st.lobbyRoster[idx];

        const ImVec2 rMin(pxRight + 8.0f * ui.s, rowY);
        const ImVec2 rMax(pxRight + rosterW - 8.0f * ui.s, rowY + rowH);

        const ImU32 bgCol = player.isHost ? IM_COL32(24, 42, 68, 210) : IM_COL32(14, 20, 30, 180);
        ui.dl->AddRectFilled(rMin, rMax, bgCol, 3.0f * ui.s);

        const ImU32 stripCol = player.isHost ? IM_COL32(255, 200, 80, 255) : (player.isDev ? IM_COL32(255, 180, 50, 255) : IM_COL32(80, 160, 240, 220));
        ui.dl->AddRectFilled(rMin, ImVec2(rMin.x + 4.0f * ui.s, rMax.y), stripCol, 2.0f * ui.s);

        float textX = rMin.x + 14.0f * ui.s;

        if (player.isHost) {
            Text(ui, NobleStyle::SmallFont(), 12.0f, ImVec2(textX, rowY + 6.0f * ui.s), IM_COL32(255, 200, 80, 255), "[HOST]");
            textX += 44.0f * ui.s;
        }

        std::string lobbyUser = player.username;
        if (player.isDev) {
            lobbyUser += " [DEV]";
        }
        Text(ui, NobleStyle::TitleFont(), 17.0f, ImVec2(textX, rowY + 4.0f * ui.s),
             player.isDev ? IM_COL32(255, 215, 0, 255) : kWhite,
             lobbyUser.c_str());

        std::string tagStr = "[" + (player.serviceTag.empty() ? "117" : player.serviceTag) + "]";
        float tagX = textX + TextWidth(NobleStyle::TitleFont(), 17.0f * ui.s, lobbyUser.c_str()) + 8.0f * ui.s;
        Text(ui, NobleStyle::SmallFont(), 13.0f, ImVec2(tagX, rowY + 7.0f * ui.s), IM_COL32(150, 170, 190, 200), tagStr.c_str());

        if (player.isDev) {
            float devX = tagX + TextWidth(NobleStyle::SmallFont(), 13.0f * ui.s, tagStr.c_str()) + 8.0f * ui.s;
            ui.dl->AddRectFilled(ImVec2(devX, rowY + 6.0f * ui.s), ImVec2(devX + 44.0f * ui.s, rowY + 22.0f * ui.s), IM_COL32(180, 130, 20, 220), 2.0f * ui.s);
            Text(ui, NobleStyle::SmallFont(), 11.0f, ImVec2(devX + 5.0f * ui.s, rowY + 7.0f * ui.s), IM_COL32(255, 255, 255, 255), "DEV");
        }

        char subInfo[96];
        if (player.isHost) {
            std::snprintf(subInfo, sizeof(subInfo), "P:ID %llu Â· Host Â· Session Leader", static_cast<unsigned long long>(player.peerId));
        } else {
            std::snprintf(subInfo, sizeof(subInfo), "P:ID %llu Â· Ready Â· %dms", static_cast<unsigned long long>(player.peerId), player.ping);
        }
        Text(ui, NobleStyle::SmallFont(), 12.0f, ImVec2(rMin.x + 14.0f * ui.s, rowY + 25.0f * ui.s), kDim, subInfo);

        rowY += rowH + 4.0f * ui.s;
    }

    // ── BOTTOM ACTION BUTTONS ─────────────────────────────────────────────────
    if (st.isLobbyHost) {
        if (BigAction(ui, "btn_start_match", ImVec2(pxLeft, actionsY), colW - 140.0f * ui.s, "START MATCH", "Launch match for all players in lobby", true, true, false)) {
            const std::string variantFileName = st.lobbyVariantPath.empty()
                ? ""
                : std::filesystem::path(st.lobbyVariantPath).filename().string();
            MasterServerClient::Get().LaunchLobbySync(st.lobbySessionId, st.lobbyMap, st.lobbyMode, variantFileName, st.lobbyDifficulty, st.lobbySkulls);











                    { "session_id", st.lobbySessionId }
                }}
            };
            // Duplicate WS launch removed (Finding #6)

            choice.play = true;
            choice.host = true;
            choice.sessionId = st.lobbySessionId;
            choice.sessionToken = st.lobbySessionToken;
            choice.hostOnline = (st.lobbyNetMode == 1);
            choice.hostPublic = (st.lobbyPrivacy == 0);
            choice.hostMaxPlayers = st.lobbyMaxPlayers;
            choice.map = st.lobbyMap;
            choice.mode = st.lobbyMode;
            choice.variantPath = st.lobbyVariantPath;
            choice.difficulty = st.lobbyDifficulty;
            choice.skulls = st.lobbySkulls;
            done = true;
        }

        if (BigAction(ui, "btn_cancel_lobby", ImVec2(pxLeft + colW - 130.0f * ui.s, actionsY), 130.0f * ui.s, "CANCEL", "Close Lobby", false, true, false) || back) {
            MasterServerClient::Get().StopAdvertising();
            SessionManager::Get().StopHosting();
            NetworkTransport::Get().StopRelay();
            st.screen = Screen::CustomGame;
        }
    } else {
        BigAction(ui, "btn_waiting_match", ImVec2(pxLeft, actionsY), colW - 140.0f * ui.s, "WAITING FOR HOST", "Host will launch match synchronously", false, false, false);

        if (BigAction(ui, "btn_leave_lobby", ImVec2(pxLeft + colW - 130.0f * ui.s, actionsY), 130.0f * ui.s, "LEAVE", "Exit Lobby", false, true, false) || back) {
            uint64_t pId = AuthClient::Get().IsLoggedIn() ? static_cast<uint64_t>(AuthClient::Get().GetUserId()) : 2ULL;
            MasterServerClient::Get().LeaveLobbySync(st.lobbySessionId, pId);
            NetworkTransport::Get().StopRelay();
            st.screen = Screen::Servers;
        }
    }

    DrawHints(ui, { { Prompt::Accept, st.isLobbyHost ? "Start Match" : "Ready" }, { Prompt::Back, st.isLobbyHost ? "Cancel Lobby" : "Leave Lobby" } });
}

void DrawFrontEndContents(OverlayContext& ctx, ID3D11Device* device, ID3D11DeviceContext* context,
                          FrontEndChoice& choice, bool& done) {
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

    MenuState& st = ctx.st;
    {
        static int s_lastScreen = -1;
        if (static_cast<int>(st.screen) != s_lastScreen) {
            NOBLE_INFO("FrontEndMenu: screen {} -> {}", s_lastScreen, static_cast<int>(st.screen));
            s_lastScreen = static_cast<int>(st.screen);
        }
    }
    Ui ui;
    ui.dl = ImGui::GetWindowDrawList();
    ui.size = vp->Size;
    ui.s = std::max(0.75f, vp->Size.y / 720.0f);
    DrawBackdrop(ui, st.screen);
    DrawAccountModal(ui);
    DrawDevMenuModal(ui);

    UpdateSceneForScreen(st.screen);

    const float left = ui.size.x * 0.075f;
    const float top = ui.size.y * 0.09f;
    const SettingsData settings = Settings::Get().Snapshot();
    s_hintThisFrame = std::exchange(s_hintClicked, -1);
    const bool back = ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false) ||
                      HintClicked(Prompt::Back);
    auto goBack = [&](Screen to) {
        st.screen = to;
        st.focusRow = 0;
        MenuSounds::Back();
    };
    auto saveSkulls = [&] {
        SettingsData d = Settings::Get().Snapshot();
        d.campaignSkulls = st.skulls;
        Settings::Get().Update(d);
    };

    switch (st.screen) {
    case Screen::Main: {
        // While the quit box is open nothing behind it reacts to the mouse (no hover, sounds or clicks).
        const bool modal = st.confirmQuit;
        ImGui::BeginDisabled(modal);
        DrawParty(ui, settings.playerName, "In Main Menu Lobby");
        DrawTitle(ui, ImVec2(left, ui.size.y * 0.30f), "MAIN MENU");
        Text(ui, nullptr, 17.0f, ImVec2(left + 4.0f * ui.s, ui.size.y * 0.30f + 66.0f * ui.s), kDim,
             "This party is local to this PC.");
        std::vector<ListItem> items;
        for (const MainEntry& e : kMainEntries) items.push_back({ e.label, e.enabled });
        const float listY = ui.size.y * 0.30f + 104.0f * ui.s;
        const int hit = DrawList(ui, "main", ImVec2(left, listY), 440.0f * ui.s, items, st.mainIndex, !modal, 27.0f);
        ImGui::EndDisabled();
        const float descY = listY + items.size() * 27.0f * ui.s + 10.0f * ui.s;
        Text(ui, nullptr, 17.0f, ImVec2(left + 4.0f * ui.s, descY), kDim, kMainEntries[st.mainIndex].description,
             left + 430.0f * ui.s);
        if (!modal) {
            if (hit >= 0) {
                if (kMainEntries[hit].quit) {
                    st.confirmQuit = true;
                    st.quitChoice = 1;
                } else if (std::string(kMainEntries[hit].label) == "Account") {
                    s_showAccountModal = true;
                } else {
                    st.screen = kMainEntries[hit].target;
                }
                st.focusRow = 0;
            } else if (back) {
                // Esc on the main menu asks before closing Noble.
                st.confirmQuit = true;
                st.quitChoice = 1;
                MenuSounds::Back();
            }
            DrawHints(ui, { { Prompt::Accept, "Select" }, { Prompt::Back, "Quit" } });
        } else {
            // Dim everything and show the question in the middle.
            ui.dl->AddRectFilled(ImVec2(0, 0), ui.size, IM_COL32(0, 0, 0, 150));
            const float bw = 460.0f * ui.s, bh = 190.0f * ui.s;
            const ImVec2 b0((ui.size.x - bw) * 0.5f, (ui.size.y - bh) * 0.5f);
            const ImVec2 b1(b0.x + bw, b0.y + bh);
            ui.dl->AddRectFilled(b0, b1, IM_COL32(12, 20, 30, 245));
            ui.dl->AddRect(b0, b1, IM_COL32(143, 216, 255, 160), 0.0f, 0, 1.2f);
            ui.dl->AddLine(ImVec2(b0.x, b0.y + 3.0f * ui.s), ImVec2(b1.x, b0.y + 3.0f * ui.s), kAccent, 3.0f * ui.s);
            Text(ui, NobleStyle::TitleFont(), 26.0f, ImVec2(b0.x + 24.0f * ui.s, b0.y + 22.0f * ui.s), kWhite, "QUIT PROJECT NOBLE?");
            Text(ui, nullptr, 17.0f, ImVec2(b0.x + 24.0f * ui.s, b0.y + 62.0f * ui.s), kDim, "Are you sure you want to close the game?");
            const float btnW = (bw - 24.0f * ui.s * 3.0f) * 0.5f;
            const float btnY = b1.y - 64.0f * ui.s;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_RightArrow) ||
                ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow) ||
                ImGui::IsKeyPressed(ImGuiKey_Tab) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight)) {
                st.quitChoice = 1 - st.quitChoice;
                MenuSounds::Scroll();
            }
            const bool yes = ActionButton(ui, "quit_yes", ImVec2(b0.x + 24.0f * ui.s, btnY), btnW, "YES", st.quitChoice == 0);
            const bool no = ActionButton(ui, "quit_no", ImVec2(b0.x + 48.0f * ui.s + btnW, btnY), btnW, "NO", st.quitChoice == 1);
            if (yes || (AcceptPressed() && st.quitChoice == 0)) {
                st.confirmQuit = false;
                done = true;
            } else if (no || back || (AcceptPressed() && st.quitChoice == 1)) {
                st.confirmQuit = false;
                if (!no) MenuSounds::Back();
            }
            DrawHints(ui, { { Prompt::Accept, "Select" }, { Prompt::Back, "Cancel" } });
        }
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
        if (DrawSourceTabs(ui, ImVec2(left + 34.0f * ui.s, top + 70.0f * ui.s), st.campaignSource,
                           static_cast<int>(ctx.campaignMods.size()))) st.focusRow = 0;
        const bool modded = st.campaignSource == 1;
        const std::vector<MapEntry> missions = modded ? ModdedMaps(ctx.campaignMods, ModMode::Campaign, kMissions, Count(kMissions), st.openMods)
                                                      : BaseMaps(kMissions, Count(kMissions));
        int& pick = modded ? st.campaignModded : st.mission;
        FixSelection(missions, pick);
        std::vector<ListItem> items = ToItems(missions);
        if (missions.empty()) items.push_back({ "No Reach campaign mods installed", false, false });
        const float listY = top + 108.0f * ui.s;
        const float listRow = 42.0f;
        const int listRows = std::max(4, static_cast<int>((ui.size.y - 74.0f * ui.s - listY) / (listRow * ui.s)));
        const int hit = DrawList(ui, "missions", ImVec2(left + 34.0f * ui.s, listY), 400.0f * ui.s, items, pick,
                                 st.focusRow == 0, listRow, listRows, false);
        if (hit >= 0 && !missions.empty() && missions[hit].header) ToggleMod(st.openMods, ModMode::Campaign, *missions[hit].mod);
        else if (hit >= 0) st.focusRow = 1;
        FixSelection(missions, pick);
        const MapEntry* row = missions.empty() ? nullptr : &missions[pick];
        const MapEntry* mission = (row && !row->header) ? row : nullptr;
        const ModEntry* mod = mission ? mission->mod : nullptr;

        // Right column, laid out from the bottom up so the buttons never reach the hint row:
        // PLAY SOLO / HOST CO-OP, then skulls + score, difficulty tiles, and the banner fills the rest.
        const float px = ui.size.x * 0.47f, pw = ui.size.x * 0.44f;
        const float actionsY = ui.size.y - 58.0f * ui.s - 14.0f * ui.s - 50.0f * ui.s;
        const float rowsH = 98.0f * ui.s + 8.0f * ui.s + 36.0f * ui.s + 30.0f * ui.s + 10.0f * ui.s;
        const float bannerTop = top + 96.0f * ui.s;
        const float blurbH = 30.0f * ui.s;
        const float bannerH = std::clamp(actionsY - rowsH - blurbH - bannerTop, 110.0f * ui.s, pw * 292.0f / 668.0f);
        const std::string kicker = mod ? "MOD: " + Upper(mod->title) : row && row->header ? std::string("MOD") : std::string("HALO: REACH \xC2\xB7 CAMPAIGN");
        float y = HeroBanner(ui, ImVec2(px, bannerTop), pw, bannerH, mission ? mission->scenario : "", kicker,
                             row ? (row->header ? row->mod->title.c_str() : row->name.c_str()) : "",
                             row ? row->blurb.c_str() : "");
        y = std::max(y, actionsY - rowsH);
        y += DifficultyTiles(ui, "diff", ImVec2(px, y), pw, st.difficulty, st.focusRow == 1) + 8.0f * ui.s;
        const std::string skullText = SkullSummary(st.skulls);
        if (OptionRow(ui, "skulls", ImVec2(px, y), pw, "SKULLS", skullText.c_str(), st.focusRow == 2)) {
            st.screen = Screen::Skulls;
        }
        ImGui::SetCursorScreenPos(ImVec2(px, y));
        if (ImGui::InvisibleButton("##skullrow", ImVec2(pw * 0.46f, 32.0f * ui.s))) st.screen = Screen::Skulls;
        y += 36.0f * ui.s;
        const float stripW = SkullStrip(ui, ImVec2(px + 10.0f * ui.s, y + 2.0f * ui.s), 20.0f * ui.s, st.skulls, 13);
        char mult[64];
        std::snprintf(mult, sizeof(mult), "Score multiplier  x%.2f", ScoreMultiplier(st.difficulty, st.skulls));
        Text(ui, nullptr, 16.0f, ImVec2(px + 10.0f * ui.s + (stripW > 0.0f ? stripW + 12.0f * ui.s : 0.0f), y + 3.0f * ui.s), kDim, mult);
        const float lobbyY = actionsY - 38.0f * ui.s;
        DrawLobbyControls(ui, ImVec2(px, lobbyY), pw, st);

        const bool canStart = mission && mission->enabled;
        const float bw = (pw - 10.0f * ui.s) * 0.5f;
        std::string sub = mission ? std::string(mission->name) + " · " + kDifficultyNames[st.difficulty] : std::string("Pick a mission");
        const bool startSolo = (BigAction(ui, "start", ImVec2(px, actionsY), bw, "PLAY SOLO", sub.c_str(), true, canStart, st.focusRow == 3) ||
                                (st.focusRow == 3 && AcceptPressed())) && canStart;
        const char* coopSub = st.lobbyNetMode == 1 ? (st.lobbyPrivacy == 0 ? "Public Online P2P" : "Private Online P2P") : "Local LAN Co-op";
        const bool hostCoop = BigAction(ui, "coop", ImVec2(px + bw + 10.0f * ui.s, actionsY), pw - bw - 10.0f * ui.s, "HOST CO-OP", coopSub, true, canStart);
        if (hostCoop) {
            SetupHostLobby(st, mission->scenario, "campaign", "", "Campaign", st.difficulty, st.skulls, 4);
        } else if (startSolo) {
            choice.play = true;
            choice.host = false;
            choice.mode = "campaign";
            choice.map = mission->scenario;
            choice.difficulty = st.difficulty;
            choice.skulls = st.skulls;
            if (mod) { choice.modFolder = mod->folder; choice.modTitle = mod->title; }
            done = true;
        }
        if (st.focusRow > 0 && (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_Tab) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown))) st.focusRow = std::min(3, st.focusRow + 1);
        if (st.focusRow > 0 && (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp))) st.focusRow = std::max(1, st.focusRow - 1);
        if (st.focusRow == 2 && AcceptPressed()) st.screen = Screen::Skulls;
        if (back) {
            if (st.focusRow > 0) st.focusRow = 0;
            else goBack(Screen::Main);
        }
        DrawHints(ui, { { Prompt::Accept, "Select" }, { Prompt::Back, "Back" }, { Prompt::Change, "Change" }, { Prompt::Tabs, "Vanilla / Modded" } });
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
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight)) st.skull = std::min(kSkullCount - 1, st.skull + 1);
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft)) st.skull = std::max(0, st.skull - 1);
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown)) st.skull = std::min(kSkullCount - 1, st.skull + 5);
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp)) st.skull = std::max(0, st.skull - 5);
        bool toggle = AcceptPressed() || ImGui::IsKeyPressed(ImGuiKey_Space, false);

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

        const float by = ui.size.y - 104.0f * ui.s;
        ui.dl->AddRectFilledMultiColor(ImVec2(left + 40.0f * ui.s, by), ImVec2(ui.size.x * 0.80f, by + 40.0f * ui.s),
                                       IM_COL32(40, 52, 64, 210), IM_COL32(40, 52, 64, 60), IM_COL32(40, 52, 64, 60), IM_COL32(40, 52, 64, 210));
        Text(ui, NobleStyle::TitleFont(), 24.0f, ImVec2(left + 52.0f * ui.s, by + 6.0f * ui.s), kWhite, settings.playerName.c_str());
        char total[64];
        std::snprintf(total, sizeof(total), "x%.2f", ScoreMultiplier(st.difficulty, st.skulls));
        Text(ui, NobleStyle::TitleFont(), 24.0f, ImVec2(ui.size.x * 0.80f - 90.0f * ui.s, by + 6.0f * ui.s), kWhite, total);
        if (back) goBack(Screen::Campaign);
        DrawHints(ui, { { Prompt::Accept, "Toggle" }, { Prompt::Back, "Back" } });
        break;
    }

    case Screen::Firefight:
    case Screen::CustomGame:
    case Screen::Forge: {
        const bool ff = st.screen == Screen::Firefight;
        const bool forge = st.screen == Screen::Forge;
        DrawTitle(ui, ImVec2(left, top), ff ? "FIREFIGHT" : forge ? "FORGE" : "CUSTOM GAME", ff ? "SURVIVAL" : forge ? "EDIT" : "MULTIPLAYER");
        const MenuMap* baseMaps = ff ? kFirefightMaps : kMaps;
        const int baseCount = ff ? Count(kFirefightMaps) : Count(kMaps);
        const std::vector<const ModEntry*>& modList = ff ? ctx.ffMods : ctx.mpMods;
        int& source = ff ? st.ffSource : forge ? st.forgeSource : st.mpSource;
        float listTop = top + 96.0f * ui.s;
        {
            if (DrawSourceTabs(ui, ImVec2(left + 34.0f * ui.s, top + 70.0f * ui.s), source, static_cast<int>(modList.size()))) {
                st.focusRow = 0;
            }
            listTop = top + 108.0f * ui.s;
        }
        const bool modded = source == 1;
        const ModMode modMode = ff ? ModMode::Firefight : ModMode::Multiplayer;
        const std::vector<MapEntry> maps = modded ? ModdedMaps(modList, modMode, baseMaps, baseCount, st.openMods)
                                                  : BaseMaps(baseMaps, baseCount);
        int& mapIndex = modded ? (ff ? st.ffModded : forge ? st.forgeModded : st.mpModded) : (ff ? st.ffMap : forge ? st.forgeMap : st.map);
        FixSelection(maps, mapIndex);
        std::vector<ListItem> items = ToItems(maps);
        if (maps.empty()) items.push_back({ ff ? "No Reach Firefight mods installed" : "No Reach multiplayer mods installed", false, false });
        const float listRow = 40.0f;
        const int listRows = std::max(4, static_cast<int>((ui.size.y - 74.0f * ui.s - listTop) / (listRow * ui.s)));
        const int hit = DrawList(ui, "maps", ImVec2(left + 34.0f * ui.s, listTop), 380.0f * ui.s, items, mapIndex,
                                 st.focusRow == 0, listRow, listRows, false);
        if (hit >= 0 && !maps.empty() && maps[hit].header) ToggleMod(st.openMods, modMode, *maps[hit].mod);
        else if (hit >= 0) st.focusRow = 1;
        FixSelection(maps, mapIndex);
        const MapEntry* row = maps.empty() ? nullptr : &maps[mapIndex];
        const MapEntry* map = (row && !row->header) ? row : nullptr;
        const ModEntry* mod = map ? map->mod : nullptr;

        const float px = ui.size.x * 0.47f, pw = ui.size.x * 0.44f;
        const float actionsY = ui.size.y - 58.0f * ui.s - 14.0f * ui.s - 50.0f * ui.s;
        const bool forgeMissing = forge && !std::filesystem::exists(ctx.forgeVariant);
        const float rowsH = (forge ? 0.0f : 36.0f * ui.s) + (ff ? 30.0f * ui.s + 98.0f * ui.s + 8.0f * ui.s : 0.0f) +
                            (forgeMissing ? 26.0f * ui.s : 0.0f) + 10.0f * ui.s;
        const float bannerTop = top + 96.0f * ui.s;
        const float bannerH = std::clamp(actionsY - rowsH - 30.0f * ui.s - bannerTop, 110.0f * ui.s, pw * 292.0f / 668.0f);
        const std::string kicker = mod ? "MOD: " + Upper(mod->title)
                                 : std::string(ff ? "HALO: REACH \xC2\xB7 FIREFIGHT" : forge ? "HALO: REACH \xC2\xB7 FORGE" : "HALO: REACH \xC2\xB7 MULTIPLAYER");
        float y = HeroBanner(ui, ImVec2(px, bannerTop), pw, bannerH, map ? map->scenario : "", kicker,
                             row ? (row->header ? row->mod->title.c_str() : row->name.c_str()) : "",
                             row ? row->blurb.c_str() : "");
        y = std::max(y, actionsY - rowsH);
        int focusRows = 1;
        if (!forge) {
            const std::vector<MenuVariant>& list = ff ? ctx.ffVariants : ctx.variants;
            int& v = ff ? st.ffVariant : st.variant;
            const char* label = list.empty() ? "Default" : list[std::clamp(v, 0, static_cast<int>(list.size()) - 1)].label.c_str();
            const int dv = OptionRow(ui, "variant", ImVec2(px, y), pw, "GAME TYPE", label, st.focusRow == focusRows);
            if (dv && !list.empty()) v = (v + dv + static_cast<int>(list.size())) % static_cast<int>(list.size());
            y += 36.0f * ui.s;
            ++focusRows;
        }
        if (ff) {
            // Spec §4.3: per-game overrides (time limit, lives, skulls) once Noble can write Firefight variants.
            Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(px + 10.0f * ui.s, y + 4.0f * ui.s), kDisabled, "FIREFIGHT OPTIONS \xC2\xB7 DEFAULT");
            const char* later = "Coming later";
            Text(ui, nullptr, 15.0f, ImVec2(px + pw - TextWidth(nullptr, 15.0f * ui.s, later) - 10.0f * ui.s, y + 6.0f * ui.s), kDisabled, later);
            y += 30.0f * ui.s;
        }
        if (ff) {
            y += DifficultyTiles(ui, "ffdiff", ImVec2(px, y), pw, st.difficulty, st.focusRow == focusRows) + 8.0f * ui.s;
            ++focusRows;
        }
        if (forgeMissing) {
            Text(ui, nullptr, 16.0f, ImVec2(px + 10.0f * ui.s, y), IM_COL32(255, 190, 120, 255),
                 "MCC's Forge game type (00_basic_editing_054.bin) wasn't found.");
            y += 26.0f * ui.s;
        }
        const float lobbyY = actionsY - 38.0f * ui.s;
        DrawLobbyControls(ui, ImVec2(px, lobbyY), pw, st);

        const bool canStart = map && map->enabled;
        const float bw = (pw - 10.0f * ui.s) * 0.5f;
        std::string sub;
        if (!map) sub = "Pick a map";
        else if (ff) sub = "Hold out alone · " + std::string(kDifficultyNames[st.difficulty]);
        else if (forge) sub = map->name + " · Forge";
        else {
            const std::vector<MenuVariant>& list = ctx.variants;
            sub = map->name + " · " + (list.empty() ? std::string("Default") : list[std::clamp(st.variant, 0, static_cast<int>(list.size()) - 1)].label);
        }

        const char* soloLabel = ff ? "PLAY SOLO" : forge ? "SOLO FORGE" : "PLAY LOCAL";
        const char* hostLabel = ff ? "HOST CO-OP" : forge ? "HOST FORGE" : "HOST LOBBY";
        const char* hostSub = st.lobbyNetMode == 1 ? (st.lobbyPrivacy == 0 ? "Public Online P2P" : "Private Online P2P") : "Local LAN Lobby";

        const bool startSolo = (BigAction(ui, "start", ImVec2(px, actionsY), bw, soloLabel, sub.c_str(), true, canStart, st.focusRow == focusRows) ||
                                (st.focusRow == focusRows && AcceptPressed())) && canStart;
        const bool startHost = BigAction(ui, "host", ImVec2(px + bw + 10.0f * ui.s, actionsY), pw - bw - 10.0f * ui.s, hostLabel, hostSub, true, canStart);

        if (startHost) {
            std::string varPath, varName;
            if (ff) {
                if (!ctx.ffVariants.empty()) {
                    varPath = ctx.ffVariants[st.ffVariant].path.string();
                    varName = ctx.ffVariants[st.ffVariant].label;
                } else {
                    varName = "Firefight Standard";
                }
            } else if (forge) {
                varPath = ctx.forgeVariant.string();
                varName = "Basic Editing";
            } else {
                if (!ctx.variants.empty()) {
                    varPath = ctx.variants[st.variant].path.string();
                    varName = ctx.variants[st.variant].label;
                } else {
                    varName = "Slayer";
                }
            }
            const std::string mode = ff ? "firefight" : (forge ? "forge" : "multiplayer");
            SetupHostLobby(st, map ? map->scenario : "20_sword_slayer", mode, varPath, varName, ff ? st.difficulty : 1, 0, ff ? 4 : 16);
        } else if (startSolo) {
            choice.play = true;
            choice.host = false;
            choice.map = map->scenario;
            if (mod) { choice.modFolder = mod->folder; choice.modTitle = mod->title; }
            if (ff) {
                choice.mode = "firefight";
                choice.difficulty = st.difficulty;
                if (!ctx.ffVariants.empty()) choice.variantPath = ctx.ffVariants[st.ffVariant].path.string();
            } else {
                choice.mode = "multiplayer";
                if (forge) choice.variantPath = ctx.forgeVariant.string();
                else if (!ctx.variants.empty()) choice.variantPath = ctx.variants[st.variant].path.string();
            }
            done = true;
        }
        if (st.focusRow > 0 && (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_Tab) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown))) st.focusRow = std::min(focusRows, st.focusRow + 1);
        if (st.focusRow > 0 && (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp))) st.focusRow = std::max(1, st.focusRow - 1);
        if (back) {
            if (st.focusRow > 0) st.focusRow = 0;
            else goBack(Screen::Main);
        }
        if (forge) DrawHints(ui, { { Prompt::Accept, "Select" }, { Prompt::Back, "Back" }, { Prompt::Tabs, "Vanilla / Modded" } });
        else DrawHints(ui, { { Prompt::Accept, "Select" }, { Prompt::Back, "Back" }, { Prompt::Change, "Change" }, { Prompt::Tabs, "Vanilla / Modded" } });
        break;
    }

    case Screen::Armory: {
        // MCC Halo: Reach "CUSTOMIZATION / APPEARANCE" header + nameplate + live in-engine 3D Spartan/Elite
        Text(ui, NobleStyle::SmallFont(), 15.0f, ImVec2(left, top - 8.0f * ui.s), kAccent, "CUSTOMIZATION");
        Text(ui, NobleStyle::HeroFont(), 44.0f, ImVec2(left, top + 6.0f * ui.s), kWhite, "APPEARANCE");

        // Top-right player nameplate with service tag
        {
            const auto& prof = UIManager::GetArmorCustomizer().GetProfile();
            const float plateW = 280.0f * ui.s;
            const float plateH = 42.0f * ui.s;
            const ImVec2 p0(ui.size.x - left - plateW, top + 6.0f * ui.s);
            const ImVec2 p1(p0.x + plateW, p0.y + plateH);
            ui.dl->AddRectFilledMultiColor(p0, p1,
                IM_COL32(36, 58, 82, 230), IM_COL32(82, 112, 142, 210),
                IM_COL32(62, 88, 115, 210), IM_COL32(28, 46, 68, 230));
            ui.dl->AddRect(p0, p1, IM_COL32(165, 205, 240, 180), 0.0f, 0, 1.2f);
            const ImVec2 emb(p0.x + 22.0f * ui.s, p0.y + plateH * 0.5f);
            ui.dl->AddCircleFilled(emb, 13.0f * ui.s, IM_COL32(16, 28, 42, 240));
            ui.dl->AddTriangleFilled(ImVec2(emb.x - 7 * ui.s, emb.y - 3 * ui.s),
                                     ImVec2(emb.x + 7 * ui.s, emb.y - 3 * ui.s),
                                     ImVec2(emb.x, emb.y + 7 * ui.s), kAccent);
            const std::string tagLine = "[" + prof.serviceTag + "]";
            Text(ui, NobleStyle::SmallFont(), 14.0f, ImVec2(p0.x + 44.0f * ui.s, p0.y + 4.0f * ui.s), IM_COL32(190, 215, 235, 230), tagLine.c_str());
            Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(p0.x + 44.0f * ui.s, p0.y + 20.0f * ui.s), kWhite, settings.playerName.c_str());
        }

        // Thin horizontal rule with active "HALO: REACH" underline
        const float ruleY = top + 64.0f * ui.s;
        ui.dl->AddLine(ImVec2(left, ruleY), ImVec2(ui.size.x - left, ruleY), IM_COL32(140, 180, 220, 55), 1.0f);
        const char* gameTab = "HALO: REACH";
        const float tabW = TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, gameTab);
        const float tabX = ui.size.x * 0.5f - tabW * 0.5f;
        Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(tabX, ruleY - 24.0f * ui.s), kWhite, gameTab);
        ui.dl->AddLine(ImVec2(tabX - 8.0f * ui.s, ruleY), ImVec2(tabX + tabW + 8.0f * ui.s, ruleY), kAccent, 2.5f * ui.s);

        ImGui::SetCursorScreenPos(ImVec2(left, ruleY + 12.0f * ui.s));
        ImGui::BeginChild("##armory_panel", ImVec2(ui.size.x - left * 1.35f, ui.size.y - (ruleY + 12.0f * ui.s) - 62.0f * ui.s),
                          ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
        UIManager::GetArmorCustomizer().Render(device, context);
        ImGui::EndChild();

        if (back && !ImGui::GetIO().WantTextInput) {
            goBack(Screen::Main);
        }
        DrawHints(ui, { { Prompt::Back, "Back" }, { Prompt::Change, "Change Piece" }, { Prompt::Tabs, "Rotate Character (Q/E or Drag)" } });
        break;
    }

    case Screen::Servers:
    case Screen::Settings:
    case Screen::Career:
    case Screen::Mods: {
        const char* title = st.screen == Screen::Servers ? "GAMES" : st.screen == Screen::Settings ? "SETTINGS"
                          : st.screen == Screen::Career ? "SERVICE RECORD" : "MODS";
        // These screens use the whole screen, like Settings.
        const bool full = true;
        const float px = full ? 24.0f * ui.s : left;
        const float py = full ? 86.0f * ui.s : top + 84.0f * ui.s;
        if (full) {
            ui.dl->AddRectFilled(ImVec2(0, 0), ui.size, IM_COL32(4, 8, 12, 215));
            DrawTitle(ui, ImVec2(px, 10.0f * ui.s), title);
        } else {
            DrawTitle(ui, ImVec2(left, top), title);
        }
        ImGui::SetCursorScreenPos(ImVec2(px, py));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.03f, 0.05f, 0.07f, full ? 0.60f : 0.82f));
        ImGui::BeginChild("##panel", ImVec2(ui.size.x - px * 2.0f, ui.size.y - py - 62.0f * ui.s),
                          ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
        if (st.screen == Screen::Servers) {
            UIManager::GetServerBrowser().Render();
        } else if (st.screen == Screen::Settings) {
            ctx.settingsPanel.Render();
        } else if (st.screen == Screen::Career) {
            ctx.careerPanel.Render();
        } else {
            if (!ctx.modsScanned) {
                const auto steamapps = ctx.reachDir.parent_path().parent_path().parent_path();
                wchar_t exe[MAX_PATH] = {};
                GetModuleFileNameW(nullptr, exe, MAX_PATH);
                UIManager::GetWorkshopManager().RefreshMods(steamapps / "workshop" / "content" / "976730",
                                                            std::filesystem::path(exe).parent_path() / "data" / "mods");
                ctx.modsScanned = true;
            }
            UIManager::GetWorkshopManager().Render();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        if (back && !ImGui::GetIO().WantTextInput && !ctx.settingsPanel.IsCapturingKey()) {
            goBack(st.screen == Screen::Servers ? Screen::CustomGame : Screen::Main);
        }
        DrawHints(ui, { { Prompt::Back, "Back" } });
        break;
    }

    case Screen::PreGameLobby: {
        DrawPreGameLobby(ctx, ui, left, top, choice, done, back);
        break;
    }
    }

    ImGui::End();
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

void FrontEndMenu::Initialize(const std::filesystem::path& reachDir) {
    OverlayContext& ctx = GetOverlayCtx();
    ctx.reachDir = reachDir;
    ctx.variants = ScanVariants(reachDir, false);
    ctx.ffVariants = ScanVariants(reachDir, true);
    ctx.forgeVariant = reachDir / "game_variants" / kForgeVariant;

    ctx.mods = ModCatalog::Scan(VFS::GetWorkshopPath(), VFS::GetUserStoragePath() / "mods");
    ctx.campaignMods.clear();
    ctx.ffMods.clear();
    ctx.mpMods.clear();
    for (const ModEntry& m : ctx.mods) {
        auto replaces = [&](const MenuMap* base, int count) {
            return std::any_of(m.mapFiles.begin(), m.mapFiles.end(), [&](const std::string& f) { return KnownScenario(f, base, count); });
        };
        if (!m.campaign.empty() || replaces(kMissions, Count(kMissions))) ctx.campaignMods.push_back(&m);
        if (!m.firefight.empty() || replaces(kFirefightMaps, Count(kFirefightMaps))) ctx.ffMods.push_back(&m);
        if (!m.multiplayer.empty() || replaces(kMaps, Count(kMaps))) ctx.mpMods.push_back(&m);
    }

    ctx.st = MenuState{};
    ctx.st.variant = FindVariant(ctx.variants, L"slayer_054.bin");
    ctx.st.ffVariant = FindVariant(ctx.ffVariants, L"hr_ff_standard_1set.bin");
    ctx.st.skulls = Settings::Get().Snapshot().campaignSkulls;

    ReachAudio::SetMusicVolume(Settings::Get().Snapshot().volumeMusic);
    ReachAudio::PlayMenuMusic(Settings::Get().Snapshot().menuMusicTrack);

    int startTab = 0;
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (std::wstring(argv[i]) == L"-menu-screen") ctx.st.screen = ParseScreenArg(argv[i + 1]);
            if (std::wstring(argv[i]) == L"-menu-capture") ctx.captureFrame = _wtoi64(argv[i + 1]);
            if (std::wstring(argv[i]) == L"-menu-tab") startTab = _wtoi(argv[i + 1]);
        }
        if (argv) LocalFree(argv);
    }
    ctx.settingsPanel.SetTab(startTab);
    ctx.careerPanel.SetTab(startTab);
    ctx.modsScanned = false;
    ctx.totalFrames = 0;
    ctx.sceneReadyFrames = 0;
    ctx.closed = false;
    ctx.initialized = true;

    UIManager::GetArmorCustomizer().SyncToReachArmory();

    // Server Browser Join Pre-Game Lobby Handler
    UIManager::GetServerBrowser().SetJoinLobbyHandler([&ctx](const ServerEntry& s) {
        ctx.st.isLobbyHost = false;
        ctx.st.lobbySessionId = s.id;
        ctx.st.lobbyHostEndpoint = s.hostAddress + ":" + std::to_string(s.port);
        ctx.st.lobbyMap = s.mapName.empty() ? "20_sword_slayer" : s.mapName;
        ctx.st.lobbyMode = s.gameMode.empty() ? "multiplayer" : s.gameMode;
        ctx.st.lobbyVariantName = s.variantName.empty() ? "Default" : s.variantName;
        ctx.st.lobbyMaxPlayers = s.maxPlayers;
        ctx.st.lobbyRoster.clear();
        ctx.st.lobbyLaunchPending = false;
        {   // a launch or roster left over from an earlier lobby doesn't apply to this one
            std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
            s_lobbyEvents = PendingLobbyEvents{};
        }
        ctx.st.lastLobbyPoll = ImGui::GetTime();

        std::string pName = AuthClient::Get().IsLoggedIn() ? AuthClient::Get().GetUsername() : Settings::Get().Snapshot().playerName;
        if (pName.empty()) pName = "Guest";
        uint64_t pId = AuthClient::Get().IsLoggedIn() ? static_cast<uint64_t>(AuthClient::Get().GetUserId()) : 2ULL;
        std::string sTag = Settings::Get().Snapshot().serviceTag.empty() ? "117" : Settings::Get().Snapshot().serviceTag;

        LobbyState state = MasterServerClient::Get().JoinLobbySync(s.id, pId, pName, AuthClient::Get().IsDeveloper(), sTag, 11774);
        if (state.ok) {
            ctx.st.lobbyRoster = state.roster;
            ctx.st.lobbyMap = state.map;
            ctx.st.lobbyMode = state.mode;
            ctx.st.lobbyVariantName = state.variant;
            ctx.st.lobbyDifficulty = state.difficulty;
            ctx.st.lobbySkulls = state.skulls;
        } else {
            LobbyPlayer hostP;
            hostP.peerId = 1;
            hostP.username = s.name;
            hostP.isHost = true;
            hostP.ping = s.ping >= 0 ? s.ping : 25;
            ctx.st.lobbyRoster.push_back(hostP);

            LobbyPlayer me;
            me.peerId = pId;
            me.username = pName;
            me.serviceTag = sTag;
            me.isHost = false;
            me.isDev = AuthClient::Get().IsDeveloper();
            me.ping = s.ping >= 0 ? s.ping : 25;
            ctx.st.lobbyRoster.push_back(me);
        }

        ctx.st.screen = Screen::PreGameLobby;
    });

    // Real-time WebSocket Lobby Callbacks
    // These run on the WebSocket relay thread: they only queue the event (see PendingLobbyEvents).
    NetworkTransport::Get().SetLobbyLaunchCallback([](const std::string& lMap, const std::string& lMode,
                                                      const std::string& lVar, int lDiff, uint64_t lSkulls,
                                                      const std::string& hIp, uint16_t hPort, const std::string& sId) {
        NOBLE_INFO("FrontEndMenu: Received WebSocket real-time lobby launch event! (map: {}, mode: {})", lMap, lMode);
        std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
        if (s_lobbyEvents.launch) return;   // the host's launch can arrive twice
        s_lobbyEvents.launch = true;
        s_lobbyEvents.map = lMap;
        s_lobbyEvents.mode = lMode;
        s_lobbyEvents.variant = lVar;
        s_lobbyEvents.difficulty = lDiff;
        s_lobbyEvents.skulls = lSkulls;
        if (!hIp.empty()) s_lobbyEvents.hostEndpoint = hIp + ":" + std::to_string(hPort);
        s_lobbyEvents.sessionId = sId;
    });

    NetworkTransport::Get().SetLobbyRosterCallback([](const std::string& rosterJson) {
        try {
            auto j = nlohmann::json::parse(rosterJson);
            if (j.contains("roster") && j["roster"].is_array()) {
                std::vector<LobbyPlayer> updated;
                for (const auto& item : j["roster"]) {
                    LobbyPlayer p;
                    p.peerId = item.value("peerId", 1ULL);
                    p.machineId = item.value("machineId", "");
                    p.username = item.value("username", "Spartan");
                    p.serviceTag = item.value("serviceTag", "117");
                    p.isHost = item.value("isHost", false);
                    p.isDev = item.value("isDev", false);
                    p.ping = item.value("ping", 0);
                    updated.push_back(p);
                }
                std::lock_guard<std::mutex> lock(s_lobbyEventMutex);
                s_lobbyEvents.roster = true;
                s_lobbyEvents.rosterPlayers = std::move(updated);
            }
        } catch (...) {}
    });

    UpdateSceneForScreen(ctx.st.screen);

    NOBLE_INFO("FrontEndMenu: initialized in-engine 3D menu overlay ({} game variants, {} Firefight variants)",
               ctx.variants.size(), ctx.ffVariants.size());
}

void FrontEndMenu::RenderOverlay(IDXGISwapChain* swapChain) {
    if (!swapChain) return;
    s_overlaySwapChain.store(swapChain);
    s_lastEngineOverlayFrame.store(GetTickCount64());
    std::lock_guard<std::mutex> lock(s_overlayMutex);
    RenderOverlayImpl(swapChain, false);
}

bool FrontEndMenu::HoldFrame() {
    OverlayContext& ctx = GetOverlayCtx();
    if (!ctx.initialized || ctx.closed || !ReachArmory::IsMenuMode()) return false;
    const ULONGLONG last = s_lastEngineOverlayFrame.load();
    IDXGISwapChain* swapChain = s_overlaySwapChain.load();
    if (!swapChain || last == 0 || GetTickCount64() - last < 1000) return false;
    static bool s_logged = false;
    if (!s_logged) {
        s_logged = true;
        NOBLE_INFO("FrontEndMenu: engine stopped drawing the menu; Noble draws it itself");
    }
    {
        std::lock_guard<std::mutex> lock(s_overlayMutex);
        RenderOverlayImpl(swapChain, true);
        FrameCapture::OnFrame(nullptr, 0);   // keeps -quit-after working in test runs
    }
    swapChain->Present(1, 0);
    return true;
}

void FrontEndMenu::RenderOverlayImpl(IDXGISwapChain* swapChain, bool clear) {
    OverlayContext& ctx = GetOverlayCtx();
    if (!ctx.initialized || ctx.closed || !swapChain || !ImGui::GetCurrentContext()) return;

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

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.DisplaySize = ImVec2(static_cast<float>(bbDesc.Width), static_cast<float>(bbDesc.Height));

        static LARGE_INTEGER s_freq{}, s_last{};
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        if (s_freq.QuadPart == 0) QueryPerformanceFrequency(&s_freq);
        float dt = s_last.QuadPart ? static_cast<float>(now.QuadPart - s_last.QuadPart) / static_cast<float>(s_freq.QuadPart) : 1.0f / 60.0f;
        if (dt <= 0.0f || dt > 0.25f) dt = 1.0f / 60.0f;
        s_last = now;
        io.DeltaTime = dt;

        FeedEngineThreadInputs(hwnd, io);
        if (clear) {
            // No engine frame underneath: start from the menu's dark backdrop colour.
            const float black[4] = { 0.02f, 0.03f, 0.05f, 1.0f };
            context->ClearRenderTargetView(rtv, black);
        }

        ImGui_ImplDX11_NewFrame();
        ImGui::NewFrame();

        FrontEndChoice choice;
        bool done = false;
        DrawFrontEndContents(ctx, device, context, choice, done);

        ImGui::Render();

        ID3D11RenderTargetView* oldRtv = nullptr;
        ID3D11DepthStencilView* oldDsv = nullptr;
        context->OMGetRenderTargets(1, &oldRtv, &oldDsv);
        context->OMSetRenderTargets(1, &rtv, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        context->OMSetRenderTargets(1, &oldRtv, oldDsv);
        if (oldRtv) oldRtv->Release();
        if (oldDsv) oldDsv->Release();

        ++ctx.totalFrames;
        if (ReachArmory::IsSceneReady()) {
            ++ctx.sceneReadyFrames;
        }

        if (ctx.captureFrame > 0) {
            const bool readyToCapture = (ctx.sceneReadyFrames >= ctx.captureFrame) || (ctx.totalFrames >= ctx.captureFrame + 480);
            if (readyToCapture) {
                FrameCapture::SaveNow(swapChain, ctx.totalFrames);
                NOBLE_INFO("FrontEndMenu: -menu-capture done at frame {} (sceneReadyFrames={}), closing",
                           ctx.totalFrames, ctx.sceneReadyFrames);
                ctx.closed = true;
                ReachAudio::StopMusic();
                PostMessageW(hwnd, WM_CLOSE, 0, 0);
            }
        }

        if (done && !ctx.closed) {
            ctx.closed = true;
            ReachAudio::StopMusic();
            if (choice.play) {
                std::wstring args = L"-map \"" + ToWide(choice.map) + L"\" -mode \"" + ToWide(choice.mode) + L"\"";
                args += L" -difficulty " + std::to_wstring(choice.difficulty);
                if (!choice.variantPath.empty()) args += L" -variant \"" + ToWide(choice.variantPath) + L"\"";
                if (choice.skulls != 0) args += L" -skulls " + std::to_wstring(choice.skulls);
                if (!choice.modFolder.empty()) args += L" -mod \"" + choice.modFolder.wstring() + L"\"";
                if (choice.host) {
                    args += L" -host";
                    if (choice.hostOnline) args += L" -online";
                    else args += L" -lan";
                    if (choice.hostPublic) args += L" -public";
                    else args += L" -private";
                    args += L" -max-players " + std::to_wstring(choice.hostMaxPlayers);
                    if (!choice.sessionId.empty()) {
                        args += L" -session-id \"" + ToWide(choice.sessionId) + L"\"";
                        if (!choice.sessionToken.empty()) {
                            args += L" -session-token \"" + ToWide(choice.sessionToken) + L"\"";
                        }
                    }
                } else if (!choice.connectAddress.empty()) {
                    // A guest launched from the in-engine menu (pre-game lobby): the match process joins the host.
                    // Without this the relaunch started an unconnected local match (main.cpp's in-process path for
                    // the Noble-drawn menu reads choice.connectAddress itself).
                    args += L" -connect \"" + ToWide(choice.connectAddress) + L"\"";
                    if (!choice.sessionId.empty()) args += L" -session-id \"" + ToWide(choice.sessionId) + L"\"";
                }
                NOBLE_INFO("FrontEndMenu: launching match via Relaunch ({} {})", choice.mode, choice.map);
                ShowWindow(hwnd, SW_HIDE); UpdateWindow(hwnd);
                Relaunch::Start(args);
            } else {
                NOBLE_INFO("FrontEndMenu: quit from menu");
            }
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
        }
    }

    if (rtv) rtv->Release();
    if (backBuffer) backBuffer->Release();
    if (context) context->Release();
    device->Release();
}

FrontEndChoice FrontEndMenu::Run(Window& window, D3D11Renderer& renderer, const std::filesystem::path& reachDir) {
    Initialize(reachDir);
    OverlayContext& ctx = GetOverlayCtx();
    FrontEndChoice choice;
    bool done = false;

    window.SetCustomWndProc([](HWND h, UINT m, WPARAM w, LPARAM l) { return UIManager::HandleWndProc(h, m, w, l); });

    UIManager::GetServerBrowser().SetDirectJoinHandler([&choice, &done](const std::string& map, const std::string& mode, const std::string& addr, const std::string& sessId) {
        choice.play = true;
        choice.host = false;
        choice.map = map;
        choice.mode = mode;
        choice.connectAddress = addr;
        choice.sessionId = sessId;
        done = true;
    });

    while (!done && window.ProcessMessages()) {
        renderer.BeginFrame();
        UIManager::BeginFrame();
        ++ctx.totalFrames;

        DrawFrontEndContents(ctx, renderer.GetDevice(), renderer.GetContext(), choice, done);

        UIManager::EndFrame();
        if (ctx.captureFrame && ctx.totalFrames == ctx.captureFrame) {
            FrameCapture::SaveNow(renderer.GetSwapChain(), ctx.totalFrames);
            choice.play = false;
            done = true;
        }
        renderer.EndFrame(true);
    }

    ReachAudio::StopMusic();
    UIManager::GetServerBrowser().SetDirectJoinHandler(nullptr);
    UIManager::GetServerBrowser().SetJoinLobbyHandler(nullptr);
    window.SetCustomWndProc(nullptr);
    return choice;
}

} // namespace noble
