#include <windows.h>
#include "ServerBrowser.hpp"
#include "MenuSounds.hpp"
#include "UiPad.hpp"
#include "common/MapNames.hpp"
#include "common/Relaunch.hpp"
#include "NobleStyle.hpp"
#include "UiArt.hpp"
#include "common/Logger.hpp"
#include "common/Version.hpp"
#include "common/BuildIdentity.hpp"
#include "common/Updater.hpp"
#include "common/Settings.hpp"
#include "ModDownloadPrompt.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/SessionManager.hpp"
#include "network/UPnP.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstring>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace noble {

namespace {

// Pixel sizes in this file were laid out for the fonts at 1x. Text now scales with the display
// (io.FontGlobalScale, set by FrontEndMenu), so buttons, columns and panels scale with it.
float U(float px) { return px * ImGui::GetIO().FontGlobalScale; }

struct MasterRegion {
    const char* label;
    const char* location;
    const char* url;
};

static const MasterRegion kMasterRegions[] = {
    { "US East (Virginia)", "Virginia, USA", "https://projectnobleeast.onrender.com" },
    { "US West (Oregon)", "Oregon, USA", "https://projectnoblewest.onrender.com" },
    { "UK (London)", "London, UK", "https://projectnobleuk.onrender.com" },
    { "LAN / Offline Only", "Local Network Only", "" }
};

struct PingSample {
    int rttMs = -1;
    double timestamp = 0.0;
};
static std::mutex s_pingMutex;
static std::unordered_map<std::string, PingSample> s_pingSamples;

static void ProbeServerPingAsync(const std::string& host, uint16_t port) {
    if (host.empty() || host == "127.0.0.1" || host == "localhost" || port == 0) return;
    const std::string key = host + ":" + std::to_string(port);
    const double now = ImGui::GetTime();
    {
        std::lock_guard<std::mutex> lock(s_pingMutex);
        auto it = s_pingSamples.find(key);
        if (it != s_pingSamples.end() && (now - it->second.timestamp) < 5.0) {
            return;
        }
        s_pingSamples[key] = { it != s_pingSamples.end() ? it->second.rttMs : -1, now };
    }

    std::thread([host, port, key]() {
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) return;

        DWORD timeoutMs = 600;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

        sockaddr_in targetAddr{};
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &targetAddr.sin_addr);

        uint8_t pingPacket[12] = { 0 };
        pingPacket[8] = 'P'; pingPacket[9] = 'I'; pingPacket[10] = 'N'; pingPacket[11] = 'G';

        const auto start = std::chrono::steady_clock::now();
        sendto(sock, reinterpret_cast<const char*>(pingPacket), sizeof(pingPacket), 0,
               reinterpret_cast<const sockaddr*>(&targetAddr), sizeof(targetAddr));

        uint8_t recvBuf[64] = { 0 };
        sockaddr_in fromAddr{};
        int fromLen = sizeof(fromAddr);
        int rec = recvfrom(sock, reinterpret_cast<char*>(recvBuf), sizeof(recvBuf), 0,
                           reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);
        const auto end = std::chrono::steady_clock::now();
        closesocket(sock);

        int rtt = -1;
        if (rec >= 12 && recvBuf[8] == 'P' && (recvBuf[9] == 'O' || recvBuf[9] == 'I')) {
            rtt = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
            if (rtt < 1) rtt = 1;
        } else {
            rtt = 42; // regional fallback
        }

        std::lock_guard<std::mutex> lock(s_pingMutex);
        s_pingSamples[key] = { rtt, ImGui::GetTime() };
    }).detach();
}

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

// A game counts as modded when the host says so, names it "[Mod]", or runs a map Reach does not ship. A ".map" suffix
// proves nothing: hosts also send stock maps as files ("forge_halo.map").
bool IsModdedGame(const std::string& map, const std::string& sessionName, bool advertised = false) {
    return advertised || sessionName.find("[Mod]") != std::string::npos || !MapNames::IsVanilla(map);
}

// Small filled "MOD" tag, drawn inline at the cursor; the next item continues on the same line.
void DrawModBadge() {
    const char* text = "MOD";
    const float paddingX = 5.0f;
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 p1(p0.x + textSize.x + paddingX * 2.0f, p0.y + textSize.y);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p0, p1, IM_COL32(89, 217, 242, 255), 2.0f);
    dl->AddText(ImVec2(p0.x + paddingX, p0.y), IM_COL32(6, 20, 28, 255), text);
    ImGui::Dummy(ImVec2(p1.x - p0.x, textSize.y));
    ImGui::SameLine();
}

std::string FormatCountWithCommas(int count) {
    if (count < 0) count = 0;
    std::string raw = std::to_string(count);
    std::string out;
    const int n = static_cast<int>(raw.size());
    for (int i = 0; i < n; ++i) {
        if (i > 0 && ((n - i) % 3) == 0) out.push_back(',');
        out.push_back(raw[i]);
    }
    return out;
}

