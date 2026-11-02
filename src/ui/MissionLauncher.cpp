#include "MissionLauncher.hpp"
#include "engine/BlamEngine.hpp"
#include "network/SessionManager.hpp"
#include <imgui.h>

namespace noble {

MissionLauncher::MissionLauncher() {
    InitializeMapRegistry();
}

void MissionLauncher::InitializeMapRegistry() {
    m_maps = {
        // Campaign Missions
        { "m05", "Noble Actual", "Campaign", "Prologue: Spartan-B312 arrives on Reach and meets Noble Team.", "m05.map" },
        { "m10", "Winter Contingency", "Campaign", "Investigate the Visegrad communications relay failure.", "m10.map" },
        { "m20", "ONI: Sword Base", "Campaign", "Defend the Office of Naval Intelligence facility against Covenant siege.", "m20.map" },
        { "m30", "Nightfall", "Campaign", "Infiltrate enemy lines under cover of darkness alongside Jun-A266.", "m30.map" },
        { "m35", "Tip of the Spear", "Campaign", "Massive armored spearhead assault destroying Covenant anti-air spires.", "m35.map" },
        { "m45", "Long Night of Solace", "Campaign", "Operation: UPPER CUT - Board the Covenant supercarrier via Sabre orbital fighters.", "m45.map" },
        { "m50", "Exodus", "Campaign", "Evacuate civilian survivors through the besieged towers of New Alexandria.", "m50.map" },
        { "m52", "New Alexandria", "Campaign", "Pilot a Falcon through skyscraper valleys to defend communication jammers.", "m52.map" },
        { "m60", "The Package", "Campaign", "Extract Dr. Catherine Halsey and the precious AI package from SWORD Base.", "m60.map" },
        { "m70", "The Pillar of Autumn", "Campaign", "Deliver the Cortana fragment to Captain Keyes at the Aszod ship breaking facility.", "m70.map" },
        { "m70_bonus", "Lone Wolf", "Campaign", "Survive against overwhelming Covenant forces. Spartan-B312's final stand.", "m70_bonus.map" },

        // Multiplayer Maps
        { "forge_halo", "Forge World", "Multiplayer", "Massive open canvas encompassing Canyon, Island, Gulch, and Quarry.", "forge_halo.map" },
        { "70_boneyard", "Boneyard", "Multiplayer", "Large asymmetric UNSC drydock facility designed for Invasion and Big Team Battle.", "70_boneyard.map" },
        { "50_panopticon", "Boardwalk", "Multiplayer", "Waterfront promenade in New Alexandria with a central plaza and walkways.", "50_panopticon.map" },
        { "30_settlement", "Powerhouse", "Multiplayer", "Hydroelectric facility nestled in Reach's rugged mountain wetlands.", "30_settlement.map" },
        { "20_sword_slayer", "Sword Base", "Multiplayer", "Vertical multi-tier research atrium featuring interior gravity lifts.", "20_sword_slayer.map" },
        { "45_aftship", "Zealot", "Multiplayer", "Covenant Corvette interior with low-gravity vacuum exterior catwalks.", "45_aftship.map" },
        { "45_launch_station", "Countdown", "Multiplayer", "Sabre rocket assembly launch platform with multiple close-quarters combat lanes.", "45_launch_station.map" },
        { "52_ivory_tower", "Reflection", "Multiplayer", "Luxurious executive penthouse with marble waterfalls overlooking New Alexandria.", "52_ivory_tower.map" },
        { "35_island", "Spire", "Multiplayer", "Towering Covenant spire on a coastal island, with a gravity lift and vehicle plateaus.", "35_island.map" },
        { "dlc_invasion", "Breakpoint", "Multiplayer", "High-altitude glacier research laboratory built for heavy vehicle combat.", "dlc_invasion.map" },
        { "dlc_medium", "Tempest", "Multiplayer", "Forerunner coastline with twin interlocking bases.", "dlc_medium.map" },
        { "dlc_slayer", "Anchor 9", "Multiplayer", "Orbital space station with hangars and zero-G airlocks.", "dlc_slayer.map" },
        { "condemned", "Condemned", "Multiplayer", "Orbital comms station taking direct plasma bombardments from Covenant cruisers.", "condemned.map" },
        { "cex_beaver_creek", "Battle Canyon", "Multiplayer", "Remastered classic canyon with twin bases, teleporters, and mid-creek cover.", "cex_beaver_creek.map" },
        { "cex_damnation", "Penance", "Multiplayer", "Covenant water-filtration facility with intricate vertical catwalks.", "cex_damnation.map" },
        { "cex_hangemhigh", "High Noon", "Multiplayer", "Sprawling asymmetric tomb of catwalks and open cross-canyon sniper sightlines.", "cex_hangemhigh.map" },
        { "cex_headlong", "Breakneck", "Multiplayer", "Vast construction dock in New Mombasa with crane walkways and highways.", "cex_headlong.map" },
        { "cex_prisoner", "Solitary", "Multiplayer", "Multi-tiered detention center with high central platforms and deadly dropoffs.", "cex_prisoner.map" },
        { "cex_timberland", "Ridgeline", "Multiplayer", "Evergreen alpine valley surrounded by sheer cliffs and snaking mountain streams.", "cex_timberland.map" },

        // Firefight Maps
        { "ff10_prototype", "Overlook", "Firefight", "Waterfront mining operation defense against dropship waves.", "ff10_prototype.map" },
        { "ff20_courtyard", "Courtyard", "Firefight", "ONI Sword Base executive exterior plaza under siege.", "ff20_courtyard.map" },
        { "ff30_waterfront", "Waterfront", "Firefight", "Agricultural reservoir with elevated snipers and vehicle perimeters.", "ff30_waterfront.map" },
        { "ff45_corvette", "Corvette", "Firefight", "Ardent Pray supercarrier hangar bay defense with heavy plasma fire.", "ff45_corvette.map" },
        { "ff50_park", "Beachhead", "Firefight", "Coastal retreat surrounded by fortified concrete observation platforms.", "ff50_park.map" },
        { "ff60_airview", "Holdout", "Firefight", "High-rise rooftop landing pads under continuous aerial Banshee strafes.", "ff60_airview.map" },
        { "ff60_icecave", "Glacier", "Firefight", "Sub-zero research cavern with narrow defensive bottlenecks.", "ff60_icecave.map" },
        { "ff70_holdout", "Outpost", "Firefight", "Aszod scrapyard defense surrounding the final transport landing zone.", "ff70_holdout.map" },
        { "ff_unearthed", "Unearthed", "Firefight", "Titanium strip mine with high-speed Warthog lanes and heavy Shade turrets.", "ff_unearthed.map" },
        { "cex_ff_halo", "Installation 04", "Firefight", "Alpha Halo cliffside canyon featuring Covenant phantom dropships.", "cex_ff_halo.map" }
    };
}

void MissionLauncher::Render(BlamEngine* engine) {
    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "TACTICAL OPERATIONS // SELECT MISSION & GAME MODE");
    ImGui::Separator();
    ImGui::Spacing();

