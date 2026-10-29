#include "Relaunch.hpp"
#include "Logger.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/SessionManager.hpp"

#include <windows.h>
#include <mutex>

namespace noble {
namespace Relaunch {

namespace {
std::mutex s_mutex;
std::wstring s_currentArgs;
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
