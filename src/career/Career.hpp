#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace noble {

// Player career: credits (cR), rank and lifetime totals, kept in data\userdata\career.json.
//
// After every match the engine hands the host MCC's game-results record (host slot 6, see
// ReachHost.cpp). Noble reads the local player's numbers from it and pays credits in the spirit of
// Reach's own system: pay per minute played that grows with rank, a performance bonus and a chance
// of a random bonus. Unlike Reach there is no daily limit, and campaign, Firefight, Forge and modded
// maps/game types all pay the same way. The exact amounts Bungie used were never published, so the
// constants in Career.cpp are Noble's own and are meant to be tuned.
namespace Career {

constexpr size_t kResultsSize = 0x5D138;  // MCC game-results record (haloreach+0x39128 fills it)

struct Rank {
    const char* name;
    int64_t credits;   // total cR needed
};
const std::vector<Rank>& Ranks();             // Recruit .. Inheritor (50 ranks)
int RankIndex(int64_t totalCredits);

// One line of the post-game cR breakdown.
struct AwardLine {
    std::string label;
    int64_t credits = 0;
};

// One row of the post-game scoreboard (from the results record).
struct ScoreRow {
    std::string name;      // "You (NobleSix)" for the local player, "Player N" for others (no names in the record)
    bool local = false;
    int team = -1;         // -1 when the game has no teams
    int64_t score = 0;
    int64_t kills = 0;
    int64_t deaths = 0;
    int64_t assists = 0;
    int64_t medals = 0;
};

// Challenge or commendation progress made by one match (post-game "exit experience").
struct ProgressLine {
    int kind = 0;              // 0 daily challenge, 1 weekly challenge, 2 commendation
    int mode = 3;              // 0 campaign, 1 Firefight, 2 multiplayer, 3 any
    std::string title;
    std::string art;           // commendations: art stem ("campaign_demon"), else empty
    int64_t before = 0;
    int64_t after = 0;
    int64_t target = 1;        // challenge target / commendation's next tier count
    bool completed = false;    // challenge done, or commendation tier reached, by this match
    int tier = 0;              // commendations: tier after this match (0..6)
};

struct Award {
    bool valid = false;
    std::string type;      // "campaign", "firefight", "multiplayer"
    std::string map;
    double minutes = 0.0;
    std::vector<ScoreRow> players;
    std::vector<AwardLine> lines;
    std::vector<ProgressLine> progress;
    int difficulty = -1;   // campaign / Firefight
    int64_t total = 0;
    int64_t creditsBefore = 0;
    int64_t creditsAfter = 0;
    int rankBefore = 0;
    int rankAfter = 0;
};

struct Totals {
    int64_t credits = 0;
    int64_t creditsToday = 0;   // informational only (no daily limit)
    std::string day;            // YYYY-MM-DD creditsToday belongs to
    int64_t gamesPlayed = 0;
    int64_t kills = 0;
    int64_t deaths = 0;
    int64_t assists = 0;
    int64_t medals = 0;
    double minutesPlayed = 0.0;
    int64_t creditsSpent = 0;               // cR spent in the Armory (rank still follows cR earned, as in Reach)
    std::vector<std::string> armoryOwned;   // bought Armory items, "<slot>:<permutation id>"
    bool armoryUnlockAll = false;            // developer unlock: every Armory item usable
};

// One match in the service record history (newest first).
struct MatchSummary {
    std::string date;      // YYYY-MM-DD HH:MM
    std::string type;      // BaseTypeName
    std::string map;
    int difficulty = -1;   // campaign / Firefight only
    double minutes = 0.0;
    int64_t kills = 0, deaths = 0, assists = 0, medals = 0, score = 0;
    int64_t credits = 0;
};

// Everything the SERVICE RECORD screen shows.
struct Record {
    Totals totals;
    int64_t games[3] = {};          // by BaseType
    double minutes[3] = {};
    int64_t bestMatchCredits = 0;
    int64_t challengesCompleted = 0;
    std::vector<std::pair<std::string, int>> campaignBest;  // scenario -> best difficulty completed (0..3)
    std::vector<MatchSummary> history;
};
Record GetRecord();

// Daily and weekly challenges (Reach style: new ones every day / every Monday). Progress comes from
// finished matches; a completed challenge pays its reward on the post-game screen.
struct Challenge {
    std::string title;
    std::string description;
    bool weekly = false;
    int mode = 3;            // 0 campaign, 1 Firefight, 2 multiplayer, 3 any
    int64_t progress = 0;
    int64_t target = 1;
    int64_t reward = 0;
    bool done = false;
};
std::vector<Challenge> Challenges();
std::string ChallengeResetText();
std::string DailyResetText();       // "Resets in 7 h 12 min" (Reach's 11:00 GMT)
std::string WeeklyResetText();      // "Resets in 3 days 4 h" (Monday 11:00 GMT)   // "Daily challenges reset in 5 h 12 min"

// Commendations: lifetime tracks with tiers (Iron .. Onyx, then Max), each tier pays once.
struct Commendation {
    std::string title;
    std::string description;
    int64_t value = 0;
    int tier = 0;                   // tiers reached, 0..6
    int64_t nextTarget = 0;         // 0 when maxed
    int64_t prevTarget = 0;
    int mode = 0;                   // 0 campaign, 1 Firefight, 2 multiplayer
    std::string art;                // "campaign_demon" -> data\ui\reach\commendations\campaign_demon.png
    bool tracked = false;           // false: Noble can't count it yet (stays at 0)
    int64_t nextReward = 0;         // cR paid when the next tier is reached
};
std::vector<Commendation> Commendations();
const char* CommendationTierName(int tier);   // 0 = "None", 1 = "Steel" ... 6 = "Max"

// What a match pays as. Modded content always pays exactly like its base type: the base type is the
// engine game mode BlamEngine launches with (campaign 1, Firefight 0x40B, everything else
// multiplayer, Forge included), not the map, variant or mod, so a modded campaign map earns what a
// stock mission earns (kills, medals, challenges, ...), and the same for Firefight and multiplayer.
enum class BaseType { Campaign, Firefight, Multiplayer };
BaseType BaseTypeFromEngineMode(uint32_t engineGameMode);
const char* BaseTypeName(BaseType t);

// Called by BlamEngine::LaunchMap once it knows the engine game mode.
// difficulty 0..3; skulls are MCC skull bits; map is only used for logs.
void BeginMatch(BaseType type, const std::string& map, int difficulty, uint64_t skulls);

// Called from host slot 6 (engine thread) with a copy of the results record.
void OnGameResults(const uint8_t* record, size_t size);

Award LastAward();
Totals Get();

// Armory purchases. Balance = cR earned - cR spent.
int64_t Balance();
bool OwnsArmoryItem(const std::string& key);
// Spends the cR and marks the item owned; false (nothing spent) if already owned or the balance is too low.
bool BuyArmoryItem(const std::string& key, int64_t price);

// Developer tools (only accessible to developer accounts)
bool ArmoryUnlockAll();
void DevSetArmoryUnlockAll(bool on);
void DevGrantMaxCredits();
void DevSetMaxRank();
void DevResetProgress();


struct PeerStats {
    std::string playerName;
    std::string serviceTag;
    int64_t credits = 0;
    int rank = 0;
    int64_t gamesPlayed = 0;
    int64_t kills = 0;
    int64_t deaths = 0;
    int64_t assists = 0;
};
void SavePeerStats(const PeerStats& stats);
bool LoadPeerStats(const std::string& playerName, PeerStats& outStats);


} // namespace Career
} // namespace noble
