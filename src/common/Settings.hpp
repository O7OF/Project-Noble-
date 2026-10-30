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
    float fieldOfView = 78.0f;     // horizontal degrees; 78 is Halo: Reach's own default (haloreach+0xB42C10)
    bool verticalSync = false;     // Noble default: off (Tyler)
    int fpsLimit = 0;              // 0 = uncapped
    bool showFps = false;

    // Controls
    float mouseSensitivity = 1.0f; // multiplier on Noble's base mouse scale (0.0008 per pixel); horizontal
    float mouseSensitivityY = 1.0f; // vertical look speed relative to horizontal (1 = same)
    float mouseAcceleration = 0.0f; // 0 = off: extra turn speed for fast flicks (0..1)
    float mouseSmoothing = 0.0f;    // 0 = off: evens out jitter, at the cost of a little latency (0..0.9)
    bool invertLook = false;
    std::map<int, int> keyOverrides; // profile action index -> virtual-key code
    int wheelUpAction = 4;         // action the mouse wheel's up notches trigger (-1 = nothing); 4 = change weapon
    int wheelDownAction = 4;       // same for down notches (engine key 0xAE; up is 0xAF)
    bool padVibration = true;      // controller rumble (host slot 39 -> XInputSetState)
    int padLookX = 19;             // controller look speed 1..19 (profile +0x1B6 = value - 1; 19 = the stock profile)
    int padLookY = 19;             // vertical, profile +0x1B5 (same scale)
    bool padInvertLook = false;
    int padLayout = 0;             // Halo: Reach button layout (engine/PadLayouts.hpp); 0 = Default
    int padStickLayout = 0;        // 0 = Default (left moves, right looks), 1 = Southpaw (swapped)    // Noble flips the right stick's y before the engine sees it
    float padLeftDeadZone = 0.0f;  // extra radial dead zone Noble applies (0..0.5), on top of the engine's own
    float padRightDeadZone = 0.0f;
    float padVibrationStrength = 1.0f; // 0..1

    // Audio (engine volumes, 0..1, profile +0xA5C/+0xA60/+0xA64)
    float volumeGame = 1.0f;
    float volumeMusic = 1.0f;
    float volumeOther = 1.0f;
    bool menuSounds = true;
    bool masterMute = false;         // silences Noble's own audio (menu music and menu sounds)
    bool muteInBackground = false;   // ... and again whenever Noble is not the active window
    bool inviteNotifications = true;   // show a JOIN toast when a friend invites you
    std::string menuMusicTrack = "reach:menu"; // ReachAudio::MusicTracks() id, "all" or "off"

    // Gameplay
    bool subtitles = true;         // show cinematic/dialogue subtitles (ReachHost subtitle hook)
    uint64_t campaignSkulls = 0;   // MCC skull bits chosen on the SKULLS screen (launch block +0x48)

    // Menus
    std::string menuBackground = "random"; // "random", "noble" (own art), "reach" (from the MCC install)

    // Player / network
    // "Continue" fast path (spec 4.1): the last match this player started, so the main menu can offer it.
    std::string lastPlayedMode;
    std::string lastPlayedMap;
    std::string lastPlayedVariant;
    int lastPlayedDifficulty = 1;
    unsigned long long lastPlayedSkulls = 0;
    std::string lastPlayedMod;          // mod folder, empty when the game was vanilla
    std::string playerName = "NobleSix";
    std::string serviceTag = "NOBL";  // Reach service tag: 1-4 letters/digits, upper case (profile +0x1AC)
    std::string masterServerUrl = "https://outbreak-z-test-server.onrender.com"; // empty = LAN only
    std::vector<std::string> favorites;

    // Account / Auth
    std::string accountUsername = "";
    std::string accountToken = "";
    bool isDeveloper = false;
    int accountUserId = 0; // "address:port" entries saved in the SERVERS tab
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
