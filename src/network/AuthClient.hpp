#pragma once

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

    bool IsLoggedIn() const;
    bool IsDeveloper() const;
    std::string GetUsername() const;
    std::string GetSessionToken() const;
    int GetUserId() const;

private:
    AuthClient() = default;

    bool m_isLoggedIn = false;
    bool m_isDeveloper = false;
    std::string m_username;
    std::string m_sessionToken;
    int m_userId = 0;
};

} // namespace noble

