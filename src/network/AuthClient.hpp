#pragma once

#include <atomic>
#include <string>

namespace noble {

struct AuthResult {
    bool success = false;
    std::string error;
    std::string username;
    std::string token;
    bool isDeveloper = false;
    int userId = 0;
};

class AuthClient {
public:
    static AuthClient& Get();

    void Initialize();

    AuthResult Login(const std::string& username, const std::string& password);
    AuthResult Register(const std::string& username, const std::string& password);
    void Logout();

    // Re-asks the master server whether this account is a developer (POST /api/dev/check).
    // The cached login keeps the flag from the last login, so an account promoted with
    // set_dev.js after logging in would otherwise keep showing as a standard account.
    // Runs on a background thread; safe to call repeatedly.
    void RefreshDeveloperStatusAsync();

    bool IsLoggedIn() const;
    bool IsDeveloper() const;
    std::string GetUsername() const;
    std::string GetSessionToken() const;
    int GetUserId() const;

private:
    AuthClient() = default;

    bool m_isLoggedIn = false;
    std::atomic<bool> m_isDeveloper{ false };
    std::atomic<bool> m_refreshing{ false };
    std::string m_username;
    std::string m_sessionToken;
    int m_userId = 0;
};

} // namespace noble

