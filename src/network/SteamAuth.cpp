#include "SteamAuth.hpp"
#include "host/SteamWorkshop.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>
#include <vector>

namespace noble {
namespace {
using InitFn = bool(__cdecl*)();
using ShutdownFn = void(__cdecl*)();
using UserFn = void*(__cdecl*)();
using TicketFn = uint32_t(__cdecl*)(void*, void*, int, uint32_t*);
using IdFn = uint64_t(__cdecl*)(void*);

HMODULE s_module = nullptr;
void* s_user = nullptr;
std::string s_lastId;

std::wstring Widen(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
    if (!w.empty() && w.back() == L'\0') w.pop_back();
    return w;
}

std::string Hex(const std::vector<uint8_t>& bytes) {
    static const char* digits = "0123456789ABCDEF";
    std::string out;
    out.reserve(bytes.size() * 2);
    for (uint8_t b : bytes) { out.push_back(digits[b >> 4]); out.push_back(digits[b & 15]); }
    return out;
}

bool EnsureSteam() {
    if (s_user) return true;
    const std::filesystem::path dll = SteamWorkshop::FindSteamApi64();
    if (dll.empty()) return false;
    s_module = LoadLibraryW(dll.c_str());
    if (!s_module) return false;
    auto init = reinterpret_cast<InitFn>(GetProcAddress(s_module, "SteamAPI_Init"));
    if (!init || !init()) {
        NOBLE_WARN("SteamAuth: SteamAPI_Init failed (is Steam running and logged in?)");
        return false;
    }
    UserFn user = nullptr;
    for (const char* name : { "SteamAPI_SteamUser_v023", "SteamAPI_SteamUser_v022", "SteamAPI_SteamUser_v021", "SteamAPI_SteamUser_v020" }) {
        user = reinterpret_cast<UserFn>(GetProcAddress(s_module, name));
        if (user) break;
    }
    if (!user) return false;
    s_user = user();
    return s_user != nullptr;
}

std::string HttpPost(const std::string& baseUrl, const std::string& path, const std::string& body, int& status) {
    status = 0;
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    parts.dwSchemeLength = static_cast<DWORD>(-1);
    parts.dwHostNameLength = static_cast<DWORD>(-1);
    parts.dwUrlPathLength = static_cast<DWORD>(-1);
    std::wstring url = Widen(baseUrl);
    if (!WinHttpCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &parts)) return {};
    std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
    const bool https = parts.nScheme == INTERNET_SCHEME_HTTPS;
    const INTERNET_PORT port = parts.nPort ? parts.nPort : (https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);
    std::wstring requestPath = parts.dwUrlPathLength ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"";
    while (!requestPath.empty() && requestPath.back() == L'/') requestPath.pop_back();
    requestPath += Widen(path);
    HINTERNET session = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    if (!session) return {};
    WinHttpSetTimeouts(session, 5000, 5000, 5000, 5000);
    HINTERNET connect = WinHttpConnect(session, host.c_str(), port, 0);
    HINTERNET request = connect ? WinHttpOpenRequest(connect, L"POST", requestPath.c_str(), nullptr,
                                                     WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                     https ? WINHTTP_FLAG_SECURE : 0) : nullptr;
    std::string response;
    if (request) {
        std::wstring headers = L"Content-Type: application/json\r\nAccept: application/json\r\n";
        if (WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(headers.size()),
                               const_cast<char*>(body.data()), static_cast<DWORD>(body.size()),
                               static_cast<DWORD>(body.size()), 0) &&
            WinHttpReceiveResponse(request, nullptr)) {
            DWORD code = 0, size = sizeof(code);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &code, &size, WINHTTP_NO_HEADER_INDEX);
            status = static_cast<int>(code);
            DWORD read = 0;
            char buffer[2048];
            while (WinHttpReadData(request, buffer, sizeof(buffer), &read) && read > 0) response.append(buffer, read);
        }
        WinHttpCloseHandle(request);
    }
    if (connect) WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return response;
}
} // namespace

std::string SteamAuth::LastSteamId() { return s_lastId; }

SteamAuthResult SteamAuth::Authenticate(const std::string& registryUrl, const std::string& deviceId) {
    SteamAuthResult result;
    if (registryUrl.empty()) { result.error = "No registry URL configured"; return result; }
    if (!EnsureSteam()) { result.error = "Steam is not running or MCC's steam_api64.dll was not found"; return result; }
    auto ticketFn = reinterpret_cast<TicketFn>(GetProcAddress(s_module, "SteamAPI_ISteamUser_GetAuthSessionTicket"));
    auto idFn = reinterpret_cast<IdFn>(GetProcAddress(s_module, "SteamAPI_ISteamUser_GetSteamID"));
    if (!ticketFn || !idFn) { result.error = "Steam auth exports missing from steam_api64.dll"; return result; }

    std::vector<uint8_t> ticket(1024);
    uint32_t ticketSize = 0;
    if (ticketFn(s_user, ticket.data(), static_cast<int>(ticket.size()), &ticketSize) != 0 || ticketSize == 0) {
        result.error = "Steam did not issue an auth ticket";
        return result;
    }
    ticket.resize(ticketSize);
    const uint64_t steamId = idFn(s_user);
    char idText[32];
    std::snprintf(idText, sizeof(idText), "%llu", static_cast<unsigned long long>(steamId));
    s_lastId = idText;

    nlohmann::json body = { { "ticket", Hex(ticket) }, { "deviceId", deviceId } };
    int status = 0;
    const std::string response = HttpPost(registryUrl, "/api/auth/steam", body.dump(), status);
    if (status < 200 || status >= 300) {
        result.error = "Steam verification failed (HTTP " + std::to_string(status) + ")";
        return result;
    }
    try {
        const auto j = nlohmann::json::parse(response);
        result.success = true;
        result.steamId = j.value("steam_id", s_lastId);
        result.username = j.value("username", "Steam " + std::string(idText));
        result.token = j.value("session_token", "");
        result.isDeveloper = j.value("is_developer", false);
        NOBLE_INFO("SteamAuth: verified Steam ID {} ({})", result.steamId, result.username);
    } catch (...) {
        result.error = "Registry returned an unreadable Steam response";
    }
    return result;
}

} // namespace noble
