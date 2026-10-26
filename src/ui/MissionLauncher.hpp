#pragma once

#include <string>
#include <vector>

namespace noble {

class BlamEngine;

struct MapInfo {
    std::string internalName; // e.g. "forge_halo", "m10"
    std::string displayName;  // e.g. "Forge World", "Winter Contingency"
    std::string category;     // "Campaign", "Multiplayer", "Firefight"
    std::string description;
    std::string fileName;     // "forge_halo.map", "m10.map"
};

class MissionLauncher {
public:
    MissionLauncher();
    ~MissionLauncher() = default;

    void Render(BlamEngine* engine);

    const std::vector<MapInfo>& GetAllMaps() const { return m_maps; }

private:
    void RenderCampaignTab(BlamEngine* engine);
    void RenderMultiplayerTab(BlamEngine* engine);
    void RenderFirefightTab(BlamEngine* engine);
    void RenderForgeTab(BlamEngine* engine);

    void InitializeMapRegistry();

    std::vector<MapInfo> m_maps;
    int m_selectedCampaignIndex = 1;    // Default: Winter Contingency (m10)
    int m_selectedMpIndex = 0;          // Default: Forge World
    int m_selectedFfIndex = 0;          // Default: Overlook
    int m_selectedDifficulty = 2;       // 0=Easy, 1=Normal, 2=Heroic, 3=Legendary
    bool m_playAsElite = false;

    // Skulls
    bool m_skullIron = false;
    bool m_skullBlackEye = false;
    bool m_skullToughLuck = false;
    bool m_skullCatch = false;
    bool m_skullFog = false;
    bool m_skullFamine = false;
    bool m_skullThunderstorm = false;
    bool m_skullTilt = false;
    bool m_skullMythic = false;
    bool m_skullCowbell = false;
    bool m_skullGruntBirthday = true;
    bool m_skullIwhbyd = true;

    // Multiplayer variant
    int m_selectedVariantIndex = 0;
};

} // namespace noble