    static int modeTab = 0;
    const char* tabs[] = { "CAMPAIGN", "MULTIPLAYER", "FIREFIGHT", "FORGE" };

    for (int i = 0; i < 4; ++i) {
        if (i > 0) ImGui::SameLine();
        bool active = (modeTab == i);
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.75f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        if (ImGui::Button(tabs[i], ImVec2(140, 30))) {
            modeTab = i;
        }

        if (active) {
            ImGui::PopStyleColor(2);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    switch (modeTab) {
        case 0: RenderCampaignTab(engine); break;
        case 1: RenderMultiplayerTab(engine); break;
        case 2: RenderFirefightTab(engine); break;
        case 3: RenderForgeTab(engine); break;
    }
}

void MissionLauncher::RenderCampaignTab(BlamEngine* engine) {
    ImGui::Columns(2, "CampaignLayout", false);
    ImGui::SetColumnWidth(0, 480.0f);

    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "STORY MISSIONS");
    ImGui::Spacing();

    std::vector<int> campaignIndices;
    for (int i = 0; i < static_cast<int>(m_maps.size()); ++i) {
        if (m_maps[i].category == "Campaign") {
            campaignIndices.push_back(i);
        }
    }

    for (int idx : campaignIndices) {
        auto& map = m_maps[idx];
        bool isSelected = (m_selectedCampaignIndex == idx);

        std::string label = "[" + map.internalName + "] " + map.displayName;
        if (ImGui::Selectable(label.c_str(), isSelected, 0, ImVec2(0, 24))) {
            m_selectedCampaignIndex = idx;
        }
    }

    ImGui::NextColumn();

    // Selected Mission Details & Settings Panel
    if (m_selectedCampaignIndex >= 0 && m_selectedCampaignIndex < static_cast<int>(m_maps.size())) {
        auto& sel = m_maps[m_selectedCampaignIndex];

        ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "%s", sel.displayName.c_str());
        ImGui::TextDisabled("File: maps/%s", sel.fileName.c_str());
        ImGui::Spacing();
        ImGui::TextWrapped("%s", sel.description.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Difficulty Selection
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "DIFFICULTY");
        const char* diffs[] = { "Easy", "Normal", "Heroic", "Legendary" };
        ImGui::Combo("##DifficultyCombo", &m_selectedDifficulty, diffs, IM_ARRAYSIZE(diffs));

        ImGui::Spacing();
        ImGui::Checkbox("Play as Elite (Sangheili)", &m_playAsElite);
        ImGui::Spacing();

        // Skulls Selection
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "TACTICAL SKULLS");
        ImGui::Checkbox("Iron", &m_skullIron); ImGui::SameLine(100);
        ImGui::Checkbox("Black Eye", &m_skullBlackEye); ImGui::SameLine(220);
        ImGui::Checkbox("Tough Luck", &m_skullToughLuck);

