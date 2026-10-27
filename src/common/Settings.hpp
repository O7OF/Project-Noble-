#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace noble {

// Player settings, saved as JSON in <exe folder>\data\userdata\settings.json.
// Read once at start-up; the SETTINGS tab and the pause menu change them and call Save().
// Values that end up in the engine's profile (slot 34) are applied when the profile is built and,
// where safe, again while the game runs (see ReachHost ApplySettingsToProfile).
struct SettingsData {
    // Display
    int width = 1280;
    int height = 720;
    bool borderless = false;       // borderless window covering the monitor

    // Controls
    float mouseSensitivity = 1.0f; // multiplier on Noble's base mouse scale (0.0008 per pixel)
    bool invertLook = false;
    std::map<int, int> keyOverrides; // profile action index -> virtual-key code

    // Audio (engine volumes, 0..1, profile +0xA5C/+0xA60/+0xA64)
    float volumeGame = 1.0f;
    float volumeMusic = 1.0f;
    float volumeOther = 1.0f;
    bool menuSounds = true;

    // Gameplay
    bool subtitles = true;         // show cinematic/dialogue subtitles (ReachHost subtitle hook)
    uint64_t campaignSkulls = 0;   // MCC skull bits chosen on the SKULLS screen (launch block +0x48)

    // Menus
    std::string menuBackground = "random"; // "random", "noble" (own art), "reach" (from the MCC install)

    // Player / network
    std::string playerName = "NobleSix";
    std::string masterServerUrl;   // e.g. http://203.0.113.5:8080 ; empty = LAN only
    std::vector<std::string> favorites; // "address:port" entries saved in the SERVERS tab
};

class Settings {
public:
    static Settings& Get();

    void Load();
    bool Save();

    SettingsData Snapshot() const;
    void Update(const SettingsData& data);   // replace and save

    std::filesystem::path FilePath() const;

private:
    Settings() = default;
    mutable std::mutex m_mutex;
    SettingsData m_data;
    bool m_loaded = false;
};

} // namespace noble
