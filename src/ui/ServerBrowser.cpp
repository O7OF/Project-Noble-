#include <windows.h>
#include <shellapi.h>
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
#include "network/InviteCode.hpp"
#include "network/UPnP.hpp"
#include "FrontEndMenu.hpp"
#include "NobleUi.hpp"

#include <imgui.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <charconv>
#include <cstring>
#include <mutex>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace noble {

namespace {

float U(float px) { return px * NobleUi::Scale(); }

// Design option switch for the list presentation: "-server-browser-style 1|2|3".
//   2 = large picture cards on a blue-glass panel (Tyler's pick, default),
//   1 = the older tactical cards with amber corner brackets,
//   3 = compact one-line table (dense, no thumbnails).
// 1 and 3 exist so the layouts can still be compared in captures.
int BrowserStyle() {
    static const int s_style = [] {
        const wchar_t* cmd = GetCommandLineW();
        const wchar_t* p = cmd ? wcsstr(cmd, L"-server-browser-style ") : nullptr;
        const int style = p ? _wtoi(p + 22) : 2;
        return (style >= 1 && style <= 3) ? style : 2;
    }();
    return s_style;
}

// Is the Workshop item installed? The check walks the Workshop cache and data\mods folders, so cache the answer for a
// couple of seconds: the list draws up to a screenful of cards every frame and most of them share no mod at all.
bool ModInstalledCached(const std::string& publishedFileId) {
    if (publishedFileId.empty()) return true;
    static std::mutex mutex;
    static std::unordered_map<std::string, bool> cache;
    static uint64_t stamp = 0;
    const uint64_t now = GetTickCount64();
    std::lock_guard<std::mutex> lock(mutex);
    if (stamp == 0 || now - stamp > 2000) {
        cache.clear();
        stamp = now;
    }
    const auto it = cache.find(publishedFileId);
    if (it != cache.end()) return it->second;
    const bool installed = ModDownloadPrompt::IsInstalled(publishedFileId);
    cache.emplace(publishedFileId, installed);
    return installed;
}

static std::atomic<uint64_t> s_lastJoinAttemptMs{ 0 };
static int s_browserPane = 0;   // 0 = server cards list, 1 = right game details buttons
static int s_detailBtnIdx = 0;  // 0 = primary action, 1 = favorite, 2 = copy invite
static bool s_detailTriggerFocused = false; // Accept pressed this frame while the details pane has focus

bool CanStartJoinNow(std::string& statusOut) {
    if (FrontEndMenu::IsMatchRunning()) {
        statusOut = "A match is already running. Close it before joining another server.";
        NOBLE_WARN("ServerBrowser: join ignored because a match process is already running");
        return false;
    }
    const uint64_t now = GetTickCount64();
    const uint64_t prev = s_lastJoinAttemptMs.load();
    if (prev != 0 && now >= prev && (now - prev) < 1500) {
        statusOut = "Connection already starting...";
        NOBLE_WARN("ServerBrowser: duplicate join ignored within 1500ms");
        return false;
    }
    return true;
}

void StampJoinAttemptNow() {
    s_lastJoinAttemptMs.store(GetTickCount64());
}

std::string UpperAscii(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}

std::string EllipsizeLabel(ImFont* font, float pxSize, const std::string& text, float maxW) {
    if (maxW <= 4.0f || text.empty()) return "";
    ImFont* f = font ? font : ImGui::GetFont();
    if (f->CalcTextSizeA(pxSize, FLT_MAX, 0.0f, text.c_str()).x <= maxW) return text;
    const float dotsW = f->CalcTextSizeA(pxSize, FLT_MAX, 0.0f, "...").x;
    std::string out = text;
    while (!out.empty()) {
        out.pop_back();
        if (f->CalcTextSizeA(pxSize, FLT_MAX, 0.0f, out.c_str()).x + dotsW <= maxW) {
            return out + "...";
        }
    }
    return "...";
}

void DrawSignalBars(ImDrawList* dl, ImVec2 pos, float s, int ping) {
    const int bars = (ping < 0) ? 0 : (ping <= 75) ? 3 : (ping <= 145) ? 2 : 1;
    const ImU32 activeCol = (ping <= 75) ? IM_COL32(82, 224, 128, 255)
                          : (ping <= 145) ? IM_COL32(255, 198, 72, 255)
                                          : IM_COL32(245, 95, 85, 255);
    const ImU32 offCol = IM_COL32(55, 72, 92, 160);
    const float bw = 4.0f * s, gap = 2.5f * s, baseH = 14.0f * s;
    for (int i = 0; i < 3; ++i) {
        const float h = baseH * (0.42f + 0.29f * i);
        const ImVec2 b0(pos.x + i * (bw + gap), pos.y + baseH - h);
        const ImVec2 b1(b0.x + bw, pos.y + baseH);
        dl->AddRectFilled(b0, b1, i < bars ? activeCol : offCol, 1.0f * s);
    }
}

struct MasterRegion {
    const char* label;
    const char* location;
    std::string url;
};

static const MasterRegion kMasterRegions[] = {
    { "Global Main Master Server", "Main Backend (Render)", MasterDefaultUrl() },
    { "LAN / Offline Only", "Local Network Only", "" }
};

struct PingSample {
    int rttMs = -1;
    ULONGLONG timestampMs = 0;
};
static std::mutex s_pingMutex;
static std::unordered_map<std::string, PingSample> s_pingSamples;

static void ProbeServerPingAsync(const std::string& host, uint16_t port) {
    if (host.empty() || host == "127.0.0.1" || host == "localhost" || port == 0) return;
    const std::string key = host + ":" + std::to_string(port);
    const ULONGLONG nowMs = GetTickCount64();
    {
        std::lock_guard<std::mutex> lock(s_pingMutex);
        auto it = s_pingSamples.find(key);
        if (it != s_pingSamples.end() && nowMs >= it->second.timestampMs &&
            (nowMs - it->second.timestampMs) < 5000ULL) {
            return;
        }
        s_pingSamples[key] = { it != s_pingSamples.end() ? it->second.rttMs : -1, nowMs };
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
            rtt = -1; // no reply: unknown (drawn "-"), never a made-up regional number
        }

        std::lock_guard<std::mutex> lock(s_pingMutex);
        s_pingSamples[key] = { rtt, GetTickCount64() };
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
    // Test-only hooks: "-invite-code-selftest" runs the codec round-trip/negative tests; "-join-code-test <code>"
    // pre-fills the join-by-code modal for a capture; "-invite-code-make <session>" logs a valid code to copy.
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i + 1 < argc; ++i) {
        if (_wcsicmp(argv[i], L"-join-code-test") == 0) {
            std::wstring ws(argv[i + 1]);
            const std::string code(ws.begin(), ws.end());
            std::snprintf(m_joinCode, sizeof(m_joinCode), "%s", code.c_str());
            m_showJoinCodeModal = true;
        } else if (_wcsicmp(argv[i], L"-invite-code-make") == 0) {
            std::wstring ws(argv[i + 1]);
            InviteCodeInfo info;
            info.sessionId.assign(ws.begin(), ws.end());
            NOBLE_INFO("InviteCode make: session {} -> '{}'", info.sessionId, EncodeInviteCode(info));
        }
    }
    if (argv) LocalFree(argv);
    if (wcsstr(GetCommandLineW(), L"-invite-code-selftest")) {
        const int failures = InviteCodeSelfTest();
        NOBLE_INFO("InviteCode self-test: {} failure(s)", failures);
    }
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
            e.gameVersion = std::string(noble::VERSION_STRING);
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
                e.ping = -1; // not measured yet
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
        e.gameVersion = m.gameVersion;
        e.source = m.regionName.empty() ? "ONLINE" : m.regionName;
        e.region = m.regionName.empty() ? "ONLINE" : m.regionName;
        add(e);
    }

    // Test option -server-browser-demo: a mixed list (modes, maps, modded sessions) that can't be joined (marked as
    // our own), to check moving, scrolling, badges, the details pane and the mod-download state.
    static const bool s_demoRows = wcsstr(GetCommandLineW(), L"-server-browser-demo") != nullptr;
    if (s_demoRows) {
        struct DemoRow {
            const char* name;
            const char* mode;
            const char* map;
            const char* variant;
            int players;
            int maxPlayers;
            int ping;
            const char* source;
            const char* modId;
            const char* modTitle;
        };
        static const DemoRow kDemoRows[] = {
            {"Demo modded 12", "multiplayer", "20_sword_slayer", "Slayer", 16, 16, 51, "ONLINE", "1234567890",
             "Noble Map Pack"},
            {"Demo server 11", "forge", "forge_halo", "Forge", 5, 16, 68, "ONLINE", nullptr, nullptr},
            {"Demo server 10", "campaign", "m10", "Campaign Co-op", 3, 4, 55, "ONLINE", nullptr, nullptr},
            {"Demo server 09", "firefight", "ff50_park", "Firefight", 4, 4, 96, "ONLINE", nullptr, nullptr},
            {"Demo modded 08", "multiplayer", "20_sword_slayer", "Slayer", 8, 16, 51, "ONLINE", "3127073871",
             "Improved Trooper Campaign"},
            {"Demo server 07", "multiplayer", "35_island", "CTF", 6, 16, 74, "ONLINE", nullptr, nullptr},
            {"Demo modded 06", "firefight", "ff20_courtyard", "Firefight", 2, 4, 88, "ONLINE", "1234567890",
             "Covenant Firefight Pack"},
            {"Demo server 05", "multiplayer", "dlc_slayer", "Invasion", 10, 16, 112, "ONLINE", nullptr, nullptr},
            {"Demo server 04", "forge", "forge_halo", "Forge", 3, 16, 132, "LAN", nullptr, nullptr},
            {"Demo modded 03", "multiplayer", "50_panopticon", "Infection", 9, 16, 63, "ONLINE", "9876543210",
             "Noble Map Pack"},
        };
        for (const DemoRow& d : kDemoRows) {
            ServerEntry demo;
            demo.name = d.name;
            demo.hostAddress = "demo";
            demo.port = static_cast<decltype(demo.port)>(list.size() + 1);
            demo.gameMode = d.mode;
            demo.mapName = d.map;
            demo.variantName = d.variant;
            demo.playerCount = d.players;
            demo.maxPlayers = d.maxPlayers;
            demo.ping = d.ping;
            demo.source = d.source;
            demo.region = d.source;
            demo.isOwn = (demo.name != "Demo modded 12");   // the first demo row shows the joinable details path
            demo.isMod = d.modId != nullptr;
            if (d.modId) demo.modId = d.modId;
            if (d.modTitle) demo.modTitle = d.modTitle;
            demo.mapDescription = demo.isMod ? ("Custom map from " + demo.modTitle) : std::string();
            demo.modDescription = demo.isMod ? demo.modTitle : std::string();
            demo.build = kBuildTag;
            if (demo.name == "Demo modded 12") demo.build = "0.19.98 (Closed Source)";   // version-issue demo row
            list.push_back(std::move(demo));
        }
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
    // every 8 s (/api/browser already includes Players Online & Total Players, avoiding a second request).
    const double now = ImGui::GetTime();
    if (now - m_lastMasterRequest > 8.0) {
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

    if (m_servers.empty() || m_selectedServerIndex < 0) s_browserPane = 0;

    RenderFilterBar();
    ImGui::Dummy(ImVec2(0.0f, U(6.0f)));

    const float availW = ImGui::GetContentRegionAvail().x;
    const float detailsWidth = std::clamp(availW * 0.30f, U(300.0f), U(410.0f));
    const float paneGap = U(12.0f);
    const float listWidth = std::max(U(320.0f), availW - detailsWidth - paneGap);
    const float bodyH = std::max(U(180.0f), ImGui::GetContentRegionAvail().y - U(4.0f));
    const float sScale = NobleUi::Scale();
    ImDrawList* parentDl = ImGui::GetWindowDrawList();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, BrowserStyle() >= 2 ? IM_COL32(10, 20, 31, 226) : IM_COL32(8, 14, 22, 210));
    ImGui::PushStyleColor(ImGuiCol_Border, BrowserStyle() >= 2 ? IM_COL32(86, 148, 190, 150) : IM_COL32(45, 75, 102, 120));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(U(10.0f), U(10.0f)));

    ImGui::BeginChild("##serverlist", ImVec2(listWidth, bodyH), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    if (UiPad::IsVirtualCursorActive() ? ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) : (s_browserPane == 0)) {
        UiPad::ScrollWindow(false, true);
    }
    RenderServerList();
    ImGui::EndChild();
    NobleUi::DrawTacticalFrame(parentDl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale,
                               s_browserPane == 0, true, false,
                               BrowserStyle() == 1 && s_browserPane == 0);

    ImGui::SameLine(0.0f, paneGap);

    // Details content scrolls, but FAVOURITE / COPY INVITE are pinned in a footer child so they are always visible and
    // clickable (they used to sit below the fold with mod warnings in the pane - Tyler's "can't click Favourite").
    const float detailsFooterH = U(54.0f);
    const ImVec2 detailsOrigin = ImGui::GetCursorScreenPos();
    ImGui::BeginChild("##serverdetails", ImVec2(detailsWidth, bodyH - detailsFooterH), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    if (UiPad::IsVirtualCursorActive() ? ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) : (s_browserPane == 1)) {
        UiPad::ScrollWindow(false, true);
    }
    RenderServerDetails();
    ImGui::EndChild();
    NobleUi::DrawTacticalFrame(parentDl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale,
                               s_browserPane == 1, m_selectedServerIndex >= 0, false,
                               BrowserStyle() == 1 && s_browserPane == 1);

    ImGui::SetCursorScreenPos(ImVec2(detailsOrigin.x, detailsOrigin.y + bodyH - detailsFooterH));
    ImGui::BeginChild("##serveractions", ImVec2(detailsWidth, detailsFooterH), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    RenderServerActions();
    ImGui::EndChild();
    NobleUi::DrawTacticalFrame(parentDl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale,
                               s_browserPane == 1, m_selectedServerIndex >= 0, false, false);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    if (m_showDirectConnectModal) RenderDirectConnectModal();
    if (m_showJoinCodeModal) RenderJoinCodeModal();
    if (m_showMasterModal) RenderMasterServerModal();
}

