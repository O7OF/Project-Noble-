#include "Career.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <windows.h>
#include <nlohmann/json.hpp>

#include <algorithm>
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
enum class Metric { Games, CampaignMissions, FirefightGames, MultiplayerGames, Minutes, Kills, Medals, Assists,
                    HeroicMission, LegendaryMission, SkullMission };
struct ChallengeDef {
    const char* title;
    const char* description;
    Metric metric;
    int64_t target;
    int64_t reward;
};
// Rotation: each day shows the next 7 daily challenges in this list (wrapping round), each week the
// next 5 weekly ones, so consecutive days never repeat a challenge and every one comes back in turn.
// The order mixes the modes so any 7 in a row cover campaign, Firefight, multiplayer and general goals.
const ChallengeDef kDailyPool[] = {
    { "Daily Grind", "Finish 3 games of any kind.", Metric::Games, 3, 1500 },
    { "Mission Ready", "Complete a campaign mission.", Metric::CampaignMissions, 1, 1500 },
    { "Hold the Line", "Finish a Firefight game.", Metric::FirefightGames, 1, 1500 },
    { "Spartan Games", "Finish 2 multiplayer games.", Metric::MultiplayerGames, 2, 1500 },
    { "Body Count", "Get 50 kills in any mode.", Metric::Kills, 50, 2000 },
    { "Time on Duty", "Play for 45 minutes.", Metric::Minutes, 45, 2000 },
    { "Decorated", "Earn 10 medals.", Metric::Medals, 10, 1500 },
    { "No Easy Way Out", "Complete a campaign mission on Heroic or Legendary.", Metric::HeroicMission, 1, 2500 },
    { "Wave Breaker", "Finish 3 Firefight games.", Metric::FirefightGames, 3, 3000 },
    { "Arena Regular", "Finish 5 multiplayer games.", Metric::MultiplayerGames, 5, 3000 },
    { "Team Player", "Get 15 assists in any mode.", Metric::Assists, 15, 2000 },
    { "Skull Session", "Complete a campaign mission with a skull on.", Metric::SkullMission, 1, 2000 },
    { "Long Shift", "Play for 90 minutes.", Metric::Minutes, 90, 3500 },
    { "Campaign Push", "Complete 3 campaign missions.", Metric::CampaignMissions, 3, 3000 },
    { "Slaughterhouse", "Get 150 kills in any mode.", Metric::Kills, 150, 4000 },
    { "Medal Haul", "Earn 30 medals.", Metric::Medals, 30, 3500 },
    { "Marathon", "Finish 6 games of any kind.", Metric::Games, 6, 2500 },
    { "Legendary Effort", "Complete a campaign mission on Legendary.", Metric::LegendaryMission, 1, 4000 },
};
const ChallengeDef kWeeklyPool[] = {
    { "Weekly Campaigner", "Complete 6 campaign missions.", Metric::CampaignMissions, 6, 15000 },
    { "Weekly Survivor", "Finish 8 Firefight games.", Metric::FirefightGames, 8, 15000 },
    { "Weekly Arena", "Finish 15 multiplayer games.", Metric::MultiplayerGames, 15, 15000 },
    { "Weekly Veteran", "Play for 5 hours.", Metric::Minutes, 300, 15000 },
    { "Weekly Slayer", "Get 500 kills in any mode.", Metric::Kills, 500, 15000 },
    { "Weekly Legend", "Complete 2 campaign missions on Legendary.", Metric::LegendaryMission, 2, 20000 },
    { "Weekly Regular", "Finish 20 games of any kind.", Metric::Games, 20, 15000 },
    { "Weekly Decorated", "Earn 100 medals.", Metric::Medals, 100, 15000 },
    { "Weekly Supporter", "Get 75 assists in any mode.", Metric::Assists, 75, 15000 },
    { "Weekly Skull Collector", "Complete 5 campaign missions with a skull on.", Metric::SkullMission, 5, 15000 },
};
constexpr int kDailyCount = 7;
constexpr int kWeeklyCount = 5;

struct CommendationDef {
    const char* title;
    const char* description;
    int64_t targets[6];   // Iron, Bronze, Silver, Gold, Onyx, Max
};
const CommendationDef kCommendations[] = {
    { "Campaign Veteran", "Campaign missions completed", { 5, 15, 40, 100, 250, 500 } },
    { "Firefight Survivor", "Firefight games finished", { 5, 15, 40, 100, 250, 500 } },
    { "Spartan Warrior", "Multiplayer games finished", { 10, 30, 75, 200, 500, 1000 } },
    { "Time Served", "Hours played", { 1, 5, 15, 40, 100, 250 } },
    { "Killing Machine", "Kills in every mode", { 100, 500, 1500, 5000, 15000, 50000 } },
    { "Medal Collector", "Medals earned", { 25, 100, 300, 1000, 3000, 10000 } },
};
constexpr int64_t kCommendationReward[6] = { 500, 1500, 3000, 6000, 12000, 25000 };
const char* const kTierNames[7] = { "None", "Iron", "Bronze", "Silver", "Gold", "Onyx", "Max" };

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
    std::map<std::string, int> commendationTier;      // title -> tiers paid
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

// Daily period = today's date; weekly period = the date of this week's Monday.
std::string DailyKey() { return Today(); }
std::string WeeklyKey() {
    std::tm tm = LocalNow();
    const int sinceMonday = (tm.tm_wday + 6) % 7;
    tm.tm_mday -= sinceMonday;
    tm.tm_hour = 12;
    std::mktime(&tm);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string("W") + buf;
}

