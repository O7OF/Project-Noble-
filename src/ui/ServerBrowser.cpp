#include "ServerBrowser.hpp"
#include "network/SessionManager.hpp"
#include "network/UPnP.hpp"
#include <imgui.h>
#include <algorithm>

namespace noble {

ServerBrowser::ServerBrowser() {
    RefreshServers();
}

void ServerBrowser::RefreshServers() {
    m_servers.clear();

    // Trigger subnet probe for active hosts
    SessionManager::Get().TriggerDiscoveryScan();

    // Fetch real live discovered sessions from SessionManager
    auto discovered = SessionManager::Get().GetDiscoveredSessions();
    for (const auto& d : discovered) {
        ServerEntry e;
        e.name = d.sessionName;
        e.hostAddress = d.hostAddress;
        e.port = d.port;
        e.ping = d.pingMs;
        e.playerCount = d.playerCount;
        e.maxPlayers = d.maxPlayers;
        e.gameMode = d.gameMode;
        e.mapName = d.mapName;
        e.variantName = d.gametypeName;
        e.isPasswordProtected = d.isPasswordProtected;
        m_servers.push_back(e);
    }
}

void ServerBrowser::Render() {
    RenderFilterBar();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Two-column layout: Server List on Left, Match Details on Right
    ImGui::Columns(2, "ServerBrowserColumns", false);
    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.70f);

    RenderServerList();

    ImGui::NextColumn();

    RenderServerDetails();

    ImGui::Columns(1);

    if (m_showDirectConnectModal) {
        RenderDirectConnectModal();
    }

    if (m_showHostModal) {
        RenderHostModal();
    }
}

void ServerBrowser::RenderFilterBar() {
    // Top Network Status & Mode Bar
    NetworkMode currentMode = SessionManager::Get().GetNetworkMode();
    bool isOnline = (currentMode == NetworkMode::Online_P2P);

    if (isOnline) {
        ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "[NETWORK MODE: ONLINE P2P // UPNP ACTIVE]");
        ImGui::SameLine();
        if (ImGui::SmallButton("SWAP TO LOCAL (LAN / OFFLINE)")) {
            SessionManager::Get().SetNetworkMode(NetworkMode::Local_LAN);
        }
    } else {
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "[NETWORK MODE: LOCAL LAN / OFFLINE]");
        ImGui::SameLine();
        if (ImGui::SmallButton("SWAP TO ONLINE (UPNP P2P)")) {
            SessionManager::Get().SetNetworkMode(NetworkMode::Online_P2P);
        }
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 380.0f);
    ImGui::TextDisabled("%s", UPnP::Get().GetStatus().c_str());

    ImGui::Spacing();

    // Action buttons
    if (ImGui::Button("Refresh List", ImVec2(100, 26))) {
        RefreshServers();
    }
    ImGui::SameLine();

    bool isHosting = SessionManager::Get().IsHosting();
    if (isHosting) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("STOP HOSTING", ImVec2(130, 26))) {
            SessionManager::Get().StopHosting();
            RefreshServers();
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.3f, 1.0f));
        if (ImGui::Button("HOST GAME...", ImVec2(110, 26))) {
            m_showHostModal = true;
        }
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    if (ImGui::Button("Direct Connect...", ImVec2(130, 26))) {
        m_showDirectConnectModal = true;
    }

    ImGui::Spacing();

    // Search filter input
    ImGui::Text("Search:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("##Search", m_searchQuery, IM_ARRAYSIZE(m_searchQuery));

    ImGui::SameLine();
    ImGui::Text("Mode:");
    ImGui::SameLine();
    const char* modeNames[] = { "All Modes", "Multiplayer", "Campaign", "Firefight", "Forge" };
    ImGui::SetNextItemWidth(130);
    ImGui::Combo("##ModeCombo", &m_modeFilter, modeNames, IM_ARRAYSIZE(modeNames));

    ImGui::SameLine();
    ImGui::Checkbox("Hide Full", &m_hideFull);
    ImGui::SameLine();
    ImGui::Checkbox("Hide Empty", &m_hideEmpty);
}

