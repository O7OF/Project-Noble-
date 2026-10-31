#pragma once

#include <filesystem>
#include <string>

namespace noble {

// Steam Workshop downloads for a lobby's required mod.
//
// Noble is not launched by Steam, so there is no Steamworks SDK link in the client. When a download is
// needed we run a short-lived copy of noble-client.exe with "-steam-subscribe <id>", which loads MCC's own
// steam_api64.dll, subscribes to the item and exits; Steam keeps downloading it. The menu process only
// polls the Workshop folder, so Steam cannot block or crash it. If Steam is missing, we open the item's
// Workshop page and wait for the player to subscribe.
namespace SteamWorkshop {

// A Steam published-file id: 1-20 digits, non-zero.
bool IsPublishedFileId(const std::string& id);

// MCC's steam_api64.dll (MCC/Binaries/Win64/steam_api64.dll), or empty.
std::filesystem::path FindSteamApi64();

// Installed item folder: the Workshop cache first, then Noble's data/mods. Empty when not installed
// (or still downloading).
std::filesystem::path FindInstalledFolder(const std::string& publishedFileId);

// Opens the item in Steam (or its web page when Steam is not installed).
bool OpenWorkshopPage(const std::string& publishedFileId);

// One download attempt started by the join prompt. All methods are called from the render thread.
class Download {
public:
    Download() = default;
    ~Download();
    Download(const Download&) = delete;
    Download& operator=(const Download&) = delete;

    // Starts the subscription (child process) or the manual Workshop-page fallback. Returns false only
    // when nothing at all could be started; |message| always explains the current stage.
    bool Start(const std::string& publishedFileId, const std::string& title, std::string& message);

    // Re-checks the child process and the Workshop folder; cheap enough for every frame.
    void Tick();

    bool IsDone() const { return m_stage == Stage::Done; }
    bool IsFailed() const { return m_stage == Stage::Failed; }
    bool NeedsManualSubscribe() const { return m_stage == Stage::Manual; }
    bool IsUsingSteam() const { return m_stage == Stage::Starting || m_stage == Stage::Downloading; }
    const std::string& Status() const { return m_message; }
    const std::filesystem::path& Folder() const { return m_folder; }

    void Cancel();

private:
    enum class Stage { Idle, Starting, Downloading, Manual, Done, Failed };

    void StartManualFallback(const std::string& reason);
    void CloseChild();

    std::string m_id;
    std::string m_title;
    std::string m_message;
    std::filesystem::path m_folder;
    Stage m_stage = Stage::Idle;
    void* m_child = nullptr;          // HANDLE, opaque here so the header stays Win32-free
    unsigned long long m_startedAt = 0;
    unsigned long long m_lastCheck = 0;
};

// Entry point for "-steam-subscribe <id> -steam-dll <path>": loads the DLL, subscribes and exits.
// Exit codes: 0 subscribed/downloading/installed, 2 no DLL, 3 SteamAPI_Init failed, 4 subscribe failed,
// 5 bad id. This runs before the window/engine start.
int SubscribeChildMain(const std::string& publishedFileId, const std::filesystem::path& steamApiDll);

} // namespace SteamWorkshop
} // namespace noble
