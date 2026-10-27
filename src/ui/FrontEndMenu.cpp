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

std::vector<MenuVariant> ScanVariants(const std::filesystem::path& reachDir) {
    std::vector<MenuVariant> list;
    std::error_code ec;
    const auto dir = reachDir / "game_variants";
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec) || entry.path().extension() != L".bin") continue;
        std::wstring stem = entry.path().stem().wstring();
        std::wstring lower = stem;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
        if (lower.rfind(L"hr_ff_", 0) == 0) continue; // Firefight variants need Firefight maps
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
    const std::vector<MenuVariant> variants = ScanVariants(reachDir);
    int selectedMap = kDefaultMap;
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
                    ImGui::TextDisabled("Coming soon: campaign launch blocks are not worked out yet.");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("FIREFIGHT")) {
                    ImGui::TextDisabled("Coming soon: firefight launch blocks are not worked out yet.");
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
        NOBLE_INFO("FrontEndMenu: PLAY {} with '{}'", choice.map, choice.variantPath);
    } else {
        NOBLE_INFO("FrontEndMenu: quit from menu");
    }
    return choice;
}

} // namespace noble
