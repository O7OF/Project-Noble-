#pragma once

namespace noble {

// Feeds the physical controller into Dear ImGui so menus can be driven with a pad (D-pad / left stick navigation,
// A/B/X/Y, Start/Back, bumpers and triggered L2/R2). ImGui's DX11 + Win32 backends do not provide gamepad state, so
// without this every `ImGuiKey_Gamepad*` check in SettingsPanel was dead: no tab switching, no preset cycling, no
// Apply/Discard from a controller. Call once per frame, before ImGui::NewFrame().
void FeedGamepadToImGui();

} // namespace noble
