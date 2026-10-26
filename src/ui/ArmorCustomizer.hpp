#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace noble {

struct ArmorProfile {
    int species = 0; // 0 = Spartan, 1 = Elite
    int gender = 0;  // 0 = Male, 1 = Female

    // Spartan Permutations
    int helmet = 0;
    int leftShoulder = 0;
    int rightShoulder = 0;
    int chest = 0;
    int wrist = 0;
    int utility = 0;
    int visorColor = 0;
    int kneeGuards = 0;
    int armorEffect = 0;

    // Elite Permutations
    int eliteArmor = 0; // Minor, Major, Ultra, Zealot, Ranger, General, SpecOps, Field Marshal

    // Colors
    int primaryColor = 0;
    int secondaryColor = 12;
    int detailColor = 4;

    // Identity
    char serviceTag[5] = "B312";
    int voice = 0;
};

class ArmorCustomizer {
public:
    ArmorCustomizer();
    ~ArmorCustomizer() = default;

    void Render();
    bool LoadProfile(const std::filesystem::path& path = "data/appearance.json");
    bool SaveProfile(const std::filesystem::path& path = "data/appearance.json");

    const ArmorProfile& GetProfile() const { return m_profile; }

private:
    void RenderSpartanCustomizer();
    void RenderEliteCustomizer();
    void RenderColorPalettePicker(const char* label, int& selectedColorIndex);

    ArmorProfile m_profile;
    bool m_isDirty = false;
};

} // namespace noble
