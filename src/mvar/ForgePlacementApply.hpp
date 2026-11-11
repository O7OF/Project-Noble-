#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace noble::mvar {

// One object placed in Forge, captured at object_new: the tag handle identifies the definition, the transform is the
// world placement. This is the raw material for a variant object record.
struct WorldPlacement {
    uint32_t tag = 0;
    float position[3] = {};
    float forward[3] = { 1.0f, 0.0f, 0.0f };
    float up[3] = { 0.0f, 0.0f, 1.0f };
};

struct MapVariant;

// Adds the placements that are not already in `variant` by cloning an existing record of the same tag definition and
// replacing its transform. Load-time placements (the map's own objects) match an existing record by position and are
// skipped. Returns how many records were added; `error` is set when none could be (e.g. no free slot).
int ApplyWorldPlacements(MapVariant& variant, const std::vector<WorldPlacement>& placements, std::string& error);

// Same, but with the map's build palette (tag -> {variant_index, quota_index}) so a definition the variant does not
// contain yet can still be written: a record with the same palette entry is cloned, or a minimal one is synthesised.
using TagPalette = std::map<uint32_t, std::pair<int, int>>;
int ApplyWorldPlacements(MapVariant& variant, const std::vector<WorldPlacement>& placements, const TagPalette& palette,
                         std::string& error);

} // namespace noble::mvar
