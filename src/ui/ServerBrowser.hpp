#pragma once

#include <string>
#include <vector>
#include <functional>

namespace noble {

struct ServerEntry {
    std::string name;
    std::string hostAddress;
    uint16_t port = 11774;
    int ping = 0;
    int playerCount = 0;
    int maxPlayers = 16;
    std::string gameMode;      // "Slayer", "CTF", "Firefight", "Infection", "Forge", "Campaign"
    std::string mapName;       // "Forge World", "Boneyard", "Countdown", etc.
    std::string variantName;   // "Team Slayer", "SWAT", "Infection", etc.
    bool isPasswordProtected = false;
    bool hasWorkshopMod = false;
    std::string workshopModName;
    bool isFavorite = false;
};

class ServerBrowser {
public:
    ServerBrowser();
    ~ServerBrowser() = default;

    void Render();
    void RefreshServers();

    const std::vector<ServerEntry>& GetServers() const { return m_servers; }

private:
    void RenderFilterBar();
    void RenderServerList();
    void RenderServerDetails();
    void RenderDirectConnectModal();

    std::vector<ServerEntry> m_servers;
    int m_selectedServerIndex = -1;

    // Filters
    char m_searchQuery[128] = { 0 };
    int m_modeFilter = 0; // 0 = All, 1 = Multiplayer, 2 = Firefight, 3 = Campaign, 4 = Forge
    bool m_hideFull = false;
    bool m_hideEmpty = false;
    bool m_showOnlyFavorites = false;

    // Direct connect modal state
    bool m_showDirectConnectModal = false;
    char m_directConnectIp[64] = "127.0.0.1:11774";
    char m_directConnectPassword[64] = { 0 };
};

} // namespace noble
