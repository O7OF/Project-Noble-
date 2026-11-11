#include "mvar/ForgePlacementApply.hpp"
#include "mvar/MapVariant.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace noble::mvar {
namespace {

// The variant's own records are quantized; convert a world position back onto the same grid.
uint32_t QuantizeAxis(float world, float minimum, float maximum, int bits) {
    if (bits <= 0) return 0;
    const double range = static_cast<double>(maximum) - static_cast<double>(minimum);
    if (range <= 0.0) return 0;
    const double steps = static_cast<double>(1 << bits);
    double raw = (static_cast<double>(world) - static_cast<double>(minimum)) / range * steps - 0.5;
    if (raw < 0.0) raw = 0.0;
    if (raw > steps - 1.0) raw = steps - 1.0;
    return static_cast<uint32_t>(std::llround(raw));
}

float DistanceSquared(const float a[3], const float b[3]) {
    const float dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
    return dx * dx + dy * dy + dz * dz;
}

} // namespace

int ApplyWorldPlacements(MapVariant& variant, const std::vector<WorldPlacement>& placements, std::string& error) {
    if (!variant.valid) {
        error = "the variant is not valid";
        return 0;
    }
    MapVariantHeader& header = variant.header;

    // Associate tag definitions with an existing record: a load-time placement sits exactly where its record does.
    std::vector<int> tagTemplate;   // tag -> object index, 0 = unknown
    std::vector<uint32_t> tagKeys;
    constexpr float kSamePosition = 0.01f;   // squared distance
    std::vector<bool> recordAlreadyPlaced(header.objects.size(), false);
    for (const WorldPlacement& placement : placements) {
        int best = -1;
        float bestDistance = kSamePosition;
        for (size_t i = 0; i < header.objects.size(); ++i) {
            const ForgeObject& object = header.objects[i];
            if (!object.present) continue;
            const float distance = DistanceSquared(placement.position, object.position);
            if (distance >= bestDistance) continue;
            bestDistance = distance;
            best = static_cast<int>(i);
        }
        if (best < 0) continue;
        recordAlreadyPlaced[best] = true;
        const auto it = std::find(tagKeys.begin(), tagKeys.end(), placement.tag);
        const size_t slot = it == tagKeys.end() ? (tagKeys.push_back(placement.tag), tagKeys.size() - 1)
                                                : static_cast<size_t>(it - tagKeys.begin());
        if (tagTemplate.size() <= slot) tagTemplate.resize(slot + 1, -1);
        tagTemplate[slot] = best;
    }

    int added = 0;
    for (const WorldPlacement& placement : placements) {
        // Already in the map (its own object, or an earlier placement): skip.
        bool existing = false;
        for (size_t i = 0; i < header.objects.size() && !existing; ++i) {
            if (!header.objects[i].present) continue;
            if (DistanceSquared(placement.position, header.objects[i].position) < kSamePosition) existing = true;
        }
        if (existing) continue;

        // Clone a record of the same tag definition so type/quota/shape/properties match; only the transform changes.
        const auto it = std::find(tagKeys.begin(), tagKeys.end(), placement.tag);
        if (it == tagKeys.end()) {
            error = "no record of that tag definition in the map yet";
            continue;
        }
        const int templateIndex = tagTemplate[static_cast<size_t>(it - tagKeys.begin())];
        if (templateIndex < 0) continue;

        int freeSlot = -1;
        for (size_t i = 0; i < header.objects.size(); ++i) {
            if (!header.objects[i].present) { freeSlot = static_cast<int>(i); break; }
        }
        if (freeSlot < 0) {
            error = "no free object slot left";
            break;
        }
        ForgeObject& object = header.objects[static_cast<size_t>(freeSlot)];
        object = header.objects[static_cast<size_t>(templateIndex)];
        object.present = true;
        for (int axis = 0; axis < 3; ++axis) {
            object.positionRaw[axis] = QuantizeAxis(placement.position[axis], header.boundsMin[axis],
                                                    header.boundsMax[axis], header.positionBits[axis]);
        }
        for (int axis = 0; axis < 3; ++axis) object.position[axis] = placement.position[axis];
        // Orientation is kept from the template: Forge snaps walls to the same axes, and the axis-angle encoding needs
        // the engine's reference frame to invert exactly.
        ++added;
    }
    header.presentObjects += added;
    return added;
}

} // namespace noble::mvar
