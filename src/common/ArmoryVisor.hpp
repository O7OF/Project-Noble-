#pragma once

#include <string>
#include <string_view>

namespace noble {

// The 30 Spartan visor finishes in MCC global-item order (HR_Visor_* items 157..186). haloreach.dll+0x2EDC5E is
// patched (see EnginePatches) to translate a visor item id straight to engine internal visor index `item - 157`, so
// this array's position is also the engine's internal visor slot for that finish.
//
// IMPORTANT (2026-11-15): only the first five of those are real stock visors. haloreach.dll's pmcg visor translation
// table (0x3B2F4) only produces internal slots 0..6, and the stock game has five visor items (157..161), so the
// patched direct mapping into 5..29 is what made extended finishes render WHITE in game (Tyler's blood_orange report).
// Use ArmoryVisorStockIndex() for the profile/appearance-record path (always a valid stock slot) and the exact RGB for
// the local material patch.
inline constexpr std::string_view kArmoryVisorItemIds[30] = {
    "default",        "silver",        "blue",          "black",         "gold",
    "rose_gold",      "emerald",       "ice",           "crimson",       "onyx",
    "violet",         "amber",         "cyan",          "ruby",          "sapphire",
    "amethyst",       "bronze",        "sunfire",       "lime",          "platinum",
    "obsidian",       "pearl",         "electric_blue", "blood_orange",  "mint",
    "copper",         "midnight",      "magenta",       "aquamarine",    "titanium",
};

// Approximate sRGB colours of the 30 Armory finishes (same data as kVisorColors in ArmorCustomizer.cpp). Only used to
// pick the nearest valid stock visor slot for the engine profile/record paths; the exact colour for the local player is
// applied by the map shader patch in ReachArmory::SyncMatchLiveBipeds.
inline constexpr float kArmoryVisorRgb[30][3] = {
    {0.85f, 0.53f, 0.14f},  {0.78f, 0.83f, 0.90f},  {0.18f, 0.54f, 0.95f},  {0.11f, 0.13f, 0.16f},
    {0.98f, 0.78f, 0.18f},  {0.93f, 0.58f, 0.55f},  {0.10f, 0.65f, 0.38f},  {0.55f, 0.25f, 0.85f},
    {0.55f, 0.85f, 1.00f},  {0.85f, 0.12f, 0.18f},  {0.05f, 0.05f, 0.06f},  {0.95f, 0.48f, 0.08f},
    {0.08f, 0.85f, 0.92f},  {0.92f, 0.12f, 0.28f},  {0.10f, 0.25f, 0.92f},  {0.70f, 0.15f, 0.85f},
    {0.80f, 0.45f, 0.20f},  {1.00f, 0.35f, 0.02f},  {0.42f, 0.92f, 0.18f},  {0.92f, 0.94f, 0.98f},
    {0.08f, 0.08f, 0.10f},  {0.95f, 0.92f, 0.88f},  {0.05f, 0.80f, 1.00f},  {0.95f, 0.22f, 0.05f},
    {0.35f, 0.88f, 0.65f},  {0.85f, 0.45f, 0.25f},  {0.05f, 0.08f, 0.25f},  {0.88f, 0.10f, 0.60f},
    {0.15f, 0.85f, 0.75f},  {0.70f, 0.72f, 0.75f},
};

// Resolve the Armory UI catalog index + visor string id to the MCC item slot (0..29). The Armory UI lists the finishes
// in a slightly different order than MCC's item order (violet/ice/crimson/onyx), and legacy saves have no id.
inline int ArmoryVisorItemIndex(int uiIndex, std::string_view id) {
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
        static constexpr int kUiToItem[30] = {
            0,  1,  2,  3,  4,  5,  6,  10, 7,  8,  9,  11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        };
        itemIdx = kUiToItem[uiIndex];
    }
    return itemIdx < 0 ? 0 : itemIdx;
}

// Engine-internal visor index for the appearance record / profile item path. The engine only renders stock slots 0..4
// (Black and Gold swap because MCC's item order is Default/Silver/Blue/Black/Gold while the engine's is
// Default/Silver/Blue/Gold/Black). Extended finishes are mapped to the nearest stock slot so they never render white;
// their exact colour is shown by the local material patch.
inline int ArmoryVisorStockIndex(int uiIndex, std::string_view id) {
    const int itemIdx = ArmoryVisorItemIndex(uiIndex, id);
    if (itemIdx < 5) {
        if (itemIdx == 3) return 4; // MCC "black" item slot -> engine internal Black
        if (itemIdx == 4) return 3; // MCC "gold" item slot -> engine internal Gold
        return itemIdx;
    }
    // Nearest stock colour among Default (0), Silver (1), Blue (2), Engine Gold (3), Engine Black (4).
    static constexpr float kStock[5][3] = {
        {0.85f, 0.53f, 0.14f}, {0.78f, 0.83f, 0.90f}, {0.18f, 0.54f, 0.95f},
        {0.98f, 0.78f, 0.18f}, {0.11f, 0.13f, 0.16f},
    };
    // Engine slot order: 0 default, 1 silver, 2 blue, 3 gold, 4 black.
    int best = 0;
    float bestDist = 1e9f;
    for (int s = 0; s < 5; ++s) {
        const float dr = kArmoryVisorRgb[itemIdx][0] - kStock[s][0];
        const float dg = kArmoryVisorRgb[itemIdx][1] - kStock[s][1];
        const float db = kArmoryVisorRgb[itemIdx][2] - kStock[s][2];
        const float d = dr * dr + dg * dg + db * db;
        if (d < bestDist) {
            bestDist = d;
            best = s;
        }
    }
    return best;
}

// Kept for callers that only need the engine slot used for the profile item (same as ArmoryVisorStockIndex).
inline int ArmoryVisorEngineIndex(int uiIndex, std::string_view id) {
    return ArmoryVisorStockIndex(uiIndex, id);
}

} // namespace noble
