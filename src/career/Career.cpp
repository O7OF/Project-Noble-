#include "Career.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <windows.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <map>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <mutex>
#include <random>

namespace noble {
namespace Career {

namespace {

// ---------------------------------------------------------------------------------------------
// MCC game-results record (0x5D138 bytes), as built by haloreach+0x39128 from the engine's own
// results at haloreach+0x28B83B0. Offsets confirmed from that function; the meaning of individual
// general-stat slots is still provisional (see kStat* below) until checked against real matches.
//
//   +0x0000 u32  game category (engine mode byte mapped to MCC's list)
//   +0x0020      16 player records, 0x1C8 bytes each
//                  +0x00 u8 present, +0x08 u64 player id, +0x18 i32, +0x28/+0x2C i32, +0x30 u8,
//                  +0x34 i32 (score; read from a different engine field in mode 2), ...
//   +0x1CA0      16 x 4 bytes team data
//   +0x1CE0      16 per-player stat blocks, 0x5448 bytes each:
//                  +0x000  23 x u32 general stats (MCC numbering)
//                  +0x05C  11 x u32
//                  +0x0AC  0x180 x u32 medal counts (MCC medal numbering)
//                  +0x6AC  0x32 x u32
//                  ...     weapon and damage tables
//   +0x56160     per-player pairs, +0x5D120..+0x5D130 match totals
// ---------------------------------------------------------------------------------------------
constexpr size_t kPlayerRecords = 0x20;
constexpr size_t kPlayerRecordSize = 0x1C8;
constexpr size_t kStatBlocks = 0x1CE0;
constexpr size_t kStatBlockSize = 0x5448;
constexpr size_t kGeneralStats = 23;
constexpr size_t kMedalOffset = 0xAC;
constexpr size_t kMedalCount = 0x180;
constexpr int kMaxPlayers = 16;

// General-stat slots, assuming Reach keeps Halo 3's statistic order (games played, completed, won,
// tied, rounds completed, rounds won, round score, total score, kills, assists, deaths, ...) and
// MCC's remap (haloreach+0x3B208: engine 0->0, 1->1, n->n+1 for n >= 2). PROVISIONAL.
constexpr int kStatScore = 8;
constexpr int kStatKills = 9;
constexpr int kStatAssists = 10;
constexpr int kStatDeaths = 11;

// ---------------------------------------------------------------------------------------------
// Pay. Noble's own numbers (Bungie never published theirs); tune freely.
// ---------------------------------------------------------------------------------------------
constexpr double kPayPerMinuteBase = 13.0;      // Recruit
constexpr double kPayPerMinutePerRank = 6.0;    // + this per rank above Recruit
constexpr double kMinuteCap = 90.0;             // minutes paid at most per match, every base type
constexpr int64_t kPerKill = 10;
constexpr int64_t kPerAssist = 5;
constexpr int64_t kPerMedal = 25;
constexpr int kRandomBonusOneIn = 8;            // chance of a random bonus
constexpr int kJackpotOneIn = 200;              // chance a random bonus is the jackpot
constexpr int64_t kRandomBonusMin = 100;
constexpr int64_t kRandomBonusMax = 3000;
constexpr int64_t kJackpot = 7777;

const std::vector<Rank> kRanks = {
    { "Recruit", 0 },                         { "Private", 7500 },
    { "Corporal", 10000 },                    { "Corporal Grade 1", 15000 },
    { "Sergeant", 20000 },                    { "Sergeant Grade 1", 26250 },
    { "Sergeant Grade 2", 32500 },            { "Warrant Officer", 45000 },
    { "Warrant Officer Grade 1", 78000 },     { "Warrant Officer Grade 2", 111000 },
    { "Warrant Officer Grade 3", 144000 },    { "Captain", 210000 },
    { "Captain Grade 1", 233000 },            { "Captain Grade 2", 256000 },
    { "Captain Grade 3", 279000 },            { "Major", 325000 },
    { "Major Grade 1", 350000 },              { "Major Grade 2", 375000 },
    { "Major Grade 3", 400000 },              { "Lt. Colonel", 450000 },
    { "Lt. Colonel Grade 1", 480000 },        { "Lt. Colonel Grade 2", 510000 },
    { "Lt. Colonel Grade 3", 540000 },        { "Commander", 600000 },
    { "Commander Grade 1", 650000 },          { "Commander Grade 2", 700000 },
    { "Commander Grade 3", 750000 },          { "Colonel", 850000 },
    { "Colonel Grade 1", 960000 },            { "Colonel Grade 2", 1070000 },
    { "Colonel Grade 3", 1180000 },           { "Brigadier", 1400000 },
    { "Brigadier Grade 1", 1520000 },         { "Brigadier Grade 2", 1640000 },
    { "Brigadier Grade 3", 1760000 },         { "General", 2000000 },
    { "General Grade 1", 2200000 },           { "General Grade 2", 2350000 },
    { "General Grade 3", 2500000 },           { "General Grade 4", 2650000 },
    { "Field Marshall", 3000000 },            { "Hero", 3700000 },
    { "Legend", 4600000 },                    { "Mythic", 5650000 },
    { "Noble", 7000000 },                     { "Eclipse", 8500000 },
    { "Nova", 11000000 },                     { "Forerunner", 13000000 },
    { "Reclaimer", 16500000 },                { "Inheritor", 20000000 },
};

// Campaign: bonus for finishing a mission, by difficulty (Easy, Normal, Heroic, Legendary), plus
// 10% of it per skull that was on (up to +100%).
constexpr int64_t kMissionBonus[4] = { 250, 500, 1000, 2000 };

// ---------------------------------------------------------------------------------------------
// Challenges and commendations. Reach's own lists came from Bungie's servers; these are Noble's,
// built only on what the results record and the launch settings tell us.
// ---------------------------------------------------------------------------------------------
// Halo: Reach's own challenges (docs/career/challenges.json, 2010-2012), the ones Noble can track from
// a finished game (see tools/career/gen_reach_challenges.py, which writes ReachChallenges.inc).
enum class ChallengeMetric { Kills, Assists, Games, Missions, Score, DailyChallenges, AllDailies };
struct ReachChallengeDef {
    bool weekly;
    int mode;                 // 0 campaign, 1 Firefight, 2 multiplayer, 3 any
    const char* title;
    const char* description;
    int64_t reward;
    ChallengeMetric metric;
    int64_t target;
    bool perGame;             // best single game instead of a running total
    int minDifficulty;        // -1 = any
    bool exactDifficulty;     // "on Heroic" (exact) vs "on Heroic or harder"
    uint64_t skulls;          // MCC skull bits that must all be on
    const char* mission;      // scenario ("m10"), empty = any
    bool noDeaths;
    int maxMinutes;           // 0 = no time limit
};
const ReachChallengeDef kReachChallenges[] = {
#include "ReachChallenges.inc"
};
constexpr int kDailyCount = 7;
constexpr int kWeeklyCount = 5;

// Halo: Reach's 45 commendations (16 campaign, 15 Firefight, 14 multiplayer) with Reach's tier counts
// (Steel, Bronze, Silver, Gold, Onyx, Max). Noble can only count some of them from what it reads at
// the end of a game; the others stay at 0 (tracked = false) until headshots, weapon types, enemy
// classes, multikills, sprees, medals and vehicle kills are decoded from the results record.
enum class CommendationMetric { None, Assists, Missions, FlawlessMissions, NoDeathGames };
struct CommendationDef {
    int mode;                 // 0 campaign, 1 Firefight, 2 multiplayer
    const char* key;          // art: data\ui\reach\commendations\<mode>_<key>.png
    const char* title;
    const char* description;
    CommendationMetric metric;
    int64_t targets[6];       // Steel, Bronze, Silver, Gold, Onyx, Max
    int64_t rewards[6];       // Reach's cR per tier (Max paid nothing); Noble pays TierReward()
};
using CM = CommendationMetric;
// Requirements, tier counts and cR are Reach's (Halopedia). Heavy Weapon's cR was unknown there, so it uses the averages.
const CommendationDef kCommendations[] = {
    { 0, "cannon_fodder", "Cannon Fodder", "Kill any infantry-class enemy in Campaign", CM::None, { 250, 750, 3000, 7000, 12000, 20000 }, { 200, 600, 1200, 2400, 2400, 0 } },
    { 0, "demon", "Demon", "Complete a Campaign mission (Noble Actual doesn't count)", CM::Missions, { 9, 18, 81, 135, 270, 540 }, { 400, 400, 400, 400, 400, 0 } },
    { 0, "flawless_cowboy", "Flawless Cowboy", "Complete a Campaign mission without dying (Noble Actual doesn't count)", CM::FlawlessMissions, { 5, 10, 25, 50, 150, 300 }, { 450, 450, 900, 1500, 2000, 0 } },
    { 0, "leadership_element", "Leadership Element", "Kill a leader-class enemy in Campaign", CM::None, { 100, 375, 750, 3000, 6000, 10000 }, { 400, 750, 1250, 2500, 2500, 0 } },
    { 0, "nice_arm", "Nice Arm", "Kill an enemy with a Frag or Plasma Grenade in Campaign", CM::None, { 50, 250, 500, 4000, 8000, 13000 }, { 200, 750, 1000, 2000, 2000, 0 } },
    { 0, "pinpoint", "Pinpoint", "Kill an enemy with a headshot in Campaign", CM::None, { 50, 250, 500, 4000, 8000, 20000 }, { 100, 750, 1000, 2000, 2000, 0 } },
    { 0, "precisely", "Precisely", "Kill an enemy with a precision weapon in Campaign (DMR, Needle Rifle, Focus Rifle, Sniper Rifle)", CM::None, { 250, 750, 4000, 10000, 20000, 32000 }, { 500, 750, 1000, 3000, 3000, 0 } },
    { 0, "right_of_way", "Right of Way", "Kill an enemy vehicle in Campaign", CM::None, { 50, 200, 1000, 2400, 4800, 7200 }, { 250, 400, 800, 1600, 1600, 0 } },
    { 0, "small_arms", "Small Arms", "Kill an enemy with a pistol in Campaign (Magnum, Plasma Pistol)", CM::None, { 250, 750, 3000, 6000, 10000, 16000 }, { 250, 375, 750, 1250, 1250, 0 } },
    { 0, "specops", "SpecOps", "Kill a specialist-class enemy in Campaign", CM::None, { 175, 525, 2100, 6000, 10000, 14000 }, { 200, 600, 1200, 2400, 2400, 0 } },
    { 0, "splash_damage", "Splash Damage", "Kill an enemy with a heavy weapon in Campaign (Rocket Launcher, Grenade Launcher, Plasma Launcher, Spartan Laser, Concussion Rifle, Fuel Rod Gun, Target Locator)", CM::None, { 100, 375, 750, 1500, 3000, 6000 }, { 200, 750, 1500, 2000, 2000, 0 } },
    { 0, "standard_issue", "Standard Issue", "Kill an enemy with an automatic weapon in Campaign (Needler, Assault Rifle, Plasma Rifle, Plasma Repeater, Spiker)", CM::None, { 250, 750, 3000, 6000, 10000, 15000 }, { 500, 750, 1000, 3000, 3000, 0 } },
    { 0, "super_soldier", "Super Soldier", "Earn a spree of any type in Campaign", CM::None, { 25, 175, 500, 2000, 5000, 9000 }, { 200, 700, 1500, 2000, 2000, 0 } },
    { 0, "support_role", "Support Role", "Earn an assist in Campaign", CM::Assists, { 100, 375, 750, 1500, 3000, 6000 }, { 200, 500, 1000, 1500, 1500, 0 } },
    { 0, "walking_tank", "Walking Tank", "Earn a multikill in Campaign", CM::None, { 25, 175, 500, 2000, 5000, 9000 }, { 200, 700, 1500, 2000, 2000, 0 } },
    { 0, "war_machine", "War Machine", "Kill an enemy with a vehicle in Campaign", CM::None, { 200, 500, 800, 3200, 6400, 14000 }, { 250, 700, 1400, 2000, 2000, 0 } },
    { 1, "backup", "Backup", "Earn an assist in Firefight", CM::Assists, { 50, 250, 750, 1500, 3000, 8000 }, { 300, 750, 1500, 2400, 2400, 0 } },
    { 1, "dome_inspector", "Dome Inspector", "Kill an enemy with a headshot in Firefight", CM::None, { 50, 250, 500, 4000, 8000, 20000 }, { 150, 425, 1000, 2400, 2400, 0 } },
    { 1, "get_loud", "Get Loud", "Kill an enemy with a heavy weapon in Firefight (Rocket Launcher, Grenade Launcher, Plasma Launcher, Spartan Laser, Concussion Rifle, Fuel Rod Gun, Target Locator)", CM::None, { 100, 375, 750, 2000, 4000, 8000 }, { 200, 750, 1500, 2000, 2000, 0 } },
    { 1, "grounded", "Grounded", "Kill an enemy vehicle in Firefight", CM::None, { 50, 200, 400, 2500, 6000, 12000 }, { 250, 400, 800, 1600, 1600, 0 } },
    { 1, "in_command", "In Command", "Kill a leader-class enemy in Firefight (Elites)", CM::None, { 75, 375, 750, 3000, 6000, 10000 }, { 250, 400, 800, 1600, 1600, 0 } },
    { 1, "longshot", "Longshot", "Kill an enemy with a precision weapon in Firefight (DMR, Needle Rifle, Focus Rifle, Sniper Rifle)", CM::None, { 250, 750, 4000, 10000, 20000, 32000 }, { 500, 750, 1000, 3000, 3000, 0 } },
    { 1, "methodical", "Methodical", "Earn a spree of any type in Firefight", CM::None, { 25, 175, 500, 2500, 5000, 10000 }, { 200, 700, 1500, 2000, 2000, 0 } },
    { 1, "numbers_game", "Numbers Game", "Earn any multikill in Firefight", CM::None, { 25, 175, 500, 2500, 5000, 15000 }, { 200, 700, 1500, 2000, 2000, 0 } },
    { 1, "perfectionist", "Perfectionist", "Complete a round without dying in Firefight (counted per game for now)", CM::NoDeathGames, { 50, 250, 500, 2000, 5000, 8000 }, { 250, 1250, 2000, 2000, 2000, 0 } },
    { 1, "pull_the_pin", "Pull the Pin", "Kill an enemy with a grenade in Firefight", CM::None, { 50, 250, 500, 3000, 5000, 9000 }, { 200, 750, 1000, 2000, 2000, 0 } },
    { 1, "riflin_through", "Riflin' Through", "Kill an enemy with an automatic weapon in Firefight (Needler, Assault Rifle, Plasma Rifle, Plasma Repeater, Spiker)", CM::None, { 250, 750, 3000, 10000, 20000, 32000 }, { 500, 750, 1000, 2000, 2000, 0 } },
    { 1, "specialized", "Specialized", "Kill a specialist-class enemy in Firefight (Brutes, Jackals, Skirmishers, Spec-Ops Grunts)", CM::None, { 175, 525, 2000, 6000, 10000, 14000 }, { 200, 600, 1200, 2400, 2400, 0 } },
    { 1, "target_practice", "Target Practice", "Kill an infantry-class enemy in Firefight (Grunts)", CM::None, { 250, 750, 3000, 7000, 12000, 20000 }, { 200, 600, 2000, 2400, 2400, 0 } },
    { 1, "trigger_happy", "Trigger Happy", "Kill an enemy with a pistol in Firefight (Magnum, Plasma Pistol)", CM::None, { 250, 750, 3000, 6000, 10000, 16000 }, { 500, 750, 1500, 2000, 2000, 0 } },
    { 1, "vehicular", "Vehicular", "Kill an enemy with a vehicle or turret in Firefight", CM::None, { 200, 500, 800, 3200, 6400, 14000 }, { 250, 700, 1400, 2000, 2000, 0 } },
    { 2, "any_spree", "Any Spree", "Earn any type of spree in Multiplayer", CM::None, { 5, 50, 250, 1000, 4000, 10000 }, { 250, 425, 1200, 2400, 2400, 0 } },
    { 2, "assistant", "Assistant", "Earn an assist in Multiplayer", CM::Assists, { 50, 250, 1000, 4000, 8000, 20000 }, { 150, 425, 1200, 2400, 2400, 0 } },
    { 2, "close_quarters", "Close Quarters", "Kill an enemy in close quarters in Multiplayer (Grav Hammer, Energy Sword, Shotgun, Melee)", CM::None, { 50, 125, 400, 1600, 4000, 8000 }, { 150, 375, 1200, 2400, 2400, 0 } },
    { 2, "crack_shot", "Crack Shot", "Kill an enemy with a precision weapon in Multiplayer (DMR, Needle Rifle, Focus Rifle, Sniper Rifle)", CM::None, { 100, 500, 4000, 10000, 20000, 32000 }, { 300, 1000, 2000, 3000, 3000, 0 } },
    { 2, "downshift", "Downshift", "Drive a vehicle while your passenger or gunner kills an enemy in Multiplayer", CM::None, { 5, 50, 750, 4000, 8000, 20000 }, { 100, 250, 875, 1200, 1200, 0 } },
    { 2, "grenadier", "Grenadier", "Kill an enemy with a Frag or Plasma Grenade in Multiplayer", CM::None, { 25, 125, 500, 4000, 8000, 14000 }, { 100, 200, 500, 750, 750, 0 } },
    { 2, "heavy_weapon", "Heavy Weapon", "Kill an enemy with a heavy weapon in Multiplayer (Rocket Launcher, Grenade Launcher, Plasma Launcher, Spartan Laser, Concussion Rifle, Fuel Rod Gun)", CM::None, { 25, 150, 750, 3000, 7000, 14000 }, { 200, 700, 1000, 2400, 2400, 0 } },
    { 2, "jack_of_all_trades", "Jack of All Trades", "Perform an incredible feat in Multiplayer (Hijacker, Skyjack, Revenge, Avenger, Yoink, Close Call, Killjoy, ... medals)", CM::None, { 50, 125, 400, 1600, 4800, 9600 }, { 150, 375, 1000, 1600, 1600, 0 } },
    { 2, "mobile_asset", "Mobile Asset", "Kill an enemy with a vehicle in Multiplayer", CM::None, { 25, 125, 500, 4000, 8000, 14000 }, { 100, 200, 500, 750, 750, 0 } },
    { 2, "multikill", "Multikill", "Earn any multikill in Multiplayer", CM::None, { 10, 125, 500, 2500, 5000, 15000 }, { 250, 450, 875, 1400, 1400, 0 } },
    { 2, "one_shot", "One Shot", "Kill an enemy with a headshot in Multiplayer", CM::None, { 50, 250, 1000, 4000, 8000, 20000 }, { 150, 425, 1200, 2400, 2400, 0 } },
    { 2, "rear_admiral", "Rear Admiral", "Kill an enemy with an assassination in Multiplayer", CM::None, { 5, 25, 100, 1000, 4000, 8000 }, { 300, 500, 750, 1000, 1000, 0 } },
    { 2, "sidearm", "Sidearm", "Kill an enemy with a pistol in Multiplayer (Pistol, Plasma Pistol)", CM::None, { 50, 250, 1000, 4000, 8000, 10000 }, { 100, 200, 500, 750, 750, 0 } },
    { 2, "trigger_man", "Trigger Man", "Kill an enemy with an automatic weapon in Multiplayer (Needler, Assault Rifle, Plasma Rifle, Plasma Repeater, Spiker)", CM::None, { 100, 500, 4000, 10000, 20000, 32000 }, { 300, 1000, 2000, 3000, 3000, 0 } },
};
const char* const kTierNames[7] = { "None", "Steel", "Bronze", "Silver", "Gold", "Onyx", "Max" };

// Lifetime data beyond the totals.
struct Extra {
    int64_t games[3] = {};
    double minutes[3] = {};
    int64_t bestMatch = 0;
    int64_t challengesCompleted = 0;
    std::map<std::string, int> campaignBest;          // scenario -> best difficulty
    std::vector<MatchSummary> history;                // newest first, at most 20
    std::map<std::string, int64_t> challengeProgress; // "<period>:<d|w><index>" -> progress
    std::map<std::string, bool> challengeClaimed;
    std::map<std::string, int> commendationTier;      // "<mode>_<key>" -> tiers paid
    std::map<std::string, int64_t> commendationCount; // "<mode>_<key>" -> count
};

std::mutex s_mutex;
bool s_loaded = false;
Totals s_totals;
Extra s_extra;
Award s_lastAward;

BaseType s_type = BaseType::Multiplayer;
std::string s_map;
int s_difficulty = 1;
uint64_t s_skulls = 0;
std::chrono::steady_clock::time_point s_matchStart = std::chrono::steady_clock::now();

std::filesystem::path UserDir() {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path() / "data" / "userdata";
}

std::string Today() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return buf;
}

void LoadLocked() {
    if (s_loaded) return;
    s_loaded = true;
    std::ifstream in(UserDir() / "career.json");
    if (!in) return;
    try {
        nlohmann::json j = nlohmann::json::parse(in);
        s_totals.credits = j.value("credits", int64_t{0});
        s_totals.creditsToday = j.value("creditsToday", int64_t{0});
        s_totals.day = j.value("day", std::string());
        s_totals.gamesPlayed = j.value("gamesPlayed", int64_t{0});
        s_totals.kills = j.value("kills", int64_t{0});
        s_totals.deaths = j.value("deaths", int64_t{0});
        s_totals.assists = j.value("assists", int64_t{0});
        s_totals.medals = j.value("medals", int64_t{0});
        s_totals.minutesPlayed = j.value("minutesPlayed", 0.0);
        if (j.contains("record") && j["record"].is_object()) {
            const nlohmann::json& r = j["record"];
            const char* names[3] = { "campaign", "firefight", "multiplayer" };
            for (int i = 0; i < 3; ++i) {
                if (r.contains("games")) s_extra.games[i] = r["games"].value(names[i], int64_t{0});
                if (r.contains("minutes")) s_extra.minutes[i] = r["minutes"].value(names[i], 0.0);
            }
            s_extra.bestMatch = r.value("bestMatchCredits", int64_t{0});
            s_extra.challengesCompleted = r.value("challengesCompleted", int64_t{0});
            if (r.contains("campaignBest")) for (auto& [k, v] : r["campaignBest"].items()) s_extra.campaignBest[k] = v.get<int>();
            if (r.contains("challengeProgress")) for (auto& [k, v] : r["challengeProgress"].items()) s_extra.challengeProgress[k] = v.get<int64_t>();
            if (r.contains("challengeClaimed")) for (auto& [k, v] : r["challengeClaimed"].items()) s_extra.challengeClaimed[k] = v.get<bool>();
            if (r.contains("commendations")) for (auto& [k, v] : r["commendations"].items()) s_extra.commendationTier[k] = v.get<int>();
            if (r.contains("commendationCounts")) for (auto& [k, v] : r["commendationCounts"].items()) s_extra.commendationCount[k] = v.get<int64_t>();
            if (r.contains("history") && r["history"].is_array()) {
                for (const auto& h : r["history"]) {
                    MatchSummary m;
                    m.date = h.value("date", std::string());
                    m.type = h.value("type", std::string());
                    m.map = h.value("map", std::string());
                    m.difficulty = h.value("difficulty", -1);
                    m.minutes = h.value("minutes", 0.0);
                    m.kills = h.value("kills", int64_t{0});
                    m.deaths = h.value("deaths", int64_t{0});
                    m.assists = h.value("assists", int64_t{0});
                    m.medals = h.value("medals", int64_t{0});
                    m.score = h.value("score", int64_t{0});
                    m.credits = h.value("credits", int64_t{0});
                    s_extra.history.push_back(m);
                }
            }
        }
    } catch (...) {
        NOBLE_WARN("Career: career.json could not be read, starting from zero");
    }
}

void SaveLocked() {
    nlohmann::json j;
    j["credits"] = s_totals.credits;
    j["creditsToday"] = s_totals.creditsToday;
    j["day"] = s_totals.day;
    j["gamesPlayed"] = s_totals.gamesPlayed;
    j["kills"] = s_totals.kills;
    j["deaths"] = s_totals.deaths;
    j["assists"] = s_totals.assists;
    j["medals"] = s_totals.medals;
    j["minutesPlayed"] = s_totals.minutesPlayed;
    {
        nlohmann::json r;
        const char* names[3] = { "campaign", "firefight", "multiplayer" };
        for (int i = 0; i < 3; ++i) {
            r["games"][names[i]] = s_extra.games[i];
            r["minutes"][names[i]] = s_extra.minutes[i];
        }
        r["bestMatchCredits"] = s_extra.bestMatch;
        r["challengesCompleted"] = s_extra.challengesCompleted;
        r["campaignBest"] = s_extra.campaignBest;
        // Only this week's challenge entries are worth keeping.
        r["challengeProgress"] = s_extra.challengeProgress;
        r["challengeClaimed"] = s_extra.challengeClaimed;
        r["commendations"] = s_extra.commendationTier;
        r["commendationCounts"] = s_extra.commendationCount;
        nlohmann::json h = nlohmann::json::array();
        for (const MatchSummary& m : s_extra.history) {
            h.push_back({ { "date", m.date }, { "type", m.type }, { "map", m.map }, { "difficulty", m.difficulty },
                          { "minutes", m.minutes }, { "kills", m.kills }, { "deaths", m.deaths }, { "assists", m.assists },
                          { "medals", m.medals }, { "score", m.score }, { "credits", m.credits } });
        }
        r["history"] = h;
        j["record"] = r;
    }
    std::error_code ec;
    std::filesystem::create_directories(UserDir(), ec);
    const auto path = UserDir() / "career.json";
    const auto tmp = UserDir() / "career.json.tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        out << j.dump(2);
    }
    std::filesystem::rename(tmp, path, ec);
    if (ec) NOBLE_WARN("Career: could not save career.json ({})", ec.message());
}

uint32_t U32(const uint8_t* p, size_t off) {
    uint32_t v;
    std::memcpy(&v, p + off, sizeof(v));
    return v;
}
uint64_t U64(const uint8_t* p, size_t off) {
    uint64_t v;
    std::memcpy(&v, p + off, sizeof(v));
    return v;
}

// ---------------------------------------------------------------- challenges

std::tm LocalNow() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    return tm;
}

