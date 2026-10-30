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

// Full-screen background for the front-end menu. Picked once per run (see SettingsData::menuBackground, which can
// name one file as "file:<name>").
UiTexture MenuBackground();

// Loads and caches the menu background now. Call it once from the thread that owns the D3D device (menu init):
// the fallback path that draws the menu from the main thread when the engine stops must not create textures.
void WarmMenuBackground();

// The player's own background pictures (data\ui\custom\backgrounds), file names sorted, for the picker.
std::vector<std::string> MenuBackgroundFiles();
// Drops the cached list so a picture imported this session shows up in the picker at once.
void RefreshMenuBackgroundFiles();
// Copies an image the player picked into data\ui\custom\backgrounds and returns the stored file name.
bool ImportMenuBackgroundImage(const std::filesystem::path& source, std::string& outName);

// Full-screen picture for the loading screen of this scenario ("m10", "20_sword_slayer", ...).
UiTexture LoadingBackground(const std::string& scenario);

// Map preview (316x160, or 668x292 when large) from MCC's levels pack; empty if the map has none.
UiTexture MapPreview(const std::string& scenario, bool large);

// "What does this map look like" for menus that show an arbitrary server's map: MCC's map preview, then a
// picture the player or a mod supplied as data\ui\custom\loading\<scenario> (or data\ui\noble\loading\...).
// Empty when there is none (a modded map without a picture), so callers draw a placeholder instead of a
// picture of some other map.
UiTexture MapPhoto(const std::string& scenario);

// Skull icon (128x128) by MCC icon name ("IRON", "BLACKEYE", ...).
UiTexture SkullIcon(const std::string& iconName);

// Controller picture for the controls settings page: MCC's own picture (texture packs).
UiTexture ControllerImage();

// Noble's Xbox 360 controller picture (controller.png in data\ui\custom or data\ui\noble, front view,
// 1600x1000 layout); empty when there is none, then the page falls back to ControllerImage().
UiTexture NobleControllerImage();

// Reach art the player supplies (rank icons, difficulty icons, ...), never shipped with Noble:
// data\ui\custom\reach\<category>\ first, then data\ui\reach\<category>\ (both under the git-ignored
// data folder). Files are found by stem: ranks\07_warrant_officer.png, difficulty\heroic.png, ...
UiTexture ReachArt(const std::string& category, const std::string& stem);

// Rank icon for Career::Ranks() index 0..49: ranks\<NN>_<name>.png (any file whose name starts with
// the two-digit index). Empty when the player has none; callers then draw text only.
UiTexture RankIcon(int rankIndex);

// Difficulty icon 0 easy, 1 normal, 2 heroic, 3 legendary: difficulty\<name>.png.
UiTexture DifficultyIcon(int difficulty);

struct NameplateInfo {
    int id = 1;
    std::string name;
    std::string image;
};

struct EmblemInfo {
    int id = 1;
    std::string name;
    std::string image1;
};

const std::vector<NameplateInfo>& GetNameplateCatalog();
const std::vector<EmblemInfo>& GetEmblemCatalog();

// Nameplate texture for index 1..536
UiTexture Nameplate(int id);

// Emblem texture for index 1..330
UiTexture Emblem(int id);
UiTexture EmblemByName(const std::string& image1);

// Draws `tex` inside the rectangle, keeping its aspect ratio (centred, nothing cropped).
void DrawContain(ImDrawList* dl, const UiTexture& tex, ImVec2 min, ImVec2 max, ImU32 tint = IM_COL32_WHITE);

// Draws `tex` to cover the rectangle (keeps the aspect ratio, crops the overflow).
void DrawCover(ImDrawList* dl, const UiTexture& tex, ImVec2 min, ImVec2 max, ImU32 tint = IM_COL32_WHITE);

} // namespace UiArt
} // namespace noble
