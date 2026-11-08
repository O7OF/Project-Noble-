#pragma once

#include <windows.h>
#include <functional>
#include <string>
#include <vector>

struct IDXGISwapChain;

namespace noble {

// Client-side Forge object palette. Like the classic Forge palette, this is drawn by Noble
// instead of the engine's 16-slot map-variant palette: tabs and items live in data\forge_palette.json,
// so adding an item never requires rebuilding a map. Placement is routed through ForgePatches.
class ForgeMenu {
public:
    struct Item {
        std::string name;
        std::string tag;          // tag path, e.g. objects\vehicles\human\pelican\pelican
        std::string description;
        int tagIndex = -1;       // map tag index for the engine create call
        int maxAllowed = 16;
        float price = 0.0f;
    };
    struct Tab {
        std::string name;
        std::vector<Item> items;
    };

    // Loads data\forge_palette.json once. Called from main after the data dir is known.
    static void Initialize(const std::wstring& dataDir);

    static bool IsOpen();
    static void Toggle();
    static void Close();
    static bool WantsPointer();

    // Queues the selected item for placement. ReachHost installs this once so the UI does not
    // depend on engine internals.
    using PlaceHandler = std::function<bool(const Item&)>;
    static void SetPlaceHandler(PlaceHandler handler);

    // Engine thread, before PauseMenu::Render: opens the demo palette once the map is ready.
    static void Tick(bool loading);
    // Drawn inside PauseMenu's ImGui frame (PauseMenu::Render extraOverlay).
    static void DrawOverlay();

private:
    static void Draw();
    static void DrawPromptBadge();
    static bool CanOpen();
    static void AddFallbackTabs();
    static void ResetSelection();
    static bool s_open;
    static bool s_demo;
    static bool s_initialized;
    static bool s_demoOpened;
    static int s_tab;
    static int s_item;
    static std::vector<Tab> s_tabs;
    static std::string s_status;
    static unsigned long long s_statusAt;
    static PlaceHandler s_place;
};

} // namespace noble