void ServerBrowser::RenderFilterBar() {
    SessionManager& sessions = SessionManager::Get();
    const bool hosting = sessions.IsHosting();
    const float sScale = NobleUi::Scale();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* titleFont = NobleStyle::TitleFont() ? NobleStyle::TitleFont() : ImGui::GetFont();
    ImFont* bodyFont = ImGui::GetFont();

    const bool modalBlocking = ImGui::GetIO().WantTextInput || m_showDirectConnectModal || m_showMasterModal || ModDownloadPrompt::Get().IsActive();
    if (!modalBlocking) {
        if (UiPad::TabPrev()) {
            m_modeFilter = (m_modeFilter + 4) % 5;
            MenuSounds::Tab(-1);
        }
        if (UiPad::TabNext()) {
            m_modeFilter = (m_modeFilter + 1) % 5;
            MenuSounds::Tab(1);
        }
        if (UiPad::ActionX() || ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
            m_lastMasterRequest = -100.0;
            m_lastRefresh = -100.0;
            sessions.TriggerDiscoveryScan();
            MasterServerClient::Get().RequestList();
            m_status = "Refreshing server list...";
            MenuSounds::Click();
        }
    }

    // Live player stats (top right) + NAT telemetry pill
    int activeInServers = 0;
    for (const auto& s : m_servers) {
        if (s.name.find("Master") != std::string::npos || s.name.find("Relay") != std::string::npos) continue;
        if (s.playerCount > 0) activeInServers += s.playerCount;
    }
    const int onlineCount = std::max({ MasterServerClient::Get().OnlinePlayers(), activeInServers, 1 });
    const int totalCount = std::max(MasterServerClient::Get().TotalPlayers(), onlineCount);
    const std::string onlineStr = FormatCountWithCommas(onlineCount) + " ONLINE";
    const std::string totalStr = FormatCountWithCommas(totalCount) + " TOTAL";

    const bool upnpActive = UPnP::Get().IsActive();
    const UPnP::NatType natType = UPnP::Get().GetNatType();
    const char* natLabel = UPnP::Get().GetNatTypeString();
    const ImU32 natCol = (natType == UPnP::NatType::Open) ? IM_COL32(82, 224, 128, 255)
                       : (natType == UPnP::NatType::Moderate) ? IM_COL32(255, 198, 72, 255)
                                                              : IM_COL32(245, 95, 85, 255);
    const std::string curUrl = MasterServerClient::Get().Url();
    const int curRegion = curUrl.empty() ? 1 : 0;

    // -------------------------------------------------------------------------
    // Row 1: Tactical Mode Tabs (Left) + NAT Pill & Live Player Counter (Right)
    // -------------------------------------------------------------------------
    const ImVec2 row1Origin = ImGui::GetCursorScreenPos();
    const float fullW = ImGui::GetContentRegionAvail().x;
    const float tabH = 34.0f * sScale;
    const float tabFontPx = 15.0f * sScale;
    const float counterFontPx = 17.0f * sScale;
    const float subCounterPx = 13.0f * sScale;

    const ImVec2 onlineSz = titleFont->CalcTextSizeA(counterFontPx, FLT_MAX, 0.0f, onlineStr.c_str());
    const ImVec2 totalSz = bodyFont->CalcTextSizeA(subCounterPx, FLT_MAX, 0.0f, totalStr.c_str());
    const float statsW = std::max(onlineSz.x, totalSz.x) + 8.0f * sScale;

    std::string natBadgeText = std::string("NAT: ") + UpperAscii(natLabel ? natLabel : "UNKNOWN");
    const ImVec2 natTextSz = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, natBadgeText.c_str());
    const float natPillW = natTextSz.x + 28.0f * sScale;

    const float rightBlockW = statsW + natPillW + 16.0f * sScale;
    const float tabsBudgetW = std::max(U(300.0f), fullW - rightBlockW - U(12.0f));

    static const char* kModeLabels[] = { "ALL", "MULTIPLAYER", "CAMPAIGN", "FIREFIGHT", "FORGE" };
    float cursorX = row1Origin.x;
    const bool showPadHints = UiPad::HasGamepad();
    if (showPadHints) {
        const ImVec2 lbSz = bodyFont->CalcTextSizeA(13.0f * sScale, FLT_MAX, 0.0f, "[LB]");
        dl->AddText(bodyFont, 13.0f * sScale, ImVec2(cursorX, row1Origin.y + (tabH - lbSz.y) * 0.5f),
                    IM_COL32(143, 216, 255, 220), "[LB]");
        cursorX += lbSz.x + 8.0f * sScale;
    }

    const float perTabMaxW = (tabsBudgetW - (showPadHints ? 76.0f * sScale : 0.0f)) / 5.0f;
    for (int i = 0; i < 5; ++i) {
        const ImVec2 ts = titleFont->CalcTextSizeA(tabFontPx, FLT_MAX, 0.0f, kModeLabels[i]);
        const float w = std::min(perTabMaxW - 6.0f * sScale, ts.x + 28.0f * sScale);
        const ImVec2 p0(cursorX, row1Origin.y);
        const ImVec2 p1(cursorX + w, row1Origin.y + tabH);

        ImGui::SetCursorScreenPos(p0);
        char tid[32];
        std::snprintf(tid, sizeof(tid), "##modeTab%d", i);
        if (ImGui::InvisibleButton(tid, ImVec2(w, tabH))) {
            m_modeFilter = i;
            MenuSounds::Tab(1);
        }
        const bool hovered = ImGui::IsItemHovered();
        const bool active = (m_modeFilter == i);
        const ImU32 bgCol = active ? IM_COL32(28, 62, 92, 235)
                          : hovered ? IM_COL32(20, 42, 64, 210)
                                    : IM_COL32(10, 18, 28, 170);
        dl->AddRectFilled(p0, p1, bgCol, 2.0f * sScale);
        dl->AddRect(p0, p1, active ? IM_COL32(108, 210, 255, 210) : hovered ? IM_COL32(100, 155, 195, 130) : IM_COL32(45, 70, 92, 110), 2.0f * sScale);
        if (active) {
            dl->AddRectFilled(ImVec2(p0.x + 2.0f * sScale, p1.y - 3.0f * sScale),
                              ImVec2(p1.x - 2.0f * sScale, p1.y), NobleUi::kAmber);
        }
        const ImVec2 textPos(p0.x + (w - ts.x) * 0.5f, p0.y + (tabH - ts.y) * 0.5f);
        dl->AddText(titleFont, tabFontPx, textPos, active ? IM_COL32(255, 248, 230, 255) : IM_COL32(190, 210, 226, 230), kModeLabels[i]);
        cursorX += w + 6.0f * sScale;
    }

    if (showPadHints) {
        const ImVec2 rbSz = bodyFont->CalcTextSizeA(13.0f * sScale, FLT_MAX, 0.0f, "[RB]");
        dl->AddText(bodyFont, 13.0f * sScale, ImVec2(cursorX + 2.0f * sScale, row1Origin.y + (tabH - rbSz.y) * 0.5f),
                    IM_COL32(143, 216, 255, 220), "[RB]");
    }

    // Right side of Row 1: NAT/UPnP compact indicator pill + Online/Total telemetry
    {
        const float natX = row1Origin.x + fullW - statsW - natPillW - 12.0f * sScale;
        const ImVec2 n0(natX, row1Origin.y + 4.0f * sScale);
        const ImVec2 n1(natX + natPillW, row1Origin.y + tabH - 4.0f * sScale);
        ImGui::SetCursorScreenPos(n0);
        ImGui::InvisibleButton("##natStatusPill", ImVec2(natPillW, n1.y - n0.y));
        const bool natHovered = ImGui::IsItemHovered();
        dl->AddRectFilled(n0, n1, natHovered ? IM_COL32(22, 36, 52, 230) : IM_COL32(10, 18, 28, 200), 3.0f * sScale);
        dl->AddRect(n0, n1, natCol, 3.0f * sScale, 0, 1.0f * sScale);
        dl->AddCircleFilled(ImVec2(n0.x + 11.0f * sScale, (n0.y + n1.y) * 0.5f), 4.0f * sScale, natCol);
        dl->AddText(bodyFont, 13.5f * sScale, ImVec2(n0.x + 20.0f * sScale, n0.y + (n1.y - n0.y - natTextSz.y) * 0.5f),
                    IM_COL32(226, 236, 245, 240), natBadgeText.c_str());
        if (natHovered) {
            ImGui::BeginTooltip();
            ImGui::Text("NAT Type: %s  |  UPnP: %s", natLabel ? natLabel : "Unknown", upnpActive ? "Enabled" : "Disabled");
            ImGui::Text("Master Region: %s (%s)", curRegion == 0 ? kMasterRegions[0].label : "LAN / Offline Only",
                        MasterServerClient::Get().Enabled() ? MasterServerClient::Get().Status().c_str() : "Offline");
            if (natType != UPnP::NatType::Open || !upnpActive) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.28f, 1.0f),
                                   "To host public matches reliably, enable UPnP in your router or forward UDP port 11774.");
            }
            ImGui::EndTooltip();
        }

        const float statRight = row1Origin.x + fullW;
        dl->AddText(titleFont, counterFontPx, ImVec2(statRight - onlineSz.x, row1Origin.y - 1.0f * sScale),
                    IM_COL32(143, 216, 255, 255), onlineStr.c_str());
        dl->AddText(bodyFont, subCounterPx, ImVec2(statRight - totalSz.x, row1Origin.y + onlineSz.y + 1.0f * sScale),
                    IM_COL32(160, 180, 198, 220), totalStr.c_str());
    }

    dl->AddLine(ImVec2(row1Origin.x, row1Origin.y + tabH + 5.0f * sScale),
                ImVec2(row1Origin.x + fullW, row1Origin.y + tabH + 5.0f * sScale),
                IM_COL32(72, 128, 175, 110), 1.0f * sScale);

    ImGui::SetCursorScreenPos(ImVec2(row1Origin.x, row1Origin.y + tabH + 10.0f * sScale));

    // -------------------------------------------------------------------------
    // Row 2: Search Box + Toggle Filter Pills + Sort + Action Buttons
    // -------------------------------------------------------------------------
    const float rowStartX = ImGui::GetCursorScreenPos().x;
    const float rowMaxRight = rowStartX + fullW;
    const float pillH = 28.0f * sScale;
    const float pillGap = 6.0f * sScale;

    auto ensureSameLineOrWrap = [&](float itemW) {
        const float curX = ImGui::GetCursorScreenPos().x;
        if (curX > rowStartX + 4.0f && curX + itemW > rowMaxRight) {
            ImGui::NewLine();
        }
    };

    const float searchW = U(210.0f);
    ImGui::SetNextItemWidth(searchW);
    ImGui::InputTextWithHint("##Search", "Search server or map...", m_searchQuery, IM_ARRAYSIZE(m_searchQuery));

    auto drawFilterPill = [&](const char* id, const char* label, bool& value) {
        const ImVec2 ts = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, label);
        const float w = ts.x + 24.0f * sScale;
        ImGui::SameLine(0.0f, pillGap);
        ensureSameLineOrWrap(w);
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const ImVec2 p1(p0.x + w, p0.y + pillH);
        if (ImGui::InvisibleButton(id, ImVec2(w, pillH))) {
            value = !value;
            MenuSounds::Click();
        }
        const bool hov = ImGui::IsItemHovered();
        dl->AddRectFilled(p0, p1, value ? IM_COL32(28, 70, 98, 235) : (hov ? IM_COL32(20, 38, 56, 210) : IM_COL32(10, 18, 28, 190)), 2.0f * sScale);
        dl->AddRect(p0, p1, value ? NobleUi::kAccent : (hov ? IM_COL32(120, 180, 225, 190) : IM_COL32(55, 82, 108, 130)), 2.0f * sScale);
        dl->AddCircleFilled(ImVec2(p0.x + 9.0f * sScale, (p0.y + p1.y) * 0.5f), 3.0f * sScale,
                            value ? NobleUi::kAmber : IM_COL32(78, 96, 114, 180));
        dl->AddText(bodyFont, 13.5f * sScale, ImVec2(p0.x + 16.0f * sScale, p0.y + (pillH - ts.y) * 0.5f),
                    value ? IM_COL32(244, 248, 252, 255) : IM_COL32(180, 198, 214, 220), label);
    };

    drawFilterPill("##fltFav", "FAVOURITES", m_showOnlyFavorites);
    drawFilterPill("##fltFull", "HIDE FULL", m_hideFull);
    drawFilterPill("##fltEmpty", "ACTIVE ONLY", m_hideEmpty);
    drawFilterPill("##fltPass", "NO PASSWORD", m_hidePassworded);
    drawFilterPill("##fltProg", "OPEN LOBBY", m_hideInProgress);

    // Sort Cycle Pill
    static const char* kSortLabels[] = { "SORT: PLAYERS", "SORT: PING", "SORT: NAME", "SORT: MODE" };
    const char* sortLabel = kSortLabels[std::clamp(m_sortBy, 0, 3)];
    {
        const ImVec2 ts = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, sortLabel);
        const float w = ts.x + 20.0f * sScale;
        ImGui::SameLine(0.0f, pillGap);
        ensureSameLineOrWrap(w);
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const ImVec2 p1(p0.x + w, p0.y + pillH);
        if (ImGui::InvisibleButton("##sortCyclePill", ImVec2(w, pillH))) {
            m_sortBy = (m_sortBy + 1) % 4;
            MenuSounds::Click();
        }
        const bool hov = ImGui::IsItemHovered();
        dl->AddRectFilled(p0, p1, hov ? IM_COL32(24, 48, 70, 220) : IM_COL32(12, 22, 34, 200), 2.0f * sScale);
        dl->AddRect(p0, p1, hov ? NobleUi::kAccent : IM_COL32(68, 102, 132, 160), 2.0f * sScale);
        dl->AddText(bodyFont, 13.5f * sScale, ImVec2(p0.x + 10.0f * sScale, p0.y + (pillH - ts.y) * 0.5f),
                    IM_COL32(205, 228, 245, 245), sortLabel);
    }

    auto drawActionBtn = [&](const char* id, const char* label, bool danger) -> bool {
        const ImVec2 ts = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, label);
        const float w = ts.x + 22.0f * sScale;
        ImGui::SameLine(0.0f, pillGap);
        ensureSameLineOrWrap(w);
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const ImVec2 p1(p0.x + w, p0.y + pillH);
        const bool clicked = ImGui::InvisibleButton(id, ImVec2(w, pillH));
        const bool hov = ImGui::IsItemHovered();
        const ImU32 bg = danger ? (hov ? IM_COL32(150, 36, 36, 240) : IM_COL32(110, 24, 24, 225))
                                : (hov ? IM_COL32(34, 78, 112, 235) : IM_COL32(16, 34, 52, 215));
        const ImU32 border = danger ? IM_COL32(235, 95, 85, 210) : (hov ? NobleUi::kAmber : IM_COL32(84, 150, 200, 175));
        dl->AddRectFilled(p0, p1, bg, 2.0f * sScale);
        dl->AddRect(p0, p1, border, 2.0f * sScale);
        dl->AddText(bodyFont, 13.5f * sScale, ImVec2(p0.x + 11.0f * sScale, p0.y + (pillH - ts.y) * 0.5f),
                    IM_COL32(244, 248, 252, 255), label);
        return clicked;
    };

    if (drawActionBtn("##refreshBtn", showPadHints ? "REFRESH [X]" : "REFRESH [F5]", false)) {
        m_lastMasterRequest = -100.0;
        m_lastRefresh = -100.0;
        sessions.TriggerDiscoveryScan();
        MasterServerClient::Get().RequestList();
        // The keyboard path below already shows this; the button used to refresh silently, which made it look like the
        // click did nothing (Tyler's "can't click Refresh" report).
        m_status = "Refreshing server list...";
        MenuSounds::Click();
    }
    if (drawActionBtn("##directConnBtn", "DIRECT CONNECT", false)) {
        m_showDirectConnectModal = true;
        MenuSounds::Click();
    }
    if (drawActionBtn("##joinCodeBtn", "JOIN WITH CODE", false)) {
        m_showJoinCodeModal = true;
        m_joinCodeError.clear();
        MenuSounds::Click();
    }
    const char* regShort = (curRegion == 0) ? "REGION: GLOBAL" : "REGION: LAN ONLY";
    if (drawActionBtn("##regionBtn", regShort, false)) {
        m_selectedRegion = curRegion;
        m_showMasterModal = true;
        MenuSounds::Click();
    }
    if (hosting && drawActionBtn("##stopHostBtn", "STOP HOSTING", true)) {
        sessions.StopHosting();
        MasterServerClient::Get().StopAdvertisingSync();
        m_status = "Session closed. Stopped hosting.";
        RefreshServers();
        MenuSounds::Click();
    }

    if (!m_status.empty()) {
        ImGui::Dummy(ImVec2(0.0f, 4.0f * sScale));
        ImDrawList* sdl = ImGui::GetWindowDrawList();
        const ImVec2 s0 = ImGui::GetCursorScreenPos();
        const ImVec2 ts = bodyFont->CalcTextSizeA(14.0f * sScale, fullW - 20.0f * sScale, 0.0f, m_status.c_str());
        const ImVec2 s1(s0.x + fullW, s0.y + ts.y + 8.0f * sScale);
        sdl->AddRectFilled(s0, s1, IM_COL32(14, 34, 52, 210), 2.0f * sScale);
        sdl->AddRectFilled(s0, ImVec2(s0.x + 3.0f * sScale, s1.y), NobleUi::kAccent);
        sdl->AddText(bodyFont, 14.0f * sScale, ImVec2(s0.x + 12.0f * sScale, s0.y + 4.0f * sScale),
                     IM_COL32(190, 232, 255, 255), m_status.c_str());
        ImGui::Dummy(ImVec2(fullW, s1.y - s0.y));
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
    const float sScale = NobleUi::Scale();
    const int style = BrowserStyle();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* titleFont = NobleStyle::TitleFont() ? NobleStyle::TitleFont() : ImGui::GetFont();
    ImFont* bodyFont = ImGui::GetFont();

    std::vector<const ServerEntry*> visible;
    visible.reserve(m_servers.size());
    for (const ServerEntry& s : m_servers) {
        if (PassesFilters(s)) visible.push_back(&s);
    }

    auto lowerAscii = [](std::string text) {
        for (char& c : text) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
        return text;
    };
    std::stable_sort(visible.begin(), visible.end(), [&](const ServerEntry* a, const ServerEntry* b) {
        switch (m_sortBy) {
        case 1: { // Ping ascending (unknown ping at bottom)
            const int pa = a->ping >= 0 ? a->ping : 99999;
            const int pb = b->ping >= 0 ? b->ping : 99999;
            if (pa != pb) return pa < pb;
            return a->playerCount > b->playerCount;
        }
        case 2: // Name ascending
            return lowerAscii(a->name) < lowerAscii(b->name);
        case 3: // Mode ascending
            if (lowerAscii(a->gameMode) != lowerAscii(b->gameMode)) return lowerAscii(a->gameMode) < lowerAscii(b->gameMode);
            return a->playerCount > b->playerCount;
        case 0:
        default: // Players descending, then ping ascending
            if (a->playerCount != b->playerCount) return a->playerCount > b->playerCount;
            const int pa = a->ping >= 0 ? a->ping : 99999;
            const int pb = b->ping >= 0 ? b->ping : 99999;
            return pa < pb;
        }
    });

    // Keep the selection on something that is actually on screen: the default is m_servers[0], which sorting can move
    // below the fold (the GAME DETAILS pane used to describe a server that was not visible at the top of the list).
    if (!visible.empty()) {
        const bool selectedVisible = m_selectedServerIndex >= 0 &&
                                     m_selectedServerIndex < static_cast<int>(m_servers.size()) &&
                                     std::find(visible.begin(), visible.end(), &m_servers[m_selectedServerIndex]) != visible.end();
        if (!selectedVisible) m_selectedServerIndex = static_cast<int>(visible.front() - m_servers.data());
    }

    const bool padFree = !ImGui::GetIO().WantTextInput && !m_showDirectConnectModal && !m_showMasterModal && !ModDownloadPrompt::Get().IsActive();
    bool scrollToSelected = false;
    if (padFree && s_browserPane == 0 && !visible.empty()) {
        const int count = static_cast<int>(visible.size());
        int position = -1;
        for (int k = 0; k < count; ++k) {
            if (static_cast<int>(visible[k] - m_servers.data()) == m_selectedServerIndex) {
                position = k;
                break;
            }
        }
        int next = (position >= 0) ? position : 0;
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) || UiPad::Down()) next = (next + 1) % count;
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) || UiPad::Up()) next = (next - 1 + count) % count;
        if (UiPad::PageDown() || ImGui::IsKeyPressed(ImGuiKey_PageDown)) next = std::min(next + 6, count - 1);
        if (UiPad::PageUp() || ImGui::IsKeyPressed(ImGuiKey_PageUp)) next = std::max(next - 6, 0);
        if (next != position) {
            m_selectedServerIndex = static_cast<int>(visible[next] - m_servers.data());
            scrollToSelected = (position >= 0);
            if (position >= 0) MenuSounds::Scroll();
        }

        if (UiPad::Right() || ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) {
            if (m_selectedServerIndex >= 0) {
                s_browserPane = 1;
                s_detailBtnIdx = 0;
                MenuSounds::Scroll();
            }
        }

        if (UiPad::AcceptPressed() || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
            if (m_selectedServerIndex >= 0 && m_selectedServerIndex < static_cast<int>(m_servers.size())) {
                const auto& sel = m_servers[m_selectedServerIndex];
                if (!sel.isOwn) {
                    UiPad::ConsumeAccept();
                    AttemptJoin(sel);
                }
            }
        }
        if (UiPad::ActionY() || ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
            if (m_selectedServerIndex >= 0 && m_selectedServerIndex < static_cast<int>(m_servers.size())) {
                ToggleFavorite(m_servers[m_selectedServerIndex].hostAddress, m_servers[m_selectedServerIndex].port);
                RefreshServers();
                MenuSounds::Click();
            }
        }
    }

    const float listAvailW = ImGui::GetContentRegionAvail().x;
    if (visible.empty()) {
        const float boxH = U(120.0f);
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const ImVec2 p1(p0.x + listAvailW, p0.y + boxH);
        dl->AddRectFilled(p0, p1, IM_COL32(12, 22, 34, 200), 2.0f * sScale);
        NobleUi::DrawTacticalFrame(dl, p0, p1, sScale, false, false, false, false);
        const char* title = "NO MATCHING SESSIONS FOUND";
        const char* sub = m_servers.empty()
            ? "Scanning LAN & Master Server relays... Press [X] / [F5] to refresh or click DIRECT CONNECT."
            : "Try clearing one or more active mode or search filters.";
        const ImVec2 tsz = titleFont->CalcTextSizeA(18.0f * sScale, FLT_MAX, 0.0f, title);
        const ImVec2 ssz = bodyFont->CalcTextSizeA(14.0f * sScale, listAvailW - U(40), 0.0f, sub);
        dl->AddText(titleFont, 18.0f * sScale, ImVec2(p0.x + (listAvailW - tsz.x) * 0.5f, p0.y + U(30)), IM_COL32(190, 214, 234, 235), title);
        dl->AddText(bodyFont, 14.0f * sScale, ImVec2(p0.x + (listAvailW - ssz.x) * 0.5f, p0.y + U(62)), IM_COL32(140, 165, 188, 210), sub, nullptr, listAvailW - U(40));
        ImGui::Dummy(ImVec2(listAvailW, boxH));
        return;
    }

    const bool compact = (style == 3);
    const float cardH = compact ? U(46.0f) : (style == 2 ? U(98.0f) : U(74.0f));
    const float cardGap = compact ? U(3.0f) : U(6.0f);
    const float pad = U(6.0f);
    const float thumbH = compact ? 0.0f : cardH - pad * 2.0f;
    const float thumbW = compact ? 0.0f : thumbH * (16.0f / 9.0f);
    const float metricsW = compact ? U(150.0f) : U(165.0f);

    for (const ServerEntry* entry : visible) {
        const ServerEntry& s = *entry;
        const int i = static_cast<int>(entry - m_servers.data());
        const bool selected = (m_selectedServerIndex == i);

        const ImVec2 c0 = ImGui::GetCursorScreenPos();
        const ImVec2 c1(c0.x + listAvailW, c0.y + cardH);

        char rowId[32];
        std::snprintf(rowId, sizeof(rowId), "##srvCard%d", i);
        if (ImGui::InvisibleButton(rowId, ImVec2(listAvailW, cardH))) {
            m_selectedServerIndex = i;
            s_browserPane = 0;
            MenuSounds::Click();
        }
        const bool hovered = ImGui::IsItemHovered();
        if (hovered && ImGui::IsMouseDoubleClicked(0) && !s.isOwn) {
            AttemptJoin(s);
        }
        if (scrollToSelected && selected) {
            ImGui::SetScrollHereY(0.45f);
        }

        const bool focusedCard = (s_browserPane == 0 && selected);
        const ImU32 cardBg = selected ? IM_COL32(20, 44, 66, 235)
                           : hovered  ? IM_COL32(16, 33, 50, 220)
                                      : IM_COL32(11, 20, 31, 205);
        dl->AddRectFilled(c0, c1, cardBg, 2.0f * sScale);

        // Crisp 16:9 map preview thumbnail (no upscaling). The compact style (3) drops it for a dense text list.
        ImVec2 t1(c0.x, c1.y);
        if (!compact) {
            const ImVec2 t0(c0.x + pad, c0.y + pad);
            t1 = ImVec2(t0.x + thumbW, t0.y + thumbH);
            dl->AddRectFilled(t0, t1, IM_COL32(7, 12, 18, 255));
            const std::string scenario = MapNames::ScenarioOf(s.mapName);
            const std::string& artKey = scenario.empty() ? s.mapName : scenario;
            UiTexture thumbTex = UiArt::MapPreview(artKey, true);
            if (!thumbTex) thumbTex = UiArt::MapPhoto(artKey);
            if (thumbTex) {
                UiArt::DrawCoverCrisp(dl, thumbTex, t0, t1, IM_COL32(255, 255, 255, 255), 1.0f);
            } else {
                dl->AddLine(t0, t1, IM_COL32(34, 54, 74, 90), 1.0f);
                dl->AddLine(ImVec2(t1.x, t0.y), ImVec2(t0.x, t1.y), IM_COL32(34, 54, 74, 90), 1.0f);
                const char* noArt = s.isMod ? "MOD" : "MAP";
                const ImVec2 nas = bodyFont->CalcTextSizeA(12.0f * sScale, FLT_MAX, 0.0f, noArt);
                dl->AddText(bodyFont, 12.0f * sScale, ImVec2(t0.x + (thumbW - nas.x) * 0.5f, t0.y + (thumbH - nas.y) * 0.5f),
                            IM_COL32(110, 138, 165, 200), noArt);
            }
            dl->AddRect(t0, t1, IM_COL32(90, 140, 180, 120), 0.0f, 0, 1.0f);

            if (s.isOwn || s.isFavorite) {
                const char* corner = s.isOwn ? "YOU" : "FAV";
                const ImU32 cornerBg = s.isOwn ? IM_COL32(22, 130, 185, 235) : IM_COL32(195, 145, 22, 235);
                const ImVec2 cs = bodyFont->CalcTextSizeA(11.0f * sScale, FLT_MAX, 0.0f, corner);
                dl->AddRectFilled(ImVec2(t0.x + 3.0f * sScale, t0.y + 3.0f * sScale),
                                  ImVec2(t0.x + cs.x + 11.0f * sScale, t0.y + cs.y + 6.0f * sScale), cornerBg, 2.0f * sScale);
                dl->AddText(bodyFont, 11.0f * sScale, ImVec2(t0.x + 7.0f * sScale, t0.y + 4.5f * sScale),
                            IM_COL32(250, 252, 255, 255), corner);
            }
        }

        // Info column
        const float textLeft = compact ? c0.x + U(14.0f) : t1.x + U(14.0f);
        const float textRight = c1.x - metricsW - U(12.0f);
        const float infoW = std::max(U(120.0f), textRight - textLeft);

        struct PillBadge { const char* text; ImU32 bg; ImU32 fg; };
        std::vector<PillBadge> badges;
        if (!compact) {
            if (s.isMod) {
                const bool modReady = s.modId.empty() || ModInstalledCached(s.modId);
                if (modReady) {
                    badges.push_back({ "MOD", IM_COL32(60, 195, 235, 240), IM_COL32(8, 22, 30, 255) });
                } else {
                    badges.push_back({ "MOD - NOT INSTALLED", IM_COL32(255, 186, 74, 240), IM_COL32(34, 22, 4, 255) });
                }
            }
            if (s.isPasswordProtected) badges.push_back({ "LOCKED", IM_COL32(215, 165, 38, 235), IM_COL32(22, 16, 6, 255) });
            if (s.inProgress) badges.push_back({ "LIVE", IM_COL32(225, 128, 35, 235), IM_COL32(255, 248, 238, 255) });
            if (s.build.find("-dev") != std::string::npos) badges.push_back({ "DEV", IM_COL32(95, 78, 175, 230), IM_COL32(240, 238, 255, 255) });
            const bool compatible = s.build.empty() || noble::AreBuildsCompatible(s.build, noble::kBuildTag);
            if (!compatible) badges.push_back({ "OLD/MISMATCH", IM_COL32(190, 52, 52, 235), IM_COL32(255, 242, 242, 255) });
        }

        float badgeBlockW = 0.0f;
        const float badgeFontPx = 11.5f * sScale;
        for (const auto& b : badges) {
            badgeBlockW += bodyFont->CalcTextSizeA(badgeFontPx, FLT_MAX, 0.0f, b.text).x + 16.0f * sScale;
        }

        const float namePx = compact ? 15.5f * sScale : (style == 2 ? 20.0f * sScale : 17.5f * sScale);
        const float maxNameW = std::max(U(80.0f), infoW - badgeBlockW - U(8.0f));
        const std::string displayName = EllipsizeLabel(titleFont, namePx, s.name.empty() ? "Unnamed Halo: Reach Server" : s.name, maxNameW);
        const ImVec2 nameSz = titleFont->CalcTextSizeA(namePx, FLT_MAX, 0.0f, displayName.c_str());
        const float rowTopY = compact ? c0.y + U(7.0f) : (style == 2 ? c0.y + U(17.0f) : c0.y + U(13.0f));
        dl->AddText(titleFont, namePx, ImVec2(textLeft, rowTopY),
                    selected ? IM_COL32(255, 250, 238, 255) : IM_COL32(228, 238, 248, 255), displayName.c_str());

        float bx = textLeft + nameSz.x + 8.0f * sScale;
        for (const auto& b : badges) {
            const ImVec2 bs = bodyFont->CalcTextSizeA(badgeFontPx, FLT_MAX, 0.0f, b.text);
            const float bw = bs.x + 12.0f * sScale;
            const float bh = bs.y + 4.0f * sScale;
            const float by = rowTopY + (nameSz.y - bh) * 0.5f;
            dl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + bw, by + bh), b.bg, 2.0f * sScale);
            dl->AddText(bodyFont, badgeFontPx, ImVec2(bx + 6.0f * sScale, by + 2.0f * sScale), b.fg, b.text);
            bx += bw + 5.0f * sScale;
        }

        // Subtitle line: Mode  ·  Map  ·  Game Type  ·  Region
        const float subPx = compact ? 11.5f * sScale : 13.5f * sScale;
        const float rowSubY = rowTopY + nameSz.y + (compact ? U(1.0f) : U(6.0f));
        std::string subLine = UpperAscii(s.gameMode.empty() ? "MULTIPLAYER" : s.gameMode);
        subLine += "   |   " + (s.mapName.empty() ? std::string("Unknown Map") : MapNames::Display(s.mapName));
        if (!s.variantName.empty()) subLine += "   |   " + s.variantName;
        if (s.isMod) {
            subLine += "   |   ";
            subLine += s.modTitle.empty() ? std::string("MODDED MAP") : ("MOD: " + s.modTitle);
        }
        if (!s.region.empty() && s.region != "YOU") subLine += "   |   " + s.region;
        else if (!s.source.empty()) subLine += "   |   " + s.source;

        const std::string subClip = EllipsizeLabel(bodyFont, subPx, subLine, infoW);
        dl->AddText(bodyFont, subPx, ImVec2(textLeft, rowSubY), IM_COL32(158, 182, 206, 230), subClip.c_str());

        // Right metrics column: Players + capacity bar, Ping + signal bars
        const float mx0 = c1.x - metricsW;
        dl->AddLine(ImVec2(mx0, c0.y + U(10)), ImVec2(mx0, c1.y - U(10)), IM_COL32(60, 92, 120, 95), 1.0f);

        char playerBuf[32];
        std::snprintf(playerBuf, sizeof(playerBuf), "%d / %d", s.playerCount, s.maxPlayers);
        const float plPx = compact ? 14.5f * sScale : 16.5f * sScale;
        const ImVec2 plSz = titleFont->CalcTextSizeA(plPx, FLT_MAX, 0.0f, playerBuf);
        const float plX = mx0 + (compact ? U(10.0f) : U(16.0f));
        const float plY = compact ? c0.y + U(6.0f) : c0.y + U(15.0f);
        dl->AddText(titleFont, plPx, ImVec2(plX, plY), IM_COL32(236, 244, 250, 255), playerBuf);

        const float capW = compact ? U(44.0f) : U(62.0f);
        const float capH = compact ? 3.0f * sScale : 4.0f * sScale;
        const ImVec2 bar0(plX, plY + plSz.y + U(5.0f));
        const ImVec2 bar1(plX + capW, bar0.y + capH);
        dl->AddRectFilled(bar0, bar1, IM_COL32(30, 48, 66, 200), 1.0f * sScale);
        if (s.maxPlayers > 0 && s.playerCount > 0) {
            const float fillFrac = std::clamp(static_cast<float>(s.playerCount) / static_cast<float>(s.maxPlayers), 0.0f, 1.0f);
            const ImU32 fillCol = (fillFrac >= 0.99f) ? IM_COL32(235, 110, 82, 255) : NobleUi::kAccent;
            dl->AddRectFilled(bar0, ImVec2(bar0.x + capW * fillFrac, bar1.y), fillCol, 1.0f * sScale);
        }

        // Ping + signal strength on right edge of card
        const float pingX = c1.x - U(20.0f);
        const float midY = c0.y + cardH * 0.5f;
        DrawSignalBars(dl, ImVec2(pingX - U(18.0f), midY - 7.0f * sScale), sScale, s.ping);
        if (s.ping >= 0) {
            char pBuf[24];
            std::snprintf(pBuf, sizeof(pBuf), "%d ms", s.ping);
            const ImU32 pCol = (s.ping <= 75) ? IM_COL32(82, 224, 128, 255)
                             : (s.ping <= 145) ? IM_COL32(255, 198, 72, 255)
                                               : IM_COL32(245, 95, 85, 255);
            const ImVec2 pSz = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, pBuf);
            dl->AddText(bodyFont, 13.5f * sScale, ImVec2(pingX - U(26.0f) - pSz.x, midY - pSz.y * 0.5f), pCol, pBuf);
        } else {
            const ImVec2 pSz = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, "-- ms");
            dl->AddText(bodyFont, 13.5f * sScale, ImVec2(pingX - U(26.0f) - pSz.x, midY - pSz.y * 0.5f),
                        IM_COL32(125, 145, 165, 180), "-- ms");
        }

        if (compact) {
            dl->AddRectFilled(c0, c1, cardBg, 2.0f * sScale);
            dl->AddRect(c0, c1, selected ? IM_COL32(143, 216, 255, 200) : IM_COL32(72, 112, 148, 110),
                        2.0f * sScale, 0, 1.0f);
            dl->AddLine(ImVec2(c0.x + U(6.0f), c1.y + cardGap * 0.5f),
                        ImVec2(c1.x - U(6.0f), c1.y + cardGap * 0.5f), IM_COL32(64, 100, 136, 80), 1.0f);
        } else {
            // A thin plate border keeps the cards separated in the blue-glass style (cyan, not amber).
            dl->AddRect(c0, c1, selected ? IM_COL32(143, 216, 255, 170) : IM_COL32(66, 112, 150, 95),
                        2.0f * sScale, 0, 1.0f);
            NobleUi::DrawTacticalFrame(dl, c0, c1, sScale, focusedCard, selected, hovered, focusedCard && style == 1);
        }
        ImGui::Dummy(ImVec2(listAvailW, cardGap));
    }
}

