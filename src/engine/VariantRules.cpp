#include "engine/VariantRules.hpp"

#include "common/Logger.hpp"
#include "vfs/VFS.hpp"

#include <nlohmann/json.hpp>
#include <windows.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>

namespace noble {
namespace VariantRules {

namespace {

constexpr int kNameOffset = 0x8C;   // UTF-16, 32 characters
constexpr int kNameChars = 32;

bool SafeDecode(HMODULE module, const void* file, void* out, void* status) {
    __try {
        using Fn = char (*)(const void*, void*, void*);
        return reinterpret_cast<Fn>(reinterpret_cast<uintptr_t>(module) + 0x373A0)(file, out, status) != 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

} // namespace

const std::vector<Rule>& Rules() {
    // Weapon numbers checked one by one in a real match. An empty name is a number nobody can pick.
    static const std::vector<std::string> s_weapons = {
        "Default", "", "None", "DMR", "Assault Rifle", "Plasma Pistol", "Spiker", "Energy Sword", "Magnum", "Needler", "Plasma Rifle",
        "Rocket Launcher", "Shotgun", "Sniper Rifle", "Spartan Laser", "Gravity Hammer", "Plasma Repeater", "Needle Rifle", "Focus Rifle",
        "Plasma Launcher", "Concussion Rifle", "Grenade Launcher", "Golf Club", "Fuel Rod Gun",
    };
    static const std::vector<std::string> s_shields = { "Normal", "None" };
    static const std::vector<Rule> s_rules = {
        { "teams", "Teams", "General Settings", "Players fight in teams instead of every player for themselves.", 0x2BC, 0, 0, 1, "", nullptr },
        { "time_limit", "Time Limit", "General Settings", "Minutes per round. 0 means no limit.", 0x2BE, -1, 0, 60, " min", nullptr },
        { "rounds", "Number of Rounds", "General Settings", "How many rounds the match plays.", 0x2BF, -1, 1, 15, "", nullptr },
        { "sudden_death", "Sudden Death", "General Settings", "Extra seconds when an objective is in play as time runs out. 0 turns it off.", 0x2C0, -1, 0, 120, " s", nullptr },
        { "respawn_time", "Respawn Time", "General Settings", "Seconds before a player comes back.", 0x2C7, -1, 0, 60, " s", nullptr },
        { "suicide_penalty", "Suicide Penalty", "General Settings", "Extra seconds after killing yourself.", 0x2C8, -1, 0, 60, " s", nullptr },
        { "betrayal_penalty", "Betrayal Penalty", "General Settings", "Extra seconds after killing a teammate.", 0x2C9, -1, 0, 60, " s", nullptr },
        { "primary_weapon", "Primary Weapon", "Player Traits", "What everyone spawns holding.", 0x319, -1, -3, 20, "", &s_weapons },
        { "secondary_weapon", "Secondary Weapon", "Player Traits", "The second weapon everyone spawns with.", 0x31A, -1, -3, 20, "", &s_weapons },
        { "shields", "Shields", "Player Traits", "None removes everyone's energy shield.", 0x307, -1, 0, 1, "", &s_shields },
    };
    return s_rules;
}

std::filesystem::path Directory() { return VFS::GetUserStoragePath() / "game_modes"; }

bool IsPresetFile(const std::filesystem::path& file) { return _wcsicmp(file.extension().c_str(), L".json") == 0; }

bool Load(const std::filesystem::path& file, Preset& out) {
    std::ifstream in(file, std::ios::binary);
    if (!in) return false;
    try {
        const nlohmann::json j = nlohmann::json::parse(in);
        Preset preset;
        preset.name = j.value("name", std::string());
        preset.base = j.value("base", std::string());
        // Only a bare file name: a preset can't reach outside game_variants.
        preset.base = std::filesystem::path(preset.base).filename().string();
        if (preset.base.empty()) return false;
        if (j.contains("rules") && j["rules"].is_object()) {
            for (const Rule& rule : Rules()) {
                const auto it = j["rules"].find(rule.key);
                if (it == j["rules"].end() || !it->is_number_integer()) continue;
                preset.values[rule.key] = std::clamp(it->get<int>(), rule.minValue, rule.maxValue);
            }
        }
        out = std::move(preset);
        return true;
    } catch (const std::exception& e) {
        NOBLE_WARN("VariantRules: '{}' is not a valid game mode ({})", file.string(), e.what());
        return false;
    }
}

bool Save(const std::filesystem::path& file, const Preset& preset) {
    std::error_code ec;
    std::filesystem::create_directories(file.parent_path(), ec);
    nlohmann::json j;
    j["name"] = preset.name;
    j["base"] = preset.base;
    j["rules"] = nlohmann::json::object();
    for (const auto& [key, value] : preset.values) j["rules"][key] = value;
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << j.dump(2);
    return static_cast<bool>(out);
}

int Read(const Rule& rule, const uint8_t* decoded) {
    const uint8_t byte = decoded[rule.offset];
    if (rule.bit >= 0) return (byte >> rule.bit) & 1;
    return rule.minValue < 0 ? static_cast<int8_t>(byte) : byte;
}

void Write(const Rule& rule, uint8_t* decoded, int value) {
    value = std::clamp(value, rule.minValue, rule.maxValue);
    uint8_t& byte = decoded[rule.offset];
    if (rule.bit >= 0) byte = static_cast<uint8_t>(value ? byte | (1u << rule.bit) : byte & ~(1u << rule.bit));
    else byte = static_cast<uint8_t>(value);
}

void Apply(const Preset& preset, uint8_t* decoded) {
    for (const Rule& rule : Rules()) {
        const auto it = preset.values.find(rule.key);
        if (it != preset.values.end()) Write(rule, decoded, it->second);
    }
    if (!preset.name.empty()) {
        wchar_t wide[kNameChars] = {};
        MultiByteToWideChar(CP_UTF8, 0, preset.name.c_str(), -1, wide, kNameChars - 1);
        std::memcpy(decoded + kNameOffset, wide, sizeof(wide));
    }
}

bool Decode(const std::filesystem::path& file, std::vector<uint8_t>& decoded) {
    const HMODULE module = GetModuleHandleW(L"haloreach.dll");
    if (!module) return false;
    std::ifstream in(file, std::ios::binary);
    if (!in) return false;
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (bytes.size() < 0x5329) return false;
    decoded.assign(kDecodedSize, 0);
    alignas(16) uint8_t status[256] = {};
    return SafeDecode(module, bytes.data(), decoded.data(), status);
}

} // namespace VariantRules
} // namespace noble