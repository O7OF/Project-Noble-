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
    // Offsets are from @deepseek's patch, decode and diff map (docs/plans/variant-field-map.md). That map's trait block at
    // 0x2D0 is the respawn traits; the base traits a player spawns with are the same layout at 0x304, proven in a match.
    // Choices and their order are the game's own, from HREK's multiplayer\game_variant_settings\player_traits_template
    // tags. Index 0 is "unchanged", which is the game's normal value, so percent traits show it as 100%. `order` is what a
    // player steps through: lowest to highest, capped at 300%, with the game's separate explicit 100% left out.
    using Names = std::vector<std::string>;
    using Order = std::vector<int>;
    static const Names s_weapons = {
        "Default", "", "None", "DMR", "Assault Rifle", "Plasma Pistol", "Spiker", "Energy Sword", "Magnum", "Needler", "Plasma Rifle",
        "Rocket Launcher", "Shotgun", "Sniper Rifle", "Spartan Laser", "Gravity Hammer", "Plasma Repeater", "Needle Rifle", "Focus Rifle",
        "Plasma Launcher", "Concussion Rifle", "Grenade Launcher", "Golf Club", "Fuel Rod Gun",
    };
    static const Names s_resist = { "100%", "10%", "50%", "90%", "100%", "110%", "150%", "200%", "300%", "500%", "1000%", "2000%", "Invulnerable" };
    static const Order s_resistOrder = { 1, 2, 3, 0, 5, 6, 7, 8 };
    static const Names s_shieldSize = { "100%", "0% (no shields)", "100%", "150%", "200%", "300%", "400%" };
    static const Order s_shieldSizeOrder = { 1, 0, 3, 4, 5 };
    static const Names s_healthRate = { "100%", "0%", "50%", "90%", "100%", "110%", "200%" };
    static const Order s_healthRateOrder = { 1, 2, 3, 0, 5, 6 };
    static const Names s_shieldRate = { "100%", "0%", "10%", "25%", "50%", "75%", "90%", "100%", "110%", "125%", "150%", "200%" };
    static const Order s_shieldRateOrder = { 1, 2, 3, 4, 5, 6, 0, 8, 9, 10, 11 };
    static const Names s_damage = { "100%", "0%", "25%", "50%", "75%", "90%", "100%", "110%", "125%", "150%", "200%", "300%" };
    static const Names s_speed = { "100%", "0%", "25%", "50%", "75%", "90%", "100%", "110%", "120%", "150%", "200%", "300%" };
    static const Order s_stepOrder = { 1, 2, 3, 4, 5, 0, 7, 8, 9, 10, 11 };
    static const Names s_gravity = { "100%", "50%", "75%", "100%", "150%", "200%" };
    static const Order s_gravityOrder = { 1, 2, 0, 4, 5 };
    static const Names s_offOn = { "Default", "Off", "On" };
    static const Names s_ammo = { "Default", "Off", "On", "Bottomless" };
    static const Names s_pickup = { "Default", "On", "Off" };
    static const Names s_usage = { "Default", "Off", "Normal", "On", "Unlimited" };
    static const Names s_grenades = { "Default", "Map Default", "None", "1 Frag", "2 Frag", "3 Frag", "4 Frag", "1 Plasma", "2 Plasma", "3 Plasma", "4 Plasma",
                                      "1 of Each", "2 of Each", "3 of Each", "4 of Each" };
    static const Names s_camo = { "Default", "Off", "Poor", "Good" };
    static const Names s_who = { "Default", "None", "Allies", "All" };
    static const Names s_tracker = { "Default", "Off", "Friends", "Normal", "All" };
    static const Names s_range = { "Default", "10 m", "25 m", "75 m", "150 m" };
    static const Names s_abilities = { "Default", "", "None", "Ability 0", "Ability 1", "Ability 2", "Ability 3", "Ability 4", "Ability 5", "Ability 6", "Ability 7" };
    constexpr const char* kGeneral = "General Settings";
    constexpr const char* kShields = "Shields and Health";
    constexpr const char* kWeapons = "Weapons and Damage";
    constexpr const char* kMove = "Movement";
    constexpr const char* kLook = "Appearance";
    constexpr const char* kSense = "Sensors";
    static const std::vector<Rule> s_rules = {
        { "teams", "Teams", kGeneral, "Players fight in teams instead of every player for themselves.", 0x2BC, 0, 0, 1, "", nullptr },
        { "score_to_win", "Score to Win", kGeneral, "Points needed to win. Each game type counts them its own way.", 0xF840, -1, 0, 1000, "", nullptr, 2 },
        { "time_limit", "Time Limit", kGeneral, "Minutes per round. 0 means no limit.", 0x2BE, -1, 0, 60, " min", nullptr },
        { "rounds", "Number of Rounds", kGeneral, "How many rounds the match plays.", 0x2BF, -1, 1, 15, "", nullptr },
        { "sudden_death", "Sudden Death", kGeneral, "Extra seconds when an objective is in play as time runs out. 0 turns it off.", 0x2C0, -1, 0, 120, " s", nullptr },
        { "respawn_time", "Respawn Time", kGeneral, "Seconds before a player comes back.", 0x2C7, -1, 0, 60, " s", nullptr },
        { "suicide_penalty", "Suicide Penalty", kGeneral, "Extra seconds after killing yourself.", 0x2C8, -1, 0, 60, " s", nullptr },
        { "betrayal_penalty", "Betrayal Penalty", kGeneral, "Extra seconds after killing a teammate.", 0x2C9, -1, 0, 60, " s", nullptr },
        { "lives_per_round", "Lives Per Round", kGeneral, "0 means unlimited.", 0x2C5, -1, 0, 63, "", nullptr },
        { "team_lives", "Team Lives Per Round", kGeneral, "0 means unlimited.", 0x2C6, -1, 0, 63, "", nullptr },
        { "respawn_growth", "Respawn Time Growth", kGeneral, "Seconds added to each respawn.", 0x2CA, -1, 0, 15, " s", nullptr },
        { "traits_duration", "Respawn Traits Duration", kGeneral, "Seconds the respawn traits last.", 0x2CC, -1, 0, 60, " s", nullptr },
        { "damage_resist", "Damage Resistance", kShields, "How much punishment players take. 100% is the game's normal.", 0x304, -1, 0, 12, "", &s_resist, 1, &s_resistOrder },
        { "health_rate", "Health Recharge Rate", kShields, "How quickly health returns. 100% is the game's normal.", 0x306, -1, 0, 6, "", &s_healthRate, 1, &s_healthRateOrder },
        { "shield_mult", "Shield Multiplier", kShields, "Size of the shield. 100% is the game's normal; 0% removes it.", 0x307, -1, 0, 6, "", &s_shieldSize, 1, &s_shieldSizeOrder },
        { "shield_rate", "Shield Recharge Rate", kShields, "How quickly shields return. 100% is the game's normal.", 0x308, -1, 0, 11, "", &s_shieldRate, 1, &s_shieldRateOrder },
        { "headshot_immune", "Immune to Headshots", kShields, "Headshots do no extra damage.", 0x30A, -1, 0, 2, "", &s_offOn },
        { "assassin_immune", "Immune to Assassination", kShields, "Players can't be assassinated.", 0x30C, -1, 0, 2, "", &s_offOn },
        { "weapon_damage", "Damage Modifier", kWeapons, "Damage weapons deal. 100% is the game's normal.", 0x310, -1, 0, 11, "", &s_damage, 1, &s_stepOrder },
        { "melee_damage", "Melee Modifier", kWeapons, "Damage melee deals. 100% is the game's normal.", 0x311, -1, 0, 11, "", &s_damage, 1, &s_stepOrder },
        { "primary_weapon", "Primary Weapon", kWeapons, "What everyone spawns holding.", 0x319, -1, -3, 20, "", &s_weapons },
        { "secondary_weapon", "Secondary Weapon", kWeapons, "The second weapon everyone spawns with.", 0x31A, -1, -3, 20, "", &s_weapons },
        { "ability", "Starting Equipment", kWeapons, "The armor ability players spawn with. Which number is which isn't confirmed yet.", 0x31B, -1, -3, 7, "", &s_abilities },
        { "grenade_count", "Grenade Count", kWeapons, "Grenades at spawn.", 0x314, -1, 0, 14, "", &s_grenades },
        { "weapon_pickup", "Weapon Pickup", kWeapons, "Whether weapons can be picked up.", 0x313, -1, 0, 2, "", &s_pickup },
        { "infinite_ammo", "Infinite Ammo", kWeapons, "On keeps reserve ammo full. Bottomless also never reloads.", 0x315, -1, 0, 3, "", &s_ammo },
        { "ability_usage", "Equipment Usage", kWeapons, "Whether armor abilities can be used.", 0x318, -1, 0, 4, "", &s_usage },
        { "speed", "Player Speed", kMove, "How fast players move. 100% is the game's normal.", 0x31C, -1, 0, 11, "", &s_speed, 1, &s_stepOrder },
        { "gravity", "Player Gravity", kMove, "How strongly players fall. 100% is the game's normal.", 0x31D, -1, 0, 5, "", &s_gravity, 1, &s_gravityOrder },
        { "active_camo", "Active Camo", kLook, "How invisible players are.", 0x324, -1, 0, 3, "", &s_camo },
        { "waypoint", "Waypoint", kLook, "Who sees a player's marker.", 0x325, -1, 0, 3, "", &s_who },
        { "visible_name", "Visible Name", kLook, "Who sees a player's name.", 0x326, -1, 0, 3, "", &s_who },
        { "tracker_mode", "Motion Tracker Mode", kSense, "When the motion tracker works.", 0x32C, -1, 0, 4, "", &s_tracker },
        { "tracker_range", "Motion Tracker Range", kSense, "How far it sees.", 0x32D, -1, 0, 4, "", &s_range },
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
                const int value = std::clamp(it->get<int>(), rule.minValue, rule.maxValue);
                // Only the game's own choices count: a number with no name, or past the 300% cap, is dropped.
                if (rule.order && std::find(rule.order->begin(), rule.order->end(), value) == rule.order->end()) continue;
                if (rule.names && (value - rule.minValue >= static_cast<int>(rule.names->size()) || (*rule.names)[value - rule.minValue].empty())) continue;
                preset.values[rule.key] = value;
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
    if (rule.bytes == 2) return decoded[rule.offset] | (decoded[rule.offset + 1] << 8);
    const uint8_t byte = decoded[rule.offset];
    if (rule.bit >= 0) return (byte >> rule.bit) & 1;
    return rule.minValue < 0 ? static_cast<int8_t>(byte) : byte;
}

void Write(const Rule& rule, uint8_t* decoded, int value) {
    value = std::clamp(value, rule.minValue, rule.maxValue);
    if (rule.bytes == 2) {
        decoded[rule.offset] = static_cast<uint8_t>(value & 0xFF);
        decoded[rule.offset + 1] = static_cast<uint8_t>(value >> 8);
        return;
    }
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