void ServerBrowser::RenderServerDetails() {
    const float sScale = NobleUi::Scale();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* titleFont = NobleStyle::TitleFont() ? NobleStyle::TitleFont() : ImGui::GetFont();
    ImFont* bodyFont = ImGui::GetFont();

    const float headerPx = 19.0f * sScale;
    const ImVec2 h0 = ImGui::GetCursorScreenPos();
    const float availW = ImGui::GetContentRegionAvail().x;
    dl->AddText(titleFont, headerPx, h0, IM_COL32(236, 244, 252, 255), "GAME DETAILS");
    if (s_browserPane == 1 && UiPad::HasGamepad()) {
        const char* pHint = "[LEFT] LIST   [A] ACTIVATE";
        const ImVec2 phSz = bodyFont->CalcTextSizeA(11.5f * sScale, FLT_MAX, 0.0f, pHint);
        dl->AddText(bodyFont, 11.5f * sScale, ImVec2(h0.x + availW - phSz.x, h0.y + 4.0f * sScale),
                    NobleUi::kAmber, pHint);
    }
    dl->AddLine(ImVec2(h0.x, h0.y + 26.0f * sScale), ImVec2(h0.x + availW, h0.y + 26.0f * sScale),
                IM_COL32(78, 138, 185, 130), 1.0f * sScale);
    ImGui::Dummy(ImVec2(availW, 32.0f * sScale));

    if (m_selectedServerIndex < 0 || m_selectedServerIndex >= static_cast<int>(m_servers.size())) {
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped("Select a server from the list to inspect its map artwork, player count, ping, and mod requirements.");
        ImGui::Spacing();
        ImGui::TextWrapped("Press [A] or Double-Click a session to join the pre-game lobby.");
        ImGui::Spacing();
        ImGui::TextWrapped("%d session(s) currently tracked.", static_cast<int>(m_servers.size()));
        ImGui::PopStyleColor();
        return;
    }

    const ServerEntry& s = m_servers[m_selectedServerIndex];

    const bool modalBlocking = ImGui::GetIO().WantTextInput || m_showDirectConnectModal || m_showMasterModal || ModDownloadPrompt::Get().IsActive();
    bool& triggerFocusedBtn = s_detailTriggerFocused;
    triggerFocusedBtn = false;
    if (!modalBlocking && s_browserPane == 1) {
        if (UiPad::Left() || ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false)) {
            s_browserPane = 0;
            MenuSounds::Scroll();
        } else if (UiPad::Down() || ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)) {
            s_detailBtnIdx = (s_detailBtnIdx + 1) % 3;
            MenuSounds::Scroll();
        } else if (UiPad::Up() || ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)) {
            s_detailBtnIdx = (s_detailBtnIdx + 2) % 3;
            MenuSounds::Scroll();
        } else if (UiPad::Right() && s_detailBtnIdx == 1) {
            s_detailBtnIdx = 2;
            MenuSounds::Scroll();
        } else if (UiPad::AcceptPressed() || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
            UiPad::ConsumeAccept();
            triggerFocusedBtn = true;
        }
        if (UiPad::ActionY() || ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
            ToggleFavorite(s.hostAddress, s.port);
            RefreshServers();
            MenuSounds::Click();
        }
    }

    // Crisp 16:9 Map Artwork Header (never upscaled above native asset resolution)
    {
        const float photoW = ImGui::GetContentRegionAvail().x;
        const float photoH = std::min(photoW * 9.0f / 16.0f, U(76.0f));
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const ImVec2 p1(p0.x + photoW, p0.y + photoH);
        dl->AddRectFilled(p0, p1, IM_COL32(8, 13, 20, 255));
        const std::string scenario = MapNames::ScenarioOf(s.mapName);
        const std::string& artKey = scenario.empty() ? s.mapName : scenario;
        UiTexture photo = UiArt::MapPhoto(artKey);
        if (!photo) photo = UiArt::MapPreview(artKey, true);
        if (photo) {
            UiArt::DrawCoverCrisp(dl, photo, p0, p1, IM_COL32(255, 255, 255, 255), 1.0f);
        } else {
            const char* missing = s.isMod ? "NO PICTURE FROM THIS MOD" : "NO MAP PICTURE";
            const ImVec2 ts = bodyFont->CalcTextSizeA(13.5f * sScale, FLT_MAX, 0.0f, missing);
            dl->AddText(bodyFont, 13.5f * sScale, ImVec2(p0.x + (photoW - ts.x) * 0.5f, p0.y + (photoH - ts.y) * 0.5f),
                        IM_COL32(120, 138, 158, 210), missing);
        }
        dl->AddRectFilledMultiColor(ImVec2(p0.x, p1.y - photoH * 0.55f), p1,
                                    IM_COL32(4, 8, 14, 0), IM_COL32(4, 8, 14, 0),
                                    IM_COL32(4, 8, 14, 240), IM_COL32(4, 8, 14, 240));
        NobleUi::DrawTacticalFrame(dl, p0, p1, sScale, false, true, false, false);

        const std::string mapTitle = UpperAscii(s.mapName.empty() ? std::string("Unknown map") : MapNames::Display(s.mapName));
        const std::string clippedMap = EllipsizeLabel(titleFont, 18.0f * sScale, mapTitle, photoW - 20.0f * sScale);
        dl->AddText(titleFont, 18.0f * sScale, ImVec2(p0.x + 10.0f * sScale, p1.y - 26.0f * sScale),
                    IM_COL32(244, 248, 252, 255), clippedMap.c_str());
        ImGui::Dummy(ImVec2(photoW, photoH));
    }

    ImGui::Dummy(ImVec2(0.0f, 8.0f * sScale));

    // Server title & address header
    const std::string srvTitle = s.name.empty() ? "Unnamed Session" : s.name;
    const std::string srvClip = EllipsizeLabel(titleFont, 16.5f * sScale, srvTitle, availW - 4.0f * sScale);
    const ImVec2 stPos = ImGui::GetCursorScreenPos();
    dl->AddText(titleFont, 16.5f * sScale, stPos, IM_COL32(238, 245, 252, 255), srvClip.c_str());
    ImGui::Dummy(ImVec2(availW, 20.0f * sScale));

    char addrLine[128];
    std::snprintf(addrLine, sizeof(addrLine), "%s:%u%s", s.hostAddress.c_str(), s.port, s.isOwn ? "  (HOSTING)" : "");
    dl->AddText(bodyFont, 13.0f * sScale, ImGui::GetCursorScreenPos(), IM_COL32(142, 168, 192, 215), addrLine);
    ImGui::Dummy(ImVec2(availW, 18.0f * sScale));
    ImGui::Dummy(ImVec2(0.0f, 4.0f * sScale));

    // Version / mod problems sit right under the address so they are visible before the pane scrolls; every line wraps
    // inside the panel (plain TextColored does not wrap and used to run under the scrollbar - Tyler's screenshot).
    const float wrapW = (std::max)(U(60.0f), availW - 18.0f * sScale);
    const auto wrappedLine = [&](bool coloured, ImVec4 colour, const std::string& text) {
        if (coloured) ImGui::PushStyleColor(ImGuiCol_Text, colour);
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapW);
        ImGui::TextWrapped("%s", text.c_str());
        ImGui::PopTextWrapPos();
        if (coloured) ImGui::PopStyleColor();
    };
    const bool buildMismatch = !s.build.empty() && !noble::AreBuildsCompatible(s.build, noble::kBuildTag);
    const bool buildDiffers = !s.build.empty() && s.build != noble::kBuildTag;
    if (buildDiffers) {
        ImGui::Spacing();
        if (buildMismatch) {
            wrappedLine(true, ImVec4{0.97f, 0.38f, 0.38f, 1.0f},
                        std::string("VERSION MISMATCH: host runs ") +
                            (!s.gameVersion.empty() ? s.gameVersion : std::string(noble::ExtractBaseVersion(s.build))) +
                            " and you have " + std::string(noble::VERSION_STRING) +
                            " - you cannot join until one of you updates.");
        } else {
            // Different build string on the same release line (usually dev vs release): allowed, but say so.
            wrappedLine(true, ImVec4{1.0f, 0.82f, 0.35f, 1.0f},
                        std::string("Host ") +
                            (!s.gameVersion.empty() ? s.gameVersion : std::string(noble::ExtractBaseVersion(s.build))) +
                            " (yours " + std::string(noble::VERSION_STRING) + ") - compatible.");
        }
    }
    if (s.isMod || !s.modId.empty()) {
        ImGui::Spacing();
        if (s.isMod && s.modId.empty()) wrappedLine(true, ImVec4{0.35f, 0.85f, 0.95f, 1.0f}, "Custom map / modded content");
        if (!s.modId.empty()) {
            const bool modReady = ModInstalledCached(s.modId);
            const std::string modLabel = s.modTitle.empty() ? ("Workshop item " + s.modId) : s.modTitle;
            wrappedLine(false, ImVec4{}, "Required Workshop mod: " + modLabel);
            if (modReady) {
                wrappedLine(true, ImVec4{0.45f, 0.86f, 0.55f, 1.0f}, "Mod installed - ready to join.");
            } else {
                wrappedLine(true, ImVec4{1.0f, 0.78f, 0.3f, 1.0f},
                            "Mod NOT installed - DOWNLOAD MOD & JOIN installs it first.");
            }
        }
    }
    ImGui::Dummy(ImVec2(0.0f, 4.0f * sScale));

    // Structured Telemetry Key-Value Table
    int rowIdx = 0;
    const float keyW = U(90.0f);
    const float rowPadY = 4.0f * sScale;
    const float rowTextPx = 14.0f * sScale;
    auto drawInfoRow = [&](const char* key, const std::string& val, ImU32 valCol = IM_COL32(226, 236, 245, 255)) {
        const ImVec2 r0 = ImGui::GetCursorScreenPos();
        const float valMaxW = std::max(U(80.0f), availW - keyW - 12.0f * sScale);
        const std::string vClip = EllipsizeLabel(bodyFont, rowTextPx, val, valMaxW);
        const float rowH = 19.0f * sScale;
        const ImVec2 r1(r0.x + availW, r0.y + rowH);
        if ((rowIdx++ % 2) == 0) {
            dl->AddRectFilled(r0, r1, IM_COL32(14, 24, 36, 150));
        }
        dl->AddText(bodyFont, rowTextPx, ImVec2(r0.x + 6.0f * sScale, r0.y + rowPadY), IM_COL32(138, 164, 188, 220), key);
        dl->AddText(bodyFont, rowTextPx, ImVec2(r0.x + keyW, r0.y + rowPadY), valCol, vClip.c_str());
        ImGui::Dummy(ImVec2(availW, rowH));
    };

    drawInfoRow("MODE", s.gameMode + (s.inProgress ? "  [LIVE]" : ""));
    drawInfoRow("MAP", MapNames::Display(s.mapName));
    if (!s.variantName.empty()) {
        drawInfoRow("GAMETYPE", s.variantName);
    }
    {
        char pl[32];
        std::snprintf(pl, sizeof(pl), "%d / %d", s.playerCount, s.maxPlayers);
        drawInfoRow("PLAYERS", pl);
    }
    {
        const bool compat = s.build.empty() || noble::AreBuildsCompatible(s.build, noble::kBuildTag);
        // Show the same version string the main menu shows. The wire build tag (0.19.x) is only for
        // compatibility; players were seeing it here and thought the browser was on another version.
        std::string shown = !s.gameVersion.empty() ? s.gameVersion
                                                   : (s.build.empty() ? std::string("-") : s.build);
        if (!s.build.empty() && !compat) shown += " (Incompatible)";
        const ImU32 verCol = !compat ? IM_COL32(245, 95, 85, 255)
                           : (s.build.find("-dev") != std::string::npos) ? IM_COL32(255, 212, 72, 255)
                                                                         : IM_COL32(226, 236, 245, 255);
        drawInfoRow("VERSION", shown, verCol);
    }
    {
        std::string pingStr = s.ping >= 0 ? (std::to_string(s.ping) + " ms") : "-";
        const ImU32 pingCol = s.ping < 0 ? IM_COL32(138, 164, 188, 220)
                            : s.ping <= 75 ? IM_COL32(82, 224, 128, 255)
                            : s.ping <= 145 ? IM_COL32(255, 198, 72, 255)
                                            : IM_COL32(245, 95, 85, 255);
        drawInfoRow("PING", pingStr, pingCol);
    }

    if (s.isPasswordProtected) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "Password protected session");
    }
    if (s.inProgress) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped("Match in progress - join in progress is enabled.");
        ImGui::PopStyleColor();
    }

    ImGui::Dummy(ImVec2(0.0f, 6.0f * sScale));

    auto focusOutline = [&](int btnIndex) {
        if (s_browserPane == 1 && s_detailBtnIdx == btnIndex) {
            NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, true, true, false, true);
        }
    };

    if (s.isOwn) {
        ImGui::Spacing();
        ImGui::TextDisabled("You are hosting this session.");
        if (NobleStyle::DangerButton("STOP HOSTING", ImVec2(-1, U(36))) || (triggerFocusedBtn && s_detailBtnIdx == 0)) {
            SessionManager::Get().StopHosting();
            MasterServerClient::Get().StopAdvertisingSync();
            m_status = "Session closed. Stopped hosting.";
            RefreshServers();
        }
        focusOutline(0);
    } else {
        const bool compatible = s.build.empty() || noble::AreBuildsCompatible(s.build, noble::kBuildTag);
        if (!compatible) {
            const int rel = noble::CompareBuilds(noble::kBuildTag, s.build);
            // The mismatch itself is already shown in the wrapped warning above; here only the update actions remain.
            if (rel < 0) {
                Updater& up = Updater::Get();
                if (up.IsInitialized()) {
                    const Updater::Status us = up.Snapshot();
                    if (us.state == Updater::State::Idle || us.state == Updater::State::UpToDate ||
                        us.state == Updater::State::Error) {
                        if (us.state == Updater::State::Error && !us.error.empty()) {
                            ImGui::TextDisabled("Update check failed: %s", us.error.c_str());
                        }
                        if (NobleStyle::PrimaryButton("CHECK FOR UPDATE", ImVec2(-1, U(38))) || (triggerFocusedBtn && s_detailBtnIdx == 0)) {
                            up.StartCheck();
                        }
                        focusOutline(0);
                    } else if (us.state == Updater::State::Checking) {
                        ImGui::TextDisabled("Checking for an update...");
                    } else if (us.state == Updater::State::Available) {
                        const std::string lbl = "DOWNLOAD UPDATE  " + Updater::FormatBytes(us.assetSize);
                        if (NobleStyle::PrimaryButton(lbl.c_str(), ImVec2(-1, U(38))) || (triggerFocusedBtn && s_detailBtnIdx == 0)) {
                            up.StartDownload();
                        }
                        focusOutline(0);
                    } else if (us.state == Updater::State::Downloading) {
                        ImGui::ProgressBar(us.progress, ImVec2(-1, U(22)));
                    } else if (us.state == Updater::State::Ready) {
                        if (NobleStyle::PrimaryButton("RESTART TO UPDATE", ImVec2(-1, U(38))) || (triggerFocusedBtn && s_detailBtnIdx == 0)) {
                            if (up.ApplyAndRestart()) m_closeForUpdate = true;
                        }
                        focusOutline(0);
                    }
                } else {
                    ImGui::TextDisabled("Update the game from the main menu to join this match.");
                }
            } else {
                ImGui::TextDisabled("This host is on an older version. Ask them to update.");
            }
            ImGui::BeginDisabled();
            NobleStyle::PrimaryButton("JOIN BLOCKED (VERSION)", ImVec2(-1, U(38)));
            ImGui::EndDisabled();
        } else {
            const bool modMissing = !s.modId.empty() && !ModInstalledCached(s.modId);
            const std::string joinLabel = modMissing ? "DOWNLOAD MOD & JOIN"
                                         : s.inProgress ? "JOIN IN PROGRESS"
                                                        : "JOIN PRE-GAME LOBBY";
            if (NobleStyle::PrimaryButton(joinLabel.c_str(), ImVec2(-1, U(40))) || (triggerFocusedBtn && s_detailBtnIdx == 0)) {
                AttemptJoin(s);
            }
            focusOutline(0);
        }
    }

    // FAVOURITE / COPY INVITE live in the pinned footer (RenderServerActions) so they cannot scroll out of view.
}

