#include "ServerBrowser.hpp"
#include "NobleStyle.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "network/MasterServerClient.hpp"
#include "network/SessionManager.hpp"
#include "network/UPnP.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstring>

namespace noble {

namespace {

struct HostMap {
    const char* scenario;
    const char* name;
};

const HostMap kHostMaps[] = {
    { "50_panopticon", "Boardwalk" },   { "70_boneyard", "Boneyard" },      { "45_launch_station", "Countdown" },
    { "30_settlement", "Powerhouse" },  { "52_ivory_tower", "Reflection" }, { "35_island", "Spire" },
    { "20_sword_slayer", "Sword Base" }, { "45_aftship", "Zealot" },        { "dlc_slayer", "Anchor 9" },
    { "dlc_invasion", "Breakpoint" },   { "dlc_medium", "Tempest" },        { "condemned", "Condemned" },
    { "trainingpreserve", "Highlands" }, { "forge_halo", "Forge World" },
};
const char* const kHostModes[] = { "Multiplayer", "Campaign", "Firefight", "Forge" };

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
    return s;
}

// "1.2.3.4:5678" or "1.2.3.4" -> address + port (default 11774). False for empty/bad input.
bool ParseAddress(const std::string& text, std::string& address, uint16_t& port) {
    std::string t = text;
    t.erase(std::remove_if(t.begin(), t.end(), [](unsigned char c) { return std::isspace(c) != 0; }), t.end());
    if (t.empty()) return false;
    port = 11774;
    const size_t colon = t.rfind(':');
    if (colon != std::string::npos && t.find(':') == colon) {
        int value = 0;
        const auto r = std::from_chars(t.data() + colon + 1, t.data() + t.size(), value);
        if (r.ec != std::errc() || value < 1 || value > 65535) return false;
        port = static_cast<uint16_t>(value);
        t = t.substr(0, colon);
    }
    if (t.empty() || t.size() > 253) return false;
    address = t;
    return true;
}

std::string Key(const std::string& address, uint16_t port) {
    return address + ":" + std::to_string(port);
}

} // namespace

ServerBrowser::ServerBrowser() {
    if (!UPnP::Get().Initialize()) {
        NOBLE_WARN("ServerBrowser: local network initialization failed: {}", UPnP::Get().GetStatus());
    }
    const SettingsData s = Settings::Get().Snapshot();
    std::snprintf(m_hostSessionName, sizeof(m_hostSessionName), "%s's game", s.playerName.c_str());

    const std::string masterUrl = s.masterServerUrl.empty() ? "https://outbreak-z-test-server.onrender.com" : s.masterServerUrl;
    std::snprintf(m_masterUrl, sizeof(m_masterUrl), "%s", masterUrl.c_str());
    MasterServerClient::Get().SetUrl(masterUrl);
    std::snprintf(m_directConnectIp, sizeof(m_directConnectIp), "%s:11774", UPnP::Get().GetInternalIP().c_str());
}

