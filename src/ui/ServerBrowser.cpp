#include "ServerBrowser.hpp"
#include <imgui.h>
#include <algorithm>
#include <cstring>

namespace noble {

ServerBrowser::ServerBrowser() {
    RefreshServers();
}

void ServerBrowser::RefreshServers() {
    m_servers.clear();

    // Populate default community and dedicated server discovery entries
    m_servers.push_back({
        "[US-East] Forge World - Speed Halo & Fat Kid 24/7",
        "142.250.190.46", 11774, 24, 14, 16,
        "Multiplayer", "Forge World", "Speed Halo v2.1",
        false, false, "", true
    });

    m_servers.push_back({
        "[US-Central] Noble Team - 32p Big Team Battle (Spire / Boneyard)",
        "198.51.100.22", 11774, 38, 26, 32,
        "Multiplayer", "Spire", "BTB Heavies Slayer",
        false, false, "", true
    });

    m_servers.push_back({
        "[EU-West] Firefight Arcade - Overlook & Corvette Heroic",
        "185.199.108.153", 11774, 82, 3, 4,
        "Firefight", "Overlook", "Classic Firefight",
        false, false, "", false
    });

    m_servers.push_back({
        "[NA-West] Reach Evolved Modded Campaign Co-op [Steam Workshop]",
        "203.0.113.88", 11774, 55, 2, 4,
        "Campaign", "m10_crash (Winter Contingency)", "Legendary Co-op",
        true, true, "Reach Evolved Overhaul v3.4", false
    });

    m_servers.push_back({
        "[Dedicated] MLG Hardcore Team Slayer (Sanctuary / Zealot)",
        "172.217.16.206", 11774, 19, 7, 8,
        "Multiplayer", "Zealot", "MLG v7 Slayer",
        false, false, "", false
    });

    m_servers.push_back({
        "[Community] Extinction Infection - Spire Zombie Defense",
        "104.244.42.1", 11774, 45, 15, 16,
        "Multiplayer", "Spire", "Alpha Zombie Inf",
        false, false, "", false
    });
}

void ServerBrowser::Render() {
    RenderFilterBar();

    ImGui::Separator();
    ImGui::Spacing();

    // Left pane: Server List (70% width)
    // Right pane: Server Details (30% width)
    float windowWidth = ImGui::GetContentRegionAvail().x;
    float listWidth = windowWidth * 0.70f;
    float detailsWidth = windowWidth - listWidth - 10.0f;

    ImGui::BeginChild("ServerListRegion", ImVec2(listWidth, 0), true);
    RenderServerList();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("ServerDetailsRegion", ImVec2(detailsWidth, 0), true);
    RenderServerDetails();
    ImGui::EndChild();

    if (m_showDirectConnectModal) {
        RenderDirectConnectModal();
    }
}

void ServerBrowser::RenderFilterBar() {
    ImGui::TextDisabled("FILTER:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(220.0f);
    ImGui::InputTextWithHint("##SearchQuery", "Search servers, maps, modes...", m_searchQuery, IM_ARRAYSIZE(m_searchQuery));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(130.0f);
    const char* modes[] = { "All Modes", "Multiplayer", "Firefight", "Campaign", "Forge" };
    ImGui::Combo("##ModeFilter", &m_modeFilter, modes, IM_ARRAYSIZE(modes));

    ImGui::SameLine();
    ImGui::Checkbox("Hide Full", &m_hideFull);

    ImGui::SameLine();
    ImGui::Checkbox("Hide Empty", &m_hideEmpty);

    ImGui::SameLine();
    ImGui::Checkbox("Favorites", &m_showOnlyFavorites);

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 220.0f);
    if (ImGui::Button("Direct Connect...", ImVec2(120, 0))) {
        m_showDirectConnectModal = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh", ImVec2(90, 0))) {
        RefreshServers();
    }
}

void ServerBrowser::RenderServerList() {
    static ImGuiTableFlags tableFlags = 
        ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_BordersInnerV | 
        ImGuiTableFlags_ScrollY | 
        ImGuiTableFlags_Sortable | 
        ImGuiTableFlags_Resizable;

    if (ImGui::BeginTable("ServerTable", 7, tableFlags)) {
        ImGui::TableSetupColumn("Fav", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        ImGui::TableSetupColumn("Server Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Map", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("Variant / Mode", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Players", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Ping", ImGuiTableColumnFlags_WidthFixed, 55.0f);
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(m_servers.size()); ++i) {
            auto& server = m_servers[i];

            // Apply search filter
            if (m_searchQuery[0] != '\0') {
                std::string q = m_searchQuery;
                std::string nameLower = server.name;
                std::string mapLower = server.mapName;
                std::string modeLower = server.gameMode;
                auto toLower = [](std::string& s) {
                    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    });
                };
                toLower(q); toLower(nameLower); toLower(mapLower); toLower(modeLower);

                if (nameLower.find(q) == std::string::npos && 
                    mapLower.find(q) == std::string::npos && 
                    modeLower.find(q) == std::string::npos) {
                    continue;
                }
            }

            // Mode filter
            if (m_modeFilter == 1 && server.gameMode != "Multiplayer") continue;
            if (m_modeFilter == 2 && server.gameMode != "Firefight") continue;
            if (m_modeFilter == 3 && server.gameMode != "Campaign") continue;
            if (m_modeFilter == 4 && server.gameMode != "Forge") continue;

            if (m_hideFull && server.playerCount >= server.maxPlayers) continue;
            if (m_hideEmpty && server.playerCount == 0) continue;
            if (m_showOnlyFavorites && !server.isFavorite) continue;

            ImGui::TableNextRow();

            // Col 0: Favorite toggle
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(i * 10);
            if (ImGui::Selectable(server.isFavorite ? "[*]" : "[ ]", false)) {
                server.isFavorite = !server.isFavorite;
            }
            ImGui::PopID();

            // Col 1: Server Name & Selection
            ImGui::TableSetColumnIndex(1);
            bool isSelected = (m_selectedServerIndex == i);
            std::string label = server.name;
            if (server.isPasswordProtected) label = "[Lock] " + label;
            if (server.hasWorkshopMod) label = "[Mod] " + label;

            if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                m_selectedServerIndex = i;
            }

            // Col 2: Map
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(server.mapName.c_str());

            // Col 3: Variant
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s (%s)", server.variantName.c_str(), server.gameMode.c_str());

            // Col 4: Players
            ImGui::TableSetColumnIndex(4);
            if (server.playerCount >= server.maxPlayers) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%d/%d", server.playerCount, server.maxPlayers);
            } else {
                ImGui::Text("%d/%d", server.playerCount, server.maxPlayers);
            }