void ServerBrowser::RenderServerActions() {
    const float sScale = NobleUi::Scale();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (m_selectedServerIndex < 0 || m_selectedServerIndex >= static_cast<int>(m_servers.size())) {
        ImGui::TextDisabled("Select a server from the list to favourite it or copy its invite.");
        return;
    }
    const ServerEntry& s = m_servers[m_selectedServerIndex];
    auto focusOutline = [&](int btnIndex) {
        if (s_browserPane == 1 && s_detailBtnIdx == btnIndex) {
            NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, true, true, false, true);
        }
    };
    const float halfW = (ImGui::GetContentRegionAvail().x - U(8.0f)) * 0.5f;
    if (ImGui::Button(s.isFavorite ? "UNFAVOURITE" : "FAVOURITE", ImVec2(halfW, U(32))) ||
        (s_detailTriggerFocused && s_detailBtnIdx == 1)) {
        ToggleFavorite(s.hostAddress, s.port);
        RefreshServers();
    }
    focusOutline(1);
    ImGui::SameLine(0.0f, U(8.0f));
    if (ImGui::Button("COPY INVITE", ImVec2(halfW, U(32))) ||
        (s_detailTriggerFocused && s_detailBtnIdx == 2)) {
        std::string ipToCopy = s.hostAddress;
        if (s.isOwn) {
            std::string pubIp = UPnP::Get().GetExternalIP();
            if (UPnP::Get().IsPublicIP(pubIp)) ipToCopy = pubIp;
        }
        const std::string invite = ipToCopy + ":" + std::to_string(s.port);
        ImGui::SetClipboardText(invite.c_str());
        m_status = "Copied invite address " + invite + " to clipboard.";
    }
    focusOutline(2);
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
    if (!CanStartJoinNow(m_status)) return;
    if (m_joinLobbyHandler) {
        StampJoinAttemptNow();
        UiPad::ConsumeAll();
        m_joinLobbyHandler(s);
    } else {
        TryJoin(s.hostAddress, s.port, s.isPasswordProtected);
    }
}

