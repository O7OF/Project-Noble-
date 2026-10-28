#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace noble {

class Window;
class D3D11Renderer;

// Noble's own front-end menu, shown before the engine starts. MCC ships no mainmenu.map for
// Reach (MCC draws its own menus), so instead of the engine's menu Noble shows this ImGui screen,
// lets the player pick a map and game type, and then starts that match.
//
// Base to build on: multiplayer, campaign and firefight launch from here.
struct FrontEndChoice {
    bool play = false;           // false: the player quit or closed the window
    std::string map;             // scenario name, e.g. "20_sword_slayer"
    std::string variantPath;     // full path to a game variant .bin from MCC's game_variants
    std::string mode = "multiplayer"; // "multiplayer", "campaign" or "firefight"
    int difficulty = 1;          // campaign/firefight: 0 easy, 1 normal, 2 heroic, 3 legendary
    uint64_t skulls = 0;         // campaign: MCC skull bits (launch block +0x48)
    std::filesystem::path modFolder; // mod chosen for this game (VFS::SetActiveMod), empty = none
    std::string modTitle;
};

class FrontEndMenu {
public:
    // True when the command line asks for the menu: no -map, or -map/-mode mainmenu/menu.
    static bool ShouldShow();

    // Runs the menu on the main thread until the player presses PLAY or quits.
    static FrontEndChoice Run(Window& window, D3D11Renderer& renderer, const std::filesystem::path& reachDir);
};

} // namespace noble
