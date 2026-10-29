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
    std::string source;         // "LAN", "ONLINE", "FAVORITE", "YOU"
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
    std::vector<ServerEntry> m_servers;
    int m_selectedServerIndex = -1;
    double m_lastRefresh = -100.0;
    double m_lastMasterRequest = -100.0;
    std::string m_status;

    // Filters
    char m_searchQuery[128] = { 0 };
    int m_modeFilter = 0; // 0 = All, 1 = Multiplayer, 2 = Campaign, 3 = Firefight, 4 = Forge
    bool m_hideFull = false;
    bool m_hideEmpty = false;
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
