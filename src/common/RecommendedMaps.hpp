#pragma once

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

namespace noble {

// Optional downloadable content listed in MODS > RECOMMENDED MAPS. The zips live next to the main asset on this
// client's GitHub release (publish-release.ps1 uploads the Forge World pack there), so the list can grow without a
// client update. Downloads run on a worker thread; the UI polls Status() every frame.
//
// Installing means: download the zip, verify its SHA256 against the release's "<asset>.sha256" asset, then extract
// the zip over the install folder (the zip carries data\... paths plus a readme). VFS then serves
// data\maps\forge_halo.map automatically, so no restart is needed before launching a Forge World match.
class RecommendedMaps {
public:
    struct Item {
        std::string id;                        // "forge_world"
        std::string title;                     // "Noble Forge World"
        std::string description;               // shown under the title in MODS
        std::string assetPrefix;               // release asset name prefix on the GitHub release
        std::filesystem::path installedMarker; // relative to the install dir; existing = installed
        uint64_t approxBytes = 0;              // shown until the release reports the real size
        uint64_t currentMarkerBytes = 0;       // size of installedMarker in the current pack; another size = an older
                                               // pack that needs an update (0 = any size counts as current)
    };

    enum class State { Idle, Resolving, Downloading, Verifying, Installing, Installed, Error };

    struct Status {
        State state = State::Idle;
        float progress = 0.0f;
        uint64_t received = 0;
        uint64_t total = 0;
        std::string assetName;
        std::string error;
    };

    static RecommendedMaps& Get();

    // installDir is the folder that holds Noble.exe. Re-checks which items are already installed.
    void Initialize(const std::filesystem::path& installDir);
    bool IsInitialized() const { return !m_installDir.empty(); }

    const std::vector<Item>& Items() const { return m_items; }
    // Installed and current (the marker exists with the current pack's size).
    bool IsInstalled(const Item& item) const;
    // The marker exists but is from an older pack: its size differs from currentMarkerBytes, or CHECK FOR UPDATES
    // found a newer pack on the release.
    bool IsOutdated(const Item& item) const;

    Status Snapshot(const std::string& id) const;
    // Starts a background download + install. No-op while that item is already working.
    void StartDownload(const std::string& id);

    // CHECK FOR UPDATES: asks the GitHub release list for the newest pack and compares it with what is installed (the
    // record written after a download; installs without one fall back to the marker size check). Background thread.
    struct UpdateInfo {
        bool checking = false;
        bool checked = false;
        bool available = false;     // a newer pack than the installed one is on the release
        std::string latestTag;      // release that carries the newest pack
        std::string installedTag;   // from the install record ("" when unknown)
        uint64_t latestBytes = 0;   // zip size of the newest pack
        std::string error;
    };
    void CheckForUpdates(const std::string& id);
    UpdateInfo UpdateSnapshot(const std::string& id) const;

    static std::string FormatBytes(uint64_t bytes);

private:
    RecommendedMaps() = default;
    void Worker(const std::string& id);
    void SetStatus(const std::string& id, const Status& status);

    mutable std::mutex m_mutex;
    std::filesystem::path m_installDir;
    std::vector<Item> m_items;
    std::vector<std::pair<std::string, Status>> m_status;   // keyed by item id
    std::vector<std::pair<std::string, UpdateInfo>> m_updates;   // keyed by item id
};

} // namespace noble
