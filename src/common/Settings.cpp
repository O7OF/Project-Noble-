#include "engine/KeyBindings.hpp"
#include "host/EngineLimits.hpp"
#include "Settings.hpp"
#include "BuildIdentity.hpp"
#include "DeviceIdentity.hpp"
#include "Logger.hpp"

#include <windows.h>
#include <shellapi.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <fstream>
#include <iterator>

namespace noble {

namespace {

std::atomic<float> g_uiScale{ 1.0f };   // copy of SettingsData::uiScale for the UI's per-frame reads

// Bumped when a load-time migration changes a value the player can also set by hand. Migrations for
// such a value only run on files older than the version that introduced them, so a player's choice
// is never rewritten on every load (the music-volume migration used to do exactly that: 50 % always
// came back as 15 %).
constexpr int kSettingsVersion = 2;

std::filesystem::path ExeDir() {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

template <typename T>
void Read(const nlohmann::json& j, const char* key, T& out) {
    auto it = j.find(key);
    if (it == j.end()) return;
    try {
        out = it->get<T>();
    } catch (...) {
        // keep the default for a value of the wrong type
    }
}

void Clamp(SettingsData& d) {
    d.width = std::clamp(d.width, 640, 7680);
    d.height = std::clamp(d.height, 360, 4320);
    if (d.fullscreenWidth <= 0 || d.fullscreenHeight <= 0) {
        d.fullscreenWidth = 0;   // native
        d.fullscreenHeight = 0;
    } else {
        d.fullscreenWidth = std::clamp(d.fullscreenWidth, 640, 7680);
        d.fullscreenHeight = std::clamp(d.fullscreenHeight, 360, 4320);
    }
    d.monitor = std::clamp(d.monitor, 0, 16);
    d.fieldOfView = std::clamp(d.fieldOfView, 70.0f, 120.0f);  // slider range (SETTINGS > VIDEO)
    d.uiScale = std::isfinite(d.uiScale) ? std::clamp(d.uiScale, 0.75f, 1.50f) : 1.0f;
    d.firstPersonFovMod = std::clamp(d.firstPersonFovMod, -1.20f, 1.20f);
    d.vehicleFirstPersonFovMod = std::clamp(d.vehicleFirstPersonFovMod, -1.20f, 1.20f);
    d.vehicleFirstPersonOffsetX = std::clamp(d.vehicleFirstPersonOffsetX, -1.00f, 1.00f);
    d.vehicleFirstPersonOffsetY = std::clamp(d.vehicleFirstPersonOffsetY, -1.00f, 1.00f);
    d.thirdPersonZoom = std::clamp(d.thirdPersonZoom, 0.10f, 1.00f);
    d.thirdPersonOffsetX = std::clamp(d.thirdPersonOffsetX, -1.00f, 1.00f);
    d.thirdPersonOffsetY = std::clamp(d.thirdPersonOffsetY, -1.00f, 1.00f);
    constexpr int kFpsLimits[] = { 0, 30, 60, 120, 144, 165, 200, 240, 360 };
    if (std::find(std::begin(kFpsLimits), std::end(kFpsLimits), d.fpsLimit) == std::end(kFpsLimits)) {
        d.fpsLimit = 0;
    }
    d.mouseSensitivity = std::clamp(d.mouseSensitivity, 0.1f, 10.0f);
    d.mouseSensitivityY = std::clamp(d.mouseSensitivityY, 0.25f, 4.0f);
    d.volumeGame = std::clamp(d.volumeGame, 0.0f, 1.0f);
    d.volumeMusic = std::clamp(d.volumeMusic, 0.0f, 1.0f);
    d.volumeOther = std::clamp(d.volumeOther, 0.0f, 1.0f);
    d.voiceVolume = std::clamp(d.voiceVolume, 0.0f, 2.0f);
    if (d.voiceBitrateKbps != 16 && d.voiceBitrateKbps != 24) d.voiceBitrateKbps = 20;
    d.voiceChannel = std::clamp(d.voiceChannel, 0, 2);
    // Engine limits are PINNED since 2026-11-15 (Tyler): every install runs one profile and AI actor count
    // (limits::kShippedProfile / kShippedAiActors) so no player can end up on a different game-state layout from
    // the host. The fields stay in the file for compatibility and dev diagnostics; any saved value is overwritten.
    d.extendedLimits = static_cast<int>(limits::kShippedProfile);
    d.aiActors = limits::kShippedAiActors;
    auto trimName = [](std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    };
    trimName(d.playerName);
    if (d.playerName.empty() || d.playerName == "Player") d.playerName = "NobleSix";
    if (d.playerName.size() > 20) d.playerName.resize(20);
    {
        // Service tag: Reach allows up to 4 characters, letters and digits, shown in upper case.
        std::string tag;
        for (char c : d.serviceTag) {
            if (tag.size() == 4) break;
            if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
            if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) tag.push_back(c);
        }
        d.serviceTag = tag.empty() ? "NOBL" : tag;
    }
    trimName(d.masterServerUrl);
    while (!d.masterServerUrl.empty() && d.masterServerUrl.back() == '/') d.masterServerUrl.pop_back();
    // Migration: every saved URL from the old shared cluster (or none) moves to the
    // closed-source US East node. A region the player picked on the new cluster is kept.
    {
        const std::string legacyUrls[] = {
            NOBLE_OBF("https://outbreak-z-test-server.onrender.com"),
            NOBLE_OBF("https://servertestdeploy-jfxb.onrender.com"),
            NOBLE_OBF("https://noble-server-2.onrender.com"),
            NOBLE_OBF("https://servertestdeploy-1.onrender.com"),
            NOBLE_OBF("https://projectnobleeast.onrender.com"),
            NOBLE_OBF("https://projectnoblewest.onrender.com"),
            NOBLE_OBF("https://projectnobleuk.onrender.com"),
        };
        bool legacy = d.masterServerUrl.empty();
        for (const std::string& old : legacyUrls) {
            if (d.masterServerUrl == old) legacy = true;
        }
        if (legacy) d.masterServerUrl = MasterDefaultUrl();
    }
    if (d.favorites.size() > 50) d.favorites.resize(50);
    d.wheelUpAction = std::clamp(d.wheelUpAction, -1, 65);
    d.wheelDownAction = std::clamp(d.wheelDownAction, -1, 65);
    d.padVibrationStrength = std::clamp(d.padVibrationStrength, 0.0f, 1.0f);
    d.padLookSpeedX = std::clamp(d.padLookSpeedX, 1.0f, 10.0f);
    d.padLookSpeedY = std::clamp(d.padLookSpeedY, 1.0f, 10.0f);
    d.padLeftDeadZone = std::clamp(d.padLeftDeadZone, 0.0f, 0.5f);
    d.padRightDeadZone = std::clamp(d.padRightDeadZone, 0.0f, 0.5f);
    for (auto it = d.keyOverrides.begin(); it != d.keyOverrides.end();) {
        // A key of 0 is deliberate: the player unbound that action.
        if (it->first < 0 || it->first >= KeyBindings::kActionCount || it->second < 0 || it->second >= 256) it = d.keyOverrides.erase(it);
        else ++it;
    }
}

} // namespace

Settings& Settings::Get() {
    static Settings instance;
    return instance;
}

std::filesystem::path Settings::FilePath() const {
    return ExeDir() / "data" / "userdata" / "settings.json";
}

void Settings::Load() {
    DeviceIdentityStore::MigrateNewestFile("userdata/settings.json");
    SettingsData d;
    const auto path = FilePath();
    std::ifstream in(path);
    const bool existed = in.is_open();
    int fileVersion = 0;   // 0 = a file from before this field existed
    if (in) {
        try {
            const nlohmann::json j = nlohmann::json::parse(in);
            fileVersion = j.value("settingsVersion", 0);
            d.firstStartDone = j.value("firstStartDone", true);
            Read(j, "lastSeenVersion", d.lastSeenVersion);
            Read(j, "width", d.width);
            Read(j, "height", d.height);
            Read(j, "borderless", d.borderless);
    Read(j, "networkBudgetBoost", d.networkBudgetBoost);
    Read(j, "hostUploadKbps", d.hostUploadKbps);
    Read(j, "packetRedundancy", d.packetRedundancy);
    Read(j, "hostMaxPingMs", d.hostMaxPingMs);
    Read(j, "hostFirefightAllies", d.hostFirefightAllies);
    if (d.hostFirefightAllies != 0 && d.hostFirefightAllies != 5) d.hostFirefightAllies = 0;
    Read(j, "hostFirefightAllySquads", d.hostFirefightAllySquads);
    if (d.hostFirefightAllySquads < 1 || d.hostFirefightAllySquads > 5) d.hostFirefightAllySquads = 2;
    Read(j, "hostFirefightWavePacing", d.hostFirefightWavePacing);
    if (d.hostFirefightWavePacing != 50 && d.hostFirefightWavePacing != 150 && d.hostFirefightWavePacing != 200)
        d.hostFirefightWavePacing = 100;
    Read(j, "hostMinPingMs", d.hostMinPingMs);
    if (d.hostMinPingMs < 0 || d.hostMinPingMs > 1000) d.hostMinPingMs = 0;
    if (d.hostMaxPingMs < 0 || d.hostMaxPingMs > 1000) d.hostMaxPingMs = 0;
    if (d.hostUploadKbps < 0) d.hostUploadKbps = 0;
    if (d.hostUploadKbps > 1000000) d.hostUploadKbps = 1000000;
            Read(j, "exclusiveFullscreen", d.exclusiveFullscreen);
            if (d.exclusiveFullscreen) d.borderless = true;
            Read(j, "fullscreenWidth", d.fullscreenWidth);
            Read(j, "fullscreenHeight", d.fullscreenHeight);
            Read(j, "monitor", d.monitor);
            Read(j, "fieldOfView", d.fieldOfView);
            Read(j, "firstPersonFovMod", d.firstPersonFovMod);
            Read(j, "vehicleFirstPersonFovMod", d.vehicleFirstPersonFovMod);
            Read(j, "vehicleFirstPersonOffsetX", d.vehicleFirstPersonOffsetX);
            Read(j, "vehicleFirstPersonOffsetY", d.vehicleFirstPersonOffsetY);
            Read(j, "thirdPersonZoom", d.thirdPersonZoom);
            Read(j, "thirdPersonOffsetX", d.thirdPersonOffsetX);
            Read(j, "thirdPersonOffsetY", d.thirdPersonOffsetY);
            Read(j, "verticalSync", d.verticalSync);
            Read(j, "fpsLimit", d.fpsLimit);
            Read(j, "showFps", d.showFps);
    Read(j, "showNetStats", d.showNetStats);
            Read(j, "uiScale", d.uiScale);
            Read(j, "throttleUnfocused", d.throttleUnfocused);
            Read(j, "brightness", d.brightness);
            Read(j, "fxaa", d.fxaa);
            Read(j, "menuSupersampling", d.menuSupersampling);
            Read(j, "mouseSensitivity", d.mouseSensitivity);
            Read(j, "mouseSensitivityY", d.mouseSensitivityY);
            Read(j, "mouseAcceleration", d.mouseAcceleration);
            Read(j, "mouseSmoothing", d.mouseSmoothing);
            Read(j, "mouseReticleCursor", d.mouseReticleCursor);
            Read(j, "invertLook", d.invertLook);
            Read(j, "toggleCrouch", d.toggleCrouch);
            Read(j, "toggleZoom", d.toggleZoom);
            Read(j, "wheelUpAction", d.wheelUpAction);
            Read(j, "wheelDownAction", d.wheelDownAction);
            Read(j, "padVibration", d.padVibration);
            Read(j, "padVibrationStrength", d.padVibrationStrength);
            // Older files saved 1..19; 19 stays the top of the new 1.00..10.00 scale.
            {
                int oldX = 0, oldY = 0;
                Read(j, "padLookX", oldX);
                Read(j, "padLookY", oldY);
                if (oldX > 0) d.padLookSpeedX = (static_cast<float>(oldX) + 1.0f) * 0.1f;
                if (oldY > 0) d.padLookSpeedY = (static_cast<float>(oldY) + 1.0f) * 0.1f;
            }
            // The first 1-10 scale put stock at 10.00; stock is 2.00 now, so those values are scaled to feel the same.
            {
                float oldX = 0.0f, oldY = 0.0f;
                Read(j, "padLookSpeedX", oldX);
                Read(j, "padLookSpeedY", oldY);
                if (oldX > 0.0f) d.padLookSpeedX = oldX * 0.2f;
                if (oldY > 0.0f) d.padLookSpeedY = oldY * 0.2f;
            }
            Read(j, "padLookRateX", d.padLookSpeedX);
            Read(j, "padLookRateY", d.padLookSpeedY);
            Read(j, "padInvertLook", d.padInvertLook);
            Read(j, "padLayout", d.padLayout);
            Read(j, "padStickLayout", d.padStickLayout);
            Read(j, "padToggleZoom", d.padToggleZoom);
            if (j.contains("padPaddleKeys") && j["padPaddleKeys"].is_array() && j["padPaddleKeys"].size() == d.padPaddleKeys.size()) {
                for (size_t i = 0; i < d.padPaddleKeys.size(); ++i) {
                    if (j["padPaddleKeys"][i].is_number_integer()) d.padPaddleKeys[i] = j["padPaddleKeys"][i].get<int>();
                }
                // The first default was F13-F18; move anyone still on it to F1-F6. A set the player chose stays.
                if (fileVersion < 2 && d.padPaddleKeys == std::array<int, 6>{ 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81 }) d.padPaddleKeys = SettingsData{}.padPaddleKeys;
            }
            if (j.contains("padBindings") && j["padBindings"].is_object()) {
                for (const auto& [key, value] : j["padBindings"].items()) {
                    if (value.is_number_integer()) d.padBindings[key] = value.get<int>();
                }
            }
            Read(j, "padLeftDeadZone", d.padLeftDeadZone);
            Read(j, "padRightDeadZone", d.padRightDeadZone);
            Read(j, "padVirtualCursor", d.padVirtualCursor);
            Read(j, "volumeGame", d.volumeGame);
            Read(j, "volumeMusic", d.volumeMusic);
            // Legacy 50 % default -> 15 % so menu music is not uncomfortably loud. Only for files from
            // before settingsVersion 2: after that, 50 % is a value the player picked and it must stick.
            if (fileVersion < 2 && std::fabs(d.volumeMusic - 0.50f) < 1e-4f) {
                d.volumeMusic = 0.15f;
            }
            Read(j, "volumeOther", d.volumeOther);
            Read(j, "menuSounds", d.menuSounds);
            Read(j, "masterMute", d.masterMute);
            Read(j, "muteInBackground", d.muteInBackground);
            Read(j, "inviteNotifications", d.inviteNotifications);
            Read(j, "voiceEnabled", d.voiceEnabled);
            Read(j, "voiceInputDevice", d.voiceInputDevice);
            Read(j, "voiceOutputDevice", d.voiceOutputDevice);
            Read(j, "voiceVolume", d.voiceVolume);
            Read(j, "voiceOpenMic", d.voiceOpenMic);
            Read(j, "voiceBitrateKbps", d.voiceBitrateKbps);
            Read(j, "voiceChannel", d.voiceChannel);
            Read(j, "voiceMuteAll", d.voiceMuteAll);
            Read(j, "voiceOverlayEnabled", d.voiceOverlayEnabled);
            Read(j, "textChatEnabled", d.textChatEnabled);
            Read(j, "menuMusicTrack", d.menuMusicTrack);
            if (j.contains("menuMusicDisabled") && j["menuMusicDisabled"].is_array()) {
                for (const auto& id : j["menuMusicDisabled"]) if (id.is_string()) d.menuMusicDisabled.push_back(id.get<std::string>());
            }
            Read(j, "subtitles", d.subtitles);
            Read(j, "campaignSkulls", d.campaignSkulls);
            Read(j, "extendedLimits", d.extendedLimits);   // old files: Clamp() re-pins both
            Read(j, "aiActors", d.aiActors);
            Read(j, "menuBackground", d.menuBackground);
            Read(j, "playerName", d.playerName);
            Read(j, "lastPlayedMode", d.lastPlayedMode);
            Read(j, "lastPlayedMap", d.lastPlayedMap);
            Read(j, "lastPlayedVariant", d.lastPlayedVariant);
            Read(j, "lastPlayedDifficulty", d.lastPlayedDifficulty);
            Read(j, "lastPlayedSkulls", d.lastPlayedSkulls);
            Read(j, "lastPlayedMod", d.lastPlayedMod);
            Read(j, "serviceTag", d.serviceTag);
            Read(j, "masterServerUrl", d.masterServerUrl);
            // Free federation: the West registry merges East and UK server-side. URL cleanup happens
            // in Clamp() so every entry point (file, UI, command line) is normalised the same way.
            Read(j, "favorites", d.favorites);
            Read(j, "accountUsername", d.accountUsername);
            Read(j, "accountToken", d.accountToken);
            Read(j, "rememberAccount", d.rememberAccount);
            Read(j, "isDeveloper", d.isDeveloper);
            Read(j, "isTester", d.isTester);
            Read(j, "accountUserId", d.accountUserId);
            auto keys = j.find("keyOverrides");
            if (keys != j.end() && keys->is_object()) {
                for (auto it = keys->begin(); it != keys->end(); ++it) {
                    try {
                        d.keyOverrides[std::stoi(it.key())] = it.value().get<int>();
                    } catch (...) {
                    }
                }
            }
            NOBLE_INFO("Settings: loaded {}", path.string());
        } catch (const std::exception& e) {
            NOBLE_WARN("Settings: could not read {} ({}); using defaults", path.string(), e.what());
        }
    }

    // Apply per-process command-line identity overrides passed by Relaunch::Start so multiple instances
    // running on the same PC retain their own logged-in account and player name across match relaunches.
    bool sawIdentityOverride = false;
    {
        auto narrowUtf8 = [](const wchar_t* w) -> std::string {
            if (!w || !*w) return "";
            const int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
            if (len <= 0) return "";
            std::string s(static_cast<size_t>(len - 1), '\0');
            WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), len, nullptr, nullptr);
            return s;
        };
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv) {
            for (int i = 1; i + 1 < argc; ++i) {
                const std::wstring arg = argv[i];
                if (arg == L"-player-name" || arg == L"-service-tag" || arg == L"-account-user" ||
                    arg == L"-account-token" || arg == L"-user-id" || arg == L"-account-dev" ||
                    arg == L"-account-tester" || arg == L"-remember-account") {
                    sawIdentityOverride = true;
                }
                if (arg == L"-player-name") {
                    d.playerName = narrowUtf8(argv[++i]);
                } else if (arg == L"-service-tag") {
                    d.serviceTag = narrowUtf8(argv[++i]);
                } else if (arg == L"-account-user") {
                    d.accountUsername = narrowUtf8(argv[++i]);
                } else if (arg == L"-account-token") {
                    d.accountToken = narrowUtf8(argv[++i]);
                } else if (arg == L"-user-id") {
                    d.accountUserId = _wtoi(argv[++i]);
                } else if (arg == L"-account-dev") {
                    d.isDeveloper = (_wtoi(argv[++i]) != 0);
                } else if (arg == L"-account-tester") {
                    d.isTester = (_wtoi(argv[++i]) != 0);
                } else if (arg == L"-remember-account") {
                    d.rememberAccount = true;
                }
            }
            LocalFree(argv);
        }
    }
    m_identityFromCommandLine = sawIdentityOverride;

    if (!existed) {
        // New install: pace to the display instead of No limit, which held GPUs at 100 % (players can still pick
        // No limit in SETTINGS > GRAPHICS).
        d.fpsLimit = DefaultFpsLimit();
    }
    Clamp(d);
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_data = d;
        m_loaded = true;
    }
    g_uiScale.store(d.uiScale);
    if (!existed) {
        Save();
    }
}

