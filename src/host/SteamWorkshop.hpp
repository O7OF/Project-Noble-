#pragma once

#include <filesystem>
#include <memory>
#include <string>

namespace noble {

// Steam Workshop downloads for a lobby's required mod.
//
// Noble is not launched by Steam, so there is no Steamworks SDK link in the client. When a download is
// needed we run a background copy of noble-client.exe with "-steam-subscribe <id>", which loads MCC's
// own steam_api64.dll, calls SubscribeItem + DownloadItem(highPriority=true), pumps Steam callbacks,
// and streams live ISteamUGC::GetItemDownloadInfo progress back to the menu process without ever popping
// open a Steam window automatically. If an item also exposes a direct CDN file_url and Steam is offline,
// Noble can fall back to direct WinHTTP downloading in-app.
namespace SteamWorkshop {

// A Steam published-file id: 1-20 digits, non-zero.
bool IsPublishedFileId(const std::string& id);

// MCC's steam_api64.dll (MCC/Binaries/Win64/steam_api64.dll), or empty.
std::filesystem::path FindSteamApi64();

// Installed item folder: the Workshop cache first, then Noble's data/mods. Empty when not installed
// (or still downloading).
std::filesystem::path FindInstalledFolder(const std::string& publishedFileId);

// Opens the item in Steam (or its web page when Steam is not installed). Only invoked on explicit click.
bool OpenWorkshopPage(const std::string& publishedFileId);

// One download attempt started by the join prompt. All methods are called from the render thread.
class Download {
public:
    Download() = default;
    ~Download();
    Download(const Download&) = delete;
    Download& operator=(const Download&) = delete;

    // Optional metadata hints supplied by GetPublishedFileDetails before calling Start.
    void SetHints(const std::string& directFileUrl, unsigned long long knownSizeBytes);

    // Starts the in-app Workshop download (background SteamUGC helper or direct CDN URL when offline).
    // Never opens Steam windows automatically. Returns false only when nothing could be started.
    bool Start(const std::string& publishedFileId, const std::string& title, std::string& message);

    // Re-checks progress JSON / Workshop folder / direct HTTP state; cheap enough for every frame.
    void Tick();

    bool IsDone() const { return m_stage == Stage::Done; }
    bool IsFailed() const { return m_stage == Stage::Failed; }
    bool NeedsManualSubscribe() const { return false; }
    bool IsUsingSteam() const { return m_stage == Stage::Starting || m_stage == Stage::Downloading; }
    const std::string& Status() const { return m_message; }
    const std::string& StageLabel() const { return m_stageLabel; }
    const std::filesystem::path& Folder() const { return m_folder; }

    unsigned long long BytesDownloaded() const { return m_bytesDownloaded; }
    unsigned long long BytesTotal() const { return m_bytesTotal; }
    unsigned long long ExpectedBytes() const { return m_bytesTotal > 0 ? m_bytesTotal : m_knownSizeBytes; }
    double SpeedBytesPerSec() const { return m_speedBps; }
    double EtaSeconds() const;
    float ProgressFraction() const;

    void Cancel();

private:
    enum class Stage { Idle, Starting, Downloading, DirectHttp, Done, Failed };
    struct DirectState;

    bool TryStartDirectHttp(const std::string& reason);
    void CloseChild(bool signalStop);

    std::string m_id;
    std::string m_title;
    std::string m_directFileUrl;
    unsigned long long m_knownSizeBytes = 0;
    std::string m_message;
    std::string m_stageLabel;
    std::filesystem::path m_folder;
    std::filesystem::path m_progressPath;
    std::filesystem::path m_stopPath;
    Stage m_stage = Stage::Idle;
    void* m_child = nullptr;          // HANDLE, opaque here so the header stays Win32-free
    unsigned long long m_startedAt = 0;
    unsigned long long m_lastCheck = 0;
    unsigned long long m_bytesDownloaded = 0;
    unsigned long long m_bytesTotal = 0;
    unsigned int m_steamState = 0;
    unsigned long long m_speedSampleAt = 0;
    unsigned long long m_speedSampleBytes = 0;
    double m_speedBps = 0.0;
    std::shared_ptr<DirectState> m_direct;
};

// Entry point for "-steam-subscribe <id> -steam-dll <path> [-steam-progress <json>] [-parent-pid <pid>]".
// Exit codes: 0 finished/stopped cleanly, 2 no DLL/exports, 3 SteamAPI_Init failed, 4 subscribe/download
// failed, 5 bad id. Runs before window/engine start.
int SubscribeChildMain(const std::string& publishedFileId, const std::filesystem::path& steamApiDll,
                       const std::filesystem::path& progressPath = {}, unsigned long parentPid = 0);

} // namespace SteamWorkshop
} // namespace noble
