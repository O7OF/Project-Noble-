#include "Updater.hpp"
#include "Version.hpp"
#include "Logger.hpp"

#include <nlohmann/json.hpp>

#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <functional>
#include <sstream>
#include <thread>
#include <vector>

namespace noble {
namespace {

constexpr wchar_t kApiHost[] = L"api.github.com";
constexpr wchar_t kApiPath[] = L"/repos/O7OF/Project-Noble-/releases/latest";
constexpr wchar_t kUserAgent[] = L"ProjectNoble-Updater/1.0";

std::wstring Widen(const std::string& s) {
    if (s.empty()) return L"";
    const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring w(static_cast<size_t>(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
    return w;
}

std::string Narrow(const std::wstring& w) {
    if (w.empty()) return "";
    const int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string s(static_cast<size_t>(len - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), len, nullptr, nullptr);
    return s;
}

void CloseHttp(HINTERNET& session, HINTERNET& connect, HINTERNET& request) {
    if (request) { WinHttpCloseHandle(request); request = nullptr; }
    if (connect) { WinHttpCloseHandle(connect); connect = nullptr; }
    if (session) { WinHttpCloseHandle(session); session = nullptr; }
}

bool OpenHttp(const std::wstring& host, const std::wstring& path, HINTERNET& session, HINTERNET& connect,
              HINTERNET& request, std::string& error) {
    session = WinHttpOpen(kUserAgent, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
                          WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) {
        error = "WinHttpOpen failed";
        return false;
    }
    WinHttpSetTimeouts(session, 5000, 5000, 15000, 60000);
    connect = WinHttpConnect(session, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connect) {
        error = "WinHttpConnect failed";
        CloseHttp(session, connect, request);
        return false;
    }
    request = WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                 WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request) {
        error = "WinHttpOpenRequest failed";
        CloseHttp(session, connect, request);
        return false;
    }
    WinHttpAddRequestHeaders(request, L"Accept: application/vnd.github+json\r\nUser-Agent: ProjectNoble\r\n",
                             static_cast<DWORD>(-1), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, nullptr)) {
        error = "GitHub request failed (" + std::to_string(GetLastError()) + ")";
        CloseHttp(session, connect, request);
        return false;
    }
    return true;
}

bool ReadResponseBody(HINTERNET request, std::string& body, std::string& error) {
    DWORD available = 0;
    while (WinHttpQueryDataAvailable(request, &available) && available > 0) {
        std::string chunk(available, '\0');
        DWORD read = 0;
        if (!WinHttpReadData(request, chunk.data(), available, &read)) {
            error = "WinHttpReadData failed";
            return false;
        }
        if (read == 0) break;
        if (body.size() + read > 4u * 1024u * 1024u) {
            error = "GitHub response is too large";
            return false;
        }
        body.append(chunk.data(), read);
    }
    return true;
}

DWORD ResponseStatus(HINTERNET request) {
    DWORD status = 0;
    DWORD size = sizeof(status);
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX)) {
        return 0;
    }
    return status;
}

bool SplitHttpsUrl(const std::string& url, std::wstring& host, std::wstring& path) {
    constexpr const char prefix[] = "https://";
    if (url.rfind(prefix, 0) != 0) return false;
    const size_t hostStart = sizeof(prefix) - 1;
    const size_t slash = url.find('/', hostStart);
    if (slash == std::string::npos) return false;
    host = Widen(url.substr(hostStart, slash - hostStart));
    path = Widen(url.substr(slash));
    return !host.empty() && !path.empty();
}

std::array<int, 4> ParseVersion(std::string tag) {
    if (!tag.empty() && (tag[0] == 'v' || tag[0] == 'V')) tag.erase(0, 1);
    std::array<int, 4> out{};
    int index = 0;
    size_t pos = 0;
    while (index < 4 && pos < tag.size()) {
        size_t dot = tag.find('.', pos);
        const std::string part = tag.substr(pos, dot == std::string::npos ? std::string::npos : dot - pos);
        int value = 0;
        bool any = false;
        for (const char c : part) {
            if (c < '0' || c > '9') break;
            value = value * 10 + (c - '0');
            any = true;
        }
        out[static_cast<size_t>(index++)] = any ? value : 0;
        if (dot == std::string::npos) break;
        pos = dot + 1;
    }
    return out;
}

