#include <windows.h>
#include "ServerBrowser.hpp"
#include "common/MapNames.hpp"
#include "common/Relaunch.hpp"
#include "NobleStyle.hpp"
#include "UiArt.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/SessionManager.hpp"
#include "network/UPnP.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstring>

namespace noble {

namespace {

// Closes this process's game window so the menu copy of Noble exits once the match copy has started.
// GetActiveWindow() is not enough: the browser is drawn on the engine thread, which owns no window, so it
// returned NULL and the menu stayed open behind the match.
BOOL CALLBACK CloseIfOurWindow(HWND hwnd, LPARAM) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == GetCurrentProcessId() && IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == nullptr) {
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
    }
    return TRUE;
}

void CloseThisNobleWindow() {
    EnumWindows(&CloseIfOurWindow, 0);
}


std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
    return s;
}

// "1.2.3.4:5678" or "1.2.3.4" -> address + port (default 11774). False for empty/bad input.
bool ParseAddress(const std::string& text, std::string& address, uint16_t& port) {
    std::string t = text;
    auto stripPrefix = [&](const std::string& prefix) {
        if (t.size() >= prefix.size() && _strnicmp(t.c_str(), prefix.c_str(), prefix.size()) == 0) {
            t = t.substr(prefix.size());
        }
    };
    stripPrefix("/connect ");
    stripPrefix("-connect ");
    stripPrefix("connect ");
    stripPrefix("http://");
    stripPrefix("https://");
    t.erase(std::remove_if(t.begin(), t.end(), [](unsigned char c) { return std::isspace(c) != 0 || c == '"' || c == '\''; }), t.end());
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

    const std::string masterUrl = s.masterServerUrl.empty() ? "https://outbreak-z-test-server.onrender.com" : s.masterServerUrl;
    std::snprintf(m_masterUrl, sizeof(m_masterUrl), "%s", masterUrl.c_str());
    MasterServerClient::Get().SetUrl(masterUrl);
    m_directConnectIp[0] = '\0';
}

