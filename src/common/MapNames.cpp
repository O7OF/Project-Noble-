#include "common/MapNames.hpp"

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <vector>

namespace noble {
namespace MapNames {

namespace {

struct Entry {
    const char* scenario;
    const char* name;
};

// Halo: Reach's multiplayer maps, campaign missions and Firefight maps, as the menus list them.
const std::vector<Entry>& Table() {
    static const std::vector<Entry> table = {
        // Multiplayer
        { "50_panopticon", "Boardwalk" },
        { "70_boneyard", "Boneyard" },
        { "45_launch_station", "Countdown" },
        { "30_settlement", "Powerhouse" },
        { "52_ivory_tower", "Reflection" },
        { "35_island", "Spire" },
        { "20_sword_slayer", "Sword Base" },
        { "45_aftship", "Zealot" },
        { "dlc_slayer", "Anchor 9" },
        { "dlc_invasion", "Breakpoint" },
        { "dlc_medium", "Tempest" },
        { "condemned", "Condemned" },
        { "trainingpreserve", "Highlands" },
        { "cex_beaver_creek", "Battle Canyon" },
        { "cex_damnation", "Penance" },
        { "cex_timberland", "Ridgeline" },
        { "cex_prisoner", "Solitary" },
        { "cex_hangemhigh", "High Noon" },
        { "cex_headlong", "Breakneck" },
        { "forge_halo", "Forge World" },
        // Campaign
        { "m05", "Noble Actual" },
        { "m10", "Winter Contingency" },
        { "m20", "ONI: Sword Base" },
        { "m30", "Nightfall" },
        { "m35", "Tip of the Spear" },
        { "m45", "Long Night of Solace" },
        { "m50", "Exodus" },
        { "m52", "New Alexandria" },
        { "m60", "The Package" },
        { "m70", "The Pillar of Autumn" },
        { "m70_bonus", "Lone Wolf" },
        // Firefight
        { "ff50_park", "Beachhead" },
        { "ff45_corvette", "Corvette" },
        { "ff20_courtyard", "Courtyard" },
        { "ff60_icecave", "Glacier" },
        { "ff70_holdout", "Holdout" },
        { "ff60_airview", "Outpost" },
        { "ff10_prototype", "Overlook" },
        { "ff30_waterfront", "Waterfront" },
        { "ff_unearthed", "Unearthed" },
        { "cex_ff_halo", "Installation 04" },
    };
    return table;
}

std::string Lower(std::string text) {
    for (char& c : text) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return text;
}

// "20_sword_slayer", "20_sword_slayer.map", "maps\\20_sword_slayer.map" -> "20_sword_slayer"
std::string Normalise(const std::string& raw) {
    std::string text = raw;
    if (const size_t slash = text.find_last_of("\\/"); slash != std::string::npos) text = text.substr(slash + 1);
    if (text.size() > 4 && Lower(text.substr(text.size() - 4)) == ".map") text = text.substr(0, text.size() - 4);
    return Lower(text);
}

// Campaign missions are frequently sent with a scene suffix ("m10_b", "m20_a"); the table uses the base name.
std::string BaseName(const std::string& scenario) {
    if (scenario.size() > 2 && scenario[0] == 'm' && std::isdigit(static_cast<unsigned char>(scenario[1])) &&
        scenario[scenario.size() - 2] == '_' && std::isalpha(static_cast<unsigned char>(scenario.back()))) {
        return scenario.substr(0, scenario.size() - 2);
    }
    return scenario;
}

std::string Pretty(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    bool startOfWord = true;
    for (char c : text) {
        if (c == '_' || c == '-' || c == '.') {
            if (!out.empty() && out.back() != ' ') out += ' ';
            startOfWord = true;
            continue;
        }
        out += startOfWord ? static_cast<char>(::toupper(static_cast<unsigned char>(c))) : c;
        startOfWord = false;
    }
    return out;
}

} // namespace

std::string Display(const std::string& scenarioOrFile) {
    const std::string key = Normalise(scenarioOrFile);
    if (key.empty()) return scenarioOrFile;
    for (const Entry& e : Table()) {
        if (key == e.scenario) return e.name;
    }
    const std::string base = BaseName(key);
    for (const Entry& e : Table()) {
        if (base == e.scenario) return e.name;
    }
    return Pretty(key);
}

bool IsVanilla(const std::string& scenarioOrFile) {
    const std::string key = Normalise(scenarioOrFile);
    for (const Entry& e : Table()) {
        if (key == e.scenario || BaseName(key) == e.scenario) return true;
    }
    return false;
}

std::string ScenarioOf(const std::string& name) {
    const std::string key = Normalise(name);
    for (const Entry& e : Table()) {
        if (key == e.scenario || key == Lower(e.name)) return e.scenario;
    }
    // A display name with spaces instead of underscores ("high noon") should still resolve.
    std::string underscored;
    for (char c : key) underscored += (c == ' ') ? '_' : c;
    for (const Entry& e : Table()) {
        if (underscored == e.scenario) return e.scenario;
    }
    return {};
}

// The client is a GUI binary, so results go to the same report file the game-type test writes.
namespace {
void Report(const char* format, ...) {
    char line[1024];
    va_list args;
    va_start(args, format);
    std::vsnprintf(line, sizeof(line), format, args);
    va_end(args);
    std::printf("%s", line);
    std::ofstream report("gametype-selftest.log", std::ios::app);
    if (report) report << line;
}
} // namespace

int SelfTest() {
    struct Case {
        const char* input;
        const char* expected;
    };
    const Case cases[] = {
        { "forge_halo", "Forge World" },
        { "forge_halo.map", "Forge World" },
        { "maps\\20_sword_slayer.map", "Sword Base" },
        { "20_sword_slayer", "Sword Base" },
        { "m10_b", "Winter Contingency" },
        { "m70_bonus", "Lone Wolf" },
        { "ff50_park", "Beachhead" },
        { "cex_beaver_creek", "Battle Canyon" },
        { "my_custom_map.map", "My Custom Map" },
        { "test-arena_v2.map", "Test Arena V2" },
    };
    int failures = 0;
    for (const Case& c : cases) {
        const std::string got = Display(c.input);
        if (got != c.expected) {
            ++failures;
            Report("FAIL  MapNames::Display(%s) = '%s', expected '%s'\n", c.input, got.c_str(), c.expected);
        }
    }
    if (!IsVanilla("forge_halo") || IsVanilla("my_custom_map")) {
        ++failures;
        Report("FAIL  MapNames::IsVanilla is wrong for forge_halo / my_custom_map\n");
    }
    if (ScenarioOf("sword base") != "20_sword_slayer" || ScenarioOf("Sword Base") != "20_sword_slayer") {
        ++failures;
        Report("FAIL  MapNames::ScenarioOf('sword base') = '%s'\n", ScenarioOf("sword base").c_str());
    }
    Report("%s: map names - %zu display cases, %d failure(s)\n",
           failures == 0 ? "PASS" : "FAIL", sizeof(cases) / sizeof(cases[0]), failures);
    return failures;
}

} // namespace MapNames
} // namespace noble
