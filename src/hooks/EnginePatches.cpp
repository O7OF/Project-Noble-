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

[[maybe_unused]] static uint8_t __fastcall Hook_IsArmorItemUnlocked(void* /*a1*/, void* /*a2*/, void* /*a3*/, void* /*a4*/) {
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

    // 2. Armor appearance builder stack-overflow patch (RVA 0x105324)
    // When an invalid or out-of-range armor permutation index is present (e.g. GRD/MCC/Season 8 helmets >= 64),
    // haloreach.dll at 0x1052D0..0x10531A sets the byte to 0 and flags al = 1.
    // If al != 0, it falls into 0x10532A - a bugged Bungie composite permutation builder with a 16-element stack array.
    // When elements exceed 16, it writes index -1 to stack offset [rbp+0x18], corrupting the item count to a huge number,
    // which then loops past the stack boundary and crashes with 0xC0000005 at haloreach.dll+0x105597.
    // Patching 0x105324 (0F 84 97 02 00 00, je 0x1055c1) to an unconditional jmp (E9 98 02 00 00 90, jmp 0x1055c1; nop)
    // cleanly skips the bugged fallback builder and exits safely, preventing the crash permanently.
    {
        static const uint8_t kExpectedFallbackJe[6] = { 0x0F, 0x84, 0x97, 0x02, 0x00, 0x00 };
        static const uint8_t kPatchedFallbackJmp[6] = { 0xE9, 0x98, 0x02, 0x00, 0x00, 0x90 };
        uint8_t* addr = reinterpret_cast<uint8_t*>(baseAddress + 0x105324);
        if (std::memcmp(addr, kExpectedFallbackJe, sizeof(kExpectedFallbackJe)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(addr, sizeof(kPatchedFallbackJmp), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(addr, kPatchedFallbackJmp, sizeof(kPatchedFallbackJmp));
                FlushInstructionCache(GetCurrentProcess(), addr, sizeof(kPatchedFallbackJmp));
                VirtualProtect(addr, sizeof(kPatchedFallbackJmp), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Patched armor composite fallback jump at RVA 0x105324 (je -> jmp 0x1055c1). Crash at 0x105597 resolved.");
            } else {
                NOBLE_WARN("EnginePatches: Failed to VirtualProtect armor composite fallback jump at RVA 0x105324.");
            }
        } else {
            NOBLE_INFO("EnginePatches: Armor composite fallback at RVA 0x105324 already patched or different opcode.");
        }
    }

    // 3. Remote Elite armor set replication patch (RVA 0x2EDD45)
    // haloreach.dll at 0x2EDD45 passes species=0 (Spartan) and slot=7 when writing profile[0x8C] (Elite armor set)
    // to the player object via 0x2C8070. Because 0x2C8070 only allows slot <= 3 for Elites and slot <= 7 for Spartans,
    // passing slot 7 wrote the value into the Spartan array [player + 0xAD] while leaving [player + 0xAE + 0] (the replicated
    // Elite armor set) as 0 (Elite Minor) for every other player in the network session.
    // Patching 0x2EDD45 (89 75 C0 C7 45 C4 07 00 00 00) to (C7 45 C0 01 00 00 00 89 75 C4) sets species=1 (Elite)
    // and slot=0, allowing 0x2C8070 to populate [player + 0xAE], replicating the equipped Elite armor set to all players.
    {
        static const uint8_t kExpectedEliteSetWrite[10] = { 0x89, 0x75, 0xC0, 0xC7, 0x45, 0xC4, 0x07, 0x00, 0x00, 0x00 };
        static const uint8_t kPatchedEliteSetWrite[10]  = { 0xC7, 0x45, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x89, 0x75, 0xC4 };
        uint8_t* addr = reinterpret_cast<uint8_t*>(baseAddress + 0x2EDD45);
        if (std::memcmp(addr, kExpectedEliteSetWrite, sizeof(kExpectedEliteSetWrite)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(addr, sizeof(kPatchedEliteSetWrite), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(addr, kPatchedEliteSetWrite, sizeof(kPatchedEliteSetWrite));
                FlushInstructionCache(GetCurrentProcess(), addr, sizeof(kPatchedEliteSetWrite));
                VirtualProtect(addr, sizeof(kPatchedEliteSetWrite), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Patched Elite armor set replication at RVA 0x2EDD45 (species=1, slot=0). Remote Elites now display equipped set.");
            } else {
                NOBLE_WARN("EnginePatches: Failed to VirtualProtect Elite armor set write at RVA 0x2EDD45.");
            }
        } else if (std::memcmp(addr, kPatchedEliteSetWrite, sizeof(kPatchedEliteSetWrite)) == 0) {
            NOBLE_INFO("EnginePatches: Elite armor set replication at RVA 0x2EDD45 already patched.");
        } else {
            NOBLE_WARN("EnginePatches: Unexpected bytes at RVA 0x2EDD45; Elite armor set replication patch skipped.");
        }
    }

    // 3. "IsArmorItemUnlocked" hook: TURNED OFF (Tyler/Claude 2026-10-28).
    // The signature below is a generic function prologue (mov rax,rsp / save rbx,rbp,rsi,rdi / push r14 / sub rsp,20h /
    // xor ebx,ebx / mov bpl,r8b), so FindPattern returned the FIRST function that starts this way: haloreach RVA 0x105BDC,
    // inside the player appearance builder (0x1055EC..), not 0x2C7DCC. Replacing that function with "return 1" stopped
    // the engine building player appearance, and character models became invisible in matches. Armory pieces reach the
    // game without this hook (profile item ids, ReachHost slot 34). If an unlock check is still needed, hook the exact
    // RVA after verifying it in a disassembler, check more bytes than a common prologue, and call the original.
    NOBLE_INFO("EnginePatches: IsArmorItemUnlocked hook is off (its signature matched the appearance builder at RVA 0x105BDC).");
#if 0
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
#endif

    s_isApplied = true;
    return true;
}

} // namespace noble
