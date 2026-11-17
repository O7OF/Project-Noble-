#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <algorithm>
#include <cstdio>

namespace noble {

// The 30 authentic Halo: Reach primary armour colours (kReachColors in ArmorCustomizer.cpp). Flame colour variants
// are generated from these palettes; the in-game appearance uses the custom MCC item ids below.
struct PrimaryFlameEntry {
    const char* name;  // display name (Armory)
    const char* id;    // string id suffix (flame_pri_<id>, pmcg list 80/8c item legendary_flame_pri_<id>)
    float r, g, b;
};

inline constexpr std::array<PrimaryFlameEntry, 30> kPrimaryFlames = {{
    {"STEEL",     "steel",     0.349f, 0.349f, 0.349f},
    {"SILVER",    "silver",    0.608f, 0.608f, 0.608f},
    {"WHITE",     "white",     0.847f, 0.847f, 0.847f},
    {"BROWN",     "brown",     0.427f, 0.310f, 0.220f},
    {"TAN",       "tan",       0.620f, 0.494f, 0.376f},
    {"KHAKI",     "khaki",     0.780f, 0.671f, 0.541f},
    {"SAGE",      "sage",      0.345f, 0.408f, 0.302f},
    {"OLIVE",     "olive",     0.490f, 0.569f, 0.400f},
    {"DRAB",      "drab",      0.643f, 0.725f, 0.533f},
    {"FOREST",    "forest",    0.165f, 0.369f, 0.212f},
    {"GREEN",     "green",     0.267f, 0.545f, 0.314f},
    {"SEA FOAM",  "sea_foam",  0.467f, 0.729f, 0.506f},
    {"TEAL",      "teal",      0.165f, 0.431f, 0.459f},
    {"AQUA",      "aqua",      0.278f, 0.608f, 0.635f},
    {"CYAN",      "cyan",      0.471f, 0.788f, 0.808f},
    {"BLUE",      "blue",      0.176f, 0.302f, 0.514f},
    {"COBALT",    "cobalt",    0.290f, 0.455f, 0.694f},
    {"ICE",       "ice",       0.518f, 0.678f, 0.875f},
    {"VIOLET",    "violet",    0.310f, 0.227f, 0.498f},
    {"ORCHID",    "orchid",    0.459f, 0.365f, 0.678f},
    {"LAVENDER",  "lavender",  0.651f, 0.565f, 0.855f},
    {"MAROON",    "maroon",    0.467f, 0.161f, 0.200f},
    {"BRICK",     "brick",     0.678f, 0.251f, 0.278f},
    {"ROSE",      "rose",      0.863f, 0.478f, 0.494f},
    {"RUST",      "rust",      0.557f, 0.231f, 0.133f},
    {"CORAL",     "coral",     0.784f, 0.376f, 0.239f},
    {"PEACH",     "peach",     0.941f, 0.588f, 0.451f},
    {"GOLD",      "gold",      0.639f, 0.439f, 0.141f},
    {"YELLOW",    "yellow",    0.859f, 0.651f, 0.220f},
    {"PALE",      "pale",      0.961f, 0.816f, 0.447f},
}};

// Order of the Armory effect list: 0..6 stock effects, 7 = auto match primary colour, 8..37 = primary flame variants.
inline constexpr int kArmorEffectStockCount = 7;
inline constexpr int kArmorEffectAutoIndex = 7;
inline constexpr int kArmorEffectVariantBase = 8;
inline constexpr int kArmorEffectCount = 38;
inline constexpr uint32_t kArmorEffectCustomItemBase = 0xF0;

// Custom MCC item id for a primary flame variant (translator hook maps 0xF0..0x10D to pmcg indices 7..36).
inline constexpr uint32_t ArmorFlameCustomItemId(int variantIndex) {
    return variantIndex >= 0 && variantIndex < 30 ? kArmorEffectCustomItemBase + static_cast<uint32_t>(variantIndex) : 0;
}

// Resolve an Armory effect list index to a custom item id. 0 = not a custom variant (stock effect path).
inline uint32_t ArmorEffectCustomItemId(int effectIndex, int primaryColorIndex) {
    if (effectIndex == kArmorEffectAutoIndex) return ArmorFlameCustomItemId(std::clamp(primaryColorIndex, 0, 29));
    const int variant = effectIndex - kArmorEffectVariantBase;
    return (variant >= 0 && variant < 30) ? ArmorFlameCustomItemId(variant) : 0;
}

// pmcg list item name + region/permutation used by the Armory preview and by the primary flame variants.
inline bool ArmorFlamePmcgTarget(int variantIndex, std::string& itemName, std::string& region, std::string& perm) {
    if (variantIndex < 0 || variantIndex >= 30) return false;
    itemName = std::string("legendary_flame_pri_") + kPrimaryFlames[static_cast<size_t>(variantIndex)].id;
    if (variantIndex < 25) {
        region = "armor_fx";
        char buf[32];
        std::snprintf(buf, sizeof(buf), "armor_fx_%02d", variantIndex + 6);
        perm = buf;
    } else {
        region = "armor_fx_pri";
        char buf[32];
        std::snprintf(buf, sizeof(buf), "armor_fx_pri_%02d", variantIndex - 25);
        perm = buf;
    }
    return true;
}

} // namespace noble
