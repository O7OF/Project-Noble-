#include "engine/MapGameTypes.hpp"

#include <algorithm>
#include <array>
#include <cctype>

namespace noble {
namespace MapGameTypes {

namespace {

std::wstring Lower(const std::wstring& text) {
    std::wstring out = text;
    for (wchar_t& c : out) c = static_cast<wchar_t>(::towlower(c));
    return out;
}

bool Has(const std::wstring& stem, const wchar_t* needle) {
    return stem.find(needle) != std::wstring::npos;
}

struct TypeInfo {
    const char* key;
    const char* name;
    // Maps that ship with a setup for this type. Empty = every multiplayer map supports it.
    std::vector<const char*> maps;
};

// Order matters only for readability; `FromVariantStem` checks the specific types before the generic ones.
const std::array<TypeInfo, 6> kRestricted = { {
    { "invasion", "Invasion", { "70_boneyard", "35_island", "dlc_invasion", "forge_halo" } },
    { "race", "Race", { "forge_halo" } },
    { "rally", "Rally", { "forge_halo" } },
    { "skeeball", "Skeeball", { "forge_halo" } },
    // Firefight types are only ever started from the Firefight screen, on Firefight maps.
    { "firefight", "Firefight", {} },
    { "", "", {} },
} };

// Types that work on every multiplayer map; listed so `Name` can return a proper label.
const std::array<std::pair<const char*, const char*>, 15> kUniversal = { {
    { "slayer", "Slayer" }, { "ctf", "Capture the Flag" }, { "assault", "Assault" },
    { "territories", "Territories" }, { "koth", "King of the Hill" }, { "oddball", "Oddball" },
    { "juggernaut", "Juggernaut" }, { "infection", "Infection" }, { "headhunter", "Headhunter" },
    { "stockpile", "Stockpile" }, { "vip", "VIP" }, { "action_sack", "Action Sack" },
    { "grifball", "Grifball" }, { "forged", "Forge" }, { "unknown", "Unknown" },
} };

bool IsRestricted(const std::string& gameType) {
    for (const TypeInfo& info : kRestricted) {
        if (info.key[0] != '\0' && gameType == info.key) return !info.maps.empty();
    }
    return false;
}

} // namespace

std::string FromVariantStem(const std::wstring& stem) {
    const std::wstring s = Lower(stem);
    // MCC ships them as "3nvasion_*" / "2nvasion_*" (an 'n' where the 'I' glyph would be), so match the tail.
    if (Has(s, L"nvasion")) return "invasion";
    if (Has(s, L"hr_ff_")) return "firefight";
    if (Has(s, L"vip")) return "vip";
    if ((Has(s, L"action") && Has(s, L"sack")) || Has(s, L"actionsack")) return "action_sack";
    if (Has(s, L"grifball")) return "grifball";
    if (Has(s, L"skeeball")) return "skeeball";
    if (Has(s, L"rally")) return "rally";
    if (Has(s, L"race")) return "race";
    if (Has(s, L"swat")) return "slayer";           // SWAT is a Slayer variant with different rules
    if (Has(s, L"slayer")) return "slayer";
    if (Has(s, L"ctf") || Has(s, L"flag")) return "ctf";
    if (Has(s, L"assault") || Has(s, L"bomb")) return "assault";
    if (Has(s, L"territor") || Has(s, L"plots") || Has(s, L"landgrab")) return "territories";
    if (Has(s, L"crazyking") || Has(s, L"koth") || Has(s, L"_king")) return "koth";
    if (Has(s, L"oddball")) return "oddball";
    if (Has(s, L"juggernaut") || Has(s, L"escalation")) return "juggernaut";
    if (Has(s, L"infection") || Has(s, L"safehaven")) return "infection";
    if (Has(s, L"headhunter")) return "headhunter";
    if (Has(s, L"stockpile")) return "stockpile";
    if (Has(s, L"basic_editing")) return "forged";  // MCC's Forge game type
    return {};
}

std::string FromVariantText(const std::string& text) {
    std::wstring wide;
    wide.reserve(text.size());
    for (const unsigned char c : text) {
        // The header name is UTF-8 from the variant. Type words are ASCII; anything else is a separator.
        wide.push_back(c < 0x80 ? static_cast<wchar_t>(c) : L' ');
    }
    return FromVariantStem(wide);
}

const char* Name(const std::string& gameType) {
    for (const TypeInfo& info : kRestricted) {
        if (info.key[0] != '\0' && gameType == info.key) return info.name;
    }
    for (const auto& [key, name] : kUniversal) {
        if (gameType == key) return name;
    }
    static std::string s_fallback;
    s_fallback = gameType;
    for (char& c : s_fallback) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s_fallback.c_str();
}

std::vector<std::string> RequiredMaps(const std::string& gameType) {
    std::vector<std::string> out;
    for (const TypeInfo& info : kRestricted) {
        if (info.key[0] != '\0' && gameType == info.key) {
            for (const char* map : info.maps) out.emplace_back(map);
            break;
        }
    }
    return out;
}

bool Supports(const std::string& scenario, const std::string& gameType) {
    if (gameType.empty()) return true;
    if (gameType == "firefight") return false;   // started from the Firefight screen, never as a multiplayer type
    if (!IsRestricted(gameType)) return true;
    const std::vector<std::string> maps = RequiredMaps(gameType);
    return std::find(maps.begin(), maps.end(), scenario) != maps.end();
}

std::string ConflictReason(const std::string& scenario, const std::string& mapName, const std::string& gameType) {
    if (Supports(scenario, gameType)) return {};
    const std::vector<std::string> maps = RequiredMaps(gameType);
    std::string good;
    for (size_t i = 0; i < maps.size(); ++i) {
        if (i) good += i + 1 == maps.size() ? " or " : ", ";
        // Scenario names are not player-facing; FriendlyScenario() is the caller's job, so pass the raw names
        // through and let the menu swap them for map names where it knows them.
        good += maps[i];
    }
    return std::string(Name(gameType)) + " isn't set up on " + mapName + ": pick " + good +
           ", or use a modded map (mods are never restricted).";
}

} // namespace MapGameTypes
} // namespace noble