void ServerBrowser::RefreshServers() {
    const SettingsData settings = Settings::Get().Snapshot();
    std::vector<ServerEntry> list;
    auto add = [&](ServerEntry e) {
        if (e.hostAddress.empty() || e.maxPlayers <= 0 || e.mapName.empty()) return;
        for (auto& existing : list) {
            if (existing.hostAddress == e.hostAddress && existing.port == e.port) {
                if (existing.id.empty() && !e.id.empty()) existing.id = e.id;
                return;  // keep the first source, but preserve Master Server session ID if matched
            }
        }
        const std::string key = Key(e.hostAddress, e.port);
        e.isFavorite = std::find(settings.favorites.begin(), settings.favorites.end(), key) != settings.favorites.end();
        list.push_back(std::move(e));
    };

    // 1. The game this player is currently hosting from a pre-game lobby (if any).
    SessionManager& sessions = SessionManager::Get();
    if (sessions.IsHosting()) {
        const HostConfig& cfg = sessions.GetHostConfig();
        if (cfg.maxPlayers > 0 && !cfg.mapName.empty()) {
            ServerEntry e;
            e.name = cfg.sessionName;
            std::string pubIp = UPnP::Get().GetExternalIP();
            if (UPnP::Get().IsPublicIP(pubIp)) {
                e.hostAddress = pubIp;
            } else {
                e.hostAddress = UPnP::Get().GetInternalIP();
            }
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
    }

    // 2. LAN beacons from other live instances on the subnet.
    for (const auto& d : sessions.GetDiscoveredSessions()) {
        if (d.maxPlayers <= 0 || d.mapName.empty()) continue;
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

    // 3. Master server list of active online games.
    for (const auto& m : MasterServerClient::Get().Sessions()) {
        if (m.maxPlayers <= 0 || m.map.empty()) continue;
        bool mergedWithLan = false;
        for (auto& existing : list) {
            if ((existing.hostAddress == m.address || (!m.lanAddress.empty() && existing.hostAddress == m.lanAddress)) && existing.port == m.port) {
                if (existing.id.empty() && !m.id.empty()) existing.id = m.id;
                existing.inProgress = m.inProgress;
                mergedWithLan = true;
                break;
            }
        }
        if (mergedWithLan) continue;
        ServerEntry e;
        e.id = m.id;
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

    // Highlight the first server as soon as there is one, so GAME DETAILS (map picture, players, ping)
    // shows something without the player having to click first. Escape/back still leaves the screen.
    if (m_selectedServerIndex < 0 && !m_servers.empty()) m_selectedServerIndex = 0;
    if (m_selectedServerIndex >= static_cast<int>(m_servers.size())) m_selectedServerIndex = m_servers.empty() ? -1 : 0;

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
    if (m_showMasterModal) RenderMasterServerModal();
}

void ServerBrowser::RenderFilterBar() {
    SessionManager& sessions = SessionManager::Get();
    const bool hosting = sessions.IsHosting();

    if (hosting) {
        if (NobleStyle::DangerButton("STOP HOSTING", ImVec2(140, 0))) {
            sessions.StopHosting();
            MasterServerClient::Get().StopAdvertising();
            m_status = "Session closed. Stopped hosting.";
            RefreshServers();
        }
    }
    if (hosting) ImGui::SameLine();
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
        const std::string needle = Lower(m_searchQuery);
        const std::string name = Lower(s.name);
        // Match the raw scenario name and the name players read ("Sword Base" finds "20_sword_slayer").
        const std::string map = Lower(s.mapName + " " + MapNames::Display(s.mapName));
        if (name.find(needle) == std::string::npos && map.find(needle) == std::string::npos) return false;
    }
    if (m_modeFilter == 1 && s.gameMode != "Multiplayer") return false;
    if (m_modeFilter == 2 && s.gameMode != "Campaign") return false;
    if (m_modeFilter == 3 && s.gameMode != "Firefight") return false;
    if (m_modeFilter == 4 && s.gameMode != "Forge") return false;
    if (m_hideFull && s.maxPlayers > 0 && s.playerCount >= s.maxPlayers) return false;
    if (m_hideEmpty && s.playerCount <= 0) return false;
    if (m_showOnlyFavorites && !s.isFavorite) return false;
    return true;
}

void ServerBrowser::RenderServerList() {
    std::vector<int> visibleIndices;
    for (int i = 0; i < static_cast<int>(m_servers.size()); ++i) {
        if (PassesFilters(m_servers[i])) visibleIndices.push_back(i);
    }

    if (!ImGui::GetIO().WantTextInput) {
        // Controller D-Pad & Left Stick navigation
        if (!visibleIndices.empty()) {
            int currentPos = -1;
            for (size_t k = 0; k < visibleIndices.size(); ++k) {
                if (visibleIndices[k] == m_selectedServerIndex) { currentPos = static_cast<int>(k); break; }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown)) {
                currentPos = (currentPos + 1) % static_cast<int>(visibleIndices.size());
                m_selectedServerIndex = visibleIndices[currentPos];
            }
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp)) {
                currentPos = (currentPos - 1 + static_cast<int>(visibleIndices.size())) % static_cast<int>(visibleIndices.size());
                m_selectedServerIndex = visibleIndices[currentPos];
            }
            if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false)) {
                if (m_selectedServerIndex >= 0 && m_selectedServerIndex < static_cast<int>(m_servers.size())) {
                    const auto& s = m_servers[m_selectedServerIndex];
                    if (!s.isOwn) {
                        if (!s.inProgress && m_joinLobbyHandler) m_joinLobbyHandler(s);
                        else TryJoin(s.hostAddress, s.port, s.isPasswordProtected);
                    }
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceUp, false)) {
                if (m_selectedServerIndex >= 0 && m_selectedServerIndex < static_cast<int>(m_servers.size())) {
                    ToggleFavorite(m_servers[m_selectedServerIndex].hostAddress, m_servers[m_selectedServerIndex].port);
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft, false)) {
            m_lastMasterRequest = -100.0;
            m_lastRefresh = -100.0;
            SessionManager::Get().TriggerDiscoveryScan();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false)) {
            m_modeFilter = (m_modeFilter + 4) % 5;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false)) {
            m_modeFilter = (m_modeFilter + 1) % 5;
        }
    }

    static const ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable;

    if (!ImGui::BeginTable("ServerTable", 7, flags, ImVec2(0, 0))) return;

    // Widths include the cell padding and the sort arrow, so no header is elided ("Sour...", "Playe...").
    ImGui::TableSetupColumn("Source",   ImGuiTableColumnFlags_WidthFixed, 86.0f);
    ImGui::TableSetupColumn("Server",   ImGuiTableColumnFlags_WidthStretch, 200.0f);
    ImGui::TableSetupColumn("Mode",     ImGuiTableColumnFlags_WidthFixed, 112.0f);
    ImGui::TableSetupColumn("Map",      ImGuiTableColumnFlags_WidthFixed, 140.0f);
    ImGui::TableSetupColumn("Players",  ImGuiTableColumnFlags_WidthFixed, 98.0f);
    ImGui::TableSetupColumn("Ping",     ImGuiTableColumnFlags_WidthFixed, 70.0f);
    ImGui::TableSetupColumn("Address",  ImGuiTableColumnFlags_WidthFixed, 170.0f);
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();

    int visibleIndex = 0;
    for (int i = 0; i < static_cast<int>(m_servers.size()); ++i) {
        const ServerEntry& s = m_servers[i];
        if (!PassesFilters(s)) continue;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        const bool selected = (m_selectedServerIndex == i);

        // Column 0: source tag
        if (s.isOwn) {
            ImGui::TextColored(NobleStyle::Accent(), "YOU");
        } else if (s.isFavorite) {
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "FAV");
        } else {
            ImGui::TextDisabled("%s", s.source.c_str());
        }

        // Column 1: name (selectable across row)
        ImGui::TableNextColumn();
        char label[128];
        std::snprintf(label, sizeof(label), "%s##row%d", s.name.c_str(), i);
        if (ImGui::Selectable(label, selected, ImGuiSelectableFlags_SpanAllColumns)) {
            m_selectedServerIndex = i;
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && !s.isOwn) {
            if (!s.inProgress && m_joinLobbyHandler) {
                m_joinLobbyHandler(s);
            } else {
                TryJoin(s.hostAddress, s.port, s.isPasswordProtected);
            }
        }

        // Column 2: mode
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(s.gameMode.c_str());

        // Column 3: map, by the name players know it by ("Forge World", not "forge_halo"). Modded maps keep
        // their own name and the [Mod] marker.
        ImGui::TableNextColumn();
        const std::string mapLabel = MapNames::Display(s.mapName);
        if (s.isMod) {
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.95f, 1.0f), "[Mod] %s", mapLabel.c_str());
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", s.mapName.c_str());
        } else {
            ImGui::TextUnformatted(mapLabel.c_str());
        }

        // Column 4: players
        ImGui::TableNextColumn();
        ImGui::Text("%d / %d", s.playerCount, s.maxPlayers);

        // Column 5: ping
        ImGui::TableNextColumn();
        if (s.ping >= 0) {
            const ImVec4 color = s.ping < 60 ? ImVec4(0.35f, 0.85f, 0.45f, 1.0f)
                               : s.ping < 120 ? ImVec4(0.95f, 0.85f, 0.35f, 1.0f)
                                              : ImVec4(0.95f, 0.40f, 0.35f, 1.0f);
            ImGui::TextColored(color, "%d", s.ping);
        } else {
            ImGui::TextDisabled("-");
        }

        // Column 6: address
        ImGui::TableNextColumn();
        ImGui::TextDisabled("%s:%u", s.hostAddress.c_str(), s.port);

        ++visibleIndex;
    }

    if (visibleIndex == 0) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::TextDisabled("No servers found matching current filters.");
    }

    ImGui::EndTable();
}