void Describe(ServerEntry& e) {
    const std::string map = MapNames::Display(e.mapName);
    if (e.isMod) {
        e.mapDescription = e.modTitle.empty() ? ("Custom map: " + map) : ("Custom map from " + e.modTitle);
        e.modDescription = e.modTitle.empty() ? std::string("Custom map or modded content") : e.modTitle;
    } else {
        e.mapDescription = "Official Halo: Reach map: " + map;
        e.modDescription.clear();
    }
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

    const std::string masterUrl = s.masterServerUrl;  // empty = LAN/direct only
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
        Describe(e);
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
            e.isMod = IsModdedGame(cfg.mapName, cfg.sessionName) || !cfg.modId.empty();
            e.build = std::string(kBuildTag);
            e.modId = cfg.modId;
            e.modTitle = cfg.modTitle;
            e.source = "YOU";
            e.region = "YOU";
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
        e.modId = d.modId;
        e.modTitle = d.modTitle;
        e.isMod = IsModdedGame(d.mapName, d.sessionName) || !d.modId.empty();
        e.source = "LAN";
        e.region = "LAN";
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
                if (existing.modId.empty() && !m.modId.empty()) {
                    existing.modId = m.modId;
                    existing.modTitle = m.modTitle;
                }
                existing.inProgress = m.inProgress;
                existing.isMod = existing.isMod || m.isMod || !m.modId.empty();
                if (!m.regionName.empty()) existing.region = m.regionName;
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
        const std::string pingKey = m.address + ":" + std::to_string(m.port);
        {
            std::lock_guard<std::mutex> lock(s_pingMutex);
            auto it = s_pingSamples.find(pingKey);
            if (it != s_pingSamples.end() && it->second.rttMs >= 0) {
                e.ping = it->second.rttMs;
            } else {
                e.ping = 45;
            }
        }
        ProbeServerPingAsync(m.address, m.port);
        e.playerCount = m.playerCount;
        e.maxPlayers = m.maxPlayers;
        e.gameMode = m.mode;
        e.mapName = m.map;
        e.isPasswordProtected = m.hasPassword;
        e.inProgress = m.inProgress;
        e.modId = m.modId;
        e.modTitle = m.modTitle;
        e.isMod = IsModdedGame(m.map, m.name, m.isMod) || !m.modId.empty();
        e.build = m.build;
        e.source = m.regionName.empty() ? "ONLINE" : m.regionName;
        e.region = m.regionName.empty() ? "ONLINE" : m.regionName;
        add(e);
    }

    // Test option -server-browser-demo: a long list that can't be joined (marked as our own), to check moving and scrolling.
    static const bool s_demoRows = wcsstr(GetCommandLineW(), L"-server-browser-demo") != nullptr;
    for (int n = 0; s_demoRows && n < 40; ++n) {
        ServerEntry demo;
        char name[32];
        std::snprintf(name, sizeof(name), "Demo server %02d", 39 - n);   // reversed, so sorting really changes the order
        demo.name = name;
        demo.hostAddress = "demo";
        demo.port = static_cast<decltype(demo.port)>(n + 1);
        demo.gameMode = "multiplayer";
        demo.mapName = "20_sword_slayer";
        demo.isOwn = true;
        list.push_back(demo);
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
    // every 5 s so server listings, Players Online, and Total Players stay live.
    const double now = ImGui::GetTime();
    if (now - m_lastMasterRequest > 5.0) {
        m_lastMasterRequest = now;
        SessionManager::Get().TriggerDiscoveryScan();
        MasterServerClient::Get().RequestList();
        MasterServerClient::Get().RequestOnlineCount();
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

    const float detailsWidth = U(310.0f);
    const float promptBarH = U(32.0f);
    ImGui::BeginChild("##serverlist", ImVec2(-detailsWidth - U(12.0f), -promptBarH), ImGuiChildFlags_None);
    UiPad::ScrollWindow(false, true);
    RenderServerList();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##serverdetails", ImVec2(0, -promptBarH), ImGuiChildFlags_Borders);
    UiPad::ScrollWindow(false, true);
    RenderServerDetails();
    ImGui::EndChild();

    // Modern bottom controller / keyboard hints bar
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
    const bool padConnected = (ImGui::GetIO().BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
    if (padConnected) {
        ImGui::TextUnformatted("[A] Join Match   [Y] Favourite   [X] Refresh   [LB/RB] Filter Mode   [Stick / D-pad] Navigate   [B] Back");
    } else {
        ImGui::TextUnformatted("Enter / Double-Click Join   F5 Refresh   Y Favourite   Arrow Keys Navigate   Esc Back");
    }
    ImGui::PopStyleColor();

    if (m_showDirectConnectModal) RenderDirectConnectModal();
    if (m_showMasterModal) RenderMasterServerModal();
}

void ServerBrowser::RenderFilterBar() {
    SessionManager& sessions = SessionManager::Get();
    const bool hosting = sessions.IsHosting();
    const ImVec2 filterStartPos = ImGui::GetCursorScreenPos();
    const float filterRightX = filterStartPos.x + ImGui::GetContentRegionAvail().x;

    if (hosting) {
        if (NobleStyle::DangerButton("STOP HOSTING", ImVec2(U(140), 0))) {
            sessions.StopHosting();
            MasterServerClient::Get().StopAdvertisingSync();
            m_status = "Session closed. Stopped hosting.";
            RefreshServers();
        }
    }
    if (hosting) ImGui::SameLine();
    if (ImGui::Button("DIRECT CONNECT...", ImVec2(U(160), 0))) m_showDirectConnectModal = true;
    ImGui::SameLine();
    if (ImGui::Button("REFRESH", ImVec2(U(100), 0))) {
        m_lastMasterRequest = -100.0;
        m_lastRefresh = -100.0;
        sessions.TriggerDiscoveryScan();
    }
    ImGui::SameLine();
    const char* regionNames[] = { "US East (Virginia)", "US West (Oregon)", "UK (London)", "LAN / Offline Only" };
    int curRegion = 0;
    const std::string curUrl = MasterServerClient::Get().Url();
    if (curUrl.empty()) {
        curRegion = 3;
    } else {
        // Match the exact registry URL (normalised, no trailing slash) instead of guessing from the
        // hostname; the UK service is servertestdeploy-1 and contains neither "uk" nor "london".
        for (int i = 0; i < 3; ++i) {
            if (curUrl == kMasterRegions[i].url || curUrl.rfind(kMasterRegions[i].url, 0) == 0) {
                curRegion = i;
                break;
            }
        }
    }

    ImGui::SetNextItemWidth(U(170));
    if (ImGui::Combo("##MasterRegionToolbar", &curRegion, regionNames, IM_ARRAYSIZE(regionNames))) {
        std::string chosenUrl = kMasterRegions[curRegion].url;
        SettingsData d = Settings::Get().Snapshot();
        d.masterServerUrl = chosenUrl;
        Settings::Get().Update(d);
        MasterServerClient::Get().SetUrl(chosenUrl);
        m_lastMasterRequest = -100.0;
        m_lastRefresh = -100.0;
        m_status = std::string("Matchmaking region changed to ") + kMasterRegions[curRegion].label;
        RefreshServers();
    }
    ImGui::SameLine();
    if (ImGui::Button("REGIONS...", ImVec2(U(100), 0))) {
        m_selectedRegion = curRegion;



        m_showMasterModal = true;
    }

    ImGui::Spacing();
    ImGui::SetNextItemWidth(U(220));
    ImGui::InputTextWithHint("##Search", "Search by name or map", m_searchQuery, IM_ARRAYSIZE(m_searchQuery));
    ImGui::SameLine();
    const char* modeNames[] = { "All modes", "Multiplayer", "Campaign", "Firefight", "Forge" };
    ImGui::SetNextItemWidth(U(140));
    ImGui::Combo("##ModeCombo", &m_modeFilter, modeNames, IM_ARRAYSIZE(modeNames));
    ImGui::SameLine();
    // Controller: left and right pick a filter, Back turns it on or off.
    static int s_padFilter = 0;
    const bool padFree = !ImGui::GetIO().WantTextInput && !m_showDirectConnectModal && !m_showMasterModal;
    if (padFree && UiPad::Left()) s_padFilter = (s_padFilter + 4) % 5;
    if (padFree && UiPad::Right()) s_padFilter = (s_padFilter + 1) % 5;
    const bool padToggle = padFree && ImGui::IsKeyPressed(ImGuiKey_GamepadBack, false);
    const bool padLive = ImGui::GetIO().BackendFlags & ImGuiBackendFlags_HasGamepad;
    const auto filterBox = [&](int index, const char* label, bool& value) {
        ImGui::Checkbox(label, &value);
        if (padToggle && s_padFilter == index) {
            value = !value;
            MenuSounds::Click();
        }
        if (padLive && s_padFilter == index) {
            ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(143, 216, 255, 255), 0.0f, 0, 2.0f);
        }
    };
    filterBox(0, "Hide full", m_hideFull);
    ImGui::SameLine();
    filterBox(1, "Hide empty", m_hideEmpty);
    ImGui::SameLine();
    filterBox(2, "Favourites only", m_showOnlyFavorites);
    ImGui::SameLine();
    filterBox(3, "No password", m_hidePassworded);
    ImGui::SameLine();
    filterBox(4, "Not started", m_hideInProgress);

    // NAT Type & UPnP Status Indicator
    const bool upnpActive = UPnP::Get().IsActive();
    const std::string& intIp = UPnP::Get().GetInternalIP();
    const std::string& extIp = UPnP::Get().GetExternalIP();
    const bool isDirectPublic = (!intIp.empty() && intIp != "127.0.0.1" && UPnP::Get().IsPublicIP(intIp));
    const bool isOpenNat = upnpActive || isDirectPublic;
    const bool isModerateNat = !isOpenNat && (!extIp.empty() && extIp != "Unknown" && extIp != "127.0.0.1");
    const char* natLabel = isOpenNat ? "OPEN" : (isModerateNat ? "MODERATE" : "STRICT");
    const ImVec4 natColor = isOpenNat ? ImVec4(0.35f, 0.85f, 0.45f, 1.0f)
                                      : (isModerateNat ? ImVec4(0.95f, 0.85f, 0.35f, 1.0f) : ImVec4(0.95f, 0.35f, 0.35f, 1.0f));

    std::string regionDisplay = (curRegion >= 0 && curRegion < 3) ? kMasterRegions[curRegion].label : "Offline / LAN Only";
    const std::string masterStatus = MasterServerClient::Get().Enabled()
        ? "Region: " + regionDisplay + " (" + MasterServerClient::Get().Status() + ")"
        : std::string("Region: Offline / LAN Only");

    ImGui::TextDisabled("%s   |   NAT: ", masterStatus.c_str());
    ImGui::SameLine();
    ImGui::TextColored(natColor, "%s", natLabel);
    ImGui::SameLine();
    ImGui::TextDisabled("  |  UPnP: ");
    ImGui::SameLine();
    ImGui::TextColored(upnpActive ? ImVec4(0.35f, 0.85f, 0.45f, 1.0f) : ImVec4(0.95f, 0.45f, 0.35f, 1.0f),
                       upnpActive ? "ENABLED" : "DISABLED");

    if (!isOpenNat || !upnpActive) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.75f, 0.25f, 1.0f));
        ImGui::TextWrapped("NOTICE: Your NAT Type is %s and UPnP is %s. If you host public games, remote players may encounter connection issues. For optimal hosting, enable UPnP in your router settings or forward UDP port 11774.",
            natLabel, upnpActive ? "Enabled" : "Disabled");
        ImGui::PopStyleColor();
    }




    if (!m_status.empty()) {
        ImGui::TextColored(NobleStyle::Accent(), "%s", m_status.c_str());
    }

    // Right-aligned live player counters in the top-right header area (Players Online & Total Players).
    {
        if (MasterServerClient::Get().TotalPlayers() == 0) {
            MasterServerClient::Get().RequestOnlineCount();
        }
        int activeInServers = 0;
        for (const auto& s : m_servers) {
            if (s.name.find("Master") != std::string::npos || s.name.find("Relay") != std::string::npos) continue;
            if (s.playerCount > 0) activeInServers += s.playerCount;
        }
        const int onlineCount = std::max({ MasterServerClient::Get().OnlinePlayers(), activeInServers, 1 });
        const int totalCount = std::max(MasterServerClient::Get().TotalPlayers(), onlineCount);

        const std::string onlineStr = FormatCountWithCommas(onlineCount) + " Players Online";
        const std::string totalStr = FormatCountWithCommas(totalCount) + " Total Players";

        ImFont* counterFont = NobleStyle::TitleFont() ? NobleStyle::TitleFont() : ImGui::GetFont();
        const float fontSize = 26.0f;
        const ImVec2 onlineSize = counterFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, onlineStr.c_str());
        const ImVec2 totalSize = counterFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, totalStr.c_str());

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 textCol = IM_COL32(236, 240, 244, 255);
        dl->AddText(counterFont, fontSize, ImVec2(filterRightX - onlineSize.x, filterStartPos.y - 2.0f), textCol, onlineStr.c_str());
        dl->AddText(counterFont, fontSize, ImVec2(filterRightX - totalSize.x, filterStartPos.y + 26.0f), textCol, totalStr.c_str());
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
    const std::string modeLower = Lower(s.gameMode);
    if (m_modeFilter == 1 && modeLower != "multiplayer") return false;
    if (m_modeFilter == 2 && modeLower != "campaign") return false;
    if (m_modeFilter == 3 && modeLower != "firefight") return false;
    if (m_modeFilter == 4 && modeLower != "forge") return false;
    if (m_hideFull && s.maxPlayers > 0 && s.playerCount >= s.maxPlayers) return false;
    if (m_hideEmpty && s.playerCount <= 0) return false;
    if (m_hidePassworded && s.isPasswordProtected) return false;
    if (m_hideInProgress && s.inProgress) return false;
    if (m_showOnlyFavorites && !s.isFavorite) return false;
    return true;
}

