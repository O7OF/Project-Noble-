#include "Relaunch.hpp"
#include "Logger.hpp"
#include "Settings.hpp"
#include "network/AuthClient.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/SessionManager.hpp"

#include <windows.h>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>

namespace noble {
namespace Relaunch {

namespace {
std::mutex s_mutex;
std::wstring s_currentArgs;

// main.cpp writes running\<pid>.flag beside the exe at start and removes it on a clean exit; a flag left behind
// makes the next start warn "did not exit cleanly". A hand-off to the next Noble process is a clean exit, but it
// ends with TerminateProcess below and never reached main.cpp's removal, so every join / PLAY AGAIN left a flag
// and a false warning (2026-11-10). Same path as main.cpp's OwnRunFlag().
void RemoveOwnRunFlag() {
    wchar_t exe[MAX_PATH] = {};
    if (!GetModuleFileNameW(nullptr, exe, MAX_PATH)) return;
    std::error_code ec;
    std::filesystem::remove(std::filesystem::path(exe).parent_path() / L"running" /
                                (std::to_wstring(GetCurrentProcessId()) + L".flag"),
                            ec);
}

std::wstring ToWideUtf8(const std::string& s) {
    if (s.empty()) return L"";
    const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring w(static_cast<size_t>(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
    return w;
}

// Container window mode: the successor match's arguments wait here for the launcher window, next to the
// executable (the engine moves the working directory to data\sandbox, so a relative path would not do).
std::wstring ContainerRelaunchPath() {
    wchar_t exe[MAX_PATH] = {};
    if (!GetModuleFileNameW(nullptr, exe, MAX_PATH)) return L"";
    std::wstring dir = exe;
    const size_t slash = dir.find_last_of(L"\\/");
    if (slash == std::wstring::npos) return L"";
    dir.resize(slash);
    return dir + L"\\data\\userdata\\container-relaunch.txt";
}

// Removes "-name value" (quoted or not) from a command-line string.
std::wstring RemoveArgValue(const std::wstring& args, const wchar_t* name) {
    const size_t at = args.find(name);
    if (at == std::wstring::npos) return args;
    size_t end = at + wcslen(name);
    while (end < args.size() && args[end] == L' ') ++end;
    if (end < args.size() && args[end] == L'"') {
        ++end;
        while (end < args.size() && args[end] != L'"') ++end;
        if (end < args.size()) ++end;
    } else {
        while (end < args.size() && args[end] != L' ') ++end;
    }
    std::wstring out = args.substr(0, at);
    while (end < args.size() && args[end] == L' ') ++end;
    out += args.substr(end);
    return out;
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
    for (const wchar_t* sw : { L"--guest-block", L"-armory-unlock-all" }) {
        if (std::wcsstr(GetCommandLineW(), sw) && cmd.find(sw) == std::wstring::npos) cmd += std::wstring(L" ") + sw;
    }
    const wchar_t* pCont = wcsstr(GetCommandLineW(), L"-container-hwnd");
    if (pCont && cmd.find(L"-container-hwnd") == std::wstring::npos) {
        wchar_t contBuf[64] = {};
        if (swscanf_s(pCont, L"-container-hwnd %63s", contBuf, (unsigned)std::size(contBuf)) == 1) {
            cmd += L" -container-hwnd " + std::wstring(contBuf);
        }
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

bool StartChildMatch(const std::wstring& args, void** outProcessHandle, unsigned long* outPid) {
    const bool handsOnSession = (args.find(L"-host") != std::wstring::npos && args.find(L"-session-id") != std::wstring::npos) ||
                               args.find(L"-lobby-return host") != std::wstring::npos;
    MasterServerClient::Get().Shutdown(handsOnSession);
    SessionManager::Get().Shutdown();
    NetworkTransport::Get().Shutdown();

    wchar_t exe[MAX_PATH] = {};
    if (!GetModuleFileNameW(nullptr, exe, MAX_PATH)) return false;
    std::wstring cmd = L"\"" + std::wstring(exe) + L"\"";
    if (!args.empty()) cmd += L" " + args;

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
    for (const wchar_t* sw : { L"--guest-block", L"-armory-unlock-all" }) {
        if (std::wcsstr(GetCommandLineW(), sw) && cmd.find(sw) == std::wstring::npos) cmd += std::wstring(L" ") + sw;
    }
    std::wstring dir = exe;
    const size_t slash = dir.find_last_of(L"\\/");
    if (slash != std::wstring::npos) dir.resize(slash);
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(exe, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &si, &pi)) {
        NOBLE_ERROR("Relaunch: could not start container child match (error {})", GetLastError());
        return false;
    }
    CloseHandle(pi.hThread);
    if (outProcessHandle) *outProcessHandle = pi.hProcess;
    else CloseHandle(pi.hProcess);
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

bool RequestContainerRelaunch(const std::wstring& args) {
    const std::wstring path = ContainerRelaunchPath();
    if (path.empty() || args.empty()) return false;
    // The launcher adds its own -container-hwnd, so the request carries only the match's arguments.
    const std::wstring clean = RemoveArgValue(args, L"-container-hwnd");
    const int len = WideCharToMultiByte(CP_UTF8, 0, clean.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 1) return false;
    std::string utf8(static_cast<size_t>(len - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, clean.c_str(), -1, utf8.data(), len, nullptr, nullptr);
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);
    std::ofstream out(std::filesystem::path(path), std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    out.flush();
    return static_cast<bool>(out);
}

std::wstring TakeContainerRelaunch() {
    const std::wstring path = ContainerRelaunchPath();
    if (path.empty()) return L"";
    std::ifstream in(std::filesystem::path(path), std::ios::binary);
    if (!in) return L"";
    const std::string utf8((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    std::error_code ec;
    std::filesystem::remove(path, ec);
    return ToWideUtf8(utf8);
}

void ClearContainerRelaunch() {
    const std::wstring path = ContainerRelaunchPath();
    if (path.empty()) return;
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

bool HandOff(const std::wstring& args) {
    // Container window mode: the successor match has to run inside the launcher's window, and only the
    // launcher may start it - it is the process that keeps track of the running match. Hand the arguments
    // over instead of spawning a second Noble this process cannot supervise (the launcher used to see this
    // process exit and bring its menu back over the top of the new game), and never wait for a top-level
    // window, which a hosted child does not have (that used to be a 30 s stall on PLAY AGAIN).
    if (args.find(L"-container-hwnd") != std::wstring::npos) {
        if (!RequestContainerRelaunch(args)) {
            NOBLE_ERROR("Relaunch: container match relaunch could not be handed to the launcher window");
            return false;
        }
        NOBLE_INFO("Relaunch: container match relaunch handed to the launcher window; closing this match");
        RemoveOwnRunFlag();
        TerminateProcess(GetCurrentProcess(), 0);
        return true;  // not reached
    }

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
    RemoveOwnRunFlag();
    TerminateProcess(GetCurrentProcess(), 0);  // not ExitProcess: DLL shutdown under the engine can hang
    return true;  // not reached
}

} // namespace Relaunch
} // namespace noble