void ServerBrowser::RefreshServers() {
    const SettingsData settings = Settings::Get().Snapshot();
    std::vector<ServerEntry> list;
    auto add = [&](ServerEntry e) {
        for (auto& existing : list) {
            if (existing.hostAddress == e.hostAddress && existing.port == e.port) return;  // keep the first source
        }
        const std::string key = Key(e.hostAddress, e.port);
        e.isFavorite = std::find(settings.favorites.begin(), settings.favorites.end(), key) != settings.favorites.end();
        list.push_back(std::move(e));
    };

    // 1. The game this player is hosting (always shown, even if its own beacon isn't heard).
    SessionManager& sessions = SessionManager::Get();
    if (sessions.IsHosting()) {
        const HostConfig& cfg = sessions.GetHostConfig();
        ServerEntry e;
        e.name = cfg.sessionName;
        e.hostAddress = UPnP::Get().GetInternalIP();
        e.port = cfg.port;
        e.ping = 0;
        e.playerCount = 1;
        e.maxPlayers = cfg.maxPlayers;
        e.gameMode = cfg.gameMode;
        e.mapName = cfg.mapName;
        e.variantName = cfg.gametypeName;
        e.isPasswordProtected = !cfg.password.empty();
        e.isOwn = true;
        e.isMod = (cfg.mapName.find(".map") != std::string::npos || cfg.sessionName.find("[Mod]") != std::string::npos);
        e.source = "YOU";
        add(e);
    }

    // 2. LAN beacons.
    for (const auto& d : sessions.GetDiscoveredSessions()) {
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
        e.isMod = (d.sessionName.find("[Mod]") != std::string::npos || d.mapName.find(".map") != std::string::npos);
        e.source = "LAN";
        // Our own beacon heard on the LAN: already listed as "YOU".
        if (sessions.IsHosting() && d.port == sessions.GetHostConfig().port && d.sessionName == sessions.GetHostConfig().sessionName) continue;
        add(e);
    }

    // 3. Master server list.
    for (const auto& m : MasterServerClient::Get().Sessions()) {
        ServerEntry e;
        e.name = m.name;
        e.hostAddress = m.address;
        e.port = m.port;
        e.ping = -1;
        e.playerCount = m.playerCount;
        e.maxPlayers = m.maxPlayers;
        e.gameMode = m.mode;
        e.mapName = m.map;
        e.isPasswordProtected = m.hasPassword;
        e.inProgress = m.inProgress;
        e.isMod = m.isMod || (m.name.find("[Mod]") != std::string::npos);
        e.source = "ONLINE";
        add(e);
    }

    // 4. Saved favourites that aren't advertising right now.
    for (const auto& fav : settings.favorites) {
        std::string address;
        uint16_t port = 0;
        if (!ParseAddress(fav, address, port)) continue;
        ServerEntry e;
        e.name = "Saved server";
        e.hostAddress = address;
        e.port = port;
        e.ping = -1;
        e.source = "FAVORITE";
        add(e);
    }

    // Keep the selection on the same server if it is still listed.
    std::string selectedKey;
    if (m_selectedServerIndex >= 0 && m_selectedServerIndex < static_cast<int>(m_servers.size())) {
        selectedKey = Key(m_servers[m_selectedServerIndex].hostAddress, m_servers[m_selectedServerIndex].port);
    }
    m_servers = std::move(list);
    m_selectedServerIndex = -1;
    for (int i = 0; i < static_cast<int>(m_servers.size()); ++i) {
        if (Key(m_servers[i].hostAddress, m_servers[i].port) == selectedKey) m_selectedServerIndex = i;
    }
}

void ServerBrowser::Render() {
    // Live list: re-read LAN/master results every 2 s, probe the LAN and ask the master server
    // every 10 s.
    const double now = ImGui::GetTime();
    if (now - m_lastMasterRequest > 10.0) {
        m_lastMasterRequest = now;
        SessionManager::Get().TriggerDiscoveryScan();
        MasterServerClient::Get().RequestList();
    }
    if (now - m_lastRefresh > 2.0) {
        m_lastRefresh = now;
        RefreshServers();
    }

    RenderFilterBar();
    NobleStyle::SectionGap();

    const float detailsWidth = 300.0f;
    ImGui::BeginChild("##serverlist", ImVec2(-detailsWidth - 12.0f, 0), ImGuiChildFlags_None);
    RenderServerList();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##serverdetails", ImVec2(0, 0), ImGuiChildFlags_Borders);
    RenderServerDetails();
    ImGui::EndChild();

    if (m_showDirectConnectModal) RenderDirectConnectModal();
    if (m_showHostModal) RenderHostModal();
    if (m_showMasterModal) RenderMasterServerModal();
}

