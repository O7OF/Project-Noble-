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

bool Start(const std::wstring& args, unsigned long* outPid) {
    // Release all network sockets (UDP 11774 / 11775) and master server advertisements in this
    // process BEFORE spawning the replacement process so the new process can bind port 11774 cleanly.
    // A host launching its lobby's match passes the session on (-session-id): keep it listed.
    // So does a host going back from a match to its lobby: guests are waiting on that session.
    const bool handsOnSession = (args.find(L"-host") != std::wstring::npos && args.find(L"-session-id") != std::wstring::npos) ||
                               args.find(L"-lobby-return host") != std::wstring::npos;
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
    if (outPid) *outPid = pi.dwProcessId;
    return true;
}

namespace {

struct WindowSearch {
    DWORD pid = 0;
    HWND found = nullptr;
};

BOOL CALLBACK HideOwnWindow(HWND hwnd, LPARAM pid) {
    DWORD owner = 0;
    GetWindowThreadProcessId(hwnd, &owner);
    if (owner == static_cast<DWORD>(pid) && IsWindowVisible(hwnd)) ShowWindow(hwnd, SW_HIDE);
    return TRUE;
}

// The new process's first visible top-level window: that is the moment its loading screen is on screen.
BOOL CALLBACK FindVisibleWindow(HWND hwnd, LPARAM param) {
    auto* search = reinterpret_cast<WindowSearch*>(param);
    DWORD owner = 0;
    GetWindowThreadProcessId(hwnd, &owner);
    if (owner != search->pid || !IsWindowVisible(hwnd)) return TRUE;
    RECT rect{};
    if (!GetClientRect(hwnd, &rect) || rect.right <= 0 || rect.bottom <= 0) return TRUE;
    search->found = hwnd;
    return FALSE;   // stop enumerating
}

} // namespace

bool HandOff(const std::wstring& args) {
    unsigned long childPid = 0;
    if (!Start(args, &childPid)) return false;

    // Keep THIS window on screen until the new process has a window of its own, so the switch is a swap rather than
    // a trip to the desktop (Tyler: "it shows your desktop and then it pops up"). This process keeps rendering its
    // menu/loading screen while the new one boots; when the new window appears, it is brought to the front and this
    // one disappears.
    const ULONGLONG startTick = GetTickCount64();
    WindowSearch search;
    search.pid = childPid;
    bool appeared = false;
    for (int attempt = 0; attempt < 300; ++attempt) {   // up to 30 s, then give up and swap anyway
        Sleep(100);
        search.found = nullptr;
        EnumWindows(&FindVisibleWindow, reinterpret_cast<LPARAM>(&search));
        if (search.found) { appeared = true; break; }
    }
    if (appeared) {
        ShowWindow(search.found, SW_SHOWNORMAL);
        SetForegroundWindow(search.found);
        NOBLE_INFO("Relaunch: new window up after {} ms; swapping to it", GetTickCount64() - startTick);
    } else {
        NOBLE_WARN("Relaunch: the new process did not show a window within 30 s; swapping anyway");
    }

    // The network was already released in Start, so the new process can bind its ports. Hide this window
    // and end now instead of posting WM_CLOSE and waiting for a shutdown that can take seconds (the old
    // process lingered next to the new one).
    EnumWindows(&HideOwnWindow, static_cast<LPARAM>(GetCurrentProcessId()));
    NOBLE_INFO("Relaunch: handed off to the new Noble process; closing this one");
    // Do not run Logger::Shutdown() here: it joins/flushes spdlog while the engine's threads are still alive
    // and can hang, leaving the old server-browser window frozen next to the new match process. The line above
    // is already flushed (flush_on(info)); terminate immediately.
    TerminateProcess(GetCurrentProcess(), 0);  // not ExitProcess: DLL shutdown under the engine can hang
    return true;  // not reached
}

} // namespace Relaunch
} // namespace noble