            // Col 5: Ping
            ImGui::TableSetColumnIndex(5);
            if (server.ping < 50) {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "%d ms", server.ping);
            } else if (server.ping < 100) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%d ms", server.ping);
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%d ms", server.ping);
            }

            // Col 6: Connect Button
            ImGui::TableSetColumnIndex(6);
            ImGui::PushID(i * 10 + 1);
            if (ImGui::SmallButton("Join")) {
                m_selectedServerIndex = i;
                // Initiate connection sequence
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void ServerBrowser::RenderServerDetails() {
    if (m_selectedServerIndex < 0 || m_selectedServerIndex >= static_cast<int>(m_servers.size())) {
        ImGui::TextDisabled("Select a server from the list to view match details and connect.");
        return;
    }

    const auto& server = m_servers[m_selectedServerIndex];

    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "%s", server.name.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Host Address: %s:%d", server.hostAddress.c_str(), server.port);
    ImGui::Text("Game Type:    %s", server.gameMode.c_str());
    ImGui::Text("Variant:      %s", server.variantName.c_str());
    ImGui::Text("Map Name:     %s", server.mapName.c_str());
    ImGui::Text("Players:      %d / %d", server.playerCount, server.maxPlayers);
    ImGui::Text("Ping:         %d ms", server.ping);

    ImGui::Spacing();
    if (server.isPasswordProtected) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[!] Password Protected Match");
    }

    if (server.hasWorkshopMod) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "Required Mod: %s", server.workshopModName.c_str());
        ImGui::TextDisabled("Auto-sync is enabled. Missing assets will be fetched on join.");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("JOIN MATCH", ImVec2(ImGui::GetContentRegionAvail().x, 38))) {
        // Connect routine
    }

    ImGui::Spacing();
    if (ImGui::Button("Copy Server Address", ImVec2(ImGui::GetContentRegionAvail().x, 26))) {
        std::string fullAddr = server.hostAddress + ":" + std::to_string(server.port);
        ImGui::SetClipboardText(fullAddr.c_str());
    }
}

void ServerBrowser::RenderDirectConnectModal() {
    ImGui::OpenPopup("Direct Connect to Server");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 220));

    if (ImGui::BeginPopupModal("Direct Connect to Server", &m_showDirectConnectModal, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Enter the IP address and port of the Halo: Reach host:");
        ImGui::Spacing();

        ImGui::Text("Host Address (IP:Port):");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##DirectIp", m_directConnectIp, IM_ARRAYSIZE(m_directConnectIp));

        ImGui::Spacing();
        ImGui::Text("Password (optional):");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##DirectPass", m_directConnectPassword, IM_ARRAYSIZE(m_directConnectPassword), ImGuiInputTextFlags_Password);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Connect", ImVec2(120, 0))) {
            m_showDirectConnectModal = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 0))) {
            m_showDirectConnectModal = false;
        }

        ImGui::EndPopup();
    }
}

} // namespace noble