void ServerBrowser::RenderFilterBar() {
    SessionManager& sessions = SessionManager::Get();
    const bool hosting = sessions.IsHosting();

    if (hosting) {
        if (NobleStyle::DangerButton("STOP HOSTING", ImVec2(140, 0))) {
            sessions.StopHosting();
            MasterServerClient::Get().StopAdvertising();
            m_status = "Stopped hosting.";
            RefreshServers();
        }
    } else if (NobleStyle::PrimaryButton("HOST GAME...", ImVec2(140, 0))) {
        m_showHostModal = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("DIRECT CONNECT...", ImVec2(160, 0))) m_showDirectConnectModal = true;
    ImGui::SameLine();
    if (ImGui::Button("REFRESH", ImVec2(100, 0))) {
        m_lastMasterRequest = -100.0;
        m_lastRefresh = -100.0;
        sessions.TriggerDiscoveryScan();
    }
    ImGui::SameLine();
    if (ImGui::Button("MASTER SERVER...", ImVec2(160, 0))) {
        std::snprintf(m_masterUrl, sizeof(m_masterUrl), "%s", MasterServerClient::Get().Url().c_str());
        m_showMasterModal = true;
    }

    ImGui::Spacing();
    ImGui::SetNextItemWidth(220);
    ImGui::InputTextWithHint("##Search", "Search by name or map", m_searchQuery, IM_ARRAYSIZE(m_searchQuery));
    ImGui::SameLine();
    const char* modeNames[] = { "All modes", "Multiplayer", "Campaign", "Firefight", "Forge" };
    ImGui::SetNextItemWidth(140);
    ImGui::Combo("##ModeCombo", &m_modeFilter, modeNames, IM_ARRAYSIZE(modeNames));
    ImGui::SameLine();
    ImGui::Checkbox("Hide full", &m_hideFull);
    ImGui::SameLine();
    ImGui::Checkbox("Hide empty", &m_hideEmpty);
    ImGui::SameLine();
    ImGui::Checkbox("Favourites only", &m_showOnlyFavorites);

    // Status line: LAN / master server / UPnP.
    const std::string master = MasterServerClient::Get().Enabled()
        ? "Master server: " + MasterServerClient::Get().Status()
        : std::string("Master server: not set (LAN games only)");
    ImGui::TextDisabled("%s   |   %s", master.c_str(), UPnP::Get().GetStatus().c_str());
    if (!m_status.empty()) {
        ImGui::TextColored(NobleStyle::Accent(), "%s", m_status.c_str());
    }
}

bool ServerBrowser::PassesFilters(const ServerEntry& s) const {
    if (m_searchQuery[0] != '\0') {
        const std::string q = Lower(m_searchQuery);
        if (Lower(s.name).find(q) == std::string::npos && Lower(s.mapName).find(q) == std::string::npos) return false;
    }
    if (m_modeFilter > 0) {
        const std::string want = Lower(kHostModes[m_modeFilter - 1]);
        if (Lower(s.gameMode).find(want) == std::string::npos && s.source != "FAVORITE") return false;
    }
    if (m_hideFull && s.maxPlayers > 0 && s.playerCount >= s.maxPlayers) return false;
    if (m_hideEmpty && s.playerCount == 0 && s.source != "FAVORITE") return false;
    if (m_showOnlyFavorites && !s.isFavorite) return false;
    return true;
}

