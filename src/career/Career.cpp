#include "Career.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
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

std::mutex s_mutex;
bool s_loaded = false;
Totals s_totals;
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
    int64_t kills = 0, deaths = 0, assists = 0, medals = 0;
    if (local >= 0) {
        const uint8_t* st = record + kStatBlocks + local * kStatBlockSize;
        kills = U32(st, kStatKills * 4);
        assists = U32(st, kStatAssists * 4);
        deaths = U32(st, kStatDeaths * 4);
        for (size_t m = 0; m < kMedalCount; ++m) medals += U32(st, kMedalOffset + m * 4);
        // Sanity: the stat slots are provisional; drop anything implausible rather than pay for it.
        if (kills > 1000 || assists > 1000 || deaths > 1000) kills = assists = deaths = 0;
        if (medals > 2000) medals = 0;
    }

    const std::string today = Today();
    if (s_totals.day != today) {
        s_totals.day = today;
        s_totals.creditsToday = 0;
    }

    Award a;
    a.valid = true;
    a.creditsBefore = s_totals.credits;
    a.rankBefore = RankIndex(s_totals.credits);

    // One reward structure per base type (see BaseType); today all three share these rules.
    const double paidMinutes = std::clamp(minutes, 0.0, kMinuteCap);
    const double perMinute = kPayPerMinuteBase + kPayPerMinutePerRank * a.rankBefore;
    const int64_t complete = static_cast<int64_t>(paidMinutes * perMinute + 0.5);
    a.lines.push_back({ "Game complete", complete });

    const int64_t performance = kills * kPerKill + assists * kPerAssist + medals * kPerMedal;
    if (performance > 0) a.lines.push_back({ "Performance", performance });

    static std::mt19937 rng{ std::random_device{}() };
    if (std::uniform_int_distribution<int>(1, kRandomBonusOneIn)(rng) == 1) {
        if (std::uniform_int_distribution<int>(1, kJackpotOneIn)(rng) == 1) {
            a.lines.push_back({ "Jackpot!", kJackpot });
        } else {
            const int64_t bonus = std::uniform_int_distribution<int64_t>(kRandomBonusMin / 10, kRandomBonusMax / 10)(rng) * 10;
            a.lines.push_back({ "Random bonus", bonus });
        }
    }

    // No daily limit: every match pays in full, whatever the mode (campaign, Firefight, multiplayer,
    // Forge) and whether the map or game type is stock or modded.
    int64_t total = 0;
    for (const AwardLine& l : a.lines) total += l.credits;
    a.total = total;

    s_totals.credits += total;
    s_totals.creditsToday += total;
    s_totals.gamesPlayed += 1;
    s_totals.kills += kills;
    s_totals.deaths += deaths;
    s_totals.assists += assists;
    s_totals.medals += medals;
    s_totals.minutesPlayed += minutes;
    a.creditsAfter = s_totals.credits;
    a.rankAfter = RankIndex(s_totals.credits);
    s_lastAward = a;
    SaveLocked();

    NOBLE_INFO("Career: {} on {} took {:.1f} min; local player {} kills {} assists {} deaths {} medals {}; paid {} cR "
               "(today {}), total {} cR, rank {}",
               BaseTypeName(s_type), s_map, minutes, local, kills, assists, deaths, medals, total, s_totals.creditsToday,
               s_totals.credits, kRanks[a.rankAfter].name);
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