std::array<int, 4> CurrentVersion() {
    return { VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_REVISION };
}

std::filesystem::path LocalAppData() {
    wchar_t buffer[MAX_PATH] = {};
    const DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", buffer, MAX_PATH);
    if (len > 0 && len < MAX_PATH) return std::filesystem::path(buffer);
    return std::filesystem::temp_directory_path();
}

std::filesystem::path TempDir() {
    wchar_t buffer[MAX_PATH] = {};
    const DWORD len = GetTempPathW(MAX_PATH, buffer);
    return len > 0 ? std::filesystem::path(buffer) : std::filesystem::temp_directory_path();
}

std::string SafeFilePart(const std::string& name) {
    std::string out;
    for (const char c : name) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '.' || c == '-' || c == '_') {
            out.push_back(c);
        }
    }
    return out.empty() ? "Project-Noble-update.zip" : out;
}

bool DownloadToFile(const std::wstring& host, const std::wstring& path, const std::filesystem::path& file,
                    uint64_t expectedSize, const std::function<void(uint64_t, uint64_t)>& progress, std::string& error) {
    HINTERNET session = nullptr, connect = nullptr, request = nullptr;
    if (!OpenHttp(host, path, session, connect, request, error)) return false;

    DWORD status = ResponseStatus(request);
    if (status != 200) {
        error = "GitHub returned HTTP " + std::to_string(status);
        CloseHttp(session, connect, request);
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(file.parent_path(), ec);
    const std::filesystem::path part = file.wstring() + L".part";
    std::ofstream out(part, std::ios::binary | std::ios::trunc);
    if (!out) {
        error = "cannot open " + part.string();
        CloseHttp(session, connect, request);
        return false;
    }

    DWORD contentLength = 0;
    DWORD lenSize = sizeof(contentLength);
    WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &contentLength, &lenSize, WINHTTP_NO_HEADER_INDEX);
    uint64_t total = expectedSize != 0 ? expectedSize : contentLength;

    uint64_t written = 0;
    DWORD available = 0;
    bool ok = true;
    while (WinHttpQueryDataAvailable(request, &available) && available > 0) {
        std::vector<char> chunk(available);
        DWORD read = 0;
        if (!WinHttpReadData(request, chunk.data(), available, &read)) {
            error = "WinHttpReadData failed";
            ok = false;
            break;
        }
        if (read == 0) break;
        out.write(chunk.data(), read);
        written += read;
        if (progress) progress(written, total);
    }
    out.close();
    CloseHttp(session, connect, request);

    if (!ok || written == 0 || (expectedSize != 0 && written != expectedSize)) {
        std::filesystem::remove(part, ec);
        if (error.empty()) error = "downloaded " + std::to_string(written) + " of " + std::to_string(expectedSize) + " bytes";
        return false;
    }
    std::filesystem::remove(file, ec);
    std::filesystem::rename(part, file, ec);
    if (ec) {
        error = "cannot finish download: " + ec.message();
        return false;
    }
    return true;
}

} // namespace

Updater& Updater::Get() {
    static Updater instance;
    return instance;
}

void Updater::Initialize(const std::filesystem::path& installDir) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_installDir.empty()) return;
    m_installDir = installDir;
    wchar_t module[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, module, MAX_PATH) > 0) {
        m_exeName = Narrow(std::filesystem::path(module).filename().wstring());
    }
    if (m_exeName.empty()) m_exeName = "Noble.exe";
}

void Updater::SetState(State state) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status.state = state;
}

Updater::Status Updater::Snapshot() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_status;
}

std::string Updater::FormatBytes(uint64_t bytes) {
    char buffer[64];
    if (bytes >= 1024ull * 1024ull * 1024ull) {
        std::snprintf(buffer, sizeof(buffer), "%.1f GB", static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0));
    } else if (bytes >= 1024ull * 1024ull) {
        std::snprintf(buffer, sizeof(buffer), "%.0f MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
    } else if (bytes >= 1024ull) {
        std::snprintf(buffer, sizeof(buffer), "%.0f KB", static_cast<double>(bytes) / 1024.0);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%llu B", static_cast<unsigned long long>(bytes));
    }
    return buffer;
}

void Updater::StartCheck() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_installDir.empty() || m_checkStarted) return;
        m_checkStarted = true;
        m_status = Status{};
        m_status.state = State::Checking;
    }
    std::thread([this] { CheckThread(); }).detach();
}