void ServerBrowser::RenderServerList() {
    int shown = 0;
    const ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                  ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("ServerTable", 7, flags, ImVec2(0, 0))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("SERVER", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("MODE", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("MAP", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableSetupColumn("PLAYERS", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("PING", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("ADDRESS", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(m_servers.size()); ++i) {
            const ServerEntry& s = m_servers[i];
            if (!PassesFilters(s)) continue;
            ++shown;
            ImGui::TableNextRow();
            ImGui::PushID(i);

            ImGui::TableSetColumnIndex(0);
            const ImVec4 tagColor = s.isOwn ? NobleStyle::Accent()
                                  : s.source == "LAN" ? ImVec4(0.45f, 0.85f, 0.45f, 1.0f)
                                  : s.source == "ONLINE" ? ImVec4(0.45f, 0.70f, 1.0f, 1.0f)
                                  : ImVec4(0.85f, 0.75f, 0.35f, 1.0f);
            ImGui::TextColored(tagColor, "%s%s", s.source.c_str(), s.isFavorite ? " *" : "");

            ImGui::TableSetColumnIndex(1);
            std::string label = (s.isPasswordProtected ? "[LOCKED] " : "");
            if (s.isMod) label += "[Mod] ";
            label += s.name;
            if (s.inProgress) label += "  (in progress)";
            if (ImGui::Selectable(label.c_str(), m_selectedServerIndex == i,
                                  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                m_selectedServerIndex = i;
                if (ImGui::IsMouseDoubleClicked(0)) TryJoin(s.hostAddress, s.port, s.isPasswordProtected);
            }

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(s.gameMode.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(s.mapName.c_str());
            ImGui::TableSetColumnIndex(4);
            if (s.source == "FAVORITE") ImGui::TextDisabled("-");
            else ImGui::Text("%d / %d", s.playerCount, s.maxPlayers);
            ImGui::TableSetColumnIndex(5);
            if (s.ping < 0) {
                ImGui::TextDisabled("-");
            } else {
                const ImVec4 pingColor = s.ping < 60 ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f)
                                       : s.ping < 120 ? ImVec4(0.95f, 0.8f, 0.3f, 1.0f)
                                                      : ImVec4(0.95f, 0.35f, 0.35f, 1.0f);
                ImGui::TextColored(pingColor, "%d", s.ping);
            }
            ImGui::TableSetColumnIndex(6);
            ImGui::TextDisabled("%s:%u", s.hostAddress.c_str(), static_cast<unsigned>(s.port));
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (shown == 0) {
        ImGui::Spacing();
        ImGui::TextDisabled("No games found yet. The list updates by itself.");
        ImGui::TextDisabled("HOST GAME lists your own game here and on your LAN (and on the master server, if one is set).");
    }
}

void ServerBrowser::RenderServerDetails() {
    NobleStyle::Header("GAME DETAILS");
    if (m_selectedServerIndex < 0 || m_selectedServerIndex >= static_cast<int>(m_servers.size())) {
        ImGui::TextWrapped("Select a game in the list.");
        return;
    }
    const ServerEntry s = m_servers[m_selectedServerIndex];
    ImGui::TextWrapped("%s", s.name.c_str());
    ImGui::TextDisabled("%s:%u  (%s)", s.hostAddress.c_str(), static_cast<unsigned>(s.port), s.source.c_str());
    ImGui::Spacing();
    if (!s.gameMode.empty()) ImGui::Text("Mode:     %s", s.gameMode.c_str());
    if (!s.mapName.empty()) ImGui::Text("Map:      %s", s.mapName.c_str());
    if (!s.variantName.empty()) ImGui::Text("Game type: %s", s.variantName.c_str());
    if (s.source != "FAVORITE") ImGui::Text("Players:  %d / %d", s.playerCount, s.maxPlayers);
    if (s.isPasswordProtected) ImGui::TextColored(ImVec4(0.95f, 0.8f, 0.3f, 1.0f), "Password protected");
    NobleStyle::SectionGap();

    if (!s.isOwn) {
        if (NobleStyle::PrimaryButton("JOIN", ImVec2(-1, 36))) TryJoin(s.hostAddress, s.port, s.isPasswordProtected);
        ImGui::Spacing();
    }
    if (ImGui::Button(s.isFavorite ? "REMOVE FAVOURITE" : "ADD TO FAVOURITES", ImVec2(-1, 0))) {
        ToggleFavorite(s.hostAddress, s.port);
        RefreshServers();
    }
    if (ImGui::Button("COPY INVITE", ImVec2(-1, 0))) {
        std::string invite = s.hostAddress + ":" + std::to_string(s.port);
        ImGui::SetClipboardText(invite.c_str());
        m_status = "Invite copied to clipboard: " + invite;
    }
    if (ImGui::Button("COPY ADDRESS", ImVec2(-1, 0))) {
        ImGui::SetClipboardText(Key(s.hostAddress, s.port).c_str());
        m_status = "Address copied: " + Key(s.hostAddress, s.port);
    }
}

void ServerBrowser::ToggleFavorite(const std::string& address, uint16_t port) {
    SettingsData d = Settings::Get().Snapshot();
    const std::string key = Key(address, port);
    auto it = std::find(d.favorites.begin(), d.favorites.end(), key);
    if (it != d.favorites.end()) {
        d.favorites.erase(it);
        m_status = "Removed " + key + " from favourites.";
    } else {
        d.favorites.push_back(key);
        m_status = "Saved " + key + " to favourites.";
    }
    Settings::Get().Update(d);
}

void ServerBrowser::TryJoin(const std::string& address, uint16_t port, bool hasPassword) {
    (void)hasPassword;
    // Joining needs haloreach.dll's own network session (host slots 41/43 and the online session
    // interface), which Noble doesn't drive yet. Say so instead of pretending to connect.
    m_status = "Joining " + Key(address, port) +
               " isn't possible yet: Noble can't link two games together yet (online play is still being worked on).";
}

void ServerBrowser::RenderHostModal() {
    ImGui::OpenPopup("HOST A GAME");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(520, 0));
    if (ImGui::BeginPopupModal("HOST A GAME", &m_showHostModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("Local games use your device's LAN address. Online games open UDP 11774 with UPnP and publish your public address to the master server.");
        NobleStyle::SectionGap();

        ImGui::Text("Name");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##HostName", m_hostSessionName, IM_ARRAYSIZE(m_hostSessionName));
        ImGui::Text("Mode");
        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##HostMode", &m_hostModeIndex, kHostModes, IM_ARRAYSIZE(kHostModes));
        ImGui::Text("Map");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##HostMap", kHostMaps[m_hostMapIndex].name)) {
            for (int i = 0; i < static_cast<int>(IM_ARRAYSIZE(kHostMaps)); ++i) {
                if (ImGui::Selectable(kHostMaps[i].name, i == m_hostMapIndex)) m_hostMapIndex = i;
            }
            ImGui::EndCombo();
        }
        ImGui::Text("Max players");
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt("##HostMaxPlayers", &m_hostMaxPlayers, 2, 16);
        ImGui::Text("Password (optional)");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##HostPassword", m_hostPassword, IM_ARRAYSIZE(m_hostPassword), ImGuiInputTextFlags_Password);
        ImGui::Checkbox("Open UDP 11774 with UPnP and list online", &m_hostOnline);
        NobleStyle::SectionGap();

        if (NobleStyle::PrimaryButton("START HOSTING", ImVec2(180, 34))) {
            HostConfig cfg;
            cfg.sessionName = m_hostSessionName[0] ? m_hostSessionName : "Reach game";
            cfg.gameMode = kHostModes[m_hostModeIndex];
            cfg.mapName = kHostMaps[m_hostMapIndex].name;
            cfg.gametypeName = m_hostModeIndex == 2 ? "Firefight" : "Slayer";
            cfg.maxPlayers = m_hostMaxPlayers;
            cfg.port = 11774;
            cfg.password = m_hostPassword;
            cfg.mode = m_hostOnline ? NetworkMode::Online_P2P : NetworkMode::Local_LAN;
            if (!SessionManager::Get().StartHosting(cfg)) {
                m_status = "Could not start hosting. Check the network status and try again.";
                ImGui::EndPopup();
                return;
            }

            if (m_hostOnline && MasterServerClient::Get().Enabled()) {
                MasterAdvert advert;
                advert.name = cfg.sessionName;
                advert.port = cfg.port;
                advert.maxPlayers = cfg.maxPlayers;
                advert.map = cfg.mapName;
                advert.mode = cfg.gameMode;
                advert.hasPassword = !cfg.password.empty();
                advert.isMod = (cfg.mapName.find(".map") != std::string::npos || cfg.sessionName.find("[Mod]") != std::string::npos);
                MasterServerClient::Get().StartAdvertising(advert);
            }
            if (m_hostOnline && !UPnP::Get().IsActive()) {
                m_status = "Hosting on LAN only: UPnP could not open UDP 11774, so the game was not listed online.";
            } else if (m_hostOnline && !MasterServerClient::Get().Enabled()) {
                m_status = "UDP 11774 is open, but no master server is configured.";
            } else if (m_hostOnline) {
                m_status = "Hosting \"" + cfg.sessionName + "\"; router reports public IP " +
                           UPnP::Get().GetExternalIP() + ":11774.";
            } else {
                m_status = "Hosting \"" + cfg.sessionName + "\" on local address " +
                           UPnP::Get().GetInternalIP() + ":11774.";
            }
            m_lastRefresh = -100.0;
            m_showHostModal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(110, 34))) m_showHostModal = false;
        ImGui::EndPopup();
    }
}

void ServerBrowser::RenderDirectConnectModal() {
    ImGui::OpenPopup("DIRECT CONNECT");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 0));
    if (ImGui::BeginPopupModal("DIRECT CONNECT", &m_showDirectConnectModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Address (IP or IP:port)");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75.0f);
        ImGui::InputText("##Address", m_directConnectIp, IM_ARRAYSIZE(m_directConnectIp));
        ImGui::SameLine();
        if (ImGui::Button("PASTE", ImVec2(65, 0))) {
            if (const char* clip = ImGui::GetClipboardText()) std::snprintf(m_directConnectIp, sizeof(m_directConnectIp), "%s", clip);
        }
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##Password", m_directConnectPassword, IM_ARRAYSIZE(m_directConnectPassword), ImGuiInputTextFlags_Password);
        // Paste button positioned inline above
        NobleStyle::SectionGap();

        std::string address;
        uint16_t port = 0;
        const bool valid = ParseAddress(m_directConnectIp, address, port);
        if (!valid) ImGui::TextColored(ImVec4(0.95f, 0.4f, 0.4f, 1.0f), "Enter an address such as 192.168.1.20:11774");
        ImGui::BeginDisabled(!valid);
        if (NobleStyle::PrimaryButton("CONNECT", ImVec2(120, 32))) {
            TryJoin(address, port, m_directConnectPassword[0] != 0);
            m_showDirectConnectModal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("SAVE AS FAVOURITE", ImVec2(170, 32))) {
            ToggleFavorite(address, port);
            m_lastRefresh = -100.0;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(90, 32))) m_showDirectConnectModal = false;
        ImGui::EndPopup();
    }
}

void ServerBrowser::RenderMasterServerModal() {
    ImGui::OpenPopup("MASTER SERVER");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(520, 0));
    if (ImGui::BeginPopupModal("MASTER SERVER", &m_showMasterModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("The master server lists online games. The default uses the Outbreak-Z Render service; clear the address to browse LAN games only.");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##MasterUrl", "https://outbreak-z-test-server.onrender.com", m_masterUrl, IM_ARRAYSIZE(m_masterUrl));
        NobleStyle::SectionGap();
        if (NobleStyle::PrimaryButton("SAVE", ImVec2(110, 32))) {
            SettingsData d = Settings::Get().Snapshot();
            d.masterServerUrl = m_masterUrl;
            Settings::Get().Update(d);
            MasterServerClient::Get().SetUrl(d.masterServerUrl);
            m_lastMasterRequest = -100.0;
            m_status = d.masterServerUrl.empty() ? "Master server cleared: LAN games only." : "Master server set to " + d.masterServerUrl;
            m_showMasterModal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(110, 32))) m_showMasterModal = false;
        ImGui::EndPopup();
    }
}

} // namespace noble
