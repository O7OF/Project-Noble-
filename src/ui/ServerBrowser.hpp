#include <functional>
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace noble {

struct ServerEntry {
    std::string id;
    std::string name;              // lobby/advert name (never a player name)
    std::string hostPlayerName;    // the host's player name, when the source knows it
    std::string hostServiceTag;    // the host's service tag, when the source knows it
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
    std::string build;          // wire build tag, for compatibility checks
    std::string gameVersion;    // host's display version, as shown in the main menu
    std::string source;         // "LAN", "ONLINE", "FAVORITE", "YOU"
    std::string region;         // "US East", "US West", "UK", "LAN", "YOU"
    std::string mapDescription; // player-facing map blurb (stock) or mod map blurb
    std::string modDescription; // host mod title/description when the game is modded
    // Freshness: a row that drops out of one refresh is kept and flagged instead of vanishing, so the browser
    // can say "not heard lately" (and a join cannot be attempted against a dead listing without feedback).
    // LAN rows age out faster than master-server rows (the LAN beacon repeats every 1.5 s).
    double lastSeen = 0.0;      // ImGui time of the refresh that last listed this row
    int ageSeconds = 0;         // seconds since it was last heard (0 = listed in the current refresh)
    bool heard = true;          // false: kept from an earlier refresh, not heard in the latest scan
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
    void SetStatus(const std::string& status) { m_status = status; }
    void SetJoinLobbyHandler(JoinLobbyHandler handler) { m_joinLobbyHandler = std::move(handler); }
    using DirectJoinHandler = std::function<void(const std::string& map, const std::string& mode, const std::string& address, const std::string& sessionId)>;
    void SetDirectJoinHandler(DirectJoinHandler handler) { m_directJoinHandler = std::move(handler); }

    const std::vector<ServerEntry>& GetServers() const { return m_servers; }

    // True once after the player chose "RESTART TO UPDATE" here; the front end then closes so the
    // update script can replace the install.
    bool ConsumeCloseForUpdate() { const bool r = m_closeForUpdate; m_closeForUpdate = false; return r; }
    bool IsModalOpen() const { return m_showDirectConnectModal || m_showMasterModal || m_showJoinCodeModal; }
    void CloseModals() { m_showDirectConnectModal = false; m_showMasterModal = false; m_showJoinCodeModal = false; }
    // Prominent "JOIN WITH CODE" entry point (Main Menu and Social screen call this, then switch to Screen::ServerBrowser).
    void OpenJoinCodeModal();

private:
    void RenderFilterBar();
    void RenderServerList();
    void RenderServerDetails();
    // FAVOURITE / COPY INVITE row, drawn in a fixed footer under the details pane so it is always visible.
    void RenderServerActions();
    void RenderDirectConnectModal();
    void RenderMasterServerModal();
    void RenderJoinCodeModal();
    void JoinByCode();
    void TryJoin(const std::string& address, uint16_t port, bool hasPassword);
    // Version gate: returns true (and sets the status line) when this client may not join `s`.
    bool JoinBlockedByVersion(const ServerEntry& s);
    void AttemptJoin(const ServerEntry& s);
    bool PassesFilters(const ServerEntry& s) const;
    void ToggleFavorite(const std::string& address, uint16_t port);

    JoinLobbyHandler m_joinLobbyHandler;
    DirectJoinHandler m_directJoinHandler;
    std::vector<ServerEntry> m_servers;
    int m_selectedServerIndex = -1;
    double m_lastRefresh = -100.0;
    double m_lastMasterRequest = -100.0;
    std::string m_status;
    bool m_closeForUpdate = false;

    // Filters & sorting
    char m_searchQuery[128] = { 0 };
    int m_modeFilter = 0; // 0 = All, 1 = Multiplayer, 2 = Campaign, 3 = Firefight, 4 = Forge
    int m_sortBy = 0;     // 0 = Players (High), 1 = Ping (Low), 2 = Name, 3 = Map
    bool m_hideFull = false;
    bool m_hideEmpty = false;
    bool m_hidePassworded = false;   // only show games without a password
    bool m_hideInProgress = false;   // only show games that have not started
    bool m_showOnlyFavorites = false;

    // Direct connect modal state
    bool m_showDirectConnectModal = false;
    char m_directConnectIp[64] = "";
    char m_directConnectPassword[64] = { 0 };

    // Join-by-code modal state
    bool m_showJoinCodeModal = false;
    char m_joinCode[64] = {};
    std::string m_joinCodeError;

    // Master server modal
    bool m_showMasterModal = false;
    int m_selectedRegion = 0;
};

} // namespace noble
