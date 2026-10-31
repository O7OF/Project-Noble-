#include "EnginePatches.hpp"
#include "HookManager.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>
#include <atomic>
#include <cstring>
#include <intrin.h>

namespace noble {

bool EnginePatches::s_isApplied = false;
static uintptr_t s_baseAddress = 0;

// Appearance slots (verified against haloreach.dll 0x2C807C / 0x2EDA08, 2026-10-30):
//   species 0 = Spartan, slots 0..7 = helmet, L shoulder, R shoulder, chest, wrist, utility, knees, ARMOR EFFECT
//   species 1 = Elite,   slots 0..3 = ARMOR SET, ARMOR EFFECT, voice, unused
// The profile -> slot copy at 0x2EDA08 already passes the right species/slot for every field. Do NOT rewrite
// species/slot here: profile +0x8C is the Elite armor SET (slot 7 of the generic appearance record) and
// profile +0x68 is the Spartan armor EFFECT (Elite slot 0). Swapping them makes an armor effect choose the
// Elite variant and leaves Elites at Minor (the bug this file used to introduce).

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

// haloreach+0x5DF80: player_control_update(playerIndex, inputUserIndex, dt1, dt2, outA5, controlOut).
// At +0x60896..0x60952 the controller Machinima combo sets bit 10 (0x400) in [controlOut + 0x28], which
// the caller (0x61A38, at +0x625CE) consumes to toggle [player_control_globals + 0x1E3] (Alert Carry / Lower Weapon)
// and set unit control bit 21 (_unit_control_alert_carry_bit). Because haloreach+0x3C724 never maps any of the
// 66 PC keyboard slots to abstract buttons 0x10 / 0x19 / 0x1A, keyboard players could never lower their weapon.
// Hooking 0x5DF80 lets Noble's bindable "Lower weapon" action pulse bit 10 on key press.
using PlayerControlUpdateFn = void(__fastcall*)(int32_t playerIndex, int32_t inputUserIndex, float a3, float a4, void* a5, void* controlOut);
static PlayerControlUpdateFn s_originalPlayerControlUpdate = nullptr;
static std::atomic<bool> s_lowerWeaponTogglePending{false};

void EnginePatches::RequestLowerWeaponToggle() {
    s_lowerWeaponTogglePending.store(true);
}

static void __fastcall Hook_PlayerControlUpdate(int32_t playerIndex, int32_t inputUserIndex, float a3, float a4, void* a5, void* controlOut) {
    if (s_originalPlayerControlUpdate) {
        s_originalPlayerControlUpdate(playerIndex, inputUserIndex, a3, a4, a5, controlOut);
    }
    if (playerIndex == 0 && controlOut != nullptr && s_lowerWeaponTogglePending.exchange(false)) {
        auto* actionFlags = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(controlOut) + 0x28);
        *actionFlags |= (1u << 10);
    }
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
    s_baseAddress = baseAddress;

    // 0. No PlayerSetAppearanceSlot detour (0x2C807C) and no 0x2EDD5C rewrite.
    // The engine's own profile copy already uses the correct species/slot for each field (see the note at
    // the top of this file). The old detour/rewrite moved the Elite armor SET (profile +0x8C, Spartan slot 7)
    // into the Elite slot 0, where profile +0x68 (the Spartan armor EFFECT) overwrote it - which is exactly
    // "Elite always spawns as Minor" and "armor effects pick the Elite variant". Both were removed 2026-10-30.

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

