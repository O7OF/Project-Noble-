#include "FrontEndMenu.hpp"
#include "UIManager.hpp"
#include "common/Logger.hpp"
#include "common/Version.hpp"
#include "host/Window.hpp"
#include "host/D3D11Renderer.hpp"

#include <windows.h>
#include <shellapi.h>
#include <imgui.h>
#include <algorithm>
#include <cwctype>
#include <vector>

namespace noble {

namespace {

struct MenuMap {
    const char* scenario;   // passed to BlamEngine::LaunchMap
    const char* name;       // shown in the list
};

// Same maps as BlamEngine's multiplayer table (launch indices 190-209).
const MenuMap kMaps[] = {
    { "50_panopticon",     "Boardwalk" },
    { "70_boneyard",       "Boneyard" },
    { "45_launch_station", "Countdown" },
    { "30_settlement",     "Powerhouse" },
    { "52_ivory_tower",    "Reflection" },
    { "35_island",         "Spire" },
    { "20_sword_slayer",   "Sword Base" },
    { "45_aftship",        "Zealot" },
    { "dlc_slayer",        "Anchor 9" },
    { "dlc_invasion",      "Breakpoint" },
    { "dlc_medium",        "Tempest" },
    { "condemned",         "Condemned" },
    { "trainingpreserve",  "Highlands" },
    { "cex_beaver_creek",  "Battle Canyon" },
    { "cex_damnation",     "Penance" },
    { "cex_timberland",    "Ridgeline" },
    { "cex_prisoner",      "Solitary" },
    { "cex_hangemhigh",    "High Noon" },
    { "cex_headlong",      "Breakneck" },
    { "forge_halo",        "Forge World" },
};
constexpr int kDefaultMap = 6; // Sword Base: the map tested most

// Campaign missions (launch index 178-189; 188 / m70_a is the post-credits scene and is left out).
const MenuMap kMissions[] = {
    { "m05",       "Noble Actual" },
    { "m10",       "Winter Contingency" },
    { "m20",       "ONI: Sword Base" },
    { "m30",       "Nightfall" },
    { "m35",       "Tip of the Spear" },
    { "m45",       "Long Night of Solace" },
    { "m50",       "Exodus" },
    { "m52",       "New Alexandria" },
    { "m60",       "The Package" },
    { "m70",       "The Pillar of Autumn" },
    { "m70_bonus", "Lone Wolf" },
    { "m70_a",     "Epilogue" },
};
const char* const kDifficulties[] = { "Easy", "Normal", "Heroic", "Legendary" };

// Firefight maps (launch index 210-219, as BlamEngine::ResolveMapId maps them).
const MenuMap kFirefightMaps[] = {
    { "ff10_prototype",  "Beachhead" },
    { "ff45_corvette",   "Corvette" },
    { "ff20_courtyard",  "Courtyard" },
    { "ff60_icecave",    "Glacier" },
    { "ff70_holdout",    "Holdout" },
    { "ff60_airview",    "Outpost" },
    { "ff50_park",       "Overlook" },
    { "ff30_waterfront", "Waterfront" },
    { "ff_unearthed",    "Unearthed" },
    { "cex_ff_halo",     "Installation 04" },
};

struct MenuVariant {
    std::string label;
    std::filesystem::path path;
};

std::string PrettyVariantName(std::wstring stem) {
    // "slayer_pro_054" -> "slayer pro"
    const std::wstring suffix = L"_054";
    if (stem.size() > suffix.size() && stem.compare(stem.size() - suffix.size(), suffix.size(), suffix) == 0) {
        stem.erase(stem.size() - suffix.size());
    }
    std::string out;
    for (wchar_t c : stem) {
        if (c == L'_') out.push_back(' ');
        else out.push_back(c < 0x80 ? static_cast<char>(c) : '?');
    }
    return out;
}

std::vector<MenuVariant> ScanVariants(const std::filesystem::path& reachDir, bool firefight) {
    std::vector<MenuVariant> list;
    std::error_code ec;
    const auto dir = reachDir / "game_variants";
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec) || entry.path().extension() != L".bin") continue;
        std::wstring stem = entry.path().stem().wstring();
        std::wstring lower = stem;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
        const bool isFirefightVariant = lower.rfind(L"hr_ff_", 0) == 0; // only valid on Firefight maps
        if (isFirefightVariant != firefight) continue;
        if (firefight) stem = stem.substr(6); // "hr_ff_standard_1set" -> "standard_1set"
        list.push_back({ PrettyVariantName(stem), entry.path() });
    }
    std::sort(list.begin(), list.end(), [](const MenuVariant& a, const MenuVariant& b) { return a.label < b.label; });
    return list;
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
            std::wstring value = argv[i + 1];
            std::transform(value.begin(), value.end(), value.begin(), [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
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
    int selectedMap = kDefaultMap;
    int selectedMission = 1; // Winter Contingency
    bool modsScanned = false;
    int selectedFirefightMap = 0;
    int selectedFirefightVariant = 0;
    for (size_t i = 0; i < ffVariants.size(); ++i) {
        if (ffVariants[i].path.filename() == L"hr_ff_standard_1set.bin") selectedFirefightVariant = static_cast<int>(i);
    }
    int selectedDifficulty = 1;
    int selectedVariant = 0;
    for (size_t i = 0; i < variants.size(); ++i) {
        if (variants[i].path.filename() == L"slayer_054.bin") selectedVariant = static_cast<int>(i);
    }
    NOBLE_INFO("FrontEndMenu: showing menu ({} game variants found)", variants.size());

    // Route window messages to ImGui only while the menu is up (main thread owns ImGui here).
    window.SetCustomWndProc([](HWND h, UINT m, WPARAM w, LPARAM l) { return UIManager::HandleWndProc(h, m, w, l); });

    bool done = false;
    while (!done && window.ProcessMessages()) {
        renderer.BeginFrame();
        UIManager::BeginFrame();

        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
        if (ImGui::Begin("##NobleFrontEnd", nullptr, flags)) {
            ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "PROJECT NOBLE // HALO: REACH");
            ImGui::SameLine();
            ImGui::TextDisabled("v%s", VERSION_STRING.data());
            ImGui::Separator();

            if (ImGui::BeginTabBar("##modes")) {
                if (ImGui::BeginTabItem("MULTIPLAYER")) {
                    const float listHeight = ImGui::GetContentRegionAvail().y - 60.0f;
                    const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

                    ImGui::BeginChild("##maps", ImVec2(half, listHeight), ImGuiChildFlags_Borders);
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "MAP");
                    ImGui::Separator();
                    for (int i = 0; i < static_cast<int>(sizeof(kMaps) / sizeof(kMaps[0])); ++i) {
                        if (ImGui::Selectable(kMaps[i].name, selectedMap == i)) selectedMap = i;
                    }
                    ImGui::EndChild();
                    ImGui::SameLine();

                    ImGui::BeginChild("##variants", ImVec2(0, listHeight), ImGuiChildFlags_Borders);
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "GAME TYPE");
                    ImGui::Separator();
                    if (variants.empty()) {
                        ImGui::TextDisabled("No game variants found in MCC's haloreach\\game_variants.");
                    }
                    for (int i = 0; i < static_cast<int>(variants.size()); ++i) {
                        ImGui::PushID(i);
                        if (ImGui::Selectable(variants[i].label.c_str(), selectedVariant == i)) selectedVariant = i;
                        ImGui::PopID();
                    }
                    ImGui::EndChild();

                    ImGui::Spacing();
                    if (ImGui::Button("PLAY", ImVec2(200.0f, 40.0f))) {
                        choice.play = true;
                        choice.map = kMaps[selectedMap].scenario;
                        if (!variants.empty()) choice.variantPath = variants[selectedVariant].path.string();
                        done = true;
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s  -  %s", kMaps[selectedMap].name,
                                        variants.empty() ? "default" : variants[selectedVariant].label.c_str());
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("CAMPAIGN")) {
                    const float listHeight = ImGui::GetContentRegionAvail().y - 60.0f;
                    const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

                    ImGui::BeginChild("##missions", ImVec2(half, listHeight), ImGuiChildFlags_Borders);
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "MISSION");
                    ImGui::Separator();
                    for (int i = 0; i < static_cast<int>(sizeof(kMissions) / sizeof(kMissions[0])); ++i) {
                        if (ImGui::Selectable(kMissions[i].name, selectedMission == i)) selectedMission = i;
                    }
                    ImGui::EndChild();
                    ImGui::SameLine();

                    ImGui::BeginChild("##difficulty", ImVec2(0, listHeight), ImGuiChildFlags_Borders);
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "DIFFICULTY");
                    ImGui::Separator();
                    for (int i = 0; i < 4; ++i) {
                        if (ImGui::Selectable(kDifficulties[i], selectedDifficulty == i)) selectedDifficulty = i;
                    }
                    ImGui::EndChild();

                    ImGui::Spacing();
                    if (ImGui::Button("PLAY##campaign", ImVec2(200.0f, 40.0f))) {
                        choice.play = true;
                        choice.mode = "campaign";
                        choice.map = kMissions[selectedMission].scenario;
                        choice.difficulty = selectedDifficulty;
                        done = true;
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s  -  %s", kMissions[selectedMission].name, kDifficulties[selectedDifficulty]);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("FIREFIGHT")) {
                    const float listHeight = ImGui::GetContentRegionAvail().y - 60.0f;
                    const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

                    const float third = (ImGui::GetContentRegionAvail().x - 2.0f * ImGui::GetStyle().ItemSpacing.x) / 3.0f;
                    (void)half;
                    ImGui::BeginChild("##ffmaps", ImVec2(third, listHeight), ImGuiChildFlags_Borders);
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "MAP");
                    ImGui::Separator();
                    for (int i = 0; i < static_cast<int>(sizeof(kFirefightMaps) / sizeof(kFirefightMaps[0])); ++i) {
                        if (ImGui::Selectable(kFirefightMaps[i].name, selectedFirefightMap == i)) selectedFirefightMap = i;
                    }
                    ImGui::EndChild();
                    ImGui::SameLine();

                    ImGui::BeginChild("##ffvariants", ImVec2(third, listHeight), ImGuiChildFlags_Borders);
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "GAME TYPE");
                    ImGui::Separator();
                    if (ffVariants.empty()) {
                        ImGui::TextDisabled("No hr_ff_*.bin variants in MCC's game_variants.");
                    }
                    for (int i = 0; i < static_cast<int>(ffVariants.size()); ++i) {
                        ImGui::PushID(200 + i);
                        if (ImGui::Selectable(ffVariants[i].label.c_str(), selectedFirefightVariant == i)) selectedFirefightVariant = i;
                        ImGui::PopID();
                    }
                    ImGui::EndChild();
                    ImGui::SameLine();

                    ImGui::BeginChild("##ffdifficulty", ImVec2(0, listHeight), ImGuiChildFlags_Borders);
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "DIFFICULTY");
                    ImGui::Separator();
                    for (int i = 0; i < 4; ++i) {
                        ImGui::PushID(100 + i);
                        if (ImGui::Selectable(kDifficulties[i], selectedDifficulty == i)) selectedDifficulty = i;
                        ImGui::PopID();
                    }
                    ImGui::EndChild();

                    ImGui::Spacing();
                    if (ImGui::Button("PLAY##firefight", ImVec2(200.0f, 40.0f))) {
                        choice.play = true;
                        choice.mode = "firefight";
                        choice.map = kFirefightMaps[selectedFirefightMap].scenario;
                        choice.difficulty = selectedDifficulty;
                        if (!ffVariants.empty()) choice.variantPath = ffVariants[selectedFirefightVariant].path.string();
                        done = true;
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s  -  %s  -  %s", kFirefightMaps[selectedFirefightMap].name,
                                        ffVariants.empty() ? "standard" : ffVariants[selectedFirefightVariant].label.c_str(),
                                        kDifficulties[selectedDifficulty]);
                    ImGui::EndTabItem();
                }
                // The older Noble screens (still in src/ui), shown here as tabs. Their back ends
                // (P2P servers, armour, mod loading) are not wired into the engine yet.
                if (ImGui::BeginTabItem("SERVERS")) {
                    UIManager::GetServerBrowser().Render();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("ARMOR LOCKER")) {
                    UIManager::GetArmorCustomizer().Render(renderer.GetDevice(), renderer.GetContext());
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("WORKSHOP MODS")) {
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
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("SETTINGS")) {
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "SETTINGS");
                    ImGui::Separator();
                    ImGui::Text("Engine:     haloreach.dll from your MCC install (%s)", reachDir.string().c_str());
                    ImGui::Text("Renderer:   Direct3D 11");
                    ImGui::Text("Controls:   MCC default keyboard/mouse layout (built in)");
                    ImGui::Text("Mouse look: -mouse-scale <value> on the command line (default 0.0008)");
                    ImGui::TextDisabled("In-game settings (sensitivity, volume, key bindings) will go here.");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            // Quit, bottom-right.
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 130.0f, ImGui::GetWindowHeight() - 52.0f));
            if (ImGui::Button("QUIT", ImVec2(110.0f, 40.0f))) {
                done = true;
            }
        }
        ImGui::End();

        UIManager::EndFrame();
        renderer.EndFrame(true);
    }

    // From here on the engine thread owns rendering; the pause menu feeds ImGui itself.
    window.SetCustomWndProc(nullptr);
    if (choice.play) {
        NOBLE_INFO("FrontEndMenu: PLAY {} {} (difficulty {}, variant '{}')", choice.mode, choice.map, choice.difficulty, choice.variantPath);
    } else {
        NOBLE_INFO("FrontEndMenu: quit from menu");
    }
    return choice;
}

} // namespace noble