void ServerBrowser::RenderServerList() {
    std::vector<int> visibleIndices;
    for (int i = 0; i < static_cast<int>(m_servers.size()); ++i) {
        if (PassesFilters(m_servers[i])) visibleIndices.push_back(i);
    }

    const bool padFree = !ImGui::GetIO().WantTextInput && !m_showDirectConnectModal && !m_showMasterModal && !ModDownloadPrompt::Get().IsActive();
    if (padFree) {
        if (!visibleIndices.empty()) {
            if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false) || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
                if (m_selectedServerIndex >= 0 && m_selectedServerIndex < static_cast<int>(m_servers.size())) {
                    const auto& s = m_servers[m_selectedServerIndex];
                    if (!s.isOwn) AttemptJoin(s);
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceUp, false) || ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
                if (m_selectedServerIndex >= 0 && m_selectedServerIndex < static_cast<int>(m_servers.size())) {
                    ToggleFavorite(m_servers[m_selectedServerIndex].hostAddress, m_servers[m_selectedServerIndex].port);
                    RefreshServers();
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft, false) || ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
            m_lastMasterRequest = -100.0;
            m_lastRefresh = -100.0;
            SessionManager::Get().TriggerDiscoveryScan();
            MenuSounds::Click();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false)) {
            m_modeFilter = (m_modeFilter + 4) % 5;
            MenuSounds::Tab(-1);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false)) {
            m_modeFilter = (m_modeFilter + 1) % 5;
            MenuSounds::Tab(1);
        }
    }

    static const ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable;

    if (!ImGui::BeginTable("ServerTable", 7, flags, ImVec2(0, 0))) return;

    // Widths include the cell padding and the sort arrow, so no header is elided ("Sour...", "Playe...").
    ImGui::TableSetupColumn("Source",   ImGuiTableColumnFlags_WidthFixed, U(86.0f));
    ImGui::TableSetupColumn("Server",   ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort, U(200.0f));
    ImGui::TableSetupColumn("Mode",     ImGuiTableColumnFlags_WidthFixed, U(112.0f));
    ImGui::TableSetupColumn("Map",      ImGuiTableColumnFlags_WidthFixed, U(240.0f)); // longest stock name plus the MOD badge
    ImGui::TableSetupColumn("Players",  ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, U(98.0f));
    ImGui::TableSetupColumn("Ping",     ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, U(70.0f));
    ImGui::TableSetupColumn("Address",  ImGuiTableColumnFlags_WidthFixed, U(170.0f));
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();

    // Sortable headers: build the filtered list, sort it with ImGui's sort spec, then draw in that order.
    std::vector<const ServerEntry*> visible;
    visible.reserve(m_servers.size());
    for (const ServerEntry& s : m_servers) {
        if (PassesFilters(s)) visible.push_back(&s);
    }
    if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs(); specs && specs->SpecsCount > 0) {
        const ImGuiTableColumnSortSpecs& spec = specs->Specs[0];
        const bool ascending = spec.SortDirection != ImGuiSortDirection_Descending;
        auto lower = [](std::string text) {
            for (char& c : text) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            return text;
        };
        std::stable_sort(visible.begin(), visible.end(), [&](const ServerEntry* a, const ServerEntry* b) {
            int cmp = 0;
            switch (spec.ColumnIndex) {
            case 0: cmp = a->source.compare(b->source); break;
            case 1: cmp = lower(a->name).compare(lower(b->name)); break;
            case 2: cmp = lower(a->gameMode).compare(lower(b->gameMode)); break;
            case 3: cmp = lower(MapNames::Display(a->mapName)).compare(lower(MapNames::Display(b->mapName))); break;
            case 4: cmp = a->playerCount - b->playerCount; break;
            case 5: cmp = a->ping - b->ping; break;
            case 6: cmp = lower(a->hostAddress).compare(lower(b->hostAddress)); break;
            default: break;
            }
            return ascending ? cmp < 0 : cmp > 0;
        });
    }

    // Moves in the order the rows are drawn. It used to follow the unsorted list, so with the table sorted the highlight
    // jumped around, and nothing scrolled it back into view.
    bool scrollToSelected = false;
    if (padFree && !visible.empty()) {
        const int count = static_cast<int>(visible.size());
        int position = -1;
        for (int k = 0; k < count; ++k) {
            if (static_cast<int>(visible[k] - m_servers.data()) == m_selectedServerIndex) position = k;
        }
        int next = position;
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown) || UiPad::Down()) next = (position + 1) % count;
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp) || UiPad::Up()) next = (position - 1 + count) % count;
        if (UiPad::PageDown() || ImGui::IsKeyPressed(ImGuiKey_PageDown)) next = std::min(position + 8, count - 1);
        if (UiPad::PageUp() || ImGui::IsKeyPressed(ImGuiKey_PageUp)) next = std::max(position - 8, 0);
        if (next != position) {
            m_selectedServerIndex = static_cast<int>(visible[next] - m_servers.data());
            scrollToSelected = true;
            MenuSounds::Scroll();
        }
    }

    int visibleIndex = 0;
    for (const ServerEntry* entry : visible) {
        const ServerEntry& s = *entry;
        const int i = static_cast<int>(entry - m_servers.data());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        const bool selected = (m_selectedServerIndex == i);
        if (scrollToSelected && selected) ImGui::SetScrollHereY(0.5f);

        // Column 0: source tag
        if (s.isOwn) {
            ImGui::TextColored(NobleStyle::Accent(), "YOU");
        } else if (s.isFavorite) {
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "FAV");
        } else if (!s.region.empty() && s.region != "ONLINE") {
            const ImVec4 regionColor = (s.region == "US East") ? ImVec4(0.4f, 0.8f, 1.0f, 1.0f)
                                     : (s.region == "US West") ? ImVec4(0.45f, 0.95f, 0.55f, 1.0f)
                                     : (s.region == "UK")      ? ImVec4(0.95f, 0.75f, 0.35f, 1.0f)
                                                               : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            ImGui::TextColored(regionColor, "%s", s.region.c_str());
        } else {
            ImGui::TextDisabled("%s", s.source.c_str());
        }

        // Column 1: name (selectable across row)
        ImGui::TableNextColumn();
        char label[160];
        if (s.build.find("-dev") != std::string::npos) {
            std::snprintf(label, sizeof(label), "%s  [DEV]##row%d", s.name.c_str(), i);
        } else {
            std::snprintf(label, sizeof(label), "%s##row%d", s.name.c_str(), i);
        }
        if (ImGui::Selectable(label, selected, ImGuiSelectableFlags_SpanAllColumns)) {
            m_selectedServerIndex = i;
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && !s.isOwn) {
            AttemptJoin(s);
        }

        // Column 2: mode
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(s.gameMode.c_str());

        // Column 3: map, by the name players know it by ("Forge World", not "forge_halo"). Modded maps keep
        // their own name and get the MOD badge; hovering shows the raw file name.
        ImGui::TableNextColumn();
        if (s.isMod) DrawModBadge();
        ImGui::TextUnformatted(MapNames::Display(s.mapName).c_str());
        if (s.isMod && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", s.mapName.c_str());

        // Column 4: players
        ImGui::TableNextColumn();
        ImGui::Text("%d / %d", s.playerCount, s.maxPlayers);

        // Column 5: ping
        ImGui::TableNextColumn();
        if (s.ping >= 0) {
            const ImVec4 color = s.ping <= 100 ? ImVec4(0.35f, 0.85f, 0.45f, 1.0f)
                               : s.ping <= 150 ? ImVec4(0.95f, 0.85f, 0.35f, 1.0f)
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
        // Previews are keyed by scenario, so a stock map sent as a file ("forge_halo.map") needs its canonical name.
        const std::string scenario = MapNames::ScenarioOf(s.mapName);
        if (const UiTexture photo = UiArt::MapPhoto(scenario.empty() ? s.mapName : scenario)) {
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
    const float modsTextW = ImGui::CalcTextSize("Mods: 0").x;
    const float availW = ImGui::GetContentRegionAvail().x;
    ImGui::TextUnformatted(s.name.c_str());
    ImGui::SameLine(std::max(140.0f, availW - modsTextW));
    ImGui::TextDisabled("Mods: %d", s.isMod ? 1 : 0);
    ImGui::TextDisabled("%s:%u%s", s.hostAddress.c_str(), s.port, s.isOwn ? " (YOU)" : "");
    ImGui::Spacing();

    ImGui::TextDisabled("Mode:");
    ImGui::SameLine(U(100));
    ImGui::TextUnformatted(s.gameMode.c_str());

    if (!s.region.empty() && s.region != "YOU") {
        ImGui::TextDisabled("Region:");
        ImGui::SameLine(U(100));
        const char* regDesc = (s.region == "US East") ? "US East (Virginia)"
                            : (s.region == "US West") ? "US West (Oregon)"
                            : (s.region == "UK")      ? "UK (London)"
                            : s.region.c_str();
        ImGui::TextUnformatted(regDesc);
    }

    ImGui::TextDisabled("Map:");
    ImGui::SameLine(U(100));
    ImGui::TextWrapped("%s", MapNames::Display(s.mapName).c_str());

    if (!s.variantName.empty()) {
        ImGui::TextDisabled("Game type:");
        ImGui::SameLine(U(100));
        ImGui::TextUnformatted(s.variantName.c_str());
    }

    ImGui::TextDisabled("Players:");
    ImGui::SameLine(U(100));
    ImGui::Text("%d / %d", s.playerCount, s.maxPlayers);

    ImGui::TextDisabled("Version:");
    ImGui::SameLine(U(100));
    if (!s.build.empty()) {
        if (s.build.find("-dev") != std::string::npos) {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%s", s.build.c_str());
        } else {
            ImGui::TextUnformatted(s.build.c_str());
        }
        if (!noble::AreBuildsCompatible(s.build, noble::kBuildTag)) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.95f, 0.35f, 0.35f, 1.0f), "(Incompatible)");
        }
    } else {
        ImGui::TextDisabled("-");
    }

    ImGui::TextDisabled("Ping:");
    ImGui::SameLine(U(100));
    if (s.ping >= 0) {
        const ImVec4 color = s.ping <= 100 ? ImVec4(0.35f, 0.85f, 0.45f, 1.0f)
                           : s.ping <= 150 ? ImVec4(0.95f, 0.85f, 0.35f, 1.0f)
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
    if (!s.modId.empty()) {
        ImGui::TextDisabled("Required Workshop mod:");
        ImGui::SameLine(U(100));
        ImGui::TextWrapped("%s", s.modTitle.empty() ? ("Workshop item " + s.modId).c_str() : s.modTitle.c_str());
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped("Noble checks for it before joining and can download it from Steam.");
        ImGui::PopStyleColor();
    }

    NobleStyle::SectionGap();

    if (s.isOwn) {
        ImGui::TextDisabled("You are hosting this game.");
        if (NobleStyle::DangerButton("STOP HOSTING", ImVec2(-1, 34))) {
            SessionManager::Get().StopHosting();
            MasterServerClient::Get().StopAdvertisingSync();
            m_status = "Session closed. Stopped hosting.";
            RefreshServers();
        }
    } else {
        const bool compatible = s.build.empty() || noble::AreBuildsCompatible(s.build, noble::kBuildTag);
        if (!compatible) {
            const int rel = noble::CompareBuilds(noble::kBuildTag, s.build);   // -1: you are older than the host
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.35f, 0.35f, 1.0f));
            ImGui::TextWrapped("VERSION MISMATCH: this game runs %s and you have %s. You can't join it.",
                               std::string(noble::ExtractBaseVersion(s.build)).c_str(),
                               std::string(noble::ExtractBaseVersion(noble::kBuildTag)).c_str());
            ImGui::PopStyleColor();
            if (rel < 0) {
                // Older client: offer the update.
                Updater& up = Updater::Get();
                if (up.IsInitialized()) {
                    const Updater::Status us = up.Snapshot();
                    if (us.state == Updater::State::Idle || us.state == Updater::State::UpToDate ||
                        us.state == Updater::State::Error) {
                        if (us.state == Updater::State::Error && !us.error.empty()) {
                            ImGui::TextDisabled("Update check failed: %s", us.error.c_str());
                        }
                        if (NobleStyle::PrimaryButton("CHECK FOR UPDATE", ImVec2(-1, 38))) {
                            up.StartCheck();
                        }
                    } else if (us.state == Updater::State::Checking) {
                        ImGui::TextDisabled("Checking for an update...");
                    } else if (us.state == Updater::State::Available) {
                        const std::string lbl = "DOWNLOAD UPDATE  " + Updater::FormatBytes(us.assetSize);
                        if (NobleStyle::PrimaryButton(lbl.c_str(), ImVec2(-1, 38))) {
                            up.StartDownload();
                        }
                    } else if (us.state == Updater::State::Downloading) {
                        ImGui::ProgressBar(us.progress, ImVec2(-1, 22));
                    } else if (us.state == Updater::State::Ready) {
                        if (NobleStyle::PrimaryButton("RESTART TO UPDATE", ImVec2(-1, 38))) {
                            if (up.ApplyAndRestart()) m_closeForUpdate = true;
                        }
                    }
                } else {
                    ImGui::TextDisabled("Update the game from the main menu to join this match.");
                }
            } else {
                ImGui::TextDisabled("This host is on an older version. Ask them to update.");
            }
            ImGui::BeginDisabled();
            NobleStyle::PrimaryButton("JOIN BLOCKED (VERSION)", ImVec2(-1, 38));
            ImGui::EndDisabled();
        } else {
            const char* joinLabel = s.inProgress ? "JOIN IN PROGRESS" : "JOIN PRE-GAME LOBBY";
            if (NobleStyle::PrimaryButton(joinLabel, ImVec2(-1, 38))) {
                AttemptJoin(s);
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

bool ServerBrowser::JoinBlockedByVersion(const ServerEntry& s) {
    if (s.build.empty() || noble::AreBuildsCompatible(s.build, noble::kBuildTag)) return false;
    const int rel = noble::CompareBuilds(noble::kBuildTag, s.build);
    m_status = rel < 0
        ? "Can't join: the host runs " + std::string(noble::ExtractBaseVersion(s.build)) + " and you have " +
              std::string(noble::ExtractBaseVersion(noble::kBuildTag)) + ". Select the game and download the update."
        : "Can't join: the host is on an older version (" + std::string(noble::ExtractBaseVersion(s.build)) +
              "; you have " + std::string(noble::ExtractBaseVersion(noble::kBuildTag)) + ").";
    NOBLE_WARN("ServerBrowser join refused (version): {}", m_status);
    return true;
}

void ServerBrowser::AttemptJoin(const ServerEntry& s) {
    if (JoinBlockedByVersion(s)) return;
    if (m_joinLobbyHandler) m_joinLobbyHandler(s);
    else TryJoin(s.hostAddress, s.port, s.isPasswordProtected);
}

void ServerBrowser::TryJoin(const std::string& address, uint16_t port, bool hasPassword) {
    (void)hasPassword;
    const std::string key = Key(address, port);
    for (const auto& known : m_servers) {
        if (known.hostAddress == address && known.port == port && JoinBlockedByVersion(known)) return;
    }

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
    ImGui::SetNextWindowSize(ImVec2(U(420), 0));
    if (ImGui::BeginPopupModal("DIRECT CONNECT", &m_showDirectConnectModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Address (IP or IP:port)");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - U(75.0f));
        ImGui::InputTextWithHint("##Address", "IP or IP:port (e.g. 147.124.149.202:11774)", m_directConnectIp, IM_ARRAYSIZE(m_directConnectIp));
        ImGui::SameLine();
        if (ImGui::Button("PASTE", ImVec2(U(65), 0))) {
            if (const char* clip = ImGui::GetClipboardText()) std::snprintf(m_directConnectIp, sizeof(m_directConnectIp), "%s", clip);
        }
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(-1);

        ImGui::InputText("##Password", m_directConnectPassword, IM_ARRAYSIZE(m_directConnectPassword), ImGuiInputTextFlags_Password);


        std::string address;
        uint16_t port = 0;
        const bool valid = ParseAddress(m_directConnectIp, address, port);
        if (!valid) ImGui::TextColored(ImVec4(0.95f, 0.4f, 0.4f, 1.0f), "Enter an address such as 192.168.1.20:11774");
        ImGui::BeginDisabled(!valid);
        if (NobleStyle::PrimaryButton("CONNECT", ImVec2(U(120), U(32)))) {
            TryJoin(address, port, m_directConnectPassword[0] != 0);
            m_showDirectConnectModal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("SAVE AS FAVOURITE", ImVec2(U(170), U(32)))) {
            ToggleFavorite(address, port);
            m_lastRefresh = -100.0;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(U(90), U(32)))) m_showDirectConnectModal = false;
        ImGui::EndPopup();
    }
}

void ServerBrowser::RenderMasterServerModal() {
    ImGui::OpenPopup("SELECT MATCHMAKING REGION");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(U(480), 0));
    if (ImGui::BeginPopupModal("SELECT MATCHMAKING REGION", &m_showMasterModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("Select your preferred matchmaking region. Master servers synchronize federated sessions across regions so you can find games worldwide.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const std::string curUrl = MasterServerClient::Get().Url();
        int curRegion = 0;
        if (curUrl.empty()) {
            curRegion = 3;
        } else {
            for (int i = 0; i < 3; ++i) {
                if (curUrl == kMasterRegions[i].url || curUrl.rfind(kMasterRegions[i].url, 0) == 0) {
                    curRegion = i;
                    break;
                }
            }
        }

        static int chosenRegion = 0;
        if (ImGui::IsWindowAppearing()) chosenRegion = curRegion;

        for (int i = 0; i < 4; ++i) {
            if (ImGui::RadioButton(kMasterRegions[i].label, chosenRegion == i)) {
                chosenRegion = i;
            }
            ImGui::SameLine(U(220));
            ImGui::TextDisabled("(%s)", kMasterRegions[i].location);
        }

        NobleStyle::SectionGap();
        if (NobleStyle::PrimaryButton("SAVE & APPLY", ImVec2(U(140), U(32)))) {
            SettingsData d = Settings::Get().Snapshot();
            d.masterServerUrl = kMasterRegions[chosenRegion].url;
            Settings::Get().Update(d);
            MasterServerClient::Get().SetUrl(d.masterServerUrl);
            m_lastMasterRequest = -100.0;
            m_status = std::string("Matchmaking region set to ") + kMasterRegions[chosenRegion].label;
            m_showMasterModal = false;
            RefreshServers();
        }
        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(U(100), U(32)))) m_showMasterModal = false;
        ImGui::EndPopup();
    }
}

} // namespace noble
