#include <functional>
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace noble {

struct ServerEntry {
    std::string id;
    std::string name;
    std::string hostAddress;
    uint16_t port = 11774;
    int ping = 0;               // -1 = not measured
    int playerCount = 0;
    int maxPlayers = 16;
    std::string gameMode;
    std::string mapName;
    std::string variantName;
    bool isPasswordProtected = false;
    bool inProgress = false;
    bool isOwn = false;         // the game this player is hosting
    bool isFavorite = false;
    bool isMod = false;
    std::string modId;          // Steam Workshop published-file id the lobby requires (empty = none)
    std::string modTitle;
    std::string source;         // "LAN", "ONLINE", "FAVORITE", "YOU"
    std::string mapDescription; // player-facing map blurb (stock) or mod map blurb
    std::string modDescription; // host mod title/description when the game is modded
};

// SERVERS tab of the front-end menu: LAN games (UDP beacons from SessionManager), games listed on
// the master server (MasterServerClient), saved favourites, and the game this player hosts.
// The list refreshes itself every two seconds while the tab is shown.
class ServerBrowser {
public:
    ServerBrowser();
    ~ServerBrowser() = default;

    void Render();
    void RefreshServers();

    using JoinLobbyHandler = std::function<void(const ServerEntry& server)>;
    void SetJoinLobbyHandler(JoinLobbyHandler handler) { m_joinLobbyHandler = std::move(handler); }
    using DirectJoinHandler = std::function<void(const std::string& map, const std::string& mode, const std::string& address, const std::string& sessionId)>;
    void SetDirectJoinHandler(DirectJoinHandler handler) { m_directJoinHandler = std::move(handler); }

    const std::vector<ServerEntry>& GetServers() const { return m_servers; }

private:
    void RenderFilterBar();
    void RenderServerList();
    void RenderServerDetails();
    void RenderDirectConnectModal();
    void RenderMasterServerModal();
    void TryJoin(const std::string& address, uint16_t port, bool hasPassword);
    bool PassesFilters(const ServerEntry& s) const;
    void ToggleFavorite(const std::string& address, uint16_t port);

    JoinLobbyHandler m_joinLobbyHandler;
    DirectJoinHandler m_directJoinHandler;
    std::vector<ServerEntry> m_servers;
    int m_selectedServerIndex = -1;
    double m_lastRefresh = -100.0;
    double m_lastMasterRequest = -100.0;
    std::string m_status;

    // Filters
    char m_searchQuery[128] = { 0 };
    int m_modeFilter = 0; // 0 = All, 1 = Campaign, 2 = Multiplayer, 3 = Forge, 4 = Firefight, 5 = Theatre, 6 = Custom
    bool m_hideFull = false;
    bool m_hideEmpty = false;
    bool m_hidePassworded = false;   // only show games without a password
    bool m_hideInProgress = false;   // only show games that have not started
    bool m_showOnlyFavorites = false;

    // Direct connect modal state
    bool m_showDirectConnectModal = false;
    char m_directConnectIp[64] = "";
    char m_directConnectPassword[64] = { 0 };

    // Master server modal
    bool m_showMasterModal = false;
    char m_masterUrl[256] = { 0 };
};

} // namespace noble
