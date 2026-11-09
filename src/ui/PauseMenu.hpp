#pragma once

#include <windows.h>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

struct IDXGISwapChain;

namespace noble {

// Noble's in-game overlay, drawn with ImGui on top of the game. haloreach.dll has no pause menu,
// loading screen or post-game screen of its own outside MCC, so Noble draws:
//   - the pause menu (Esc): Resume, Quit to menu, Quit;
//   - a loading screen while the engine is still launching the map (otherwise the screen is black);
//   - a post-game panel when the engine reports the match is over (host slot 6): Play again,
//     Return to menu, Quit.
//
// Threading: everything here runs on the engine thread, except HoldFrame (main thread, only once the
// engine has stopped drawing; both paths share one lock so they never overlap). ReachHost calls Render() from host slot 1
// (just before the engine presents), Toggle()/IsOpen() from the slot 36 input poll and
// ShowGameOver() from slot 6. The engine owns the D3D11 immediate context on that thread, so
// drawing anywhere else is not allowed.
class PauseMenu {
public:
    static bool IsOpen();
    static void Toggle();
    static void Close();

    // Post-game panel. Once shown it stays up (the engine has nothing more to play).
    static void ShowGameOver();
    // In-match scoreboard overlay (drawn while the scoreboard key is held).
    struct ScoreboardEntry {
        std::string name;
        std::string tag;
        int rank = -1;
        int emblem = 0;
        int team = 0;           // 0 = no team, 1 = red, 2 = blue (as LobbyPlayer::team)
        int score = 0;
        int kills = 0;
        int deaths = 0;
        int assists = 0;
        int ping = -1;          // ms, -1 = not measured (drawn as "-")
        uint64_t machine = 0;   // key for the transport's live round-trip time
        bool hostPeer = false;
        bool local = false;
        bool muted = false;
    };
    static void SetScoreboardRoster(std::vector<ScoreboardEntry> entries, const std::string& mapName, const std::string& modeName);
    static void SetScoreboardSession(const std::string& sessionId);   // goes with a player report
    // A click while the scoreboard key is held: frees the pointer so the panel's player actions can be used. Esc, the
    // scoreboard key or CLOSE give the pointer back to the game. Slot 36 input poll.
    static void EnterScoreboardPointer();

    static bool IsGameOver();

    // Map shown on the loading screen (internal name, e.g. "20_sword_slayer"). Main thread, before launch.
    static void SetLoadingMap(const std::string& scenario);
    // A Workshop/mod map: its own title and pictures instead of the stock carrier's name and art.
    static void SetLoadingMapInfo(const std::string& title, const std::filesystem::path& image,
                                  const std::filesystem::path& thumbnail);

    // True while the overlay wants the mouse pointer (pause menu, post-game panel or scoreboard actions).
    static bool WantsPointer();

    // Draws whatever is visible into the swap chain's back buffer. `loading` = the engine hasn't
    // finished launching yet. Engine thread only.
    static void Render(IDXGISwapChain* swapChain, bool loading, const std::function<void()>& extraOverlay = {});

    // Main thread, called every pass of the message loop. After a match the engine tries to go to its
    // main menu map, which MCC doesn't ship, and stops drawing (no more slot-1 frames) about 7 s after
    // the results. From then on Noble draws the post-game panel itself and presents the swap chain,
    // as the engine does (main loop: loadscreen::hold_frame + Present(1, 0)). Returns true when it drew.
    static bool HoldFrame();

private:
    static void RenderImpl(IDXGISwapChain* swapChain, bool loading, bool clear, const std::function<void()>& extraOverlay);
    static void DrawMenu(HWND hwnd);
    static void DrawGameOver(HWND hwnd);
    static void DrawLoading();
};

} // namespace noble