void Updater::CheckThread() {
    std::string body;
    std::string error;
    HINTERNET session = nullptr, connect = nullptr, request = nullptr;
    if (!OpenHttp(kApiHost, kApiPath, session, connect, request, error)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.state = State::Error;
        m_status.error = error;
        NOBLE_WARN("Updater: {}", error);
        return;
    }
    const DWORD status = ResponseStatus(request);
    if (status != 200) {
        CloseHttp(session, connect, request);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.state = State::Error;
        m_status.error = "GitHub returned HTTP " + std::to_string(status) + " (is the release repository public?)";
        NOBLE_WARN("Updater: {}", m_status.error);
        return;
    }
    if (!ReadResponseBody(request, body, error)) {
        CloseHttp(session, connect, request);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.state = State::Error;
        m_status.error = error;
        NOBLE_WARN("Updater: {}", error);
        return;
    }
    CloseHttp(session, connect, request);

    const nlohmann::json release = nlohmann::json::parse(body, nullptr, false);
    if (release.is_discarded()) {
        SetState(State::Error);
        NOBLE_WARN("Updater: GitHub release JSON could not be parsed");
        return;
    }

    const std::string tag = release.value("tag_name", "");
    const std::string notes = release.value("body", "");
    const std::array<int, 4> latest = ParseVersion(tag);
    const std::array<int, 4> current = CurrentVersion();
    if (tag.empty() || latest <= current) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.state = State::UpToDate;
        m_status.latestTag = tag;
        NOBLE_INFO("Updater: up to date (local {}.{}.{}.{}, latest '{}')",
                   current[0], current[1], current[2], current[3], tag);
        return;
    }

    std::string assetName, assetUrl;
    uint64_t assetSize = 0;
    if (release.contains("assets") && release["assets"].is_array()) {
        for (const auto& asset : release["assets"]) {
            if (!asset.is_object()) continue;
            const std::string name = asset.value("name", "");
            if (name.size() < 4 || name.rfind(".zip") != name.size() - 4) continue;
            if (name.find("Project-Noble") == std::string::npos) continue;
            assetName = name;
            assetUrl = asset.value("browser_download_url", "");
            assetSize = asset.value("size", 0ull);
            break;
        }
    }
    if (assetName.empty() || assetUrl.empty()) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.state = State::Error;
        m_status.latestTag = tag;
        m_status.error = "the latest release has no Project-Noble .zip asset";
        NOBLE_WARN("Updater: {}", m_status.error);
        return;
    }

    const std::filesystem::path zip = LocalAppData() / "ProjectNoble" / "updates" / SafeFilePart(tag) / SafeFilePart(assetName);
    std::error_code ec;
    const uint64_t existingSize = std::filesystem::exists(zip, ec) && std::filesystem::is_regular_file(zip, ec)
                                      ? static_cast<uint64_t>(std::filesystem::file_size(zip, ec))
                                      : 0;
    const bool ready = existingSize != 0 && (assetSize == 0 || existingSize == assetSize);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.latestTag = tag;
        m_status.notes = notes;
        m_status.assetName = assetName;
        m_status.assetSize = assetSize;
        m_assetUrl = assetUrl;
        m_zipPath = zip;
        m_status.downloaded = ready;
        m_status.progress = ready ? 1.0f : 0.0f;
        m_status.state = ready ? State::Ready : State::Available;
    }
    NOBLE_INFO("Updater: {} available ({}), asset '{}' ({} bytes){}", tag, notes.substr(0, 80), assetName, assetSize,
               ready ? " - already downloaded" : "");
}

void Updater::StartDownload() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_downloadStarted || m_status.state != State::Available) return;
        m_downloadStarted = true;
        m_status.state = State::Downloading;
        m_status.progress = 0.0f;
    }
    std::thread([this] { DownloadThread(); }).detach();
}

