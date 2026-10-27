#pragma once

#include <imgui.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct ID3D11Device;

namespace noble {

struct UiTexture {
    ImTextureID id = 0;   // ID3D11ShaderResourceView* for the ImGui DX11 backend
    int width = 0;
    int height = 0;
    explicit operator bool() const { return id != 0; }
};

// Pictures for Noble's menus and loading screens. Nothing from Halo is shipped with Noble: images
// come from, in this order,
//   1. data\ui\custom\   the player's own files (PNG/JPG/BMP), always preferred when present;
//   2. data\ui\noble\    art the Noble team ships with releases (original work);
//   3. the player's MCC install, read at run time (MCC's UI texture packs in <MCC>\Data\UI\texturepacks);
//   4. nothing: the screen draws its plain dark style.
// Folder layout for 1 and 2:  backgrounds\*.png   loading\<scenario>.png   loading\default.png
//
// Textures are created on the D3D11 device given to Init and cached for the rest of the run.
// The device is free-threaded, so these can be called from the main or the engine thread, but each
// call that loads something reads a file, so call them before a screen needs them where possible.
namespace UiArt {

void Init(ID3D11Device* device, const std::filesystem::path& mccRoot, const std::filesystem::path& exeDir);
void Shutdown();

// An image from one of MCC's UI texture packs, e.g. ("skullstexturepack", "IRON").
UiTexture PackImage(const std::string& pack, const std::string& name);

// Full-screen background for the front-end menu. Picked once per run (see SettingsData::menuBackground).
UiTexture MenuBackground();

// Full-screen picture for the loading screen of this scenario ("m10", "20_sword_slayer", ...).
UiTexture LoadingBackground(const std::string& scenario);

// Map preview (316x160, or 668x292 when large) from MCC's levels pack; empty if the map has none.
UiTexture MapPreview(const std::string& scenario, bool large);

// Skull icon (128x128) by MCC icon name ("IRON", "BLACKEYE", ...).
UiTexture SkullIcon(const std::string& iconName);

// Controller picture for the controls settings page.
UiTexture ControllerImage();

// Draws `tex` to cover the rectangle (keeps the aspect ratio, crops the overflow).
void DrawCover(ImDrawList* dl, const UiTexture& tex, ImVec2 min, ImVec2 max, ImU32 tint = IM_COL32_WHITE);

} // namespace UiArt
} // namespace noble
