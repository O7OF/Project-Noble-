#pragma once

#include <string>
#include <string_view>

namespace noble {

// The 30 Spartan visor finishes in MCC global-item order (HR_Visor_* items 157..186). haloreach.dll+0x2EDC5E is
// patched (see EnginePatches) to translate a visor item id straight to engine internal visor index `item - 157`, so
// this array's position is also the engine's internal visor slot for that finish.
inline constexpr std::string_view kArmoryVisorItemIds[30] = {
    "default",        "silver",        "blue",          "black",         "gold",
    "rose_gold",      "emerald",       "ice",           "crimson",       "onyx",
    "violet",         "amber",         "cyan",          "ruby",          "sapphire",
    "amethyst",       "bronze",        "sunfire",       "lime",          "platinum",
    "obsidian",       "pearl",         "electric_blue", "blood_orange",  "mint",
    "copper",         "midnight",      "magenta",       "aquamarine",    "titanium",
};

// Armory UI catalog index (kVisorColors in ArmorCustomizer.cpp) + visor string id -> engine internal visor index.
//
// Resolve by string id first: the Armory UI catalog lists the finishes in a slightly different order than MCC's item
// order (the UI has violet/ice/crimson/onyx where MCC has ice/crimson/onyx/violet), so using the UI index directly made
// the game show a different finish than the one picked in the Armory (Tyler's "in game visors not matching Armory").
// The two stock finishes Black and Gold also swap slots in the engine (internal 3 = Gold, 4 = Black) because MCC's
// item order is Default/Silver/Blue/Black/Gold while the engine's internal order is Default/Silver/Blue/Gold/Black.
inline int ArmoryVisorEngineIndex(int uiIndex, std::string_view id) {
    int itemIdx = -1;
    if (!id.empty()) {
        for (int i = 0; i < 30; ++i) {
            if (kArmoryVisorItemIds[i] == id) {
                itemIdx = i;
                break;
            }
        }
    }
    if (itemIdx < 0 && uiIndex >= 0 && uiIndex < 30) {
        // Legacy save without visor_id: the Armory catalog puts violet before ice/crimson/onyx, MCC's item order does
        // not. Map the UI catalog index onto MCC's order instead of using it directly.
        static constexpr int kUiToItem[30] = {
            0,  1,  2,  3,  4,  5,  6,  10, 7,  8,  9,  11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        };
        itemIdx = kUiToItem[uiIndex];
    }
    if (itemIdx < 0) return 0;
    if (itemIdx == 3) return 4; // MCC "black" item slot -> engine internal Black
    if (itemIdx == 4) return 3; // MCC "gold" item slot -> engine internal Gold
    return itemIdx;
}

} // namespace noble
