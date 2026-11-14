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
    bool IsInstalled(const Item& item) const;

    Status Snapshot(const std::string& id) const;
    // Starts a background download + install. No-op while that item is already working.
    void StartDownload(const std::string& id);

    static std::string FormatBytes(uint64_t bytes);

private:
    RecommendedMaps() = default;
    void Worker(const std::string& id);
    void SetStatus(const std::string& id, const Status& status);

    mutable std::mutex m_mutex;
    std::filesystem::path m_installDir;
    std::vector<Item> m_items;
    std::vector<std::pair<std::string, Status>> m_status;   // keyed by item id
};

} // namespace noble
