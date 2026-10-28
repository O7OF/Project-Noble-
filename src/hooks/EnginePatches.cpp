#include "EnginePatches.hpp"
#include "HookManager.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>
#include <cstring>

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


// Halo: Reach internal armor unlock verification function checks game mode and capg unlock tags.
// Detouring to return 1 ensures all customized Spartan armor pieces, helmets, shoulders, and chests
// are permitted when spawning in multiplayer/Forge instead of falling back to default Mark V[B].
typedef uint8_t(__fastcall* IsArmorItemUnlockedFn)(void* a1, void* a2, void* a3, void* a4);
static IsArmorItemUnlockedFn s_originalIsArmorItemUnlocked = nullptr;

static uint8_t __fastcall Hook_IsArmorItemUnlocked(void* /*a1*/, void* /*a2*/, void* /*a3*/, void* /*a4*/) {
    return 1;
}

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

    // 2. Armor permutation index cap patches (appearancecap::apply @ RVA 0x10545A and 0x10552F)
    // Allows armor permutation indices >= 5 / >= 32 without clamping to base Mark V[B].
    {
        static const uint8_t kExpectedCap1[7] = { 0x80, 0xFA, 0x05, 0x72, 0x1A, 0xB1, 0x01 };
        static const uint8_t kPatchedCap1[7]  = { 0x80, 0xFA, 0xFF, 0x72, 0x1A, 0xB1, 0x01 };
        uint8_t* addr1 = reinterpret_cast<uint8_t*>(baseAddress + 0x10545A);
        if (std::memcmp(addr1, kExpectedCap1, sizeof(kExpectedCap1)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(addr1, sizeof(kPatchedCap1), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(addr1, kPatchedCap1, sizeof(kPatchedCap1));
                FlushInstructionCache(GetCurrentProcess(), addr1, sizeof(kPatchedCap1));
                VirtualProtect(addr1, sizeof(kPatchedCap1), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Patched armor appearance cap 1 at RVA 0x10545A (0x05 -> 0xFF).");
            }
        }

        static const uint8_t kExpectedCap2[6] = { 0x41, 0x80, 0xF8, 0x20, 0x73, 0x21 };
        static const uint8_t kPatchedCap2[6]  = { 0x41, 0x80, 0xF8, 0xFF, 0x73, 0x21 };
        uint8_t* addr2 = reinterpret_cast<uint8_t*>(baseAddress + 0x10552F);
        if (std::memcmp(addr2, kExpectedCap2, sizeof(kExpectedCap2)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(addr2, sizeof(kPatchedCap2), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(addr2, kPatchedCap2, sizeof(kPatchedCap2));
                FlushInstructionCache(GetCurrentProcess(), addr2, sizeof(kPatchedCap2));
                VirtualProtect(addr2, sizeof(kPatchedCap2), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Patched armor appearance cap 2 at RVA 0x10552F (0x20 -> 0xFF).");
            }
        }
    }

    // 3. Unlock all armor customization items in multiplayer/Forge (RVA 0x2C7DCC)
    uintptr_t isArmorUnlockedAddr = SigScanner::FindPattern(
        baseAddress, modInfo.SizeOfImage,
        "48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56 48 83 EC 20 33 DB 41 8A E8"
    );
    if (!isArmorUnlockedAddr) {
        uintptr_t fallback = baseAddress + 0x2C7DCC;
        const uint8_t* p = reinterpret_cast<const uint8_t*>(fallback);
        if (p[0] == 0x48 && p[1] == 0x8B && p[2] == 0xC4 && p[3] == 0x48) {
            isArmorUnlockedAddr = fallback;
            NOBLE_INFO("EnginePatches: Located IsArmorItemUnlocked via static RVA 0x2C7DCC.");
        }
    }
    if (isArmorUnlockedAddr) {
        if (HookManager::InstallHook(
            reinterpret_cast<void*>(isArmorUnlockedAddr),
            reinterpret_cast<void*>(&Hook_IsArmorItemUnlocked),
            reinterpret_cast<void**>(&s_originalIsArmorItemUnlocked),
            "haloreach_is_armor_unlocked"
        )) {
            NOBLE_INFO("EnginePatches: Successfully hooked IsArmorItemUnlocked at 0x{:X} to unlock all Spartan armor.", isArmorUnlockedAddr);
        } else {
            NOBLE_ERROR("EnginePatches: Failed to hook IsArmorItemUnlocked at 0x{:X}!", isArmorUnlockedAddr);
        }
    } else {
        NOBLE_WARN("EnginePatches: Could not locate IsArmorItemUnlocked in haloreach.dll.");
    }

    s_isApplied = true;
    return true;
}

} // namespace noble
