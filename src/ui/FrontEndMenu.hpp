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
    bool host = false;           // true: host a multiplayer/co-op lobby session
    bool hostOnline = true;      // true: UPnP online P2P; false: Local LAN
    bool hostPublic = true;      // true: listed on master server; false: private
    int hostMaxPlayers = 16;     // max players (4 for co-op, 16 for custom/forge)
    int expectedPeers = 0;       // guests currently in the lobby; host waits for them before simulation
    std::string sessionId;
    std::string sessionToken;
    std::string connectAddress;  // remote host endpoint for guest join (e.g. "127.0.0.1:11774")
    std::string map;             // scenario name, e.g. "20_sword_slayer"
    std::string variantPath;     // full path to a game variant .bin from MCC's game_variants
    std::string mode = "multiplayer"; // "multiplayer", "campaign" or "firefight"
    int difficulty = 1;          // campaign/firefight: 0 easy, 1 normal, 2 heroic, 3 legendary
    uint64_t skulls = 0;         // campaign: MCC skull bits (launch block +0x48)
    std::filesystem::path modFolder; // mod chosen for this game (VFS::SetActiveMod), empty = none
    std::string modTitle;
    std::filesystem::path mapOverride; // modded map served in place of the stock map in `map`
    std::string mapTitle;              // mod map's title (loading screen, post-game)
    std::filesystem::path mapImage;    // mod map's loading-screen picture
    std::filesystem::path mapThumb;    // mod map's preview picture
    std::filesystem::path mapVariant;  // Forge map (.mvar) laid over `map`, empty for the plain map
};

class FrontEndMenu {
public:
    // True when the command line asks for the menu: no -map, or -map/-mode mainmenu/menu.
    static bool ShouldShow();

    // The one-line blurb the menu shows for a stock map or mission (e.g. "m20"); empty when it has none.
    static std::string StockBlurb(const std::string& scenario);

    // Initializes the front-end menu state for in-engine overlay rendering over mainmenu_all.map.
    static void Initialize(const std::filesystem::path& reachDir);

    // Renders the front-end menu overlay on the engine thread (HostSlot1_FrameTick) directly over
    // the live haloreach.dll 3D mainmenu_all.map scene and Armory Spartan/Elite.
    static void RenderOverlay(IDXGISwapChain* swapChain);

    // Main thread, from the message loop: once the engine has stopped calling slot 1 for a second
    // (MCC has no Reach menu map, so the engine drops it after about a minute), draws and presents
    // the menu itself. Returns true when it drew a frame.
    static bool HoldFrame();

    // Standalone loop fallback.
    static FrontEndChoice Run(Window& window, D3D11Renderer& renderer, const std::filesystem::path& reachDir);

    // True while a child match is running inside the container window.
    static bool IsMatchRunning();
    static void SetMatchRunning(bool running);
    static void CheckMatchStatus();

private:
    static void RenderOverlayImpl(IDXGISwapChain* swapChain, bool clear);
};

} // namespace noble