// Reach's challenge day starts at 11:00 GMT (3 AM Pacific); the challenge week on Monday at 11:00 GMT.
int64_t ChallengeDay() {
    const int64_t now = static_cast<int64_t>(std::time(nullptr));
    return (now - 11 * 3600) / 86400;
}
int64_t ChallengeWeek() { return (ChallengeDay() - 4) / 7; }   // day 4 (1970-01-05) was a Monday
std::string DailyKey() { return "D" + std::to_string(ChallengeDay()); }
std::string WeeklyKey() { return "W" + std::to_string(ChallengeWeek()); }

struct ActiveChallenge {
    const ReachChallengeDef* def;
    std::string key;
    bool weekly;
};

uint64_t SplitMix(uint64_t& x) {
    uint64_t z = (x += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// A random pick from the whole pool, the same for every player and every start on that day / week
// (seeded with the day / week number). Dailies mix the modes like Reach did (campaign, Firefight,
// multiplayer and any-mode goals); no name appears twice in one set.
std::vector<int> PickChallenges(bool weekly, int64_t period, int count) {
    std::vector<int> pool;
    for (int i = 0; i < static_cast<int>(std::size(kReachChallenges)); ++i) {
        if (kReachChallenges[i].weekly == weekly) pool.push_back(i);
    }
    uint64_t seed = static_cast<uint64_t>(period) * 2654435761ull + (weekly ? 0x5EEDull : 0xDA11ull);
    for (int i = static_cast<int>(pool.size()) - 1; i > 0; --i) {
        const int j = static_cast<int>(SplitMix(seed) % static_cast<uint64_t>(i + 1));
        std::swap(pool[i], pool[j]);
    }
    // How many of each mode (campaign, Firefight, multiplayer, any) to take first.
    const int dailyMix[4] = { 2, 2, 2, 1 };
    const int weeklyMix[4] = { 2, 0, 2, 1 };
    const int* mix = weekly ? weeklyMix : dailyMix;
    std::vector<int> out;
    std::vector<std::string> names;
    auto take = [&](int idx) {
        if (static_cast<int>(out.size()) >= count) return;
        const std::string name = kReachChallenges[idx].title;
        if (std::find(names.begin(), names.end(), name) != names.end()) return;
        if (std::find(out.begin(), out.end(), idx) != out.end()) return;
        out.push_back(idx);
        names.push_back(name);
    };
    for (int mode = 0; mode < 4; ++mode) {
        int taken = 0;
        for (int idx : pool) {
            if (taken >= mix[mode]) break;
            if (kReachChallenges[idx].mode != mode) continue;
            const size_t before = out.size();
            take(idx);
            if (out.size() > before) ++taken;
        }
    }
    for (int idx : pool) take(idx);   // fill up when a mode ran short
    return out;
}

// Today's 7 daily challenges and this week's 5 weekly ones.
std::vector<ActiveChallenge> ActiveChallenges() {
    std::vector<ActiveChallenge> out;
    const std::string dayKey = DailyKey(), weekKey = WeeklyKey();
    for (int i : PickChallenges(false, ChallengeDay(), kDailyCount)) {
        out.push_back({ &kReachChallenges[i], dayKey + ":r" + std::to_string(i), false });
    }
    for (int i : PickChallenges(true, ChallengeWeek(), kWeeklyCount)) {
        out.push_back({ &kReachChallenges[i], weekKey + ":r" + std::to_string(i), true });
    }
    return out;
}

// What one finished match adds to a challenge (0 when the game doesn't count for it).
struct MatchFacts {
    BaseType type;
    int difficulty;
    uint64_t skulls;
    double minutes;
    int64_t kills;
    int64_t assists;
    int64_t deaths;
    int64_t score;
    std::string map;
};
int64_t ChallengeAmount(const ReachChallengeDef& c, const MatchFacts& f) {
    static const BaseType kModeType[3] = { BaseType::Campaign, BaseType::Firefight, BaseType::Multiplayer };
    if (c.mode < 3 && f.type != kModeType[c.mode]) return 0;
    if (c.mission && *c.mission) {
        // The map can come as "m10" or as a path ("levels\\solo\\m10\\m10"); compare the last part.
        std::string m = f.map;
        const size_t slash = m.find_last_of("\\/");
        if (slash != std::string::npos) m = m.substr(slash + 1);
        for (char& ch : m) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        if (m != c.mission) return 0;
    }
    if (c.minDifficulty >= 0) {
        if (f.type == BaseType::Multiplayer) return 0;
        if (c.exactDifficulty ? f.difficulty != c.minDifficulty : f.difficulty < c.minDifficulty) return 0;
    }
    if ((f.skulls & c.skulls) != c.skulls) return 0;
    if (c.noDeaths && f.deaths > 0) return 0;
    if (c.maxMinutes > 0 && f.minutes > c.maxMinutes) return 0;
    switch (c.metric) {
    case ChallengeMetric::Kills: return f.kills;
    case ChallengeMetric::Assists: return f.assists;
    case ChallengeMetric::Games: return 1;
    case ChallengeMetric::Missions: return f.type == BaseType::Campaign ? 1 : 0;
    case ChallengeMetric::Score: return std::max<int64_t>(0, f.score);
    default: return 0;   // DailyChallenges / AllDailies: counted after the dailies (see OnGameResults)
    }
}

// Forget challenge entries from past periods.
void PruneChallengesLocked() {
    const std::string day = DailyKey(), week = WeeklyKey();
    auto current = [&](const std::string& key) { return key.rfind(day + ":", 0) == 0 || key.rfind(week + ":", 0) == 0; };
    for (auto it = s_extra.challengeProgress.begin(); it != s_extra.challengeProgress.end();) {
        it = current(it->first) ? std::next(it) : s_extra.challengeProgress.erase(it);
    }
    for (auto it = s_extra.challengeClaimed.begin(); it != s_extra.challengeClaimed.end();) {
        it = current(it->first) ? std::next(it) : s_extra.challengeClaimed.erase(it);
    }
}

// ---------------------------------------------------------------- commendations

const char* const kModeKey[3] = { "campaign", "firefight", "multiplayer" };

std::string CommendationKey(const CommendationDef& c) { return std::string(kModeKey[c.mode]) + "_" + c.key; }

int64_t CommendationValue(size_t index) {
    const auto it = s_extra.commendationCount.find(CommendationKey(kCommendations[index]));
    return it == s_extra.commendationCount.end() ? 0 : it->second;
}

// What one finished game adds to a commendation (any difficulty). Noble Actual (m05) is a short
// intro mission, so it doesn't count as a mission (Tyler: it would be farmed); Lone Wolf does.
int64_t CommendationAmount(const CommendationDef& c, BaseType type, const std::string& map, int64_t assists, int64_t deaths) {
    static const BaseType kModeType[3] = { BaseType::Campaign, BaseType::Firefight, BaseType::Multiplayer };
    if (type != kModeType[c.mode]) return 0;
    std::string m = map;
    const size_t slash = m.find_last_of("\\/");
    if (slash != std::string::npos) m = m.substr(slash + 1);
    for (char& ch : m) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    const bool countedMission = m != "m05";
    switch (c.metric) {
    case CM::Assists: return assists;
    case CM::Missions: return countedMission ? 1 : 0;
    case CM::FlawlessMissions: return countedMission && deaths == 0 ? 1 : 0;
    case CM::NoDeathGames: return deaths == 0 ? 1 : 0;
    default: return 0;
    }
}

// Noble's payout per tier (Tyler): Reach's cR x2.5 for Steel to Gold, Onyx = 2x Gold, Max = 3x Onyx.
int64_t TierReward(const CommendationDef& c, int tier) {
    auto scaled = [&](int t) { return (c.rewards[t] * 5 + 1) / 2; };   // x2.5, rounded
    if (tier < 0 || tier > 5) return 0;
    if (tier <= 3) return scaled(tier);
    const int64_t onyx = scaled(3) * 2;
    return tier == 4 ? onyx : onyx * 3;
}

int TierFor(const CommendationDef& c, int64_t value) {
    int tier = 0;
    for (int t = 0; t < 6; ++t) if (value >= c.targets[t]) tier = t + 1;
    return tier;
}

const char* DifficultyName(int d) {
    static const char* const kNames[4] = { "Easy", "Normal", "Heroic", "Legendary" };
    return d >= 0 && d < 4 ? kNames[d] : "?";
}

// The record for this machine's player. Offline, the local player is the first one present.
int LocalPlayer(const uint8_t* rec) {
    for (int i = 0; i < kMaxPlayers; ++i) {
        const uint8_t* pr = rec + kPlayerRecords + i * kPlayerRecordSize;
        if (pr[0] != 0 || U64(pr, 0x08) != 0) return i;
    }
    return -1;
}

void LogRecord(const uint8_t* rec) {
    NOBLE_INFO("Career: results category {} (+0x08 0x{:X}, +0x10 0x{:X}, +0x18 {}, +0x19 {}), totals {} {} {} 0x{:X}",
               U32(rec, 0), U64(rec, 0x08), U64(rec, 0x10), rec[0x18], rec[0x19],
               U32(rec, 0x5D120), U32(rec, 0x5D124), U32(rec, 0x5D128), U64(rec, 0x5D130));
    for (int i = 0; i < kMaxPlayers; ++i) {
        const uint8_t* pr = rec + kPlayerRecords + i * kPlayerRecordSize;
        if (pr[0] == 0 && U64(pr, 0x08) == 0) continue;
        const uint8_t* st = rec + kStatBlocks + i * kStatBlockSize;
        std::string stats;
        for (size_t s = 0; s < kGeneralStats; ++s) stats += std::to_string(U32(st, s * 4)) + (s + 1 < kGeneralStats ? "," : "");
        std::string medals;
        for (size_t m = 0; m < kMedalCount; ++m) {
            const uint32_t n = U32(st, kMedalOffset + m * 4);
            if (n) medals += std::to_string(m) + "x" + std::to_string(n) + " ";
        }
        NOBLE_INFO("Career: player {} present {} +0x18 {} +0x28 {} +0x2C {} +0x30 {} +0x34 {} +0x38 {} | stats [{}] | medals [{}]",
                   i, pr[0], static_cast<int32_t>(U32(pr, 0x18)), static_cast<int32_t>(U32(pr, 0x28)),
                   static_cast<int32_t>(U32(pr, 0x2C)), pr[0x30], static_cast<int32_t>(U32(pr, 0x34)),
                   static_cast<int32_t>(U32(pr, 0x38)), stats, medals);
    }
}

} // namespace

const std::vector<Rank>& Ranks() { return kRanks; }

int RankIndex(int64_t totalCredits) {
    int idx = 0;
    for (size_t i = 0; i < kRanks.size(); ++i) {
        if (totalCredits >= kRanks[i].credits) idx = static_cast<int>(i);
    }
    return idx;
}

BaseType BaseTypeFromEngineMode(uint32_t engineGameMode) {
    if (engineGameMode == 1) return BaseType::Campaign;
    if (engineGameMode == 0x40B) return BaseType::Firefight;
    return BaseType::Multiplayer;
}

const char* BaseTypeName(BaseType t) {
    switch (t) {
    case BaseType::Campaign: return "campaign";
    case BaseType::Firefight: return "firefight";
    default: return "multiplayer";
    }
}

void BeginMatch(BaseType type, const std::string& map, int difficulty, uint64_t skulls) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_type = type;
    s_map = map;
    s_difficulty = difficulty;
    s_skulls = skulls;
    s_matchStart = std::chrono::steady_clock::now();
    LoadLocked();
}

