#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

struct IDXGISwapChain;

namespace noble {

class Window;
class D3D11Renderer;

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

    // Initializes the front-end menu state for in-engine overlay rendering over mainmenu_all.map.
    static void Initialize(const std::filesystem::path& reachDir);

    // Renders the front-end menu overlay on the engine thread (HostSlot1_FrameTick) directly over
    // the live haloreach.dll 3D mainmenu_all.map scene and Armory Spartan/Elite.
    static void RenderOverlay(IDXGISwapChain* swapChain);

    // Standalone loop fallback.
    static FrontEndChoice Run(Window& window, D3D11Renderer& renderer, const std::filesystem::path& reachDir);
};

} // namespace noble