void Settings::ReloadFromDisk() {
    // The match child process - and every other Noble window on this PC - shares data\userdata\settings.json.
    // Re-read it before drawing/using settings again so this process neither shows stale values nor writes them
    // back over the newer file on its next APPLY. Identity fields stay on this process's launch-time values when
    // it was relaunched with an identity (multiple windows on one PC must keep their own account and player name).
    const bool keepIdentity = m_identityFromCommandLine;
    SettingsData mine;
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        mine = m_data;
    }
    Load();
    if (keepIdentity) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_data.playerName = mine.playerName;
        m_data.serviceTag = mine.serviceTag;
        m_data.accountUsername = mine.accountUsername;
        m_data.accountToken = mine.accountToken;
        m_data.accountUserId = mine.accountUserId;
        m_data.isDeveloper = mine.isDeveloper;
        m_data.isTester = mine.isTester;
        m_data.rememberAccount = mine.rememberAccount;
    }
    const SettingsData after = Snapshot();
    NOBLE_INFO("Settings: reloaded from disk (FOV {:.0f}, volume {:.2f}, identity {})",
               after.fieldOfView, after.volumeGame,
               keepIdentity ? "kept from this window's launch command line" : "from disk");
}

void Settings::TestReloadTick() {
    // Test-only "-settings-reload-at <seconds>": fire ReloadFromDisk() once from the front-end loop. Same
    // pattern as "-fov-test-at" in CameraFov: a deterministic hook so a test can prove that the menu process
    // picks up a settings.json written by another Noble process.
    static bool s_parsed = false, s_done = false;
    static double s_at = 0.0;
    static uint64_t s_start = 0;
    const uint64_t now = GetTickCount64();
    if (!s_parsed) {
        s_parsed = true;
        s_start = now;
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (_wcsicmp(argv[i], L"-settings-reload-at") == 0) {
                s_at = _wtof(argv[i + 1]);
            }
        }
        if (argv) LocalFree(argv);
        if (s_at <= 0.0) s_done = true;
    }
    if (s_done || now - s_start < static_cast<uint64_t>(s_at * 1000.0)) return;
    s_done = true;
    NOBLE_INFO("Settings: -settings-reload-at fired");
    Settings::Get().ReloadFromDisk();
}