// Stock Noble Actual (m05) and Lone Wolf (m70_bonus) pay nothing: no cR, no challenge or commendation
// progress (Tyler: both can be finished in seconds and farmed). Matched by scenario name, so modded
// maps with their own names still pay as campaign.
bool UnpaidMission(BaseType type, const std::string& map) {
    if (type != BaseType::Campaign) return false;
    std::string m = map;
    const size_t slash = m.find_last_of("\\/");
    if (slash != std::string::npos) m = m.substr(slash + 1);
    for (char& ch : m) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return m == "m05" || m == "m70_bonus";
}

void OnGameResults(const uint8_t* record, size_t size) {
    if (!record || size < kResultsSize) return;

    // Keep the last record for decoding work (local only: it holds player names and ids, so it
    // must never be committed or shared).
    {
        std::error_code ec;
        std::filesystem::create_directories(UserDir() / "results", ec);
        std::ofstream out(UserDir() / "results" / "last_game.bin", std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(record), static_cast<std::streamsize>(size));
    }
    LogRecord(record);

    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();

    const double minutes =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - s_matchStart).count() / 60.0;
    const int local = LocalPlayer(record);
    int64_t kills = 0, deaths = 0, assists = 0, medals = 0, score = 0;
    Award a;
    a.valid = true;
    a.type = BaseTypeName(s_type);
    a.map = s_map;
    a.minutes = minutes;

    // Scoreboard rows for everyone present. The record has no player names, so only the local
    // player is named (from SETTINGS). Stat slots are provisional: implausible values show as 0.
    const SettingsData settings = Settings::Get().Snapshot();
    for (int i = 0; i < kMaxPlayers; ++i) {
        const uint8_t* pr = record + kPlayerRecords + i * kPlayerRecordSize;
        if (pr[0] == 0 && U64(pr, 0x08) == 0) continue;
        const uint8_t* st = record + kStatBlocks + i * kStatBlockSize;
        ScoreRow row;
        row.local = i == local;
        row.name = row.local ? settings.playerName + "  [" + settings.serviceTag + "]" : "Player " + std::to_string(i + 1);
        row.score = static_cast<int32_t>(U32(pr, 0x34));
        row.kills = U32(st, kStatKills * 4);
        row.assists = U32(st, kStatAssists * 4);
        row.deaths = U32(st, kStatDeaths * 4);
        for (size_t m = 0; m < kMedalCount; ++m) row.medals += U32(st, kMedalOffset + m * 4);
        if (row.kills > 1000 || row.assists > 1000 || row.deaths > 1000) row.kills = row.assists = row.deaths = 0;
        if (row.medals > 2000) row.medals = 0;
        if (row.score < -100000 || row.score > 1000000) row.score = 0;
        if (row.local) {
            kills = row.kills;
            assists = row.assists;
            deaths = row.deaths;
            medals = row.medals;
            score = row.score;
        }
        a.players.push_back(row);
    }
    std::stable_sort(a.players.begin(), a.players.end(), [](const ScoreRow& x, const ScoreRow& y) { return x.score > y.score; });

    const std::string today = Today();
    if (s_totals.day != today) {
        s_totals.day = today;
        s_totals.creditsToday = 0;
    }

    a.creditsBefore = s_totals.credits;
    a.rankBefore = RankIndex(s_totals.credits);

    const bool unpaid = UnpaidMission(s_type, s_map);

    // One reward structure per base type (see BaseType); modded content pays as its base type.
    const double paidMinutes = std::clamp(minutes, 0.0, kMinuteCap);
    const double perMinute = kPayPerMinuteBase + kPayPerMinutePerRank * a.rankBefore;
    const int64_t complete = static_cast<int64_t>(paidMinutes * perMinute + 0.5);
    a.lines.push_back({ "Game complete", complete });

    const int64_t performance = kills * kPerKill + assists * kPerAssist + medals * kPerMedal;
    if (performance > 0) a.lines.push_back({ "Performance", performance });

    // Campaign scoring: a bonus for finishing the mission by difficulty, more with skulls on.
    if (s_type == BaseType::Campaign) {
        const int d = std::clamp(s_difficulty, 0, 3);
        const int64_t bonus = kMissionBonus[d];
        a.lines.push_back({ std::string("Mission complete (") + DifficultyName(d) + ")", bonus });
        int skullCount = 0;
        for (uint64_t bits = s_skulls; bits; bits &= bits - 1) ++skullCount;
        if (skullCount > 0) {
            const int64_t skullBonus = bonus * std::min(skullCount, 10) / 10;
            a.lines.push_back({ "Skulls (" + std::to_string(skullCount) + " on)", skullBonus });
        }
        if (!s_map.empty()) {
            auto it = s_extra.campaignBest.find(s_map);
            if (it == s_extra.campaignBest.end() || it->second < d) s_extra.campaignBest[s_map] = d;
        }
    }

    static std::mt19937 rng{ std::random_device{}() };
    if (std::uniform_int_distribution<int>(1, kRandomBonusOneIn)(rng) == 1) {
        if (std::uniform_int_distribution<int>(1, kJackpotOneIn)(rng) == 1) {
            a.lines.push_back({ "Jackpot!", kJackpot });
        } else {
            const int64_t bonus = std::uniform_int_distribution<int64_t>(kRandomBonusMin / 10, kRandomBonusMax / 10)(rng) * 10;
            a.lines.push_back({ "Random bonus", bonus });
        }
    }

    if (unpaid) {
        a.lines.clear();
        a.lines.push_back({ "No cR, challenges or commendations in Noble Actual or Lone Wolf", 0 });
    }

    // Lifetime numbers first (commendations read them).
    const int typeIndex = static_cast<int>(s_type);
    s_extra.games[typeIndex] += 1;
    s_extra.minutes[typeIndex] += minutes;
    s_totals.gamesPlayed += 1;
    s_totals.kills += kills;
    s_totals.deaths += deaths;
    s_totals.assists += assists;
    s_totals.medals += medals;
    s_totals.minutesPlayed += minutes;

    // Challenges: add this match's progress, pay the ones it completes.
    PruneChallengesLocked();
    const MatchFacts facts{ s_type, s_difficulty, s_skulls, minutes, kills, assists, deaths, score, s_map };
    const std::vector<ActiveChallenge> active = ActiveChallenges();   // dailies first, then weeklies
    int newDailies = 0, dailiesDone = 0, dailyCount = 0;
    for (const ActiveChallenge& c : unpaid ? std::vector<ActiveChallenge>{} : active) {
        if (!c.weekly) ++dailyCount;
        if (s_extra.challengeClaimed[c.key]) {
            if (!c.weekly) ++dailiesDone;
            continue;
        }
        int64_t& progress = s_extra.challengeProgress[c.key];
        if (c.def->metric == ChallengeMetric::DailyChallenges) {
            progress += newDailies;
        } else if (c.def->metric == ChallengeMetric::AllDailies) {
            progress = dailyCount > 0 && dailiesDone >= dailyCount ? 1 : 0;
        } else {
            const int64_t amount = ChallengeAmount(*c.def, facts);
            progress = c.def->perGame ? std::max(progress, amount) : progress + amount;
        }
        if (progress >= c.def->target) {
            progress = c.def->target;
            s_extra.challengeClaimed[c.key] = true;
            s_extra.challengesCompleted += 1;
            if (!c.weekly) { ++newDailies; ++dailiesDone; }
            a.lines.push_back({ std::string(c.weekly ? "Weekly challenge: " : "Challenge: ") + c.def->title, c.def->reward });
        }
    }

    // Commendations: count this game, then pay every tier reached since the last match.
    for (size_t i = 0; i < std::size(kCommendations); ++i) {
        const CommendationDef& c = kCommendations[i];
        const int64_t add = unpaid ? 0 : CommendationAmount(c, s_type, s_map, assists, deaths);
        if (add > 0) s_extra.commendationCount[CommendationKey(c)] += add;
        const int reached = TierFor(c, CommendationValue(i));
        int& paid = s_extra.commendationTier[CommendationKey(c)];
        for (int t = paid; t < reached; ++t) {
            a.lines.push_back({ std::string("Commendation: ") + c.title + " (" + kTierNames[t + 1] + ")", TierReward(c, t) });
        }
        paid = std::max(paid, reached);
    }

    // No daily limit: every match pays in full, whatever the mode (campaign, Firefight, multiplayer,
    // Forge) and whether the map or game type is stock or modded.
    int64_t total = 0;
    for (const AwardLine& l : a.lines) total += l.credits;
    a.total = total;

    s_totals.credits += total;
    s_totals.creditsToday += total;
    s_extra.bestMatch = std::max(s_extra.bestMatch, total);

    MatchSummary m;
    {
        std::tm tm = LocalNow();
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm);
        m.date = buf;
    }
    m.type = BaseTypeName(s_type);
    m.map = s_map;
    m.difficulty = s_type == BaseType::Multiplayer ? -1 : s_difficulty;
    m.minutes = minutes;
    m.kills = kills;
    m.deaths = deaths;
    m.assists = assists;
    m.medals = medals;
    m.score = score;
    m.credits = total;
    s_extra.history.insert(s_extra.history.begin(), m);
    if (s_extra.history.size() > 20) s_extra.history.resize(20);

    a.creditsAfter = s_totals.credits;
    a.rankAfter = RankIndex(s_totals.credits);
    s_lastAward = a;
    SaveLocked();

    NOBLE_INFO("Career: {} on {} took {:.1f} min; local player {} kills {} assists {} deaths {} medals {}; paid {} cR "
               "(today {}), total {} cR, rank {}",
               BaseTypeName(s_type), s_map, minutes, local, kills, assists, deaths, medals, total, s_totals.creditsToday,
               s_totals.credits, kRanks[a.rankAfter].name);
}