void ServerBrowser::RenderServerList() {
    static ImGuiTableFlags flags = 
        ImGuiTableFlags_Resizable | 
        ImGuiTableFlags_Reorderable | 
        ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_BordersInnerV | 
        ImGuiTableFlags_ScrollY;

    if (m_servers.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("No active Reach servers discovered on network.");
        ImGui::TextDisabled("Click 'HOST GAME...' to start your own P2P or Local session, or 'Direct Connect' to join a friend by IP.");
        return;
    }

    if (ImGui::BeginTable("ServerTable", 7, flags, ImVec2(0, -1))) {
        ImGui::TableSetupColumn("Server Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Game Mode", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Map", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Variant", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableSetupColumn("Players", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Ping", ImGuiTableColumnFlags_WidthFixed, 55.0f);
        ImGui::TableSetupColumn("Host Address", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(m_servers.size()); ++i) {
            auto& s = m_servers[i];

            // Apply search filter
            if (m_searchQuery[0] != '\0') {
                std::string lowerQuery = m_searchQuery;
                std::string lowerName = s.name;
                std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
                if (lowerName.find(lowerQuery) == std::string::npos) continue;
            }

            ImGui::TableNextRow();

            // Col 0: Name
            ImGui::TableSetColumnIndex(0);
            bool isSelected = (m_selectedServerIndex == i);
            if (ImGui::Selectable(s.name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                m_selectedServerIndex = i;
            }

            // Col 1: Mode
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", s.gameMode.c_str());

            // Col 2: Map
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", s.mapName.c_str());

            // Col 3: Variant
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", s.variantName.c_str());

            // Col 4: Players
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%d / %d", s.playerCount, s.maxPlayers);

            // Col 5: Ping
            ImGui::TableSetColumnIndex(5);
            ImVec4 pingColor = (s.ping < 50) ? ImVec4(0.2f, 1.0f, 0.4f, 1.0f) :
                              (s.ping < 100) ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) :
                                               ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            ImGui::TextColored(pingColor, "%d ms", s.ping);

            // Col 6: Host Address
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%s:%d", s.hostAddress.c_str(), s.port);
        }
        ImGui::EndTable();
    }
}

void ServerBrowser::RenderServerDetails() {
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "SESSION DETAILS");
    ImGui::Separator();
    ImGui::Spacing();

    if (m_selectedServerIndex < 0 || m_selectedServerIndex >= static_cast<int>(m_servers.size())) {
        ImGui::TextDisabled("Select an active server from the list to view lobby details.");
        return;
    }

    auto& s = m_servers[m_selectedServerIndex];

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", s.name.c_str());
    ImGui::TextDisabled("Host: %s:%d", s.hostAddress.c_str(), s.port);
    ImGui::Spacing();

    ImGui::Text("Mode:     %s", s.gameMode.c_str());
    ImGui::Text("Map:      %s", s.mapName.c_str());
    ImGui::Text("Variant:  %s", s.variantName.c_str());
    ImGui::Text("Players:  %d / %d", s.playerCount, s.maxPlayers);
    ImGui::Text("Ping:     %d ms", s.ping);
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.75f, 0.95f, 1.0f));
    if (ImGui::Button("JOIN MATCH", ImVec2(180, 36))) {
        // Direct connect to selected host address
    }
    ImGui::PopStyleColor(2);
}

void ServerBrowser::RenderHostModal() {
    ImGui::OpenPopup("Host Self-Hosted Reach Session");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 380));

    if (ImGui::BeginPopupModal("Host Self-Hosted Reach Session", &m_showHostModal, ImGuiWindowFlags_NoResize)) {
        ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "CONFIGURE SELF-HOSTED LOBBY");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Server / Lobby Name:");
        ImGui::InputText("##HostName", m_hostSessionName, IM_ARRAYSIZE(m_hostSessionName));

        ImGui::Spacing();
        ImGui::Text("Game Mode:");
        const char* modes[] = { "Multiplayer", "Campaign Co-op", "Firefight", "Forge" };
        ImGui::Combo("##HostMode", &m_hostModeIndex, modes, IM_ARRAYSIZE(modes));

        ImGui::Spacing();
        ImGui::Text("Map Selection:");
        const char* maps[] = { "Forge World (forge_halo)", "Boneyard (70_boneyard)", "Spire (50_panopticon)", "Powerhouse (30_settlement)", "Sword Base (20_sword_slayer)", "Zealot (45_aftship)", "Countdown (45_launch_station)" };
        const char* mapIds[] = { "forge_halo", "70_boneyard", "50_panopticon", "30_settlement", "20_sword_slayer", "45_aftship", "45_launch_station" };
        ImGui::Combo("##HostMap", &m_hostMapIndex, maps, IM_ARRAYSIZE(maps));

        ImGui::Spacing();
        ImGui::Text("Max Players (Up to 32 supported):");
        ImGui::SliderInt("##HostMaxPlayers", &m_hostMaxPlayers, 2, 32);

        ImGui::Spacing();
        ImGui::Checkbox("Enable Online UPnP Port Forwarding (Allow Friends to Join over Internet)", &m_hostOnlineUPnP);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("START HOSTING", ImVec2(160, 32))) {
            HostConfig cfg;
            cfg.sessionName = m_hostSessionName;
            cfg.gameMode = modes[m_hostModeIndex];
            cfg.mapName = mapIds[m_hostMapIndex];
            cfg.gametypeName = "Slayer";
            cfg.maxPlayers = m_hostMaxPlayers;
            cfg.port = 11774;
            cfg.mode = m_hostOnlineUPnP ? NetworkMode::Online_P2P : NetworkMode::Local_LAN;

            SessionManager::Get().StartHosting(cfg);
            RefreshServers();
            m_showHostModal = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 32))) {
            m_showHostModal = false;
        }

        ImGui::EndPopup();
    }
}

void ServerBrowser::RenderDirectConnectModal() {
    ImGui::OpenPopup("Direct IP Connection");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(380, 220));

    if (ImGui::BeginPopupModal("Direct IP Connection", &m_showDirectConnectModal, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Enter Target Host IP and Port:");
        ImGui::Spacing();

        ImGui::InputText("Address:Port", m_directConnectIp, IM_ARRAYSIZE(m_directConnectIp));
        ImGui::InputText("Password", m_directConnectPassword, IM_ARRAYSIZE(m_directConnectPassword), ImGuiInputTextFlags_Password);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Connect", ImVec2(120, 30))) {
            m_showDirectConnectModal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 30))) {
            m_showDirectConnectModal = false;
        }

        ImGui::EndPopup();
    }
}

} // namespace noble