bool Settings::Save() {
    nlohmann::json j;
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        const SettingsData& d = m_data;
        j["width"] = d.width;
        j["height"] = d.height;
        j["borderless"] = d.borderless;
    j["networkBudgetBoost"] = d.networkBudgetBoost;
    j["hostUploadKbps"] = d.hostUploadKbps;
    j["hostMaxPingMs"] = d.hostMaxPingMs;
    j["hostFirefightAllies"] = d.hostFirefightAllies;
    j["hostFirefightAllySquads"] = d.hostFirefightAllySquads;
    j["hostFirefightWavePacing"] = d.hostFirefightWavePacing;
    j["hostMinPingMs"] = d.hostMinPingMs;
    j["packetRedundancy"] = d.packetRedundancy;
        j["exclusiveFullscreen"] = d.exclusiveFullscreen;
        j["fullscreenWidth"] = d.fullscreenWidth;
        j["fullscreenHeight"] = d.fullscreenHeight;
        j["monitor"] = d.monitor;
        j["fieldOfView"] = d.fieldOfView;
        j["firstPersonFovMod"] = d.firstPersonFovMod;
        j["vehicleFirstPersonFovMod"] = d.vehicleFirstPersonFovMod;
        j["vehicleFirstPersonOffsetX"] = d.vehicleFirstPersonOffsetX;
        j["vehicleFirstPersonOffsetY"] = d.vehicleFirstPersonOffsetY;
        j["thirdPersonZoom"] = d.thirdPersonZoom;
        j["thirdPersonOffsetX"] = d.thirdPersonOffsetX;
        j["thirdPersonOffsetY"] = d.thirdPersonOffsetY;
        j["verticalSync"] = d.verticalSync;
        j["fpsLimit"] = d.fpsLimit;
        j["showFps"] = d.showFps;
    j["showNetStats"] = d.showNetStats;
        j["uiScale"] = d.uiScale;
        j["throttleUnfocused"] = d.throttleUnfocused;
        j["brightness"] = d.brightness;
        j["fxaa"] = d.fxaa;
        j["menuSupersampling"] = d.menuSupersampling;
        j["mouseSensitivity"] = d.mouseSensitivity;
        j["mouseSensitivityY"] = d.mouseSensitivityY;
        j["mouseAcceleration"] = d.mouseAcceleration;
        j["mouseSmoothing"] = d.mouseSmoothing;
        j["mouseReticleCursor"] = d.mouseReticleCursor;
        j["invertLook"] = d.invertLook;
        j["toggleCrouch"] = d.toggleCrouch;
    j["toggleZoom"] = d.toggleZoom;
        j["wheelUpAction"] = d.wheelUpAction;
        j["wheelDownAction"] = d.wheelDownAction;
        j["padVibration"] = d.padVibration;
        j["padVibrationStrength"] = d.padVibrationStrength;
        j["padLookRateX"] = d.padLookSpeedX;
        j["padLookRateY"] = d.padLookSpeedY;
        j["padInvertLook"] = d.padInvertLook;
        j["padLayout"] = d.padLayout;
        j["padStickLayout"] = d.padStickLayout;
        j["padToggleZoom"] = d.padToggleZoom;
        j["padPaddleKeys"] = d.padPaddleKeys;
        if (!d.padBindings.empty()) j["padBindings"] = d.padBindings;
        j["padLeftDeadZone"] = d.padLeftDeadZone;
        j["padRightDeadZone"] = d.padRightDeadZone;
        j["padVirtualCursor"] = d.padVirtualCursor;
        j["volumeGame"] = d.volumeGame;
        j["volumeMusic"] = d.volumeMusic;
        j["volumeOther"] = d.volumeOther;
        j["menuSounds"] = d.menuSounds;
        j["masterMute"] = d.masterMute;
        j["muteInBackground"] = d.muteInBackground;
        j["inviteNotifications"] = d.inviteNotifications;
    j["firstStartDone"] = d.firstStartDone;
    j["lastSeenVersion"] = d.lastSeenVersion;
        j["voiceEnabled"] = d.voiceEnabled;
        j["voiceInputDevice"] = d.voiceInputDevice;
        j["voiceOutputDevice"] = d.voiceOutputDevice;
        j["voiceVolume"] = d.voiceVolume;
        j["voiceOpenMic"] = d.voiceOpenMic;
        j["voiceBitrateKbps"] = d.voiceBitrateKbps;
        j["voiceChannel"] = d.voiceChannel;
        j["voiceMuteAll"] = d.voiceMuteAll;
        j["voiceOverlayEnabled"] = d.voiceOverlayEnabled;
        j["textChatEnabled"] = d.textChatEnabled;
        j["menuMusicTrack"] = d.menuMusicTrack;
        j["menuMusicDisabled"] = d.menuMusicDisabled;
        j["subtitles"] = d.subtitles;
        j["campaignSkulls"] = d.campaignSkulls;
        j["extendedLimits"] = d.extendedLimits;
        j["aiActors"] = d.aiActors;
        j["menuBackground"] = d.menuBackground;
        j["playerName"] = d.playerName;
        j["lastPlayedMode"] = d.lastPlayedMode;
        j["lastPlayedMap"] = d.lastPlayedMap;
        j["lastPlayedVariant"] = d.lastPlayedVariant;
        j["lastPlayedDifficulty"] = d.lastPlayedDifficulty;
        j["lastPlayedSkulls"] = d.lastPlayedSkulls;
        j["lastPlayedMod"] = d.lastPlayedMod;
        j["serviceTag"] = d.serviceTag;
        j["masterServerUrl"] = d.masterServerUrl;
        j["favorites"] = d.favorites;
        j["accountUsername"] = d.accountUsername;
        j["accountToken"] = d.accountToken;
        j["rememberAccount"] = d.rememberAccount;
        j["isDeveloper"] = d.isDeveloper;
        j["isTester"] = d.isTester;
        j["accountUserId"] = d.accountUserId;
        nlohmann::json keys = nlohmann::json::object();
        for (const auto& [action, vk] : d.keyOverrides) keys[std::to_string(action)] = vk;
        j["keyOverrides"] = keys;
        j["settingsVersion"] = kSettingsVersion;
    }
    const auto path = FilePath();
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        NOBLE_ERROR("Settings: could not create {}: {}", path.parent_path().string(), ec.message());
    }
    // The temp name carries the process id: the launcher and the match process are both running and
    // both save settings, and a shared "settings.json.tmp" lets one of them truncate the file the
    // other is writing, which loses the save.
    const auto tmp = path.parent_path() /
                     (path.filename().string() + "." + std::to_string(GetCurrentProcessId()) + ".tmp");
    const std::string text = j.dump(2);
    bool wrote = false;
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (out) {
            out.write(text.data(), static_cast<std::streamsize>(text.size()));
            out.flush();
            wrote = out.good();
        }
        if (!wrote)
            NOBLE_WARN("Settings: could not write {} (the folder may be read-only)", tmp.string());
    }
    if (wrote) {
        // Replace the live file. MoveFileExW replaces an existing file even when another Noble
        // process still has it open for reading; std::filesystem::rename can fail with a sharing
        // violation in that case, which used to make a save silently do nothing.
        if (!MoveFileExW(tmp.wstring().c_str(), path.wstring().c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
            const DWORD winError = GetLastError();
            std::filesystem::remove(tmp, ec);
            // Last resort: write settings.json in place, so the player's change is not lost.
            std::ofstream direct(path, std::ios::binary | std::ios::trunc);
            if (direct) {
                direct.write(text.data(), static_cast<std::streamsize>(text.size()));
                direct.flush();
                wrote = direct.good();
            } else {
                wrote = false;
            }
            NOBLE_WARN("Settings: could not replace {} (Win32 error {}); wrote it in place: {}",
                       path.string(), winError, wrote ? "ok" : "FAILED");
        }
    }
    if (!wrote) {
        NOBLE_ERROR("Settings: SAVE FAILED for {} - the settings on disk were not changed", path.string());
        return false;
    }
    DeviceIdentityStore::MirrorSaveFile("userdata/settings.json");
    NOBLE_INFO("Settings: saved {} ({} values, file version {})", path.string(), j.size(), kSettingsVersion);
    return true;
}