Record GetRecord() {
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    Record r;
    r.totals = s_totals;
    for (int i = 0; i < 3; ++i) {
        r.games[i] = s_extra.games[i];
        r.minutes[i] = s_extra.minutes[i];
    }
    r.bestMatchCredits = s_extra.bestMatch;
    r.challengesCompleted = s_extra.challengesCompleted;
    for (const auto& [map, d] : s_extra.campaignBest) r.campaignBest.push_back({ map, d });
    r.history = s_extra.history;
    return r;
}

std::vector<Challenge> Challenges() {
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    std::vector<Challenge> out;
    for (const ActiveChallenge& c : ActiveChallenges()) {
        Challenge ch;
        ch.title = c.def->title;
        ch.description = c.def->description;
        ch.weekly = c.weekly;
        ch.mode = c.def->mode;
        ch.target = c.def->target;
        ch.reward = c.def->reward;
        auto p = s_extra.challengeProgress.find(c.key);
        ch.progress = p != s_extra.challengeProgress.end() ? p->second : 0;
        auto d = s_extra.challengeClaimed.find(c.key);
        ch.done = d != s_extra.challengeClaimed.end() && d->second;
        out.push_back(ch);
    }
    return out;
}

namespace {
std::string Countdown(int64_t seconds) {
    seconds = std::max<int64_t>(seconds, 60);
    const int64_t days = seconds / 86400, hours = seconds % 86400 / 3600, minutes = seconds % 3600 / 60;
    char buf[64];
    if (days > 0) std::snprintf(buf, sizeof(buf), "%lld day%s %lld h", static_cast<long long>(days), days == 1 ? "" : "s", static_cast<long long>(hours));
    else std::snprintf(buf, sizeof(buf), "%lld h %lld min", static_cast<long long>(hours), static_cast<long long>(minutes));
    return buf;
}
int64_t SecondsToNextDay() {
    const int64_t now = static_cast<int64_t>(std::time(nullptr));
    return (ChallengeDay() + 1) * 86400 + 11 * 3600 - now;
}
} // namespace