struct ActiveChallenge {
    const ChallengeDef* def;
    std::string key;
    bool weekly;
};

// Day number of the local date (days since 1970-01-01) and week number (weeks since the Monday
// 1970-01-05), used to step through the pools.
int64_t LocalDayNumber() {
    std::tm tm = LocalNow();
    tm.tm_hour = 12; tm.tm_min = 0; tm.tm_sec = 0;
    std::tm epoch{};
    epoch.tm_year = 70; epoch.tm_mon = 0; epoch.tm_mday = 1; epoch.tm_hour = 12;
    const double seconds = std::difftime(std::mktime(&tm), std::mktime(&epoch));
    return static_cast<int64_t>(seconds / 86400.0 + 0.5);
}

// Today's 7 daily challenges and this week's 5 weekly ones (see the rotation note at the pools).
std::vector<ActiveChallenge> ActiveChallenges() {
    std::vector<ActiveChallenge> out;
    const int64_t day = LocalDayNumber();
    const int64_t week = (day - 4) / 7;   // 1970-01-05 (day 4) was a Monday
    const std::string dayKey = DailyKey(), weekKey = WeeklyKey();
    const int64_t dailySize = static_cast<int64_t>(std::size(kDailyPool));
    const int64_t weeklySize = static_cast<int64_t>(std::size(kWeeklyPool));
    for (int k = 0; k < kDailyCount; ++k) {
        const int i = static_cast<int>((day * kDailyCount + k) % dailySize);
        out.push_back({ &kDailyPool[i], dayKey + ":d" + std::to_string(i), false });
    }
    for (int k = 0; k < kWeeklyCount; ++k) {
        const int i = static_cast<int>((week * kWeeklyCount + k) % weeklySize);
        out.push_back({ &kWeeklyPool[i], weekKey + ":w" + std::to_string(i), true });
    }
    return out;
}

// What one finished match adds to a challenge metric.
struct MatchFacts {
    BaseType type;
    int difficulty;
    uint64_t skulls;
    double minutes;
    int64_t kills;
    int64_t medals;
    int64_t assists;
};
int64_t MetricAmount(Metric m, const MatchFacts& f) {
    const bool campaign = f.type == BaseType::Campaign;
    switch (m) {
    case Metric::Games: return 1;
    case Metric::CampaignMissions: return campaign ? 1 : 0;
    case Metric::FirefightGames: return f.type == BaseType::Firefight ? 1 : 0;
    case Metric::MultiplayerGames: return f.type == BaseType::Multiplayer ? 1 : 0;
    case Metric::Minutes: return static_cast<int64_t>(f.minutes);
    case Metric::Kills: return f.kills;
    case Metric::Medals: return f.medals;
    case Metric::Assists: return f.assists;
    case Metric::HeroicMission: return campaign && f.difficulty >= 2 ? 1 : 0;
    case Metric::LegendaryMission: return campaign && f.difficulty >= 3 ? 1 : 0;
    case Metric::SkullMission: return campaign && f.skulls != 0 ? 1 : 0;
    }
    return 0;
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

int64_t CommendationValue(size_t index) {
    switch (index) {
    case 0: return s_extra.games[0];
    case 1: return s_extra.games[1];
    case 2: return s_extra.games[2];
    case 3: return static_cast<int64_t>(s_totals.minutesPlayed / 60.0);
    case 4: return s_totals.kills;
    case 5: return s_totals.medals;
    default: return 0;
    }
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
    const MatchFacts facts{ s_type, s_difficulty, s_skulls, minutes, kills, medals, assists };
    for (const ActiveChallenge& c : ActiveChallenges()) {
        if (s_extra.challengeClaimed[c.key]) continue;
        int64_t& progress = s_extra.challengeProgress[c.key];
        progress += MetricAmount(c.def->metric, facts);
        if (progress >= c.def->target) {
            progress = c.def->target;
            s_extra.challengeClaimed[c.key] = true;
            s_extra.challengesCompleted += 1;
            a.lines.push_back({ std::string(c.weekly ? "Weekly challenge: " : "Challenge: ") + c.def->title, c.def->reward });
        }
    }

    // Commendations: pay every tier reached since the last match.
    for (size_t i = 0; i < std::size(kCommendations); ++i) {
        const CommendationDef& c = kCommendations[i];
        const int reached = TierFor(c, CommendationValue(i));
        int& paid = s_extra.commendationTier[c.title];
        for (int t = paid; t < reached; ++t) {
            a.lines.push_back({ std::string("Commendation: ") + c.title + " (" + kTierNames[t + 1] + ")", kCommendationReward[t] });
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

std::string ChallengeResetText() {
    std::tm tm = LocalNow();
    const int minutesLeft = (23 - tm.tm_hour) * 60 + (59 - tm.tm_min) + 1;
    int daysToMonday = (8 - tm.tm_wday) % 7;
    if (daysToMonday == 0) daysToMonday = 7;
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Daily challenges reset in %d h %d min; weekly ones in %d day%s.",
                  minutesLeft / 60, minutesLeft % 60, daysToMonday, daysToMonday == 1 ? "" : "s");
    return buf;
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
        k.value = CommendationValue(i);
        k.tier = TierFor(c, k.value);
        k.prevTarget = k.tier > 0 ? c.targets[k.tier - 1] : 0;
        k.nextTarget = k.tier < 6 ? c.targets[k.tier] : 0;
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
