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

// Pictures for Noble's menus and loading screens. Images come from, in this order,
//   1. data\ui\custom\   the player's own files (PNG/JPG/BMP), always preferred when present;
//   2. data\ui\noble\    art the Noble team ships with releases (Noble's own art, plus the layered Reach menu
//                         artwork in screens\<name>\, exported from Bungie's HREK menu bitmaps; Tyler 2026-11-11);
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

// Main-menu slideshow: deals the next picture from the same shuffled deck and makes it the menu background.
// Only loads on the thread that called WarmMenuBackground (the device owner); anywhere else, or when the player
// picked one fixed picture, or when there is nothing else to show, it returns the current background unchanged.
UiTexture MenuBackgroundNext();

// Fixed background for one front-end screen: data\ui\custom\screens\<name>.(png|jpg|jpeg|bmp) first, then
// data\ui\noble\screens\<name>.*; empty when there is none (the screen then uses MenuBackground()).
// Names used by the menu: "firefight", "custom_games", "campaign", "forge". Loaded by WarmMenuBackground.
UiTexture ScreenBackground(const std::string& name);

// Layered (parallax) artwork for one screen, the way Bungie built the Reach menu backgrounds: several transparent
// layers on a 1920x1080 reference canvas, each moving by its own depth. Read from screens\<name>\layers.json in
// data\ui\custom\ then data\ui\noble\. {"width":1920,"height":1080,"layers":[{"file":"background.png","x":0,"y":0,
// "w":1920,"h":1080,"depth":0.2,"flicker":0}, ...]} (back to front; depth 0 = still, 1 = moves most; flicker 0..1 dims
// the layer in an irregular fire-light pulse).
struct LayeredLayer {
    UiTexture tex;
    float x = 0, y = 0, w = 0, h = 0;
    float depth = 0;
    float flicker = 0;
};
struct LayeredArt {
    float width = 1920.0f, height = 1080.0f;
    std::vector<LayeredLayer> layers;
    explicit operator bool() const { return !layers.empty(); }
};
// Null when the screen has no layered artwork. Loaded by WarmMenuBackground; afterwards this only reads the cache.
const LayeredArt* ScreenLayers(const std::string& name);
// Draws layered artwork to cover [min, max] (cropping like DrawCover), animated by timeSeconds and the mouse.
void DrawLayered(ImDrawList* dl, const LayeredArt& art, ImVec2 min, ImVec2 max, double timeSeconds);

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

// An image file on disk (Workshop mod art: PNG/JPG/BMP/WebP), cached by full path. Empty when the
// file is missing or cannot be decoded, so callers keep their placeholder.
UiTexture ImageFile(const std::filesystem::path& path);

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
    // Developer-entitlement plates and retired event plates. They stay in the catalog (a saved profile
    // or a remote player can still be wearing one), but the Armory only offers selectable entries.
    bool removed = false;    // never offered anywhere
    bool devOnly = false;    // offered in noble-dev only
};

// True when the Armory may offer this position in GetNameplateCatalog(): not removed, and in a
// release build not a developer-entitlement plate. Nameplate() still resolves any position.
bool NameplateSelectable(int index);

struct EmblemInfo {
    int id = 1;
    std::string name;
    std::string image1;
    std::string image2;
};

const std::vector<NameplateInfo>& GetNameplateCatalog();
const std::vector<EmblemInfo>& GetEmblemCatalog();

// Nameplate texture for a position in GetNameplateCatalog() (what the Armory stores; not the XML id).
UiTexture Nameplate(int index);

// Emblem backgrounds. Reach's own (2026-11-11): GetEmblemBackgrounds()[n - 1] is the game's background n (0 = Blank);
// MCC's playercustomization.xml list is only the fallback when the Reach art is missing.
struct EmblemBackground {
    std::string image;   // e.g. "eb_001"
    std::string name;    // e.g. "Circle"
};
const std::vector<EmblemBackground>& GetEmblemBackgrounds();
UiTexture EmblemByName(const std::string& image1);
// Alpha mask (size x size, 0..255) of a Reach emblem layer image ("reach:fg_09_p"); empty when unavailable.
std::vector<uint8_t> EmblemLayerMask(const std::string& image, int size);
// Converts an emblem saved as an MCC catalog position (appearance files from before 2026-11-11) to Reach's emblem with
// the same name: a foreground index, or for background = true a background index (0 = Blank). -1 = no Reach match.
int ReachEmblemFromMcc(int mccPosition, bool background);
// True once Init has run (the emblem / nameplate lists can be read).
bool CatalogsReady();

// Draws a player's emblem: the background (0 = none, n = GetEmblemBackgrounds()[n - 1]), then both layers of the symbol
// (symbolIndex = position in GetEmblemCatalog(), which is what the Armory stores; with Reach's emblems that is also the
// game's own foreground index).
// The three emblem layers' colours. Black in MCC's art is already transparent, so a tint colours only the shape.
struct EmblemColors {
    ImU32 primary = IM_COL32(216, 216, 216, 255);     // Reach's White
    ImU32 secondary = IM_COL32(216, 216, 216, 255);
    ImU32 background = IM_COL32(74, 116, 177, 255);   // Reach's Cobalt
};
void DrawEmblem(ImDrawList* dl, int symbolIndex, int backgroundIndex, ImVec2 min, ImVec2 max, const EmblemColors& colors = {});

// Draws `tex` inside the rectangle, keeping its aspect ratio (centred, nothing cropped).
void DrawContain(ImDrawList* dl, const UiTexture& tex, ImVec2 min, ImVec2 max, ImU32 tint = IM_COL32_WHITE);

// Draws `tex` to cover the rectangle (keeps the aspect ratio, crops the overflow).
void DrawCover(ImDrawList* dl, const UiTexture& tex, ImVec2 min, ImVec2 max, ImU32 tint = IM_COL32_WHITE);

} // namespace UiArt
} // namespace noble
