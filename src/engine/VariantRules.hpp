#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace noble {

// Player-made game modes: a shipped game variant plus a list of changed rules. Nothing is written into Reach's variant
// file format. The base file goes through the engine's own decoder and the changes are applied to the decoded record
// before it goes in the launch block. Guests receive the host's variant through the session.
// Offsets into the decoded record are in the variant notes. A preset file names rules by key; it can never name
// an offset, and every value is clamped to its rule's range, so a preset from a mod or another player can only set rules.
namespace VariantRules {

struct Rule {
    const char* key;
    const char* label;
    const char* group;       // "General Settings" or "Player Traits"
    const char* help;
    int offset;              // into the decoded record
    int bit;                 // -1 = the whole byte; otherwise a single on/off bit of it
    int minValue, maxValue;
    const char* unit;        // printed after the number; empty = a plain number
    const std::vector<std::string>* names;   // optional labels: names[value - minValue]
    int bytes = 1;           // 2 = little-endian 16-bit
    const std::vector<int>* order = nullptr;  // optional: the values a player steps through, in order
};

const std::vector<Rule>& Rules();

struct Preset {
    std::string name;
    std::string base;                  // file name in MCC's game_variants
    std::map<std::string, int> values; // only the rules that differ from the base
};

constexpr size_t kDecodedSize = 0xFC04;

bool Load(const std::filesystem::path& file, Preset& out);
bool Save(const std::filesystem::path& file, const Preset& preset);
std::filesystem::path Directory();     // where this PC's own game modes are saved
bool IsPresetFile(const std::filesystem::path& file);

int Read(const Rule& rule, const uint8_t* decoded);
void Write(const Rule& rule, uint8_t* decoded, int value);
void Apply(const Preset& preset, uint8_t* decoded);

// Runs a variant file through the loaded haloreach.dll's decoder, to show a base variant's real values.
bool Decode(const std::filesystem::path& file, std::vector<uint8_t>& decoded);

} // namespace VariantRules
} // namespace noble