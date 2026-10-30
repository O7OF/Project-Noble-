#include "engine/MapGameTypes.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

// Headless check of the map / game-type guard, runnable without MCC:
//     noble-client.exe -test-map-gametypes
// Exit code 0 when every expectation holds, 1 otherwise. Kept out of the menu code so the rule can be verified
// after any change to the tables in MapGameTypes.cpp.
namespace noble {
namespace MapGameTypes {

int SelfTest() {
    struct Case {
        const char* scenario;
        const char* type;
        bool expected;      // true = the map may host this type
        const char* note;
    };

    // Every multiplayer map Noble offers, with the types that must be allowed and the ones that must be blocked.
    const std::vector<Case> cases = {
        // Invasion only on its three shipped maps (and Forge World).
        { "70_boneyard", "invasion", true, "Boneyard ships Invasion" },
        { "35_island", "invasion", true, "Spire ships Invasion" },
        { "dlc_invasion", "invasion", true, "Breakpoint ships Invasion" },
        { "forge_halo", "invasion", true, "Forge World can build Invasion" },
        { "20_sword_slayer", "invasion", false, "Sword Base has no Invasion setup (the crash case)" },
        { "50_panopticon", "invasion", false, "Boardwalk has no Invasion setup" },
        { "52_ivory_tower", "invasion", false, "Reflection has no Invasion setup" },
        { "45_aftship", "invasion", false, "Zealot has no Invasion setup" },
        { "dlc_slayer", "invasion", false, "Anchor 9 has no Invasion setup" },
        // Race family needs a course: Forge World only.
        { "forge_halo", "race", true, "Forge World ships Race" },
        { "20_sword_slayer", "race", false, "Sword Base has no Race course" },
        { "forge_halo", "rally", true, "Forge World ships Rally" },
        { "forge_halo", "skeeball", true, "Forge World ships Skeeball" },
        { "30_settlement", "skeeball", false, "Powerhouse has no Skeeball setup" },
        // Everything else works everywhere.
        { "20_sword_slayer", "slayer", true, "Slayer everywhere" },
        { "20_sword_slayer", "ctf", true, "CTF everywhere" },
        { "70_boneyard", "assault", true, "Assault everywhere" },
        { "50_panopticon", "territories", true, "Territories everywhere" },
        { "45_launch_station", "koth", true, "King of the Hill everywhere" },
        { "52_ivory_tower", "oddball", true, "Oddball everywhere" },
        { "30_settlement", "juggernaut", true, "Juggernaut everywhere" },
        { "35_island", "infection", true, "Infection everywhere" },
        { "dlc_medium", "headhunter", true, "Headhunter everywhere" },
        { "condemned", "stockpile", true, "Stockpile everywhere" },
        { "trainingpreserve", "firefight", false, "Firefight types are not multiplayer types" },
        { "20_sword_slayer", "", true, "No game type known: never block" },
    };

    std::ofstream report("gametype-selftest.log", std::ios::trunc);
    auto out = [&report](const char* format, auto&&... args) {
        char line[1024];
        std::snprintf(line, sizeof(line), format, args...);
        std::printf("%s", line);
        if (report) report << line;
    };

    int failures = 0;
    for (const Case& c : cases) {
        const bool got = Supports(c.scenario, c.type);
        if (got != c.expected) {
            ++failures;
            out("FAIL  %-18s %-12s expected %s, got %s  (%s)\n", c.scenario, c.type,
                c.expected ? "allowed" : "blocked", got ? "allowed" : "blocked", c.note);
        }
    }

    // Variant-stem detection, using real filenames from the MCC install.
    struct StemCase { const wchar_t* stem; const char* type; };
    const std::vector<StemCase> stems = {
        { L"slayer_054", "slayer" },          { L"SWAT_054", "slayer" },
        { L"2nvasion_slayer_054", "invasion" },{ L"3nvasion_054", "invasion" },
        { L"ctf_1flag_054", "ctf" },          { L"assault_one_bomb_054", "assault" },
        { L"territories-3plot_054", "territories" }, { L"koth_crazyking_054", "koth" },
        { L"oddball_054", "oddball" },        { L"juggernaut_054", "juggernaut" },
        { L"infection_safehavens_054", "infection" }, { L"headhunter_pro_054", "headhunter" },
        { L"stockpile_054", "stockpile" },    { L"hr_ff_standard_1set", "firefight" },
        { L"race_054", "race" },              { L"00_basic_editing_054", "forged" },
        { L"hr_4v4_team_multiFlag_dmr_ar_3points_tu", "ctf" },
    };
    for (const StemCase& c : stems) {
        const std::string got = FromVariantStem(c.stem);
        if (got != c.type) {
            ++failures;
            out("FAIL  stem expected %s, got '%s'\n", c.type, got.c_str());
        }
    }

    // Every map Noble offers, against every multiplayer type it can produce: nothing must throw or crash, and the
    // blocked combinations must all be types the map genuinely cannot host.
    const char* const maps[] = { "50_panopticon", "70_boneyard", "45_launch_station", "30_settlement",
                                 "52_ivory_tower", "35_island", "20_sword_slayer", "45_aftship", "dlc_slayer",
                                 "dlc_invasion", "dlc_medium", "condemned", "trainingpreserve", "cex_beaver_creek",
                                 "cex_damnation", "cex_timberland", "cex_prisoner", "cex_hangemhigh", "cex_headlong",
                                 "forge_halo" };
    const char* const types[] = { "slayer", "ctf", "assault", "territories", "koth", "oddball", "juggernaut",
                                  "infection", "headhunter", "stockpile", "invasion", "race", "rally", "skeeball" };
    int allowed = 0, blocked = 0;
    for (const char* map : maps) {
        for (const char* type : types) {
            if (Supports(map, type)) {
                ++allowed;
            } else {
                ++blocked;
                const std::string reason = ConflictReason(map, map, type);
                if (reason.empty()) {
                    ++failures;
                    out("FAIL  %s + %s is blocked but has no reason text\n", map, type);
                }
            }
        }
    }

    out("%s: %zu rule checks, %d map x type combinations (%d allowed, %d blocked), %d failure(s)\n",
        failures == 0 ? "PASS" : "FAIL", cases.size() + stems.size(),
        allowed + blocked, allowed, blocked, failures);
    return failures == 0 ? 0 : 1;
}

} // namespace MapGameTypes
} // namespace noble