void Updater::DownloadThread() {
    std::wstring host, path;
    std::string url;
    uint64_t expected = 0;
    std::filesystem::path zip;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        url = m_assetUrl;
        expected = m_status.assetSize;
        zip = m_zipPath;
    }
    std::string error;
    if (!SplitHttpsUrl(url, host, path)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.state = State::Error;
        m_status.error = "bad asset URL";
        return;
    }

    const bool ok = DownloadToFile(host, path, zip, expected, [this](uint64_t written, uint64_t total) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.progress = total > 0 ? static_cast<float>(static_cast<double>(written) / static_cast<double>(total)) : 0.0f;
    }, error);

    std::lock_guard<std::mutex> lock(m_mutex);
    if (ok) {
        m_status.state = State::Ready;
        m_status.downloaded = true;
        m_status.progress = 1.0f;
        NOBLE_INFO("Updater: downloaded '{}' to '{}'", m_status.latestTag, zip.string());
    } else {
        m_status.state = State::Error;
        m_status.error = error;
        m_status.downloaded = false;
        NOBLE_WARN("Updater: download failed: {}", error);
    }
}

bool Updater::ApplyAndRestart() {
    std::filesystem::path zip;
    std::filesystem::path installDir;
    std::string exeName;
    std::string tag;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_applyStarted) return true;
        if (m_status.state != State::Ready || m_zipPath.empty()) return false;
        zip = m_zipPath;
        installDir = m_installDir;
        exeName = m_exeName;
        tag = m_status.latestTag;
        m_applyStarted = true;
    }
    std::error_code ec;
    if (!std::filesystem::exists(zip, ec)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.state = State::Error;
        m_status.error = "downloaded update is missing";
        m_applyStarted = false;
        return false;
    }

    const std::filesystem::path scriptDir = TempDir() / "ProjectNobleUpdate";
    std::filesystem::create_directories(scriptDir, ec);
    const std::filesystem::path script = scriptDir / "apply_update.ps1";
    {
        std::ofstream out(script, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_applyStarted = false;
            m_status.state = State::Error;
            m_status.error = "cannot write the update helper script";
            return false;
        }
        out << R"PS(
param([int]$TargetPid, [string]$ZipPath, [string]$InstallDir, [string]$ExeName)
$ErrorActionPreference = 'Continue'
$logDir = Join-Path $env:TEMP 'ProjectNobleUpdate'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$log = Join-Path $logDir 'apply_update.log'
function Log($m) { "$(Get-Date -Format o) $m" | Out-File -LiteralPath $log -Append -Encoding utf8 }
Log "waiting for process $TargetPid"
$deadline = (Get-Date).AddSeconds(60)
while ((Get-Process -Id $TargetPid -ErrorAction SilentlyContinue) -and ((Get-Date) -lt $deadline)) {
    Start-Sleep -Milliseconds 200
}
$stage = Join-Path $logDir ('stage-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force -Path $stage | Out-Null
$src = $stage
try {
    Expand-Archive -LiteralPath $ZipPath -DestinationPath $stage -Force
    $root = Get-ChildItem -LiteralPath $stage -Directory | Select-Object -First 1
    if ($root) { $src = $root.FullName }
    Log "copying $src to $InstallDir"
    & robocopy $src $InstallDir /E /R:3 /W:1 /XD userdata autosave cache /XF settings.json appearance.dat appearance.json career.dat career.json preferences.dat /NFL /NDL /NJH /NJS /NP | Out-Null
    Log "robocopy exit $LASTEXITCODE"
} catch {
    Log "extract/copy failed: $_"
}
try {
    Start-Process -FilePath (Join-Path $InstallDir $ExeName) -WorkingDirectory $InstallDir | Out-Null
    Log "started $ExeName"
} catch {
    Log "start failed: $_"
}
Start-Sleep -Seconds 2
Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $ZipPath -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $PSCommandPath -Force -ErrorAction SilentlyContinue
)PS";
        out.close();
    }

    std::wstring command = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File \"";
    command += script.wstring();
    command += L"\" -TargetPid " + std::to_wstring(GetCurrentProcessId());
    command += L" -ZipPath \"" + zip.wstring() + L"\"";
    command += L" -InstallDir \"" + installDir.wstring() + L"\"";
    command += L" -ExeName \"" + Widen(exeName) + L"\"";
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(L'\0');

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION process{};
    const BOOL launched = CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, FALSE,
                                         CREATE_NO_WINDOW, nullptr, installDir.wstring().c_str(), &startup, &process);
    if (!launched) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_status.state = State::Error;
        m_status.error = "could not start the update helper";
        m_applyStarted = false;
        return false;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    NOBLE_INFO("Updater: update helper started; restarting into '{}'", tag);
    return true;
}

} // namespace noble
