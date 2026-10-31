#include "ReachAudio.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <windows.h>
#include <objbase.h>
#include <xaudio2.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <thread>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "ole32.lib")

namespace noble {
namespace ReachAudio {

namespace {

namespace fs = std::filesystem;

constexpr size_t kInfoRecord = 280;
constexpr size_t kInfoName = 24;
constexpr uintmax_t kCacheLimit = 1536ull * 1024 * 1024;  // trim the decoded-audio cache above this
constexpr double kAllModeSeconds = 150.0;                  // "shuffle all": loop time per track (at least)

// Parts of the bank paths of the UI sounds (first Reach .info name containing it, lower case).
const char* kSfxPaths[static_cast<int>(Sfx::Count)] = {
    "game_sfx\\ui\\omaha_ui\\cursor_vertical\\cursor_vertical.aif",
    "game_sfx\\ui\\omaha_ui\\cursor_horizontal\\cursor_horizontal.aif",
    "game_sfx\\ui\\omaha_ui\\a_button\\a_button.aif",
    "game_sfx\\ui\\omaha_ui\\b_button\\b_button.aif",
    "game_sfx\\ui\\error\\error.aif",
    "bumpers\\left_bumper\\",
    "bumpers\\right_bumper\\",
    "exit_experience\\game_completion\\game_completion.aif",
    "exit_experience\\totals\\totals.aif",
    "exit_experience\\rank_up\\rank_up.aif",
    "exit_experience\\slot_machine\\slot_machine.aif",
};

struct Clip {
    WAVEFORMATEX format{};
    std::vector<uint8_t> data;
    double Seconds() const { return format.nAvgBytesPerSec ? static_cast<double>(data.size()) / format.nAvgBytesPerSec : 0.0; }
};

struct Bank {
    fs::path fsb;
    std::string tag;                 // cache file prefix ("" for Reach, so earlier cache files stay valid)
    bool checkNames = false;         // .info names match vgmstream's stream names
    std::vector<std::string> names;  // per sub-sound, lower case
    std::vector<float> seconds;      // per sub-sound, from the .info file; empty for banks without one
};

struct Piece {
    int bank = -1;
    int index = -1;
    bool Valid() const { return bank >= 0 && index >= 0; }
    int64_t Key() const { return (static_cast<int64_t>(bank) << 32) | static_cast<uint32_t>(index); }
};

struct Track {
    std::string id, game, label, sortKey;
    std::vector<Piece> intro, loops, outro;
};

std::mutex s_mutex;
std::condition_variable s_wake;
bool s_init = false;
bool s_quit = false;
std::string s_problem;
fs::path s_cache, s_vgmstream;
std::vector<Bank> s_banks;                              // [0] = Reach
std::vector<Track> s_tracks;
std::map<int64_t, std::shared_ptr<Clip>> s_clips;       // UI sounds kept in memory
std::set<int64_t> s_sfxKeys;
std::set<int64_t> s_onDisk;
std::set<int64_t> s_failed;
std::deque<Piece> s_decodeQueue;
std::thread s_worker;

IXAudio2* s_xaudio = nullptr;
IXAudio2MasteringVoice* s_master = nullptr;
std::vector<IXAudio2SourceVoice*> s_sfxVoices;
Piece s_sfx[static_cast<int>(Sfx::Count)];

std::thread s_musicThread;
std::atomic<IXAudio2SourceVoice*> s_musicVoice{ nullptr };   // voice of the thread above, for pause/resume
std::atomic<bool> s_musicPaused{ false };
std::atomic<bool> s_musicStop{false};
std::atomic<float> s_musicVolume{1.0f};
std::atomic<bool> s_muted{false};
static std::atomic<float> s_sfxVolume{1.0f};
std::string s_musicTrack;

// ---------------------------------------------------------------- helpers

std::string Lower(std::string s) {
    for (char& c : s) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::vector<std::string> Split(const std::string& s) {
    std::vector<std::string> parts;
    size_t start = 0;
    for (;;) {
        const size_t p = s.find('\\', start);
        parts.push_back(s.substr(start, p == std::string::npos ? std::string::npos : p - start));
        if (p == std::string::npos) break;
        start = p + 1;
    }
    return parts;
}

std::string Join(const std::vector<std::string>& parts, size_t count) {
    std::string s;
    for (size_t i = 0; i < count && i < parts.size(); ++i) s += (i ? "\\" : "") + parts[i];
    return s;
}

std::string StripExt(const std::string& s) {
    const size_t dot = s.find_last_of('.');
    const size_t slash = s.find_last_of('\\');
    return (dot != std::string::npos && (slash == std::string::npos || dot > slash)) ? s.substr(0, dot) : s;
}

std::string Stem(const std::string& path) {
    const size_t slash = path.find_last_of('\\');
    return StripExt(slash == std::string::npos ? path : path.substr(slash + 1));
}

bool StartsWith(const std::string& s, const char* p) { return s.compare(0, strlen(p), p) == 0; }
bool Contains(const std::string& s, const char* p) { return s.find(p) != std::string::npos; }

// "10_kurz02_nobrass\track1" -> "10 Kurz02 Nobrass"
std::string Pretty(const std::string& key) {
    std::vector<std::string> parts = Split(key);
    // Drop "trackN" and the category folder when there is a more specific name.
    std::vector<std::string> keep;
    for (const std::string& p : parts) {
        if (StartsWith(p, "track") && p.size() <= 6) continue;
        keep.push_back(p);
    }
    if (keep.size() > 1) keep.erase(keep.begin());
    std::string out;
    for (size_t i = 0; i < keep.size(); ++i) {
        if (i) out += " / ";
        bool up = true;
        for (char c : keep[i]) {
            if (c == '_') { out += ' '; up = true; continue; }
            out += up ? static_cast<char>(::toupper(static_cast<unsigned char>(c))) : c;
            up = false;
        }
    }
    return out;
}

// ---------------------------------------------------------------- catalogue

bool LoadInfo(const fs::path& info, Bank& bank) {
    std::ifstream in(info, std::ios::binary);
    if (!in) return false;
    std::vector<char> d((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (d.size() < kInfoRecord || d.size() % kInfoRecord != 0) {
        NOBLE_WARN("ReachAudio: {} has an unexpected size ({} bytes)", info.string(), d.size());
        return false;
    }
    const size_t n = d.size() / kInfoRecord;
    bank.names.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        const char* name = d.data() + i * kInfoRecord + kInfoName;
        bank.names.push_back(Lower(std::string(name, strnlen(name, kInfoRecord - kInfoName))));
        // +4 is the decoded PCM size and +8 the channel count: 16-bit 48 kHz, checked against vgmstream on 12 music pieces.
        uint32_t pcmBytes = 0, channels = 0;
        memcpy(&pcmBytes, d.data() + i * kInfoRecord + 4, 4);
        memcpy(&channels, d.data() + i * kInfoRecord + 8, 4);
        bank.seconds.push_back(channels > 0 ? static_cast<float>(pcmBytes) / (48000.0f * 2.0f * static_cast<float>(channels)) : 0.0f);
    }
    bank.checkNames = true;
    return true;
}

struct LstEntry { std::string name; int first; int count; };

bool LoadLst(const fs::path& lst, Bank& bank, std::vector<LstEntry>& entries) {
    std::ifstream in(lst, std::ios::binary);
    if (!in) return false;
    std::vector<char> d((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    auto u32 = [&](size_t p) { uint32_t v = 0; if (p + 4 <= d.size()) memcpy(&v, d.data() + p, 4); return v; };
    const uint32_t count = u32(4);
    size_t p = 8;
    for (uint32_t i = 0; i < count && p + 4 <= d.size(); ++i) {
        const uint32_t len = u32(p);
        if (len == 0 || p + 4 + len + 8 > d.size()) return false;
        LstEntry e{ Lower(std::string(d.data() + p + 4, strnlen(d.data() + p + 4, len))), 0, 0 };
        p += 4 + len;
        e.first = static_cast<int>(u32(p));
        e.count = static_cast<int>(u32(p + 4));
        p += 8;
        entries.push_back(e);
        if (bank.names.size() < static_cast<size_t>(e.first + e.count)) bank.names.resize(e.first + e.count);
        for (int k = 0; k < e.count; ++k) bank.names[e.first + k] = e.name + (e.count > 1 ? "_" + std::to_string(k + 1) : "");
    }
    return !entries.empty();
}

// Halo: CE's bank has no .info file, but an FSB5 header holds every sound's sample count. Left empty if it can't be read.
void LoadFsb5Seconds(Bank& bank) {
    std::ifstream in(bank.fsb, std::ios::binary);
    char head[60]{};
    if (!in.read(head, sizeof(head)) || memcmp(head, "FSB5", 4) != 0) return;
    int32_t version = 0, count = 0, tableBytes = 0;
    memcpy(&version, head + 4, 4);
    memcpy(&count, head + 8, 4);
    memcpy(&tableBytes, head + 12, 4);
    if (version != 1 || count <= 0 || tableBytes <= 0 || tableBytes > 64 * 1024 * 1024) return;   // version 0's header is longer
    std::vector<char> table(static_cast<size_t>(tableBytes));
    if (!in.read(table.data(), tableBytes)) return;
    static const int kRates[] = { 4000, 8000, 11000, 11025, 16000, 22050, 24000, 32000, 44100, 48000, 96000 };
    std::vector<float> seconds;
    size_t at = 0;
    for (int i = 0; i < count; ++i) {
        uint64_t sample = 0;
        if (at + 8 > table.size()) return;
        memcpy(&sample, table.data() + at, 8);
        at += 8;
        for (bool more = (sample & 1) != 0; more;) {   // optional chunks between the entries
            uint32_t chunk = 0;
            if (at + 4 > table.size()) return;
            memcpy(&chunk, table.data() + at, 4);
            at += 4 + ((chunk >> 1) & 0xFFFFFF);
            more = (chunk & 1) != 0;
        }
        const uint32_t rate = static_cast<uint32_t>((sample >> 1) & 0xF);
        if (rate >= std::size(kRates)) return;
        seconds.push_back(static_cast<float>(sample >> 34) / static_cast<float>(kRates[rate]));
    }
    bank.seconds = std::move(seconds);
}

// Piece role from the folder it sits in.
enum class Role { Intro, Loop, Outro, Skip, Single };

Role SegmentRole(const std::string& seg) {
    if (seg == "in") return Role::Intro;
    if (seg == "loop" || seg == "loops" || seg == "loops2") return Role::Loop;
    if (seg == "out") return Role::Outro;
    if (StartsWith(seg, "alt_") || seg == "loops_alt") return Role::Skip;   // alternate (combat-state) layers
    return Role::Single;
}

// Short label overrides, matched as a substring of the cue key.
struct Name { const char* match; const char* label; int order; };

const Name kReachNames[] = {
    { "reach_menu\\", "Main Menu Theme", 0 },
    { "m20_sword", "ONI: Sword Base - Sword", 10 },
    { "m35_big_push", "Tip of the Spear - Big Push", 11 },
    { "m45_katsplan1_mus", "Long Night of Solace - Kat's Plan", 12 },
    { "m45_katsplan1_music", "Long Night of Solace - Kat's Plan (II)", 13 },
    { "m45_pitstop", "Long Night of Solace - Pit Stop", 14 },
    { "m45_blastoff", "Long Night of Solace - Blast Off", 15 },
    { "m45_jorgsac", "Long Night of Solace - Jorge's Sacrifice", 16 },
    { "m50_wake", "Exodus - Wake", 17 },
    { "m50_reunited", "Exodus - Reunited", 18 },
    { "m52_airlift", "New Alexandria - Airlift", 19 },
    { "m60_returnsword", "The Package - Return to Sword Base", 20 },
    { "m70_carter_sac", "The Pillar of Autumn - Carter's Sacrifice", 22 },
    { "m70_carter", "The Pillar of Autumn - Carter", 21 },
    { "firefight_in", "Firefight Intro", 30 },
    { "exit\\track1", "Post-Game", 31 },
};
const Name kHalo3Names[] = {
    { "main_menu_delta\\", "Main Menu Theme", 0 },
    { "end_credits\\", "End Credits", 1 },
};
const Name kOdstNames[] = {
    { "atlas\\menu\\", "Main Menu Theme", 0 },
};
const Name kHalo1Names[] = {
    { "title1", "Title Theme", 0 },
    { "halo_origwguit", "Halo Theme (Guitar)", 2 },
    { "halo_orig", "Halo Theme", 1 },
};

constexpr float kShortestMenuTrack = 40.0f;   // seconds; shorter tracks stay out of the menu playlist
int s_shortTracksDropped = 0;

// Music tied to a level: its file names start with the level's code, the same code as the game's map file.
// "010la_music2_1_18.aif" -> "Sierra 117 - Cinematic".
std::string LevelCueLabel(const std::string& gameId, const std::string& key) {
    struct Level { const char* code; const char* name; };
    static const Level kHalo3[] = {
        { "010", "Sierra 117" }, { "020", "Crow's Nest" }, { "030", "Tsavo Highway" }, { "040", "The Storm" }, { "050", "Floodgate" },
        { "070", "The Ark" }, { "100", "The Covenant" }, { "110", "Cortana" }, { "120", "Halo" }, { "130", "Epilogue" },
    };
    static const Level kOdst[] = {
        { "c100", "Prepare to Drop" }, { "c200", "Epilogue" }, { "h100", "Mombasa Streets" }, { "l200", "Data Hive" }, { "l300", "Coastal Highway" },
        { "sc100", "Tayari Plaza" }, { "sc110", "Uplift Reserve" }, { "sc120", "Kizingo Boulevard" }, { "sc130", "ONI Alpha Site" },
        { "sc140", "NMPD HQ" }, { "sc150", "Kikowani Station" },
    };
    if (gameId == "h3") {
        for (const Level& l : kHalo3) if (StartsWith(key, l.code)) return std::string(l.name) + " - Cinematic";
    } else if (gameId == "odst") {
        for (const Level& l : kOdst) if (StartsWith(key, l.code)) return std::string(l.name) + " - Cinematic";
    }
    return {};
}

// Music with no documented album title gets a plain name by the kind of cue its folder says it is, and never a guessed
// title. Its id is still the cue key, so saved choices keep working if a real title is found later.
std::string CueKind(const std::string& key) {
    static const std::pair<const char*, const char*> kKinds[] = {
        { "action\\", "Action" }, { "tension\\", "Tension" }, { "heroic\\", "Heroic" }, { "poignant\\", "Poignant" },
        { "firefight\\", "Firefight" }, { "atlas\\firefight\\", "Firefight" },
    };
    for (const auto& [prefix, kind] : kKinds) if (StartsWith(key, prefix)) return kind;
    return "Score";
}

template <size_t N>
void NameTrack(Track& t, const std::string& key, const Name (&names)[N]) {
    for (const Name& n : names) {
        if (Contains(key, n.match)) {
            t.label = n.label;
            char order[8];
            snprintf(order, sizeof(order), "%03d", n.order);
            t.sortKey = std::string("0") + order;
            return;
        }
    }
    t.label = Pretty(key);
    t.sortKey = "1" + Lower(t.label);
}

// Halo: CE cutscene music: "b30_ledge_music" -> "The Silent Cartographer - Ledge".
std::string Halo1Cutscene(const std::string& stem) {
    static const std::pair<const char*, const char*> kMissions[] = {
        { "a10", "The Pillar of Autumn" }, { "a30", "Halo" }, { "a50", "The Truth and Reconciliation" },
        { "b30", "The Silent Cartographer" }, { "b40", "Assault on the Control Room" }, { "c10", "343 Guilty Spark" },
        { "c20", "The Library" }, { "c40", "Two Betrayals" }, { "d20", "Keyes" }, { "d40", "The Maw" },
    };
    if (stem == "x10_music01") return "Opening Cinematic";
    if (StartsWith(stem, "x10_music")) return "Opening Cinematic (" + stem.substr(9) + ")";
    if (stem == "x70_music") return "Ending Cinematic";
    for (const auto& [code, mission] : kMissions) {
        if (!StartsWith(stem, code) || stem.size() < 5) continue;
        std::string rest = stem.substr(4);
        const size_t m = rest.find("_music");
        if (m != std::string::npos) rest = rest.substr(0, m) + rest.substr(m + 6);
        return std::string(mission) + " - " + Pretty(rest);
    }
    return "Cinematic " + Pretty(stem);
}

bool SkipCue(const std::string& key) {
    static const char* const kSkip[] = {
        "sustain\\", "perc\\", "stingers\\", "amb\\", "invasion_temp_cues\\", "elevator_music_split",
        "atlas\\glue", "atlas\\microglue", "atlas\\pulse", "atlas\\perc", "atlas\\ambrhythm", "misc_loops\\",
        "rear_speakers", "_perc", "perc_", "percff", "\\detail",
    };
    for (const char* s : kSkip) if (Contains(key, s)) return true;
    return false;
}

// Adds the tracks of one bank. key(name) returns the music path relative to "\music\" (or "" to skip).
template <size_t N>
void AddTracks(int bankIndex, const std::string& game, const std::string& gameId,
               const std::vector<std::pair<int, std::string>>& music, const Name (&names)[N]) {
    std::map<std::string, Track> cues;
    for (const auto& [index, rel] : music) {
        std::vector<std::string> parts = Split(rel);
        if (parts.empty()) continue;
        const Role inFolder = parts.size() >= 2 ? SegmentRole(parts[parts.size() - 2]) : Role::Single;
        const Role last = SegmentRole(parts.back());          // Halo: CE tags end in "in" / "loops" / "out"
        std::string key;
        Role role;
        if (last != Role::Single) { key = Join(parts, parts.size() - 1); role = last; }
        else if (inFolder != Role::Single) { key = Join(parts, parts.size() - 2); role = inFolder; }
        else { key = StripExt(rel); role = Role::Single; }
        if (role == Role::Skip || key.empty() || SkipCue(key)) continue;
        Track& t = cues[key];
        const Piece piece{ bankIndex, index };
        if (role == Role::Intro) t.intro.push_back(piece);
        else if (role == Role::Outro) t.outro.push_back(piece);
        else t.loops.push_back(piece);
    }
    std::map<std::string, int> labels, kindCounts;
    for (auto& [key, t] : cues) {
        if (t.loops.empty()) {
            if (t.intro.empty()) continue;
            std::swap(t.loops, t.intro);   // a cue with only an intro piece
        }
        t.game = game;
        NameTrack(t, key, names);
        // Stingers and short cues aren't menu music. Banks without lengths (Halo: CE) keep everything, and so do menu themes.
        const std::vector<float>& lengths = s_banks[bankIndex].seconds;
        if (!lengths.empty() && t.sortKey != "0000") {
            float total = 0.0f;
            for (const std::vector<Piece>* pieces : { &t.intro, &t.loops, &t.outro }) {
                for (const Piece& p : *pieces) {
                    if (static_cast<size_t>(p.index) < lengths.size()) total += lengths[p.index];
                }
            }
            if (total < kShortestMenuTrack) {
                ++s_shortTracksDropped;
                continue;
            }
        }
        if (StartsWith(key, "cutscenes\\")) {
            t.label = Halo1Cutscene(key.substr(10));
        } else if (t.sortKey[0] == '1') {   // no name of its own in the tables above
            const std::string level = LevelCueLabel(gameId, key);
            if (!level.empty()) {
                t.label = level;
                t.sortKey = "1" + Lower(level);
            } else {
                const std::string kind = CueKind(key);
                char number[8];
                snprintf(number, sizeof(number), "%03d", ++kindCounts[kind]);
                t.label = kind + " " + std::to_string(kindCounts[kind]);
                t.sortKey = "2" + Lower(kind) + number;
            }
        }
        if (const int n = ++labels[t.label]; n > 1) t.label += " (" + std::to_string(n) + ")";
        t.id = gameId + ":" + (t.sortKey == "0000" ? std::string("menu") : key);
        s_tracks.push_back(std::move(t));
    }
}

void AddInfoBank(const fs::path& dir, const std::string& game, const std::string& gameId, const std::string& tag,
                 bool required, const auto& names) {
    Bank bank;
    bank.fsb = dir / "fmod" / "pc" / "sfx.fsb";
    bank.tag = tag;
    std::error_code ec;
    if (!fs::exists(bank.fsb, ec) || !LoadInfo(dir / "fmod" / "pc" / "sfx.fsb.info", bank)) {
        if (required) s_problem = "MCC's Reach sound bank (haloreach\\fmod\\pc\\sfx.fsb) was not found.";
        else NOBLE_INFO("ReachAudio: no {} music ({} not found)", game, bank.fsb.string());
        return;
    }
    std::vector<std::pair<int, std::string>> music;
    for (size_t i = 0; i < bank.names.size(); ++i) {
        const size_t m = bank.names[i].find("\\music\\");
        if (m != std::string::npos) music.emplace_back(static_cast<int>(i), bank.names[i].substr(m + 7));
    }
    s_banks.push_back(std::move(bank));
    AddTracks(static_cast<int>(s_banks.size() - 1), game, gameId, music, names);
}

void AddHalo1(const fs::path& mccDir) {
    Bank bank;
    bank.fsb = mccDir / "halo1" / "sound" / "pc" / "sounds_music.fsb";
    bank.tag = "h1";
    std::vector<LstEntry> entries;
    std::error_code ec;
    if (!fs::exists(bank.fsb, ec) || !LoadLst(mccDir / "halo1" / "sound" / "pc" / "lst" / "sounds_music.lst.bin", bank, entries)) {
        NOBLE_INFO("ReachAudio: no Halo: CE music ({} not found)", bank.fsb.string());
        return;
    }
    LoadFsb5Seconds(bank);
    if (bank.seconds.empty()) NOBLE_WARN("ReachAudio: no lengths in {}, so short Halo: CE tracks stay listed", bank.fsb.string());
    s_banks.push_back(std::move(bank));
    const int bankIndex = static_cast<int>(s_banks.size() - 1);
    std::vector<std::pair<int, std::string>> anniversary, classic;
    for (const LstEntry& e : entries) {
        std::vector<std::pair<int, std::string>>* list = nullptr;
        std::string rel;
        if (StartsWith(e.name, "sound\\old_music\\music\\")) { list = &classic; rel = e.name.substr(22); }
        else if (StartsWith(e.name, "sound\\music\\")) { list = &anniversary; rel = e.name.substr(12); }
        else if (StartsWith(e.name, "sound\\sinomatixx")) { list = &anniversary; rel = "cutscenes\\" + Stem(e.name); }
        if (!list) continue;
        for (int k = 0; k < e.count; ++k) list->emplace_back(e.first + k, rel);
    }
    AddTracks(bankIndex, "Halo: Combat Evolved Anniversary", "h1a", anniversary, kHalo1Names);
    AddTracks(bankIndex, "Halo: Combat Evolved (Classic)", "h1", classic, kHalo1Names);
}

fs::path FindVgmstream() {
    wchar_t exe[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    const fs::path dir = fs::path(exe).parent_path();
    for (const fs::path& p : { dir / "tools" / "vgmstream" / "vgmstream-cli.exe", dir / "vgmstream" / "vgmstream-cli.exe",
                               dir / "bin" / "vgmstream" / "vgmstream-cli.exe" }) {
        std::error_code ec;
        if (fs::exists(p, ec)) return p;
    }
    return {};
}

// ---------------------------------------------------------------- decoding

fs::path CachePath(Piece p) {
    const Bank& b = s_banks[p.bank];
    const std::string stem = p.index < static_cast<int>(b.names.size()) ? Stem(b.names[p.index]) : "sound";
    return s_cache / ((b.tag.empty() ? "" : b.tag + "_") + std::to_string(p.index) + "_" + stem + ".wav");
}

// Runs vgmstream-cli -s <index+1> -o <wav> <bank>, its console output going to <wav>.txt.
bool Decode(Piece p, const fs::path& wav) {
    const fs::path log = fs::path(wav).concat(L".txt");
    std::wstring cmd = L"\"" + s_vgmstream.wstring() + L"\" -s " + std::to_wstring(p.index + 1) + L" -o \"" +
                       wav.wstring() + L"\" \"" + s_banks[p.bank].fsb.wstring() + L"\"";
    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
    HANDLE out = CreateFileW(log.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = out;
    si.hStdError = out;
    PROCESS_INFORMATION pi{};
    std::vector<wchar_t> buf(cmd.begin(), cmd.end());
    buf.push_back(0);
    const BOOL ok = CreateProcessW(nullptr, buf.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr,
                                   s_vgmstream.parent_path().c_str(), &si, &pi);
    if (out != INVALID_HANDLE_VALUE) CloseHandle(out);
    if (!ok) {
        NOBLE_WARN("ReachAudio: could not run vgmstream-cli (error {})", GetLastError());
        return false;
    }
    WaitForSingleObject(pi.hProcess, 120000);
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    std::error_code ec;
    return code == 0 && fs::exists(wav, ec);
}

// The stream name vgmstream printed, for checking that record N really is the sound we wanted.
std::string StreamName(const fs::path& wav) {
    std::ifstream in(fs::path(wav).concat(L".txt"));
    std::string line;
    while (std::getline(in, line)) {
        const size_t p = line.find("stream name:");
        if (p != std::string::npos) {
            std::string v = line.substr(p + 12);
            v.erase(0, v.find_first_not_of(" \t"));
            while (!v.empty() && (v.back() == '\r' || v.back() == ' ')) v.pop_back();
            return v;
        }
    }
    return {};
}

std::shared_ptr<Clip> LoadWav(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return nullptr;
    std::vector<uint8_t> d((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (d.size() < 12 || memcmp(d.data(), "RIFF", 4) != 0 || memcmp(d.data() + 8, "WAVE", 4) != 0) return nullptr;
    auto clip = std::make_shared<Clip>();
    bool haveFmt = false;
    size_t pos = 12;
    while (pos + 8 <= d.size()) {
        uint32_t size = 0;
        memcpy(&size, d.data() + pos + 4, 4);
        const uint8_t* body = d.data() + pos + 8;
        if (pos + 8 + size > d.size()) size = static_cast<uint32_t>(d.size() - pos - 8);
        if (memcmp(d.data() + pos, "fmt ", 4) == 0 && size >= 16) {
            memcpy(&clip->format, body, 16);
            clip->format.cbSize = 0;
            haveFmt = true;
        } else if (memcmp(d.data() + pos, "data", 4) == 0) {
            clip->data.assign(body, body + size);
        }
        pos += 8 + size + (size & 1);
    }
    if (!haveFmt || clip->data.empty() || clip->format.wFormatTag != WAVE_FORMAT_PCM) return nullptr;
    return clip;
}

// Decodes one sound into the cache (once) and, for UI sounds, keeps it in memory. Worker thread.
void Prepare(Piece p) {
    const int64_t key = p.Key();
    const bool sfx = s_sfxKeys.count(key) != 0;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_clips.count(key) || (!sfx && s_onDisk.count(key))) return;
    }
    const fs::path wav = CachePath(p);
    std::error_code ec;
    if (!fs::exists(wav, ec)) {
        const fs::path part = fs::path(wav).concat(L".part");
        if (!Decode(p, part)) {
            NOBLE_WARN("ReachAudio: decoding {} #{} ({}) failed", s_banks[p.bank].fsb.string(), p.index + 1, Stem(s_banks[p.bank].names[p.index]));
            fs::remove(part, ec);
            std::lock_guard<std::mutex> lock(s_mutex);
            s_failed.insert(key);
            return;
        }
        const std::string got = Lower(StreamName(part));
        const std::string want = Stem(s_banks[p.bank].names[p.index]);
        if (s_banks[p.bank].checkNames && !got.empty() && got.find(want) == std::string::npos) {
            NOBLE_WARN("ReachAudio: {} sub-song {} is '{}', expected '{}' (bank order differs?)", s_banks[p.bank].fsb.filename().string(),
                       p.index + 1, got, want);
        } else if (!s_banks[p.bank].checkNames && !got.empty()) {
            NOBLE_DEBUG("ReachAudio: {} sub-song {} = '{}' ({})", s_banks[p.bank].fsb.filename().string(), p.index + 1, got,
                        s_banks[p.bank].names[p.index]);
        }
        fs::remove(fs::path(part).concat(L".txt"), ec);
        fs::rename(part, wav, ec);
    }
    if (!sfx) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_onDisk.insert(key);
        return;
    }
    auto clip = LoadWav(wav);
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!clip) {
        NOBLE_WARN("ReachAudio: {} is not a PCM WAV Noble can play", wav.string());
        s_failed.insert(key);
        return;
    }
    s_onDisk.insert(key);
    s_clips[key] = std::move(clip);
}

void Worker() {
    for (;;) {
        Piece p;
        {
            std::unique_lock<std::mutex> lock(s_mutex);
            s_wake.wait(lock, [] { return s_quit || !s_decodeQueue.empty(); });
            if (s_quit) return;
            p = s_decodeQueue.front();
            s_decodeQueue.pop_front();
        }
        Prepare(p);
    }
}

void Queue(Piece p, bool urgent = false) {
    if (!p.Valid()) return;
    std::lock_guard<std::mutex> lock(s_mutex);
    const int64_t key = p.Key();
    if (s_clips.count(key) || s_failed.count(key) || (s_onDisk.count(key) && !s_sfxKeys.count(key))) return;
    auto it = std::find_if(s_decodeQueue.begin(), s_decodeQueue.end(), [&](const Piece& q) { return q.Key() == key; });
    if (it != s_decodeQueue.end()) {
        if (!urgent) return;
        s_decodeQueue.erase(it);
    }
    if (urgent) s_decodeQueue.push_front(p);
    else s_decodeQueue.push_back(p);
    s_wake.notify_one();
}

std::shared_ptr<Clip> GetClip(Piece p) {
    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_clips.find(p.Key());
    return it == s_clips.end() ? nullptr : it->second;
}

// Waits for a music piece to be decoded, then reads it (not kept in memory afterwards).
std::shared_ptr<Clip> WaitClip(Piece p, int timeoutMs) {
    Queue(p, true);
    for (int waited = 0; waited < timeoutMs && !s_musicStop.load(); waited += 50) {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_failed.count(p.Key())) return nullptr;
            if (s_onDisk.count(p.Key())) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (!s_onDisk.count(p.Key())) return nullptr;
    }
    return LoadWav(CachePath(p));
}

// Keeps the decoded-audio cache under kCacheLimit, oldest files first (UI sounds are tiny and come back).
void TrimCache() {
    std::error_code ec;
    std::vector<std::pair<fs::file_time_type, fs::path>> files;
    uintmax_t total = 0;
    for (const auto& e : fs::directory_iterator(s_cache, ec)) {
        if (!e.is_regular_file(ec)) continue;
        total += e.file_size(ec);
        files.emplace_back(e.last_write_time(ec), e.path());
    }
    if (total <= kCacheLimit) return;
    std::sort(files.begin(), files.end());
    for (const auto& [time, path] : files) {
        if (total <= kCacheLimit * 3 / 4) break;
        const uintmax_t size = fs::file_size(path, ec);
        if (fs::remove(path, ec)) total -= size;
    }
}

// ---------------------------------------------------------------- music

const Track* FindTrack(const std::string& id) {
    for (const Track& t : s_tracks) if (t.id == id) return &t;
    return nullptr;
}

std::string Resolve(const std::string& id) {
    if (id == "off" || id == "all" || FindTrack(id)) return id;
    return "reach:menu";   // old settings: "shuffle", Reach menu piece names
}

void MusicThread(std::string id) {
    std::mt19937 rng(static_cast<unsigned>(GetTickCount64()));
    IXAudio2SourceVoice* voice = nullptr;
    WAVEFORMATEX voiceFormat{};
    std::deque<std::shared_ptr<Clip>> held;   // keep queued buffers alive
    float fade = 0.0f;

    auto tick = [&] {
        fade = std::min(1.0f, fade + 0.02f);
        if (voice) voice->SetVolume(s_muted.load() ? 0.0f : fade * s_musicVolume.load());
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    };
    auto queued = [&]() -> UINT32 {
        if (!voice) return 0;
        XAUDIO2_VOICE_STATE st{};
        voice->GetState(&st);
        return st.BuffersQueued;
    };
    // Queues a decoded piece after the ones already queued (keeps at most two queued).
    auto submit = [&](const std::shared_ptr<Clip>& clip, bool loopForever) -> bool {
        if (!clip || s_musicStop.load()) return false;
        while (queued() >= 2 && !s_musicStop.load()) tick();
        if (voice && memcmp(&voiceFormat, &clip->format, sizeof(WAVEFORMATEX)) != 0) {
            while (queued() > 0 && !s_musicStop.load()) tick();   // different format: finish, then switch voices
            s_musicVoice.store(nullptr);
            voice->DestroyVoice();
            voice = nullptr;
            held.clear();
        }
        if (s_musicStop.load()) return false;
        if (!voice) {
            if (FAILED(s_xaudio->CreateSourceVoice(&voice, &clip->format))) return false;
            voiceFormat = clip->format;
            voice->SetVolume(s_muted.load() ? 0.0f : fade * s_musicVolume.load());
            voice->Start();
            s_musicVoice.store(voice);   // for PauseMusic()/ResumeMusic()
        }
        XAUDIO2_BUFFER b{};
        b.AudioBytes = static_cast<UINT32>(clip->data.size());
        b.pAudioData = clip->data.data();
        b.LoopCount = loopForever ? XAUDIO2_LOOP_INFINITE : 0;
        if (FAILED(voice->SubmitSourceBuffer(&b))) return false;
        held.push_back(clip);
        while (held.size() > 4) held.pop_front();
        return true;
    };
    auto play = [&](Piece p, bool loopForever) -> bool { return submit(WaitClip(p, 120000), loopForever); };
    auto pick = [&](const std::vector<Piece>& pieces, int& last) -> Piece {
        int i = static_cast<int>(std::uniform_int_distribution<size_t>(0, pieces.size() - 1)(rng));
        if (i == last && pieces.size() > 1) i = (i + 1) % static_cast<int>(pieces.size());
        last = i;
        return pieces[i];
    };

    const bool all = id == "all";
    // "Shuffle all games" honours the playlist: tracks the player switched off in Settings > Audio are skipped.
    std::vector<std::string> disabled;
    if (all) disabled = Settings::Get().Snapshot().menuMusicDisabled;
    std::vector<const Track*> pool;
    if (all) {
        for (const Track& t : s_tracks) {
            if (std::find(disabled.begin(), disabled.end(), t.id) == disabled.end()) pool.push_back(&t);
        }
    } else if (const Track* t = FindTrack(id)) {
        pool.push_back(t);
    }
    if (pool.empty()) return;

    const Track* lastTrack = nullptr;
    int failures = 0;
    while (!s_musicStop.load() && failures < 8) {
        const Track* t = pool[std::uniform_int_distribution<size_t>(0, pool.size() - 1)(rng)];
        if (t == lastTrack && pool.size() > 1) continue;
        lastTrack = t;
        for (const Piece& p : t->intro) Queue(p);
        for (const Piece& p : t->loops) Queue(p);
        for (const Piece& p : t->outro) Queue(p);
        NOBLE_INFO("ReachAudio: music '{} - {}'", t->game, t->label);

        if (!t->intro.empty()) play(t->intro.front(), false);
        int last = -1;
        if (!all) {
            if (t->loops.size() == 1) {
                if (!play(t->loops.front(), true)) break;
                while (!s_musicStop.load()) tick();
                break;
            }
            while (!s_musicStop.load()) {
                if (play(pick(t->loops, last), false)) failures = 0;
                else if (++failures >= 8) break;
            }
            break;
        }
        // Shuffle all: each loop piece once (random order) or ~2.5 minutes, then the outro, then another track.
        double seconds = 0.0;
        size_t played = 0;
        bool ok = false;
        while (!s_musicStop.load() && (played < t->loops.size() || seconds < kAllModeSeconds) && seconds < 2 * kAllModeSeconds) {
            const Piece p = pick(t->loops, last);
            auto clip = WaitClip(p, 120000);
            if (!clip) break;
            seconds += clip->Seconds();
            ++played;
            ok = submit(clip, false) || ok;
        }
        if (!t->outro.empty()) play(t->outro.front(), false);
        failures = ok ? 0 : failures + 1;
    }
    if (voice) {
        for (float v = fade; v > 0.0f; v -= 0.05f) {
            voice->SetVolume(v * s_musicVolume.load());
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        voice->Stop();
        s_musicVoice.store(nullptr);
        voice->DestroyVoice();
    }
    s_musicPaused.store(false);
}

// ---------------------------------------------------------------- Firefight voice previews

// Reach's own preview lines for the Armory: dialog\firefight_preview_dialog\<character>\*.aif in english.fsb.
std::map<std::string, std::vector<Piece>> s_voiceLines;
std::map<std::string, size_t> s_voiceNext;   // next line per character, so repeated presses cycle
std::atomic<uint64_t> s_voiceGeneration{0};  // a newer press stops the line that is playing

void LoadVoicePreviews(const fs::path& reachDir) {
    Bank bank;
    bank.fsb = reachDir / "fmod" / "pc" / "english.fsb";
    bank.tag = "en";
    std::error_code ec;
    if (!fs::exists(bank.fsb, ec) || !LoadInfo(reachDir / "fmod" / "pc" / "english.fsb.info", bank)) {
        NOBLE_INFO("ReachAudio: no Firefight voice previews ({} not found)", bank.fsb.string());
        return;
    }
    const int bankIndex = static_cast<int>(s_banks.size());
    size_t lines = 0;
    const std::string marker = "\\firefight_preview_dialog\\";
    for (size_t i = 0; i < bank.names.size(); ++i) {
        const size_t at = bank.names[i].find(marker);
        if (at == std::string::npos) continue;
        const size_t start = at + marker.size();
        const size_t slash = bank.names[i].find('\\', start);
        if (slash == std::string::npos) continue;
        s_voiceLines[bank.names[i].substr(start, slash - start)].push_back(Piece{ bankIndex, static_cast<int>(i) });
        ++lines;
    }
    bank.names.shrink_to_fit();
    s_banks.push_back(std::move(bank));
    NOBLE_INFO("ReachAudio: {} Firefight voice preview lines for {} characters", lines, s_voiceLines.size());
}

// Waits for the line's decode, plays it once, and stays until it ends or a newer press replaces it.
void PlayVoiceLine(Piece piece, float volume, uint64_t generation) {
    Queue(piece, true);
    for (int waited = 0; waited < 20000; waited += 25) {
        if (s_voiceGeneration.load() != generation || s_quit) return;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_failed.count(piece.Key())) return;
            if (s_onDisk.count(piece.Key())) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    const std::shared_ptr<Clip> clip = LoadWav(CachePath(piece));
    if (!clip || !s_xaudio || s_voiceGeneration.load() != generation) return;
    IXAudio2SourceVoice* voice = nullptr;
    if (FAILED(s_xaudio->CreateSourceVoice(&voice, &clip->format))) return;
    XAUDIO2_BUFFER buffer{};
    buffer.AudioBytes = static_cast<UINT32>(clip->data.size());
    buffer.pAudioData = clip->data.data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    voice->SetVolume(volume);
    voice->SubmitSourceBuffer(&buffer);
    voice->Start();
    NOBLE_INFO("ReachAudio: voice preview playing ({:.1f} s, volume {:.2f})", clip->Seconds(), volume);
    for (;;) {
        XAUDIO2_VOICE_STATE state{};
        voice->GetState(&state);
        if (state.BuffersQueued == 0 || s_voiceGeneration.load() != generation || s_quit) break;
        voice->SetVolume(s_muted.load() ? 0.0f : volume);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    voice->Stop();
    voice->DestroyVoice();
}

} // namespace

bool PlayVoicePreview(const std::string& voiceId, bool female, float volume) {
    static const std::map<std::string, std::string> kFolders = {
        { "cortana", "cortana" }, { "chief", "masterchief" }, { "johnson", "sgt_johnson" }, { "buck", "odst_buck" },
        { "stacker", "trooper_sgt1" }, { "carter", "spartan1_carter" }, { "kat", "spartan2_kat" }, { "jun", "spartan3_jun" },
        { "emile", "spartan4_emile" }, { "jorge", "spartan5_jorge" }, { "dot", "auntie_dot" },
    };
    const auto folder = kFolders.find(voiceId);
    const std::string name = folder != kFolders.end() ? folder->second : female ? "spartan6_fem" : "spartan6_male";
    const auto lines = s_voiceLines.find(name);
    if (!s_xaudio || lines == s_voiceLines.end() || lines->second.empty()) {
        NOBLE_INFO("ReachAudio: no voice preview for '{}'", voiceId);
        return false;
    }
    const uint64_t generation = ++s_voiceGeneration;
    if (s_muted.load() || volume <= 0.0f) {
        NOBLE_INFO("ReachAudio: voice preview '{}' silent (muted or volume 0)", voiceId);
        return true;
    }
    size_t& next = s_voiceNext[name];
    const Piece piece = lines->second[next % lines->second.size()];
    NOBLE_INFO("ReachAudio: voice preview '{}' line {} of {}", name, next % lines->second.size() + 1, lines->second.size());
    ++next;
    std::thread(PlayVoiceLine, piece, volume, generation).detach();
    return true;
}

void Init(const fs::path& reachDir, const fs::path& cacheDir) {
    if (s_init) return;
    s_init = true;
    s_cache = cacheDir;
    AddInfoBank(reachDir, "Halo: Reach", "reach", "", true, kReachNames);
    if (s_banks.empty()) {
        NOBLE_WARN("ReachAudio: {}", s_problem);
        return;
    }
    s_vgmstream = FindVgmstream();
    if (s_vgmstream.empty()) {
        s_problem = "vgmstream-cli.exe was not found (put vgmstream in Noble's tools\\vgmstream folder).";
        NOBLE_WARN("ReachAudio: {}", s_problem);
        return;
    }
    // The other games' music, from the same MCC install (all optional).
    const fs::path mcc = reachDir.parent_path();
    AddHalo1(mcc);
    AddInfoBank(mcc / "halo3", "Halo 3", "h3", "h3", false, kHalo3Names);
    AddInfoBank(mcc / "halo3odst", "Halo 3: ODST", "odst", "odst", false, kOdstNames);
    // Halo 2 Anniversary keeps its sound in h2_maps_win64_dx11\sounds_*.dat, a format Noble can't read yet.
    NOBLE_INFO("ReachAudio: {} music tracks under {} s left out of the menu playlist", s_shortTracksDropped, kShortestMenuTrack);
    LoadVoicePreviews(reachDir);

    // Group by game (Reach first, then release order), menu themes first, then by name.
    static const char* const kGameOrder[] = { "reach:", "h1a:", "h1:", "h3:", "odst:" };
    auto gameRank = [](const Track& t) {
        for (int i = 0; i < 5; ++i) if (StartsWith(t.id, kGameOrder[i])) return i;
        return 5;
    };
    std::stable_sort(s_tracks.begin(), s_tracks.end(), [&](const Track& a, const Track& b) {
        const int ga = gameRank(a), gb = gameRank(b);
        return ga != gb ? ga < gb : a.sortKey < b.sortKey;
    });

    std::error_code ec;
    fs::create_directories(s_cache, ec);
    TrimCache();
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);  // XAudio2 needs COM; harmless if already initialised
    if (FAILED(XAudio2Create(&s_xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR)) || FAILED(s_xaudio->CreateMasteringVoice(&s_master))) {
        s_problem = "XAudio2 could not be started.";
        NOBLE_WARN("ReachAudio: {}", s_problem);
        if (s_xaudio) { s_xaudio->Release(); s_xaudio = nullptr; }
        return;
    }
    const Bank& reach = s_banks[0];
    for (int i = 0; i < static_cast<int>(Sfx::Count); ++i) {
        for (size_t n = 0; n < reach.names.size(); ++n) {
            if (Contains(reach.names[n], kSfxPaths[i])) { s_sfx[i] = Piece{ 0, static_cast<int>(n) }; break; }
        }
        if (s_sfx[i].Valid()) s_sfxKeys.insert(s_sfx[i].Key());
    }
    s_worker = std::thread(Worker);
    for (const Piece& p : s_sfx) Queue(p);
    if (const Track* menu = FindTrack("reach:menu")) {   // default menu music, ready before the menu opens
        for (const Piece& p : menu->intro) Queue(p);
        for (const Piece& p : menu->loops) Queue(p);
    }
    std::map<std::string, int> perGame;
    for (const Track& t : s_tracks) perGame[t.game]++;
    std::string summary;
    for (const auto& [game, n] : perGame) summary += (summary.empty() ? "" : ", ") + game + " " + std::to_string(n);
    NOBLE_INFO("ReachAudio: vgmstream at {}; music tracks: {}", s_vgmstream.string(), summary);
}

void Shutdown() {
    StopMusic();
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_quit = true;
    }
    s_wake.notify_all();
    if (s_worker.joinable()) s_worker.join();
    for (IXAudio2SourceVoice* v : s_sfxVoices) v->DestroyVoice();
    s_sfxVoices.clear();
    if (s_master) { s_master->DestroyVoice(); s_master = nullptr; }
    if (s_xaudio) { s_xaudio->Release(); s_xaudio = nullptr; }
}

bool PlaySfx(Sfx sfx, float volume) {
    if (!s_xaudio) return false;
    if (s_muted.load()) return true;   // muted: pretend it played, so callers do not log failures
    const Piece p = s_sfx[static_cast<int>(sfx)];
    auto clip = GetClip(p);
    if (!clip) { Queue(p); return false; }
    s_sfxVoices.erase(std::remove_if(s_sfxVoices.begin(), s_sfxVoices.end(), [](IXAudio2SourceVoice* v) {
        XAUDIO2_VOICE_STATE st{};
        v->GetState(&st);
        if (st.BuffersQueued == 0) { v->DestroyVoice(); return true; }
        return false;
    }), s_sfxVoices.end());
    if (s_sfxVoices.size() >= 16) return true;
    IXAudio2SourceVoice* v = nullptr;
    if (FAILED(s_xaudio->CreateSourceVoice(&v, &clip->format))) return false;
    XAUDIO2_BUFFER b{};
    b.AudioBytes = static_cast<UINT32>(clip->data.size());
    b.pAudioData = clip->data.data();
    b.Flags = XAUDIO2_END_OF_STREAM;
    v->SetVolume(volume);
    v->SubmitSourceBuffer(&b);
    v->Start();
    s_sfxVoices.push_back(v);
    return true;
}

void PlayMenuMusic(const std::string& requested) {
    if (!s_xaudio) return;
    const std::string id = Resolve(requested);
    if (id == s_musicTrack && s_musicThread.joinable()) return;
    StopMusic();
    s_musicTrack = id;
    if (id == "off") return;
    s_musicStop = false;
    s_musicThread = std::thread(MusicThread, id);
}

void StopMusic() {
    s_musicStop = true;
    if (s_musicThread.joinable()) s_musicThread.join();
    s_musicTrack.clear();
}

void SetMusicVolume(float volume) { s_musicVolume = std::clamp(volume, 0.0f, 1.0f); }

void SetMuted(bool muted) { s_muted.store(muted); }
bool IsMuted() { return s_muted.load(); }

bool PauseMusic() {
    IXAudio2SourceVoice* v = s_musicVoice.load();
    if (!v) return false;
    v->Stop(0);                 // keeps the buffer queue, so ResumeMusic continues where it stopped
    s_musicPaused.store(true);
    return true;
}

void ResumeMusic() {
    if (IXAudio2SourceVoice* v = s_musicVoice.load()) v->Start(0);
    s_musicPaused.store(false);
}

bool IsMusicPaused() { return s_musicPaused.load(); }

std::vector<TrackInfo> MusicTracks() {
    std::vector<TrackInfo> list;
    if (s_tracks.size() > 1) list.push_back({ "all", "", "Shuffle All Games" });
    for (const Track& t : s_tracks) list.push_back({ t.id, t.game, t.label });
    list.push_back({ "off", "", "Off" });
    return list;
}

std::string TrackLabel(const std::string& requested) {
    const std::string id = Resolve(requested);
    if (id == "off") return "Off";
    if (id == "all") return "Shuffle All Games";
    if (const Track* t = FindTrack(id)) return t->game + " - " + t->label;
    return "Halo: Reach - Main Menu Theme";
}

std::string Problem() { return s_problem; }

} // namespace ReachAudio
} // namespace noble