void ServerBrowser::TryJoin(const std::string& address, uint16_t port, bool hasPassword) {
    (void)hasPassword;
    const std::string key = Key(address, port);
    for (const auto& known : m_servers) {
        if (known.hostAddress == address && known.port == port && JoinBlockedByVersion(known)) return;
    }
    if (!CanStartJoinNow(m_status)) return;

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

    StampJoinAttemptNow();
    UiPad::ConsumeAll();

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

void ServerBrowser::OpenJoinCodeModal() {
    m_showJoinCodeModal = true;
    m_joinCodeError.clear();
    MenuSounds::Click();
}

void ServerBrowser::JoinByCode() {
    InviteCodeInfo info;
    std::string error;
    if (!DecodeInviteCode(m_joinCode, &info, &error)) {
        m_joinCodeError = error;
        return;
    }
    if (!CanStartJoinNow(m_status)) {
        m_joinCodeError = m_status;
        return;
    }
    if (!MasterServerClient::Get().Enabled()) {
        m_joinCodeError = "Join codes need the master server (SETTINGS > ONLINE SERVICES).";
        return;
    }
    const LobbyState lobby = MasterServerClient::Get().FetchLobbyStateSync(info.sessionId);
    if (!lobby.ok) {
        m_joinCodeError = "That code is not for a live lobby any more - ask for a new one.";
        return;
    }
    ServerEntry s;
    s.id = info.sessionId;
    s.name = lobby.name.empty() ? "Invite code lobby" : lobby.name;
    for (const auto& p : lobby.roster) {
        if (!p.isHost) continue;
        s.hostPlayerName = p.username;
        s.hostServiceTag = p.serviceTag;
        break;
    }
    s.hostAddress = lobby.hostIp;
    s.gameVersion = lobby.gameVersion;
    s.port = lobby.hostPort ? lobby.hostPort : 11774;
    s.mapName = lobby.map;
    s.gameMode = lobby.mode;
    s.variantName = lobby.variant;
    s.playerCount = lobby.playerCount;
    s.maxPlayers = lobby.maxPlayers;
    s.inProgress = lobby.inProgress;
    s.isMod = !lobby.modId.empty();
    s.modId = lobby.modId;
    s.modTitle = lobby.modTitle;
    s.source = "CODE";
    NOBLE_INFO("ServerBrowser: join by code -> lobby '{}' ({}:{}, session {}, {})", s.name, s.hostAddress, s.port,
               info.sessionId, InviteCodePrivacyName(info));
    if (!m_joinLobbyHandler) {
        m_joinCodeError = "Join-by-code needs the in-game menu (start Noble normally).";
        return;
    }
    StampJoinAttemptNow();
    UiPad::ConsumeAll();
    m_showJoinCodeModal = false;
    m_joinCodeError.clear();
    ImGui::CloseCurrentPopup();
    m_joinLobbyHandler(s);
}

void ServerBrowser::RenderJoinCodeModal() {
    ImGui::OpenPopup("JOIN WITH CODE");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(U(500.0f), 0));
    if (ImGui::BeginPopupModal("JOIN WITH CODE", &m_showJoinCodeModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        const float sScale = NobleUi::Scale();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (UiPad::BackPressed() || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            UiPad::ConsumeBack();
            m_showJoinCodeModal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::TextWrapped("Paste the lobby code a friend shared (Discord, chat, etc.). It works for public and private "
                           "lobbies - private lobbies are not listed in the browser, but this code still joins them.");
        ImGui::Spacing();
        ImGui::Text("Lobby code");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - U(78.0f));
        bool codeSubmitted = false;
        if (ImGui::InputTextWithHint("##LobbyCode", "NB1-XXXX-XXXX-XXXX-XXXX", m_joinCode, IM_ARRAYSIZE(m_joinCode),
                                     ImGuiInputTextFlags_EnterReturnsTrue))
            codeSubmitted = true;
        if (ImGui::IsItemEdited()) m_joinCodeError.clear();
        ImGui::SameLine();
        if (ImGui::Button("PASTE", ImVec2(U(68.0f), 0))) {
            if (const char* clip = ImGui::GetClipboardText()) std::snprintf(m_joinCode, sizeof(m_joinCode), "%s", clip);
            m_joinCodeError.clear();
        }
        if (!m_joinCodeError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.42f, 0.42f, 1.0f));
            ImGui::TextWrapped("%s", m_joinCodeError.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::TextDisabled("Typing is forgiving: lowercase and missing dashes are fine.");
        }
        ImGui::Spacing();
        const bool hasText = m_joinCode[0] != 0;
        ImGui::BeginDisabled(!hasText);
        if (NobleStyle::PrimaryButton("JOIN LOBBY", ImVec2(U(150.0f), U(34.0f))) || (hasText && codeSubmitted)) {
            JoinByCode();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(U(95.0f), U(34.0f)))) {
            m_showJoinCodeModal = false;
            m_joinCodeError.clear();
            ImGui::CloseCurrentPopup();
        }
        (void)sScale;
        (void)dl;
        ImGui::EndPopup();
    }
}

