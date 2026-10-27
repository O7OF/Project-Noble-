#pragma once

#include <filesystem>
#include <string>

namespace noble {

class Window;
class D3D11Renderer;

// Noble's own front-end menu, shown before the engine starts. MCC ships no mainmenu.map for
// Reach (MCC draws its own menus), so instead of the engine's menu Noble shows this ImGui screen,
// lets the player pick a map and game type, and then starts that match.
//
// Base to build on: multiplayer works today; Campaign and Firefight tabs are placeholders until
// their launch blocks are worked out.
struct FrontEndChoice {
    bool play = false;           // false: the player quit or closed the window
    std::string map;             // scenario name, e.g. "20_sword_slayer"
    std::string variantPath;     // full path to a game variant .bin from MCC's game_variants
};

class FrontEndMenu {
public:
    // True when the command line asks for the menu: no -map, or -map/-mode mainmenu/menu.
    static bool ShouldShow();

    // Runs the menu on the main thread until the player presses PLAY or quits.
    static FrontEndChoice Run(Window& window, D3D11Renderer& renderer, const std::filesystem::path& reachDir);
};

} // namespace noble
