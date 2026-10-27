#pragma once

#include <windows.h>
#include <string>

struct IDXGISwapChain;

namespace noble {

// Noble's in-game overlay, drawn with ImGui on top of the game. haloreach.dll has no pause menu,
// loading screen or post-game screen of its own outside MCC, so Noble draws:
//   - the pause menu (Esc): Resume, Quit to menu, Quit;
//   - a loading screen while the engine is still launching the map (otherwise the screen is black);
//   - a post-game panel when the engine reports the match is over (host slot 6): Play again,
//     Return to menu, Quit.
//
// Threading: everything here runs on the engine thread. ReachHost calls Render() from host slot 1
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
    static bool IsGameOver();

    // Map shown on the loading screen (internal name, e.g. "20_sword_slayer"). Main thread, before launch.
    static void SetLoadingMap(const std::string& scenario);

    // True while the overlay wants the mouse pointer (pause menu or post-game panel).
    static bool WantsPointer();

    // Draws whatever is visible into the swap chain's back buffer. `loading` = the engine hasn't
    // finished launching yet. Engine thread only.
    static void Render(IDXGISwapChain* swapChain, bool loading);

private:
    static void DrawMenu(HWND hwnd);
    static void DrawGameOver(HWND hwnd);
    static void DrawLoading();
};

} // namespace noble
