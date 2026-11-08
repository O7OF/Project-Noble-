#pragma once
// Extended engine limits (LIMITS-HANDOFF.md). Noble never edits the player's installed haloreach.dll:
// PrepareEngineDll builds a patched copy in Noble's own folder from the player's stock DLL and an .nlp
// patch file (data\limits\<profile>.nlp), and returns the path Loader should load. On any mismatch or
// error it returns the stock path and a reason, so the game always starts.
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace noble::limits {

enum class Profile : uint32_t { Stock = 0, V16 = 1, V16Particles = 2, Full = 3 };

struct PrepareResult {
    std::filesystem::path dllPath;   // what Loader should load (stock path on any fallback)
    Profile active = Profile::Stock; // what is actually running
    std::string reason;              // empty on success; human-readable on fallback
};

// Never throws. Safe to call before the engine exists. May take about a second the first time (writes the copy).
PrepareResult PrepareEngineDll(const std::filesystem::path& stockDll, Profile requested);

Profile ActiveProfile();                  // valid after PrepareEngineDll
const char* ProfileName(Profile p);       // "Stock", "Extended (V16)", "Extended + Particles"
uint32_t ProfileNetworkId(Profile p, int aiActors = 0);
// This process's id: active profile + the AI actor count actually applied (NOBLE_AI_ACTORS, Extended only).
// Hosts advertise it as "limits_id"; joiners refuse a lobby whose id differs (the engines would desync).
uint32_t LocalNetworkId();   // lobby / server registration; high byte = profile,
                                                        // low 16 bits = AI actor count (0 = stock 96)
std::string SaveFolderSuffix(Profile p);  // "" for Stock, "_ext_v16" / "_ext_v16p" / "_ext_full"

// Profile Full only: the light-volume routines copied into .deltrac have no .pdata entries (all section slots are
// used), so register their RUNTIME_FUNCTIONs at load, reusing the original routines' unwind info.
void RegisterUnwindTables(void* engineBase);
Profile ProfileFromInt(int value);        // settings / command line; unknown values map to Stock

// ---- Building blocks (exposed for tests and tools) ---------------------------------------------
using Bytes = std::vector<uint8_t>;
using Digest = std::array<uint8_t, 32>;

Digest Sha256(const uint8_t* data, size_t size);
std::string DigestHex(const Digest& d);

struct NlpPatch {
    uint32_t profileId = 0;
    uint32_t stockLength = 0;
    Digest stockSha{};
    uint32_t outputLength = 0;
    Digest outputSha{};
    struct Row { uint32_t offset; uint32_t length; size_t stockPos; size_t newPos; };
    std::vector<Row> rows;  // positions point into the .nlp buffer
    size_t appendPos = 0;
    uint32_t appendLength = 0;
};

// Parses and structurally validates an .nlp buffer. Returns false and sets error on any problem.
bool ParseNlp(const Bytes& nlp, NlpPatch& out, std::string& error);

// Applies a parsed patch to the stock DLL bytes. Checks the stock hash, every row's stock bytes, and
// the output hash. Returns false and sets error on any mismatch; output is only valid on success.
bool ApplyNlp(const Bytes& stock, const Bytes& nlp, const NlpPatch& patch, Bytes& output, std::string& error);

} // namespace noble::limits
