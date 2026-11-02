#pragma once

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>

namespace noble {

// GitHub release updater for the menu process.
//
// The release repository is separate from the source checkout: every release there carries one
// "Project-Noble-vX.Y.Z-win64.zip" asset. The menu checks the latest release once per launch,
// shows an "UPDATE AVAILABLE" banner, downloads the zip on request and then hands the actual
// replacement to a tiny PowerShell script that waits for this process to exit. The script keeps
// data\userdata untouched and restarts Noble.exe.
//
// The check is deliberately not silent-download: the asset is a few hundred MB (the menu map), so
// the player sees the size and chooses when to download it.
class Updater {
public:
    enum class State {
        Idle,
        Checking,
        UpToDate,
        Available,
        Downloading,
        Ready,
        Error,
    };

    struct Status {
        State state = State::Idle;
        std::string latestTag;       // e.g. "v0.9.16.24"
        std::string assetName;       // e.g. "Project-Noble-v0.9.16.24-win64.zip"
        std::string notes;           // release body (first lines, for a future details panel)
        std::string error;           // human-readable last error
        uint64_t assetSize = 0;      // bytes
        float progress = 0.0f;       // 0..1 while downloading
        bool downloaded = false;     // zip is on disk and ready to apply
    };

    static Updater& Get();

    // installDir is the folder that holds Noble.exe (or noble-dev.exe in a test run).
    void Initialize(const std::filesystem::path& installDir);
    bool IsInitialized() const { return !m_installDir.empty(); }

    // Starts a background check. Safe to call repeatedly; only one runs at a time.
    void StartCheck();
    // Starts the background download of the asset found by StartCheck().
    void StartDownload();
    // Writes and launches the update script. Returns true when the caller should close the app.
    bool ApplyAndRestart();

    Status Snapshot() const;

    // Human-readable size ("312 MB") for the banner.
    static std::string FormatBytes(uint64_t bytes);

private:
    Updater() = default;
    void CheckThread();
    void DownloadThread();
    void SetState(State state);

    mutable std::mutex m_mutex;
    std::filesystem::path m_installDir;
    std::string m_exeName;
    std::string m_assetUrl;
    std::filesystem::path m_zipPath;
    Status m_status;
    bool m_checkStarted = false;
    bool m_downloadStarted = false;
    bool m_applyStarted = false;
};

} // namespace noble