    // 3. Elite armor effects: pmcg list 0x8C -> 0x80 (RVA 0x10580C)
    // The appearance builder (0x1055EC) picks the armor effect for Elites from pmcg list at +0x8C and for
    // Spartans from the list at +0x80 (record[0x19] / record[0x18] respectively). Both lists hold the same
    // seven entries in the pack (default_armor, legendary_flame, eternal_flame, heart_attack, pestilence,
    // storm, halo_bday, all changing region "armor_fx"), but the map's own 0x8C list is empty in retail
    // match maps, so Elite effects are silently skipped. Pointing the Elite branch at 0x80 gives Elites the
    // identical effect set and leaves the Elite armor set (record[0x17] / profile +0x8C) completely alone.
    // Verified bytes at 0x10580C: B9 8C 00 00 00 (mov ecx,0x8C). The 0x80 branch follows at 0x10581B.
    {
        static const uint8_t kExpectedEliteFxList[5] = { 0xB9, 0x8C, 0x00, 0x00, 0x00 };
        static const uint8_t kPatchedEliteFxList[5]  = { 0xB9, 0x80, 0x00, 0x00, 0x00 };
        uint8_t* addr = reinterpret_cast<uint8_t*>(baseAddress + 0x10580C);
        if (std::memcmp(addr, kExpectedEliteFxList, sizeof(kExpectedEliteFxList)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(addr, sizeof(kPatchedEliteFxList), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(addr, kPatchedEliteFxList, sizeof(kPatchedEliteFxList));
                FlushInstructionCache(GetCurrentProcess(), addr, sizeof(kPatchedEliteFxList));
                VirtualProtect(addr, sizeof(kPatchedEliteFxList), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Patched Elite armor effects tag list at RVA 0x10580C (0x8C -> 0x80). In-game Elite armor effects enabled.");
            } else {
                NOBLE_WARN("EnginePatches: Failed to VirtualProtect Elite armor effects tag list at RVA 0x10580C.");
            }
        } else if (std::memcmp(addr, kPatchedEliteFxList, sizeof(kPatchedEliteFxList)) == 0) {
            NOBLE_INFO("EnginePatches: Elite armor effects tag list at RVA 0x10580C already patched.");
        } else {
            NOBLE_WARN("EnginePatches: Unexpected bytes at RVA 0x10580C; Elite armor effects tag list patch skipped.");
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

    // 4. Lower weapon (Alert Carry) availability & keybind hook (RVAs 0x5D6BA and 0x5DF80)
    // haloreach+0x5D688 (player_control_machinima_available) checks [game_globals + 0x12] != 3 at RVA 0x5D6BA
    // (80 79 12 03 74 02: cmp byte ptr [rcx+12h], 3; je 0x5D6C2). Because synthesized launch blocks set
    // launch_block[4] = 3, [rcx+12h] is 3 in matches and 0x5D688 would return 0, skipping the Alert Carry
    // toggle at 0x625B7. Patching 74 02 -> 90 90 allows Alert Carry in all in-game sessions, and hooking
    // player_control_update at RVA 0x5DF80 pulses bit 10 of [controlOut + 0x28] when the bound key is pressed.
    {
        static const uint8_t kExpectedMachinimaCheck[6] = { 0x80, 0x79, 0x12, 0x03, 0x74, 0x02 };
        static const uint8_t kPatchedMachinimaCheck[6]  = { 0x80, 0x79, 0x12, 0x03, 0x90, 0x90 };
        uint8_t* checkAddr = reinterpret_cast<uint8_t*>(baseAddress + 0x5D6BA);
        if (std::memcmp(checkAddr, kExpectedMachinimaCheck, sizeof(kExpectedMachinimaCheck)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(checkAddr, sizeof(kPatchedMachinimaCheck), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(checkAddr, kPatchedMachinimaCheck, sizeof(kPatchedMachinimaCheck));
                FlushInstructionCache(GetCurrentProcess(), checkAddr, sizeof(kPatchedMachinimaCheck));
                VirtualProtect(checkAddr, sizeof(kPatchedMachinimaCheck), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Enabled in-game Alert Carry (Lower weapon) at RVA 0x5D6BA.");
            }
        }
        static const uint8_t kExpectedControlPrologue[9] = { 0x48, 0x8B, 0xC4, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54 };
        void* controlTarget = reinterpret_cast<void*>(baseAddress + 0x5DF80);
        if (std::memcmp(controlTarget, kExpectedControlPrologue, sizeof(kExpectedControlPrologue)) == 0) {
            if (HookManager::InstallHook(
                controlTarget,
                reinterpret_cast<void*>(&Hook_PlayerControlUpdate),
                reinterpret_cast<void**>(&s_originalPlayerControlUpdate),
                "haloreach_player_control_update"
            )) {
                NOBLE_INFO("EnginePatches: Hooked PlayerControlUpdate at RVA 0x5DF80 for Lower weapon keybind.");
            } else {
                NOBLE_WARN("EnginePatches: Could not install PlayerControlUpdate detour at RVA 0x5DF80.");
            }
        } else {
            NOBLE_WARN("EnginePatches: Unexpected prologue at RVA 0x5DF80; Lower weapon keybind hook skipped.");
        }
    }
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