void ServerBrowser::RenderServerDetails() {
    if (m_selectedServerIndex < 0 || m_selectedServerIndex >= static_cast<int>(m_servers.size())) {
        NobleStyle::Header("GAME DETAILS");
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped("Select a server from the list to see its map, players, ping and mods.");
        ImGui::Spacing();
        ImGui::TextWrapped("Double-click a server to join it.");
        ImGui::Spacing();
        ImGui::TextWrapped("%d server(s) in the list.", static_cast<int>(m_servers.size()));
        ImGui::PopStyleColor();
        return;
    }

    const ServerEntry& s = m_servers[m_selectedServerIndex];
    NobleStyle::Header("GAME DETAILS");
    ImGui::Spacing();

    // Map picture: MCC's own preview for a vanilla map, or the photo the map/mod ship as
    // data\ui\custom\loading\<map>.*. Modded maps without one get a named placeholder instead of a
    // picture of a different map (2026-10-30).
    {
        const float photoW = ImGui::GetContentRegionAvail().x;
        const float photoH = photoW * 9.0f / 16.0f;
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const ImVec2 p1(p0.x + photoW, p0.y + photoH);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(p0, p1, IM_COL32(10, 15, 22, 255));
        if (const UiTexture photo = UiArt::MapPhoto(s.mapName)) {
            UiArt::DrawCover(dl, photo, p0, p1);
        } else {
            const char* missing = s.isMod ? "NO PICTURE FROM THIS MOD" : "NO MAP PICTURE";
            const ImVec2 ts = ImGui::CalcTextSize(missing);
            dl->AddText(ImVec2(p0.x + (photoW - ts.x) * 0.5f, p0.y + (photoH - ts.y) * 0.5f),
                        IM_COL32(120, 135, 150, 200), missing);
        }
        // Scrim under the map name so it stays readable on a bright picture.
        dl->AddRectFilledMultiColor(ImVec2(p0.x, p1.y - photoH * 0.5f), p1,
                                    IM_COL32(4, 8, 12, 0), IM_COL32(4, 8, 12, 0),
                                    IM_COL32(4, 8, 12, 230), IM_COL32(4, 8, 12, 230));
        dl->AddRect(p0, p1, IM_COL32(150, 185, 215, 110), 0.0f, 0, 1.0f);
        const std::string mapName = s.mapName.empty() ? std::string("Unknown map") : MapNames::Display(s.mapName);
        const char* mapLabel = mapName.c_str();
        dl->AddText(ImGui::GetFont(), 20.0f, ImVec2(p0.x + 10.0f, p1.y - 28.0f), IM_COL32(240, 244, 248, 255), mapLabel);
        ImGui::Dummy(ImVec2(photoW, photoH));
    }

    ImGui::Spacing();
    ImGui::TextWrapped("%s", s.name.c_str());
    ImGui::TextDisabled("%s:%u%s", s.hostAddress.c_str(), s.port, s.isOwn ? " (YOU)" : "");
    ImGui::Spacing();

    ImGui::TextDisabled("Mode:");
    ImGui::SameLine(100);
    ImGui::TextUnformatted(s.gameMode.c_str());

    ImGui::TextDisabled("Map:");
    ImGui::SameLine(100);
    ImGui::TextUnformatted(s.mapName.c_str());

    if (!s.variantName.empty()) {
        ImGui::TextDisabled("Game type:");
        ImGui::SameLine(100);
        ImGui::TextUnformatted(s.variantName.c_str());
    }

    ImGui::TextDisabled("Players:");
    ImGui::SameLine(100);
    ImGui::Text("%d / %d", s.playerCount, s.maxPlayers);

    ImGui::TextDisabled("Ping:");
    ImGui::SameLine(100);
    if (s.ping >= 0) {
        const ImVec4 color = s.ping < 60 ? ImVec4(0.35f, 0.85f, 0.45f, 1.0f)
                           : s.ping < 120 ? ImVec4(0.95f, 0.85f, 0.35f, 1.0f)
                                          : ImVec4(0.95f, 0.40f, 0.35f, 1.0f);
        ImGui::TextColored(color, "%d ms", s.ping);
    } else {
        ImGui::TextDisabled("-");
    }

    // Status lines wrap: the details panel is only 300 px wide, so a long line used to be cut at the edge.
    if (s.isPasswordProtected) ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "Password protected");
    if (s.inProgress) {
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped("Match in progress - joining in progress is allowed.");
        ImGui::PopStyleColor();
    }
    if (s.isMod) ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.95f, 1.0f), "Custom map / modded content");

    NobleStyle::SectionGap();

    if (s.isOwn) {
        ImGui::TextDisabled("You are hosting this game.");
        if (NobleStyle::DangerButton("STOP HOSTING", ImVec2(-1, 34))) {
            SessionManager::Get().StopHosting();
            MasterServerClient::Get().StopAdvertising();
            m_status = "Session closed. Stopped hosting.";
            RefreshServers();
        }
    } else {
        const char* joinLabel = s.inProgress ? "JOIN IN PROGRESS" : "JOIN PRE-GAME LOBBY";
        if (NobleStyle::PrimaryButton(joinLabel, ImVec2(-1, 38))) {
            if (!s.inProgress && m_joinLobbyHandler) {
                m_joinLobbyHandler(s);
            } else {
                TryJoin(s.hostAddress, s.port, s.isPasswordProtected);
            }
        }
    }

    const float halfW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
    if (ImGui::Button(s.isFavorite ? "UNFAVOURITE" : "FAVOURITE", ImVec2(halfW, 30))) {
        ToggleFavorite(s.hostAddress, s.port);
        RefreshServers();
    }
    ImGui::SameLine();
    if (ImGui::Button("COPY INVITE", ImVec2(halfW, 30))) {
        std::string ipToCopy = s.hostAddress;
        if (s.isOwn) {
            std::string pubIp = UPnP::Get().GetExternalIP();
            if (UPnP::Get().IsPublicIP(pubIp)) ipToCopy = pubIp;
        }
        const std::string invite = ipToCopy + ":" + std::to_string(s.port);
        ImGui::SetClipboardText(invite.c_str());
        m_status = "Copied invite address " + invite + " to clipboard.";
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
    const std::string key = Key(address, port);

    std::string map = "20_sword_slayer";
    std::string mode = "multiplayer";
    std::string sessionId = "";

    for (const auto& s : m_servers) {
        if (s.hostAddress == address && s.port == port) {
            if (!s.mapName.empty()) map = s.mapName;
            if (!s.gameMode.empty()) mode = s.gameMode;
            sessionId = s.id;
            break;
        }
    }

    std::transform(mode.begin(), mode.end(), mode.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (m_directJoinHandler) {
        m_status = "Connecting to " + key + "...";
        NOBLE_INFO("ServerBrowser: Direct join to {}:{} (map='{}', session='{}')", address, port, map, sessionId);
        m_directJoinHandler(map, mode, address + ":" + std::to_string(port), sessionId);
        return;
    }

    // Pass -session-id to the relaunched match process so the child process calls JoinSessionSync
    // with its own generated Guest Machine ID and bound UDP port after NetworkTransport::Initialize.
    std::wstring args = L"-map \"" + std::wstring(map.begin(), map.end()) + L"\"";
    args += L" -mode \"" + std::wstring(mode.begin(), mode.end()) + L"\"";
    args += L" -connect \"" + std::wstring(address.begin(), address.end()) + L":" + std::to_wstring(port) + L"\"";
    if (!sessionId.empty()) {
        args += L" -session-id \"" + std::wstring(sessionId.begin(), sessionId.end()) + L"\"";
    }

    m_status = "Connecting to " + key + "...";
    NOBLE_INFO("ServerBrowser: Connecting to {}:{} (map='{}', session='{}') via Relaunch", address, port, map, sessionId);
    Relaunch::HandOff(args);  // ends this process; returns only if the new one could not start
    CloseThisNobleWindow();
}

void ServerBrowser::RenderDirectConnectModal() {
    ImGui::OpenPopup("DIRECT CONNECT");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 0));
    if (ImGui::BeginPopupModal("DIRECT CONNECT", &m_showDirectConnectModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Address (IP or IP:port)");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75.0f);
        ImGui::InputTextWithHint("##Address", "IP or IP:port (e.g. 147.124.149.202:11774)", m_directConnectIp, IM_ARRAYSIZE(m_directConnectIp));
        ImGui::SameLine();
        if (ImGui::Button("PASTE", ImVec2(65, 0))) {
            if (const char* clip = ImGui::GetClipboardText()) std::snprintf(m_directConnectIp, sizeof(m_directConnectIp), "%s", clip);
        }
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##Password", m_directConnectPassword, IM_ARRAYSIZE(m_directConnectPassword), ImGuiInputTextFlags_Password);
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
