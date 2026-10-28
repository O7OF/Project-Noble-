#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <d3d11.h>
#include "ObjModelPreview.hpp"

namespace noble {

struct ArmorProfile {
    int species = 0; // 0 = Spartan, 1 = Elite
    int gender = 0;  // 0 = Male, 1 = Female

    int visorColor = 0;

    int eliteArmor = 0; // Selects a locally exported Elite harness group.

    // Colors
    int primaryColor = 0;
    int secondaryColor = 12;
    int detailColor = 4;

    // MCC OBJ groups selected as visible armor overlays; empty means the stock piece.
    std::string helmetGroup;
    std::string chestGroup;
    std::string leftShoulderGroup;
    std::string rightShoulderGroup;
    std::string wristGroup;
    std::string kneeGroup;

    // Identity
    char serviceTag[5] = "B312";
    int voice = 0;
};

class ArmorCustomizer {
public:
    ArmorCustomizer();
    ~ArmorCustomizer() = default;

    void Render(ID3D11Device* device, ID3D11DeviceContext* context);
    bool LoadProfile(const std::filesystem::path& path = "data/appearance.json");
    bool SaveProfile(const std::filesystem::path& path = "data/appearance.json");

    const ArmorProfile& GetProfile() const { return m_profile; }

private:
    void RenderSpartanCustomizer();
    void RenderEliteCustomizer();
    void RenderCharacterPreview(ID3D11Device* device, ID3D11DeviceContext* context);
    void RenderColorPalettePicker(const char* label, int& selectedColorIndex);
    void RenderGroupSelector(const char* label, std::string& selectedGroup,
                             std::string_view category, std::string_view side = {});

    ArmorProfile m_profile;
    bool m_isDirty = false;
    ObjModelPreview m_preview;
    int m_loadedSpecies = -1;
    std::vector<std::string> m_previewGroups;
    float m_previewRotation = 0.0f;
    std::string m_previewError;
};

} // namespace noble
