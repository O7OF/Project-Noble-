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

} // namespace UiPad
} // namespace noble