SettingsData Settings::Snapshot() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_data;
}

namespace {
void DropPrivateKeys(nlohmann::json& j) {
    for (const char* key : { "accountUsername", "accountToken", "accountUserId", "isDeveloper", "masterServerUrl",
                             "voiceInputDevice", "voiceOutputDevice", "firstStartDone", "lastSeenVersion",
                             "lastPlayedMode", "lastPlayedMap" }) {
        j.erase(key);
    }
}
} // namespace

bool Settings::ExportTo(const std::filesystem::path& file) {
    Save();
    try {
        std::ifstream in(FilePath());
        nlohmann::json j = nlohmann::json::parse(in);
        DropPrivateKeys(j);
        std::ofstream out(file, std::ios::binary | std::ios::trunc);
        out << j.dump(2);
        return static_cast<bool>(out);
    } catch (...) {
        return false;
    }
}

bool Settings::ImportFrom(const std::filesystem::path& file) {
    try {
        std::error_code ec;
        if (std::filesystem::file_size(file, ec) > 4 * 1024 * 1024 || ec) return false;
        std::ifstream in(file);
        nlohmann::json incoming = nlohmann::json::parse(in);
        if (!incoming.is_object()) return false;
        DropPrivateKeys(incoming);
        Save();
        std::ifstream current(FilePath());
        nlohmann::json merged = nlohmann::json::parse(current);
        current.close();
        merged.update(incoming);
        {
            std::ofstream out(FilePath(), std::ios::binary | std::ios::trunc);
            out << merged.dump(2);
            if (!out) return false;
        }
        Load();   // Load checks every value's type and range
        Save();   // so the file holds the checked values too
        return true;
    } catch (...) {
        return false;
    }
}

bool Settings::Update(const SettingsData& data) {
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_data = data;
        Clamp(m_data);
        g_uiScale.store(m_data.uiScale);
    }
    return Save();
}

float Settings::UiScale() { return g_uiScale.load(std::memory_order_relaxed); }

int Settings::DefaultFpsLimit() {
    DEVMODEW mode{};
    mode.dmSize = sizeof(mode);
    int hz = 0;
    if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode)) hz = static_cast<int>(mode.dmDisplayFrequency);
    if (hz <= 1) return 60;                          // 0/1 = "hardware default": unknown rate
    constexpr int kLimits[] = { 30, 60, 120, 144, 165, 200, 240, 360 };  // the capped values of SETTINGS' list
    int best = 30;
    for (const int limit : kLimits) {
        if (limit <= hz + 1) best = limit;           // +1: 59.94 Hz panels report 59
    }
    return best;
}

} // namespace noble
