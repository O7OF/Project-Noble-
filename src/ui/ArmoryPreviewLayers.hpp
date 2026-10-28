#pragma once

#include "ObjModelPreview.hpp"

#include <string>
#include <vector>

namespace noble {

// Which model groups of the local OBJ exports (data\armor\spartans\spartans.obj, data\armor\elite\elite.obj,
// git-ignored, never committed) make up an Armory look. The tables are O7OF's from the earlier OBJ Armory
// preview (0ff996c), looked up by the Armory's item names so the item lists can change without touching them.
struct ArmoryLook {
    bool elite = false;
    bool female = false;
    std::string helmet, helmetVariant, leftShoulder, rightShoulder, chest, wrist, utility, knees;
    bool prostheticArm = false;   // chest pieces marked [R]
    int eliteSet = 0;             // 0 Minor .. 7 Officer
};

void BuildArmoryPreviewLayers(const ArmoryLook& look, std::vector<ObjPreviewLayer>& layers, std::string& referenceGroup);

} // namespace noble