void ServerBrowser::RenderDirectConnectModal() {
    ImGui::OpenPopup("DIRECT CONNECT");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(U(440), 0));
    if (ImGui::BeginPopupModal("DIRECT CONNECT", &m_showDirectConnectModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        const float sScale = NobleUi::Scale();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        if (UiPad::BackPressed() || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            UiPad::ConsumeBack();
            m_showDirectConnectModal = false;
            ImGui::CloseCurrentPopup();
        }

        static int s_dcBtn = 0;
        if (ImGui::IsWindowAppearing()) s_dcBtn = 0;

        ImGui::Text("Address (IP or IP:port)");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - U(78.0f));
        ImGui::InputTextWithHint("##Address", "IP or IP:port (e.g. 147.124.149.202:11774)", m_directConnectIp, IM_ARRAYSIZE(m_directConnectIp));
        ImGui::SameLine();
        if (ImGui::Button("PASTE", ImVec2(U(68), 0))) {
            if (const char* clip = ImGui::GetClipboardText()) std::snprintf(m_directConnectIp, sizeof(m_directConnectIp), "%s", clip);
        }
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(-1);

        ImGui::InputText("##Password", m_directConnectPassword, IM_ARRAYSIZE(m_directConnectPassword), ImGuiInputTextFlags_Password);


        std::string address;
        uint16_t port = 0;
        const bool valid = ParseAddress(m_directConnectIp, address, port);
        if (!valid) ImGui::TextColored(ImVec4(0.95f, 0.4f, 0.4f, 1.0f), "Enter an address such as 192.168.1.20:11774");

        bool padActivate = false;
        if (!ImGui::GetIO().WantTextInput) {
            if (UiPad::Left() || ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false)) {
                s_dcBtn = (s_dcBtn + 2) % 3;
                MenuSounds::Scroll();
            } else if (UiPad::Right() || ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) {
                s_dcBtn = (s_dcBtn + 1) % 3;
                MenuSounds::Scroll();
            } else if (UiPad::AcceptPressed()) {
                UiPad::ConsumeAccept();
                padActivate = true;
            }
        }

        auto drawBtnFocus = [&](int idx) {
            if (UiPad::HasGamepad() && s_dcBtn == idx) {
                NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, true, true, false, true);
            }
        };

        ImGui::Spacing();
        ImGui::BeginDisabled(!valid);
        if (NobleStyle::PrimaryButton("CONNECT", ImVec2(U(120), U(34))) || (padActivate && valid && s_dcBtn == 0)) {
            TryJoin(address, port, m_directConnectPassword[0] != 0);
            m_showDirectConnectModal = false;
            ImGui::CloseCurrentPopup();
        }
        drawBtnFocus(0);
        ImGui::SameLine();
        if (ImGui::Button("SAVE AS FAVOURITE", ImVec2(U(175), U(34))) || (padActivate && valid && s_dcBtn == 1)) {
            ToggleFavorite(address, port);
            m_lastRefresh = -100.0;
        }
        drawBtnFocus(1);
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(U(95), U(34))) || (padActivate && s_dcBtn == 2)) {
            m_showDirectConnectModal = false;
            ImGui::CloseCurrentPopup();
        }
        drawBtnFocus(2);
        ImGui::EndPopup();
    }
}

