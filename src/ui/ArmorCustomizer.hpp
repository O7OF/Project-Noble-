#pragma once


#include <d3d11.h>
#include <filesystem>
#include <string>
#include <vector>

namespace noble {

enum class PlayerSpecies : int {
    Spartan = 0,
    Elite = 1
};

enum class BodyType : int {
    Male = 0,
    Female = 1
};

struct AppearanceProfile {
    std::string serviceTag = "N06";
    PlayerSpecies species = PlayerSpecies::Spartan; // Preferred Species for in-game spawning
    BodyType bodyType = BodyType::Male;

    // Color indices (0-29 matching Reach's 30-color palette)
    int primaryColor = 1;    // Steel
    int secondaryColor = 10; // Green

    // Spartan selections (catalog indices)
    int helmetIndex = 0;         // Mark V [B]
    int helmetVariantIndex = 0;  // Base
    int leftShoulderIndex = 0;   // Default
    int rightShoulderIndex = 0;  // Default
    int chestIndex = 0;          // Default
    int wristIndex = 0;          // Default
    int utilityIndex = 0;        // Default / None
    int visorColorIndex = 0;     // Default (Amber/Gold)
    int kneeGuardsIndex = 0;     // Default
    int armorEffectIndex = 0;    // None
    int firefightVoiceIndex = 0; // Noble Six

    // Elite selections (0=Minor, 1=Spec Ops, 2=Ranger, 3=Ultra, 4=Zealot, 5=General, 6=Field Marshal, 7=Officer)
    int eliteArmorSet = 0;
    int eliteHelmetIndex = 0;
    int eliteLeftShoulderIndex = 0;
    int eliteRightShoulderIndex = 0;
    int eliteChestIndex = 0;
    int eliteArmsIndex = 0;
    int eliteLegsIndex = 0;
    int eliteArmorEffectIndex = 0;
    int nameplateIndex = 1;
    int emblemIndex = 1;

    // Legacy string fields preserved for backward compatibility with existing appearance.json files
    std::string spartanHelmetGroup;
    std::string spartanChestGroup;
    std::string spartanLeftShoulderGroup;
    std::string spartanRightShoulderGroup;
    std::string spartanWristGroup;
    std::string spartanKneesGroup;
};

struct ReachColorEntry {
    const char* name;
    float r, g, b;
};

class ArmorCustomizer {
public:
    ArmorCustomizer();
    ~ArmorCustomizer() = default;

    void Render(ID3D11Device* device, ID3D11DeviceContext* context);

    bool LoadProfile(const std::filesystem::path& path);
    bool SaveProfile(const std::filesystem::path& path);
    void SetPreferredSpecies(PlayerSpecies species);
    PlayerSpecies GetPreferredSpecies() const { return m_profile.species; }

    void SyncToReachArmory() const;
    void UpdateArmoryCamera() const;

    const AppearanceProfile& GetProfile() const { return m_profile; }
    AppearanceProfile& GetProfileMutable() { return m_profile; }

    static std::filesystem::path ResolveDefaultProfilePath();

private:
    void RenderCharacterPreview(ID3D11Device* device, ID3D11DeviceContext* context,
                                float width, float height, bool fullBleed);
    void SyncLegacyFields();
    void MigrateFromLegacyFields();

    AppearanceProfile m_profile;
    PlayerSpecies m_previewSpecies = PlayerSpecies::Spartan;
    std::filesystem::path m_profilePath;
    bool m_dirty = false;
    mutable bool m_syncedInitial = false;

    // 0 = Standard Armory 3/4 view ("armory"), 1 = Full Body ("c_full_spartan"), 2 = close-up of the slot being edited
    // ("c_helmet", ...; default, Armory v2). The mouse wheel over the model switches between them.
    int m_cameraMode = 2;
    std::string m_focusScene = "c_full_spartan";   // close-up scene for the focused slot
    bool m_pickerScrollToEquipped = true;

    std::string m_spartanObjError, m_eliteObjError;
    float m_objYaw = -0.32f;      // MCC 3/4 left-facing angle
    bool m_autoRotate = false;

    int m_focusedRow = 0;
    bool m_showColorModal = false;
};

} // namespace noble