std::string DailyResetText() { return "Resets in " + Countdown(SecondsToNextDay()); }

std::string WeeklyResetText() {
    const int64_t daysLeft = 7 - ((ChallengeDay() - 4) % 7) - 1;   // full days after today until Monday
    return "Resets in " + Countdown(SecondsToNextDay() + daysLeft * 86400);
}

std::string ChallengeResetText() {
    return "Daily challenges: " + DailyResetText() + ". Weekly: " + WeeklyResetText() + ". (11:00 GMT, like Reach.)";
}

std::vector<Commendation> Commendations() {
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    std::vector<Commendation> out;
    for (size_t i = 0; i < std::size(kCommendations); ++i) {
        const CommendationDef& c = kCommendations[i];
        Commendation k;
        k.title = c.title;
        k.description = c.description;
        k.mode = c.mode;
        k.art = CommendationKey(c);
        k.tracked = c.metric != CM::None;
        k.value = CommendationValue(i);
        k.tier = TierFor(c, k.value);
        k.prevTarget = k.tier > 0 ? c.targets[k.tier - 1] : 0;
        k.nextTarget = k.tier < 6 ? c.targets[k.tier] : 0;
        k.nextReward = k.tier < 6 ? TierReward(c, k.tier) : 0;
        out.push_back(k);
    }
    return out;
}

const char* CommendationTierName(int tier) {
    return tier >= 0 && tier <= 6 ? kTierNames[tier] : "?";
}

Award LastAward() {
    std::lock_guard<std::mutex> lock(s_mutex);
    return s_lastAward;
}

Totals Get() {
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    return s_totals;
}

} // namespace Career
} // namespace noble
