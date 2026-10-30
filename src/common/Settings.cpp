#include "Settings.hpp"
#include "Logger.hpp"

#include <windows.h>
#include <shellapi.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>

namespace noble {

namespace {

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
    d.fieldOfView = std::clamp(d.fieldOfView, 70.0f, 120.0f);  // slider range (SETTINGS > VIDEO)
    constexpr int kFpsLimits[] = { 0, 30, 60, 120, 144, 165, 200 };
    if (std::find(std::begin(kFpsLimits), std::end(kFpsLimits), d.fpsLimit) == std::end(kFpsLimits)) {
        d.fpsLimit = 0;
    }
    d.mouseSensitivity = std::clamp(d.mouseSensitivity, 0.1f, 10.0f);
    d.mouseSensitivityY = std::clamp(d.mouseSensitivityY, 0.25f, 4.0f);
    d.volumeGame = std::clamp(d.volumeGame, 0.0f, 1.0f);
    d.volumeMusic = std::clamp(d.volumeMusic, 0.0f, 1.0f);
    d.volumeOther = std::clamp(d.volumeOther, 0.0f, 1.0f);
    if (d.playerName.empty()) d.playerName = "NobleSix";
    if (d.playerName.size() > 15) d.playerName.resize(15);  // Reach gamertags are at most 15 characters
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
    if (d.favorites.size() > 50) d.favorites.resize(50);
    d.wheelUpAction = std::clamp(d.wheelUpAction, -1, 65);
    d.wheelDownAction = std::clamp(d.wheelDownAction, -1, 65);
    d.padVibrationStrength = std::clamp(d.padVibrationStrength, 0.0f, 1.0f);
    d.padLookX = std::clamp(d.padLookX, 1, 19);
    d.padLookY = std::clamp(d.padLookY, 1, 19);
    d.padLeftDeadZone = std::clamp(d.padLeftDeadZone, 0.0f, 0.5f);
    d.padRightDeadZone = std::clamp(d.padRightDeadZone, 0.0f, 0.5f);
    for (auto it = d.keyOverrides.begin(); it != d.keyOverrides.end();) {
        if (it->first < 0 || it->first >= 66 || it->second <= 0 || it->second >= 256) it = d.keyOverrides.erase(it);
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
    SettingsData d;
    const auto path = FilePath();
    std::ifstream in(path);
    const bool existed = in.is_open();
    if (in) {
        try {
            const nlohmann::json j = nlohmann::json::parse(in);
            Read(j, "width", d.width);
            Read(j, "height", d.height);
            Read(j, "borderless", d.borderless);
            Read(j, "fieldOfView", d.fieldOfView);
            Read(j, "verticalSync", d.verticalSync);
            Read(j, "fpsLimit", d.fpsLimit);
            Read(j, "showFps", d.showFps);
            Read(j, "mouseSensitivity", d.mouseSensitivity);
            Read(j, "mouseSensitivityY", d.mouseSensitivityY);
            Read(j, "invertLook", d.invertLook);
            Read(j, "wheelUpAction", d.wheelUpAction);
            Read(j, "wheelDownAction", d.wheelDownAction);
            Read(j, "padVibration", d.padVibration);
            Read(j, "padVibrationStrength", d.padVibrationStrength);
            Read(j, "padLookX", d.padLookX);
            Read(j, "padLookY", d.padLookY);
            Read(j, "padInvertLook", d.padInvertLook);
            Read(j, "padLeftDeadZone", d.padLeftDeadZone);
            Read(j, "padRightDeadZone", d.padRightDeadZone);
            Read(j, "volumeGame", d.volumeGame);
            Read(j, "volumeMusic", d.volumeMusic);
            Read(j, "volumeOther", d.volumeOther);
            Read(j, "menuSounds", d.menuSounds);
    Read(j, "inviteNotifications", d.inviteNotifications);
            Read(j, "menuMusicTrack", d.menuMusicTrack);
            Read(j, "subtitles", d.subtitles);
            Read(j, "campaignSkulls", d.campaignSkulls);
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
            if (d.masterServerUrl.empty()) {
                d.masterServerUrl = "https://outbreak-z-test-server.onrender.com";
            }
            Read(j, "favorites", d.favorites);
            Read(j, "accountUsername", d.accountUsername);
            Read(j, "accountToken", d.accountToken);
            Read(j, "isDeveloper", d.isDeveloper);
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
                }
            }
            LocalFree(argv);
        }
    }

    Clamp(d);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data = d;
    m_loaded = true;
    if (!existed) {
        Save();
    }
}

bool Settings::Save() {
    nlohmann::json j;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const SettingsData& d = m_data;
        j["width"] = d.width;
        j["height"] = d.height;
        j["borderless"] = d.borderless;
        j["fieldOfView"] = d.fieldOfView;
        j["verticalSync"] = d.verticalSync;
        j["fpsLimit"] = d.fpsLimit;
        j["showFps"] = d.showFps;
        j["mouseSensitivity"] = d.mouseSensitivity;
        j["mouseSensitivityY"] = d.mouseSensitivityY;
        j["invertLook"] = d.invertLook;
        j["wheelUpAction"] = d.wheelUpAction;
        j["wheelDownAction"] = d.wheelDownAction;
        j["padVibration"] = d.padVibration;
        j["padVibrationStrength"] = d.padVibrationStrength;
        j["padLookX"] = d.padLookX;
        j["padLookY"] = d.padLookY;
        j["padInvertLook"] = d.padInvertLook;
        j["padLeftDeadZone"] = d.padLeftDeadZone;
        j["padRightDeadZone"] = d.padRightDeadZone;
        j["volumeGame"] = d.volumeGame;
        j["volumeMusic"] = d.volumeMusic;
        j["volumeOther"] = d.volumeOther;
        j["menuSounds"] = d.menuSounds;
    j["inviteNotifications"] = d.inviteNotifications;
        j["menuMusicTrack"] = d.menuMusicTrack;
        j["subtitles"] = d.subtitles;
        j["campaignSkulls"] = d.campaignSkulls;
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
        j["isDeveloper"] = d.isDeveloper;
        j["accountUserId"] = d.accountUserId;
        nlohmann::json keys = nlohmann::json::object();
        for (const auto& [action, vk] : d.keyOverrides) keys[std::to_string(action)] = vk;
        j["keyOverrides"] = keys;
    }
    const auto path = FilePath();
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    const auto tmp = path.string() + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) {
            NOBLE_WARN("Settings: could not write {}", tmp);
            return false;
        }
        out << j.dump(2);
    }
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        NOBLE_WARN("Settings: could not replace {}: {}", path.string(), ec.message());
        return false;
    }
    return true;
}

SettingsData Settings::Snapshot() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_data;
}

void Settings::Update(const SettingsData& data) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_data = data;
        Clamp(m_data);
    }
    Save();
}

} // namespace noble