        ImGui::Checkbox("Catch", &m_skullCatch); ImGui::SameLine(100);
        ImGui::Checkbox("Fog", &m_skullFog); ImGui::SameLine(220);
        ImGui::Checkbox("Famine", &m_skullFamine);

        ImGui::Checkbox("Thunderstorm", &m_skullThunderstorm); ImGui::SameLine(150);
        ImGui::Checkbox("Tilt", &m_skullTilt); ImGui::SameLine(220);
        ImGui::Checkbox("Mythic", &m_skullMythic);

        ImGui::Checkbox("Grunt Birthday Party", &m_skullGruntBirthday); ImGui::SameLine(200);
        ImGui::Checkbox("IWHBYD", &m_skullIwhbyd);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Launch buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.55f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.75f, 0.45f, 1.0f));
        if (ImGui::Button("PLAY MISSION (SOLO / LOCAL)", ImVec2(240, 38))) {
            if (engine) {
                engine->LaunchMap(sel.internalName, "Campaign", m_selectedDifficulty, 0);
            }
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.45f, 0.75f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.65f, 0.95f, 1.0f));
        if (ImGui::Button("HOST IN CO-OP P2P", ImVec2(200, 38))) {
            HostConfig cfg;
            cfg.sessionName = "Reach Campaign Co-op (" + sel.displayName + ")";
            cfg.gameMode = "Campaign";
            cfg.mapName = sel.internalName;
            cfg.gametypeName = diffs[m_selectedDifficulty];
            cfg.maxPlayers = 16;
            cfg.mode = SessionManager::Get().GetNetworkMode();
            SessionManager::Get().StartHosting(cfg);

            if (engine) {
                engine->LaunchMap(sel.internalName, "Campaign", m_selectedDifficulty, 0);
            }
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::Columns(1);
}

void MissionLauncher::RenderMultiplayerTab(BlamEngine* engine) {
    ImGui::Columns(2, "MpLayout", false);
    ImGui::SetColumnWidth(0, 480.0f);

    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "REACH MULTIPLAYER MAPS");
    ImGui::Spacing();

    std::vector<int> mpIndices;
    for (int i = 0; i < static_cast<int>(m_maps.size()); ++i) {
        if (m_maps[i].category == "Multiplayer") {
            mpIndices.push_back(i);
        }
    }

    for (int idx : mpIndices) {
        auto& map = m_maps[idx];
        bool isSelected = (m_selectedMpIndex == idx);

        std::string label = map.displayName + " (" + map.internalName + ")";
        if (ImGui::Selectable(label.c_str(), isSelected, 0, ImVec2(0, 22))) {
            m_selectedMpIndex = idx;
        }
    }

    ImGui::NextColumn();

    if (m_selectedMpIndex >= 0 && m_selectedMpIndex < static_cast<int>(m_maps.size())) {
        auto& sel = m_maps[m_selectedMpIndex];

        ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "%s", sel.displayName.c_str());
        ImGui::TextDisabled("Map File: maps/%s", sel.fileName.c_str());
        ImGui::Spacing();
        ImGui::TextWrapped("%s", sel.description.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Gametype selection
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "GAME VARIANT");
        const char* variants[] = { "Slayer", "Team Slayer", "Capture the Flag", "SWAT", "Infection", "Grifball", "Invasion", "King of the Hill", "Oddball", "Headhunter" };
        ImGui::Combo("##VariantCombo", &m_selectedVariantIndex, variants, IM_ARRAYSIZE(variants));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.55f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.75f, 0.45f, 1.0f));
        if (ImGui::Button("START LOCAL MATCH", ImVec2(220, 38))) {
            if (engine) {
                engine->LaunchMap(sel.internalName, "Multiplayer", 1, 0);
            }
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.45f, 0.75f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.65f, 0.95f, 1.0f));
        if (ImGui::Button("HOST P2P SERVER", ImVec2(200, 38))) {
            HostConfig cfg;
            cfg.sessionName = "Reach " + std::string(variants[m_selectedVariantIndex]) + " (" + sel.displayName + ")";
            cfg.gameMode = "Multiplayer";
            cfg.mapName = sel.internalName;
            cfg.gametypeName = variants[m_selectedVariantIndex];
            cfg.maxPlayers = 128;
            cfg.mode = SessionManager::Get().GetNetworkMode();
            SessionManager::Get().StartHosting(cfg);

            if (engine) {
                engine->LaunchMap(sel.internalName, "Multiplayer", 1, 0);
            }
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::Columns(1);
}