void ServerBrowser::RenderMasterServerModal() {
    ImGui::OpenPopup("SELECT MATCHMAKING REGION");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(U(490), 0));
    if (ImGui::BeginPopupModal("SELECT MATCHMAKING REGION", &m_showMasterModal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        const float sScale = NobleUi::Scale();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        if (UiPad::BackPressed() || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            UiPad::ConsumeBack();
            m_showMasterModal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::TextWrapped("Select your preferred matchmaking region. Master servers synchronize federated sessions across regions so you can find games worldwide.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const std::string curUrl = MasterServerClient::Get().Url();
        int curRegion = curUrl.empty() ? 1 : 0;

        static int chosenRegion = 0;
        static int s_regBtn = 0;
        if (ImGui::IsWindowAppearing()) {
            chosenRegion = curRegion;
            s_regBtn = 0;
        }

        if (UiPad::Up() || ImGui::IsKeyPressed(ImGuiKey_UpArrow, false) ||
            UiPad::Down() || ImGui::IsKeyPressed(ImGuiKey_DownArrow, false)) {
            chosenRegion = 1 - chosenRegion;
            MenuSounds::Scroll();
        }
        if (UiPad::Left() || UiPad::Right() ||
            ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false) || ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) {
            s_regBtn = 1 - s_regBtn;
            MenuSounds::Scroll();
        }

        bool padActivate = false;
        if (UiPad::AcceptPressed() || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
            UiPad::ConsumeAccept();
            padActivate = true;
        }

        for (int i = 0; i < 2; ++i) {
            if (ImGui::RadioButton(kMasterRegions[i].label, chosenRegion == i)) {
                chosenRegion = i;
            }
            if (UiPad::HasGamepad() && chosenRegion == i) {
                NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, true, true, false, true);
            }
            ImGui::SameLine(U(225));
            ImGui::TextDisabled("(%s)", kMasterRegions[i].location);
        }

        NobleStyle::SectionGap();
        if (NobleStyle::PrimaryButton("SAVE & APPLY", ImVec2(U(150), U(34))) || (padActivate && s_regBtn == 0)) {
            SettingsData d = Settings::Get().Snapshot();
            d.masterServerUrl = kMasterRegions[chosenRegion].url;
            Settings::Get().Update(d);
            MasterServerClient::Get().SetUrl(d.masterServerUrl);
            m_lastMasterRequest = -100.0;
            m_status = std::string("Matchmaking region set to ") + kMasterRegions[chosenRegion].label;
            m_showMasterModal = false;
            ImGui::CloseCurrentPopup();
            RefreshServers();
        }
        if (UiPad::HasGamepad() && s_regBtn == 0) {
            NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, true, true, false, true);
        }
        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(U(105), U(34))) || (padActivate && s_regBtn == 1)) {
            m_showMasterModal = false;
            ImGui::CloseCurrentPopup();
        }
        if (UiPad::HasGamepad() && s_regBtn == 1) {
            NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, true, true, false, true);
        }
        ImGui::EndPopup();
    }
}

} // namespace noble
