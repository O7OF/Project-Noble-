#include "EnginePatches.hpp"
#include "HookManager.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>

namespace noble {

bool EnginePatches::s_isApplied = false;

// haloreach.dll has an internal whitespace binary search function at RVA 0x362E8
// (signature: 33 D2 44 8B C2 44 8D 4A 0B 4B 8D 04 01 48 D1 E8).
// When checking characters below 0x09 (such as \u0007 property token delimiters),
// an unsigned comparison bug (cmpq %r9, %r8; jbe ...) treats mid-1 (-1 / 0xFFFFFFFFFFFFFFFF)
// as greater than 0, causing mid to become 0x7FFFFFFFFFFFFFFF on the next iteration and
// triggering an access violation (0xC0000005) at haloreach.dll + 0x362FF.
typedef uint8_t(__fastcall* IsWhitespaceFn)(uint16_t ch);
static IsWhitespaceFn s_originalIsWhitespace = nullptr;

static uint8_t __fastcall Hook_IsWhitespace(uint16_t ch) {
    if (ch < 0x09) {
        return 0; // Characters below 0x09 are never whitespace; prevents binary search underflow crash
    }
    return s_originalIsWhitespace(ch);
}

bool EnginePatches::Apply(HMODULE hReachDll) {
    if (!hReachDll) {
        NOBLE_ERROR("EnginePatches: Invalid haloreach.dll handle provided.");
        return false;
    }

    if (s_isApplied) {
        return true;
    }

    NOBLE_INFO("EnginePatches: Applying haloreach.dll stability and crash patches...");

    MODULEINFO modInfo{};
    if (!GetModuleInformation(GetCurrentProcess(), hReachDll, &modInfo, sizeof(modInfo))) {
        NOBLE_ERROR("EnginePatches: Failed to query haloreach.dll module bounds.");
        return false;
    }

    uintptr_t baseAddress = reinterpret_cast<uintptr_t>(hReachDll);

    // 1. Whitespace binary search underflow patch (crashes Spire, SWAT, text formatting with \u0007)
    // Signature: 33 D2 44 8B C2 44 8D 4A 0B 4B 8D 04 01 48 D1 E8
    uintptr_t isWhitespaceAddr = SigScanner::FindPattern(
        baseAddress, modInfo.SizeOfImage,
        "33 D2 44 8B C2 44 8D 4A 0B 4B 8D 04 01 48 D1 E8"
    );

    if (!isWhitespaceAddr) {
        // Fallback to known static RVA 0x362E8
        uintptr_t fallback = baseAddress + 0x362E8;
        const uint8_t* p = reinterpret_cast<const uint8_t*>(fallback);
        if (p[0] == 0x33 && p[1] == 0xD2 && p[2] == 0x44 && p[3] == 0x8B) {
            isWhitespaceAddr = fallback;
            NOBLE_INFO("EnginePatches: Located IsWhitespace function via static RVA offset 0x362E8.");
        }
    }

    if (isWhitespaceAddr) {
        if (HookManager::InstallHook(
            reinterpret_cast<void*>(isWhitespaceAddr),
            reinterpret_cast<void*>(&Hook_IsWhitespace),
            reinterpret_cast<void**>(&s_originalIsWhitespace),
            "haloreach_is_whitespace"
        )) {
            NOBLE_INFO("EnginePatches: Successfully hooked IsWhitespace at 0x{:X} to patch underflow crash.", isWhitespaceAddr);
        } else {
            NOBLE_ERROR("EnginePatches: Failed to install IsWhitespace detour at 0x{:X}!", isWhitespaceAddr);
        }
    } else {
        NOBLE_WARN("EnginePatches: Could not locate IsWhitespace function in haloreach.dll.");
    }

    s_isApplied = true;
    return true;
}

} // namespace noble
