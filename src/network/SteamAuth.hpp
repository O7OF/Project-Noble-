#pragma once

#include <cstdint>
#include <string>

namespace noble {

struct SteamAuthResult {
    bool success = false;
    std::string error;
    std::string steamId;
    std::string username;
    std::string token;
    bool isDeveloper = false;
};

// Loads MCC's steam_api64.dll, requests a Steam auth session ticket, and exchanges it for an
// in-memory Noble session at the registry. No password is stored or sent.
class SteamAuth {
public:
    static SteamAuthResult Authenticate(const std::string& registryUrl, const std::string& deviceId);
    static std::string LastSteamId();
};

} // namespace noble
