#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>

namespace noble {

struct FriendEntry {
    std::string username;
    std::string status;    // "pending" or "accepted"
    std::string direction; // "incoming", "outgoing", "accepted", "local"
    std::string endpoint;  // optional ip:port
};

struct GameInvite {
    std::string senderUsername;
    std::string hostIp;
    uint16_t hostPort = 11774;
    std::string mode;
    std::string map;
    std::string sessionId;
    uint64_t timestamp = 0;
};

struct UserSearchResult {
    int id = 0;
    std::string username;
};

class SocialClient {
public:
    static SocialClient& Get();

    void Initialize();
    void Shutdown();

    // Data getters (thread-safe)
    std::vector<FriendEntry> GetFriends() const;
    std::vector<FriendEntry> GetIncomingRequests() const;
    std::vector<FriendEntry> GetOutgoingRequests() const;
    std::vector<GameInvite> GetInvites() const;

    // Refresh data from server
    void RefreshFriendsAsync();
    void PollInvitesAsync();

    // Friend management
    void AddFriendDirect(const std::string& nameOrEndpoint);
    void SearchUsersAsync(const std::string& prefix, std::function<void(bool ok, const std::vector<UserSearchResult>& results)> callback);
    void SendFriendRequestAsync(const std::string& targetUsername, std::function<void(bool ok, const std::string& message)> callback = nullptr);
    void AcceptFriendRequestAsync(const std::string& senderUsername, std::function<void(bool ok, const std::string& message)> callback = nullptr);
    void RemoveFriendAsync(const std::string& targetUsername, std::function<void(bool ok, const std::string& message)> callback = nullptr);

    // Invites
    void SendInviteAsync(const std::string& targetUsername, const std::string& hostIp, uint16_t hostPort,
                         std::function<void(bool ok, const std::string& message)> callback = nullptr,
                         const std::string& mode = "Custom Game",
                         const std::string& map = "",
                         const std::string& sessionId = "");
    void ClearInviteAsync(const std::string& senderUsername);

    // Status message for UI feedback
    std::string GetLastStatusMessage() const;
    void SetStatusMessage(const std::string& msg);

private:
    SocialClient() = default;
    ~SocialClient();

    mutable std::mutex m_mutex;
    std::vector<FriendEntry> m_friends;
    std::vector<FriendEntry> m_incomingRequests;
    std::vector<FriendEntry> m_outgoingRequests;
    std::vector<GameInvite> m_invites;
    std::string m_lastStatusMsg;

    std::atomic<bool> m_running{false};
    std::thread m_pollThread;
};

} // namespace noble

