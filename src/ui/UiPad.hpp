#pragma once

struct ImGuiIO;

namespace noble {

// The one path from a controller to ImGui, for the front-end menu and the pause menu alike. Call before ImGui::NewFrame.
// ImGui's Win32 backend gamepad code is compiled out (see CMakeLists.txt): it only reads XInput slot 0 and only runs in
// the front-end, because the pause menu has no backend NewFrame.
//
// Test option "-pad-script a,down,right,y": presses those inputs one at a time, without a controller. A step is 500 ms and
// `wait` skips one. Inputs: a b x y lb rb lt rt up down left right lsup lsdown lsleft lsright start back.
namespace UiPad {

void Feed(ImGuiIO& io);

// The same questions for every custom-drawn list, so D-pad and left stick always agree. They repeat while held.
bool Up();
bool Down();
bool Left();
bool Right();
bool PageUp();     // LT
bool PageDown();   // RT

// Scrolls the current window: right stick always; D-pad and left stick with `arrows` (a page with nothing to select);
// the triggers by a page with `triggers`. Call between Begin and End of the window that scrolls.
void ScrollWindow(bool arrows, bool triggers);

} // namespace UiPad
} // namespace noble
