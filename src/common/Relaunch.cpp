#include "Relaunch.hpp"
#include "Logger.hpp"
#include "Settings.hpp"
#include "network/AuthClient.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/SessionManager.hpp"

#include <windows.h>
#include <cwchar>
#include <mutex>

namespace noble {
namespace Relaunch {

namespace {
std::mutex s_mutex;
std::wstring s_currentArgs;

std::wstring ToWideUtf8(const std::string& s) {
    if (s.empty()) return L"";
    const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring w(static_cast<size_t>(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
    return w;
}
} // namespace

void SetCurrentGameArgs(const std::wstring& args) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_currentArgs = args;
}

std::wstring CurrentGameArgs() {
    std::lock_guard<std::mutex> lock(s_mutex);
    return s_currentArgs;
}

bool Start(const std::wstring& args) {
    // Release all network sockets (UDP 11774 / 11775) and master server advertisements in this
    // process BEFORE spawning the replacement process so the new process can bind port 11774 cleanly.
    // A host launching its lobby's match passes the session on (-session-id): keep it listed.
    const bool handsOnSession = args.find(L"-host") != std::wstring::npos && args.find(L"-session-id") != std::wstring::npos;
    MasterServerClient::Get().Shutdown(handsOnSession);
    SessionManager::Get().Shutdown();
    NetworkTransport::Get().Shutdown();


    wchar_t exe[MAX_PATH] = {};
    if (!GetModuleFileNameW(nullptr, exe, MAX_PATH)) return false;
    std::wstring cmd = L"\"" + std::wstring(exe) + L"\"";
    if (!args.empty()) cmd += L" " + args;
    // Preserve this window's active player and account identity across relaunches so that when multiple
    // instances run on the same PC (sharing data\userdata\settings.json), relaunching into a match or
    // back to the menu keeps each window's logged-in account rather than reading the last-saved file.
    const SettingsData snap = Settings::Get().Snapshot();
    const std::string activeName = AuthClient::Get().IsLoggedIn() ? AuthClient::Get().GetUsername() : snap.playerName;
    if (!activeName.empty() && cmd.find(L"-player-name") == std::wstring::npos) {
        cmd += L" -player-name \"" + ToWideUtf8(activeName) + L"\"";
    }
    if (!snap.serviceTag.empty() && cmd.find(L"-service-tag") == std::wstring::npos) {
        cmd += L" -service-tag \"" + ToWideUtf8(snap.serviceTag) + L"\"";
    }
    if (AuthClient::Get().IsLoggedIn() && cmd.find(L"-account-user") == std::wstring::npos) {
        cmd += L" -account-user \"" + ToWideUtf8(AuthClient::Get().GetUsername()) + L"\"";
        cmd += L" -account-token \"" + ToWideUtf8(AuthClient::Get().GetSessionToken()) + L"\"";
        cmd += L" -user-id " + std::to_wstring(AuthClient::Get().GetUserId());
        cmd += L" -account-dev " + std::to_wstring(AuthClient::Get().IsDeveloper() ? 1 : 0);
    }
    // Test switches the menu was started with carry on to the match process.
    for (const wchar_t* sw : { L"-rr-guest-block", L"-armory-unlock-all" }) {
        if (std::wcsstr(GetCommandLineW(), sw) && cmd.find(sw) == std::wstring::npos) cmd += std::wstring(L" ") + sw;
    }
    std::wstring dir = exe;
    const size_t slash = dir.find_last_of(L"\\/");
    if (slash != std::wstring::npos) dir.resize(slash);
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(exe, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &si, &pi)) {
        NOBLE_ERROR("Relaunch: could not start Noble (error {})", GetLastError());
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

} // namespace Relaunch
} // namespace noble
