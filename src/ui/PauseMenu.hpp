#pragma once

#include <windows.h>

struct IDXGISwapChain;

namespace noble {

// In-game pause menu (Esc). haloreach.dll has no pause menu of its own outside MCC, so Noble
// draws one with ImGui on top of the game. This is a base to build on: today it has Resume and
// Quit; settings, leave-match etc. go in PauseMenu::DrawMenu().
//
// Threading: everything here runs on the engine thread. ReachHost calls Render() from host slot 1
// (just before the engine presents) and Toggle()/IsOpen() from the slot 36 input poll. The engine
// owns the D3D11 immediate context on that thread, so drawing anywhere else is not allowed.
class PauseMenu {
public:
    static bool IsOpen();
    static void Toggle();
    static void Close();

    // Draws the menu into the swap chain's back buffer if it is open. Engine thread only.
    static void Render(IDXGISwapChain* swapChain);

private:
    static void DrawMenu(HWND hwnd);
};

} // namespace noble