void MissionLauncher::RenderFirefightTab(BlamEngine* engine) {
    ImGui::Columns(2, "FfLayout", false);
    ImGui::SetColumnWidth(0, 480.0f);

    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "FIREFIGHT MAPS");
    ImGui::Spacing();

    std::vector<int> ffIndices;
    for (int i = 0; i < static_cast<int>(m_maps.size()); ++i) {
        if (m_maps[i].category == "Firefight") {
            ffIndices.push_back(i);
        }
    }

    for (int idx : ffIndices) {
        auto& map = m_maps[idx];
        bool isSelected = (m_selectedFfIndex == idx);

        std::string label = map.displayName + " (" + map.internalName + ")";
        if (ImGui::Selectable(label.c_str(), isSelected, 0, ImVec2(0, 22))) {
            m_selectedFfIndex = idx;
        }
    }

    ImGui::NextColumn();

    if (m_selectedFfIndex >= 0 && m_selectedFfIndex < static_cast<int>(m_maps.size())) {
        auto& sel = m_maps[m_selectedFfIndex];

        ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "%s", sel.displayName.c_str());
        ImGui::TextDisabled("File: maps/%s", sel.fileName.c_str());
        ImGui::Spacing();
        ImGui::TextWrapped("%s", sel.description.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "FIREFIGHT DIFFICULTY");
        const char* diffs[] = { "Easy", "Normal", "Heroic", "Legendary" };
        ImGui::Combo("##FfDiffCombo", &m_selectedDifficulty, diffs, IM_ARRAYSIZE(diffs));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("START FIREFIGHT SOLO", ImVec2(220, 36))) {
            if (engine) {
                engine->LaunchMap(sel.internalName, "Firefight", m_selectedDifficulty, 0);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("HOST FIREFIGHT CO-OP", ImVec2(200, 36))) {
            HostConfig cfg;
            cfg.sessionName = "Reach Firefight (" + sel.displayName + ")";
            cfg.gameMode = "Firefight";
            cfg.mapName = sel.internalName;
            cfg.gametypeName = diffs[m_selectedDifficulty];
            cfg.maxPlayers = 16;
            cfg.mode = SessionManager::Get().GetNetworkMode();
            SessionManager::Get().StartHosting(cfg);

            if (engine) {
                engine->LaunchMap(sel.internalName, "Firefight", m_selectedDifficulty, 0);
            }
        }
    }

    ImGui::Columns(1);
}

void MissionLauncher::RenderForgeTab(BlamEngine* engine) {
    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "FORGE WORLD // UNBOUND SANDBOX EDITOR");
    ImGui::Spacing();
    ImGui::TextWrapped("Launch into Forge World or any Reach multiplayer canvas with unlocked object limits, full rotational precision, and instant object spawning.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    static int forgeMapIndex = 0;
    const char* forgeMaps[] = { "Forge World (forge_halo)", "Spire (35_island)", "Tempest (dlc_medium)", "Anchor 9 (dlc_slayer)" };
    const char* forgeIds[]  = { "forge_halo", "35_island", "dlc_medium", "dlc_slayer" };

    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "SELECT CANVAS:");
    ImGui::Combo("##ForgeCanvasCombo", &forgeMapIndex, forgeMaps, IM_ARRAYSIZE(forgeMaps));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.75f, 0.95f, 1.0f));
    if (ImGui::Button("ENTER FORGE EDITOR (SOLO)", ImVec2(240, 40))) {
        if (engine) {
            engine->LaunchMap(forgeIds[forgeMapIndex], "Forge", 1, 0);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("HOST SHARED FORGE SESSION (P2P)", ImVec2(260, 40))) {
        HostConfig cfg;
        cfg.sessionName = "Reach Forge Session (" + std::string(forgeMaps[forgeMapIndex]) + ")";
        cfg.gameMode = "Forge";
        cfg.mapName = forgeIds[forgeMapIndex];
        cfg.gametypeName = "Forge";
        cfg.maxPlayers = 128;
        cfg.mode = SessionManager::Get().GetNetworkMode();
        SessionManager::Get().StartHosting(cfg);

        if (engine) {
            engine->LaunchMap(forgeIds[forgeMapIndex], "Forge", 1, 0);
        }
    }
    ImGui::PopStyleColor(2);
}

} // namespace noble
