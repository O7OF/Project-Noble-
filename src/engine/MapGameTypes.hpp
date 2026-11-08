#pragma once

#include <string>
#include <vector>

namespace noble {

// Which game types a Halo: Reach map is built for.
//
// Why this exists: starting a game type the map has no setup for (Invasion on Sword Base is the classic example)
// puts the engine into a match it cannot initialise, which crashes it. Noble therefore refuses to *start* a vanilla
// map with a game type that map does not support, and says so in the menu.
//
// Rules, deliberately conservative:
//   - Only the restrictive types are listed. Slayer, CTF, Assault, Territories, King of the Hill, Oddball,
//     Juggernaut, Infection, Headhunter and Stockpile work on every multiplayer map, so they are never blocked.
//   - Invasion needs its own spawn/objective layout: only Boneyard, Spire and Breakpoint were shipped for it
//     (plus Forge World, where players build their own).
//   - Race / Rally / Skeeball need a course, which only Forge World provides out of the box.
//   - Modded maps are never checked: a mod may bring its own setup, and players asked to keep mods unrestricted.
//   (Halopedia: "Invasion" lists Boneyard, Spire and Breakpoint as its maps; "Race" and "Rally" are Forge World
//    game types.)
namespace MapGameTypes {

// Canonical key for a variant file stem ("ctf_1flag_054" -> "ctf", "3nvasion_slayer_054" -> "invasion", ...).
// Returns an empty string when the stem is not recognised.
std::string FromVariantStem(const std::wstring& stem);

// The same classification for the variant's own display name (the UTF-16 name inside a .bin made in
// MCC or exported by RVT), for files whose name does not carry the game type. Returns empty when
// nothing is recognised.
std::string FromVariantText(const std::string& text);

// Human-readable name of a game-type key ("ctf" -> "CTF"). Falls back to the upper-cased key.
const char* Name(const std::string& gameType);

// Maps this game type needs a dedicated setup for. Empty when the type works everywhere.
std::vector<std::string> RequiredMaps(const std::string& gameType);

// True when `scenario` (e.g. "20_sword_slayer") can host `gameType` without a mod.
bool Supports(const std::string& scenario, const std::string& gameType);

// "Invasion isn't set up on Sword Base: pick Boneyard, Spire or Breakpoint, or run a modded map."
// Returns an empty string when nothing is wrong.
std::string ConflictReason(const std::string& scenario, const std::string& mapName, const std::string& gameType);

// Headless verification of the tables above (see MapGameTypesTest.cpp). Returns 0 when every expectation holds.
int SelfTest();

} // namespace MapGameTypes
} // namespace noble
