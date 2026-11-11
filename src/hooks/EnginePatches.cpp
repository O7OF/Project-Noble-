#include "EnginePatches.hpp"
#include "HookManager.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include "network/NetworkTransport.hpp"
#include <psapi.h>
#include <atomic>
#include <cstring>
#include <cwchar>
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

// ai_place (RVA 0x6C2B10): places an AI squad from scenario definitions.
// In distributed multiplayer/Forge mods with AI ([game_globals + 0x10] == 3, c_simulation_distributed_world),
// scenario scripts execute on both host and clients while the host ALSO replicates created AI bipeds to guests,
// producing duplicate non-replicated clones if guests run ai_place locally.
// By contrast, Campaign ([+0x10] == 1) and Firefight/Survival ([+0x110EC] != 0) run deterministic synchronous
// lockstep (c_simulation_synchronous_world): the host does NOT replicate AI entities, and BOTH host and guest
// MUST execute every scenario (ai_place ...) call on the exact same tick or the engine raises
// c_network_session_parameter_synchronous_out_of_sync (network_coop_oos_alert) right after cutscenes / wave spawns.
using AiPlaceFn = void(__fastcall*)(uint32_t aiValue, int32_t count, int32_t flag);
static AiPlaceFn s_originalAiPlace = nullptr;

static bool IsCliSynchronousMode() {
    static const bool s_syncMode = []() {
        const wchar_t* cmd = GetCommandLineW();
        if (!cmd) return false;
        std::wstring lower(cmd);
        for (wchar_t& c : lower) {
            if (c >= L'A' && c <= L'Z') c = static_cast<wchar_t>(c - L'A' + L'a');
        }
        return lower.find(L"-mode campaign") != std::wstring::npos ||
               lower.find(L"-mode \"campaign\"") != std::wstring::npos ||
               lower.find(L"-mode firefight") != std::wstring::npos ||
               lower.find(L"-mode \"firefight\"") != std::wstring::npos;
    }();
    return s_syncMode;
}

static bool IsDistributedMultiplayerGameSEH(uintptr_t base) {
    if (!base) return false;
    __try {
        // haloreach+0x058B38 (game_is_multiplayer), +0x058C04 (game_is_survival), +0x0586C4 (game_is_cooperative)
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(
            __readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        if (!tlsArray) return false;
        const uint8_t* gameGlobals = *reinterpret_cast<const uint8_t* const*>(tlsArray + 0x48);
        if (!gameGlobals) return false;
        const uint8_t gameMode = gameGlobals[0x10];          // 1 = campaign, 2 = ui_shell, 3 = multiplayer
        const uint8_t isSurvival = gameGlobals[0x110EC];     // non-zero in Firefight / Survival
        const uint8_t isCooperative = gameGlobals[0x1EAB0];  // non-zero in Co-Op sessions
        return (gameMode == 3) && (isSurvival == 0) && (isCooperative == 0);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static void __fastcall Hook_AiPlace(uint32_t aiValue, int32_t count, int32_t flag) {
    if (NetworkTransport::Get().IsActive() &&
        !NetworkTransport::Get().IsHost() &&
        !IsCliSynchronousMode() &&
        IsDistributedMultiplayerGameSEH(s_baseAddress)) {
        NOBLE_DEBUG("EnginePatches: Suppressed client-side ai_place(0x{:08X}, {}, {}) in distributed multiplayer to prevent duplicate AI clones.",
                    aiValue, count, flag);
        return;
    }

    if (s_originalAiPlace) {
        s_originalAiPlace(aiValue, count, flag);
    }
}

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

    // 3b. Elite armor effects: force r8b = 1 when applying effect (RVA 0x105837)
    // At RVA 0x105837, the engine moves dil into r8b (44 8A C7). For Elites, dil is 0 (computed from
    // record[0] bit 1). When apply_selection (0x105BDC) is called at 0x10585F, it skips applying the
    // effect tag if r8b (bpl) is 0. Patching 44 8A C7 -> 41 B0 01 (mov r8b, 1) ensures armor effects
    // always attach and render on Elites as well as Spartans.
    {
        static const uint8_t kExpectedEliteFxR8b[3] = { 0x44, 0x8A, 0xC7 };
        static const uint8_t kPatchedEliteFxR8b[3]  = { 0x41, 0xB0, 0x01 };
        uint8_t* addr = reinterpret_cast<uint8_t*>(baseAddress + 0x105837);
        if (std::memcmp(addr, kExpectedEliteFxR8b, sizeof(kExpectedEliteFxR8b)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(addr, sizeof(kPatchedEliteFxR8b), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(addr, kPatchedEliteFxR8b, sizeof(kPatchedEliteFxR8b));
                FlushInstructionCache(GetCurrentProcess(), addr, sizeof(kPatchedEliteFxR8b));
                VirtualProtect(addr, sizeof(kPatchedEliteFxR8b), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Patched Elite armor effects r8b flag at RVA 0x105837 (mov r8b, 1).");
            } else {
                NOBLE_WARN("EnginePatches: Failed to VirtualProtect Elite armor effects r8b at RVA 0x105837.");
            }
        } else if (std::memcmp(addr, kPatchedEliteFxR8b, sizeof(kPatchedEliteFxR8b)) == 0) {
            NOBLE_INFO("EnginePatches: Elite armor effects r8b at RVA 0x105837 already patched.");
        } else {
            NOBLE_WARN("EnginePatches: Unexpected bytes at RVA 0x105837; Elite armor effects r8b patch skipped.");
        }
    }

    // 3c. Extended Spartan Visors (30 finishes): bypass 0x3B2F4 translation clamp at RVA 0x2EDC5E
    // At haloreach+0x2EDC5A, the engine reads profile[+0x70] (MCC item ID) and calls 0x3B2F4.
    // Stock 0x3B2F4 only maps items 157..161 to 0..4 (returning -1 for anything else).
    // By replacing 'call 0x3B2F4; mov edx, eax' (7 bytes: E8 91 D6 D4 FF 8B D0) with
    // 'lea edx, [rcx - 0x9D]; nop' (7 bytes: 8D 91 63 FF FF FF 90), profile items 157..186
    // are converted directly to indices 0..29 without stock clamping, so player_set_visor (0x2C7F10)
    // receives and stores all 30 visor indices into [player + 0x98].
    {
        static const uint8_t kExpectedVisorCall[7] = { 0xE8, 0x91, 0xD6, 0xD4, 0xFF, 0x8B, 0xD0 };
        static const uint8_t kPatchedVisorCall[7]  = { 0x8D, 0x91, 0x63, 0xFF, 0xFF, 0xFF, 0x90 };
        uint8_t* addr = reinterpret_cast<uint8_t*>(baseAddress + 0x2EDC5E);
        if (std::memcmp(addr, kExpectedVisorCall, sizeof(kExpectedVisorCall)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(addr, sizeof(kPatchedVisorCall), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(addr, kPatchedVisorCall, sizeof(kPatchedVisorCall));
                FlushInstructionCache(GetCurrentProcess(), addr, sizeof(kPatchedVisorCall));
                VirtualProtect(addr, sizeof(kPatchedVisorCall), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Patched visor translation at RVA 0x2EDC5E (direct 30-finish mapping).");
            } else {
                NOBLE_WARN("EnginePatches: Failed to VirtualProtect visor translation at RVA 0x2EDC5E.");
            }
        } else if (std::memcmp(addr, kPatchedVisorCall, sizeof(kPatchedVisorCall)) == 0) {
            NOBLE_INFO("EnginePatches: Visor translation at RVA 0x2EDC5E already patched.");
        } else {
            NOBLE_WARN("EnginePatches: Unexpected bytes at RVA 0x2EDC5E; visor translation patch skipped.");
        }
    }

    // 3d. IsArmorItemUnlocked patch at static RVA 0x2C7DCC
    // In retail Reach, the network appearance sync block builder (0x2C7A8C) calls 0x2C7DCC to check
    // if items (including visor index at [player+0x98] and Elite armor pieces at [player+0xAE]) are
    // unlocked. If 0x2C7DCC returns 0, the engine wipes the visor back to 0 (default) and drops custom
    // pieces before sending them over the network.
    // Patching 0x2C7DCC to 'mov eax, 1; ret; nop' (7 bytes: B8 01 00 00 00 C3 90) permanently unlocks
    // all customization items and ensures all 30 visors and all Elite armor sets replicate across network peers.
    {
        static const uint8_t kExpectedUnlockPrologue[7] = { 0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x08 };
        static const uint8_t kPatchedUnlockPrologue[7]  = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3, 0x90 };
        uint8_t* addr = reinterpret_cast<uint8_t*>(baseAddress + 0x2C7DCC);
        if (std::memcmp(addr, kExpectedUnlockPrologue, sizeof(kExpectedUnlockPrologue)) == 0) {
            DWORD oldProt = 0;
            if (VirtualProtect(addr, sizeof(kPatchedUnlockPrologue), PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(addr, kPatchedUnlockPrologue, sizeof(kPatchedUnlockPrologue));
                FlushInstructionCache(GetCurrentProcess(), addr, sizeof(kPatchedUnlockPrologue));
                VirtualProtect(addr, sizeof(kPatchedUnlockPrologue), oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Patched IsArmorItemUnlocked at RVA 0x2C7DCC (mov eax, 1; ret). Full network replication enabled.");
            } else {
                NOBLE_WARN("EnginePatches: Failed to VirtualProtect IsArmorItemUnlocked at RVA 0x2C7DCC.");
            }
        } else if (std::memcmp(addr, kPatchedUnlockPrologue, sizeof(kPatchedUnlockPrologue)) == 0) {
            NOBLE_INFO("EnginePatches: IsArmorItemUnlocked at RVA 0x2C7DCC already patched.");
        } else {
            NOBLE_WARN("EnginePatches: Unexpected bytes at RVA 0x2C7DCC; IsArmorItemUnlocked patch skipped.");
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
    // 5. Membership update message decoder: count bounds (RVAs 0x4271D9 / 0x4271E8). Security fix, every build.
    // The message (registered at message table +0x680, size 0x2550, encode 0x42675C, decode 0x427140) holds 17 peer
    // entries (0x180) and 16 player entries (0xB8). The stock decoder reads both counts as 5 bits but only rejects a
    // peer count above 0x22 and a player count above 0x20, then writes that many entries into the 0x2550 buffer, so a
    // modified peer could overwrite memory past it. Tighten both limits to the buffer's real capacity (17 / 16); a
    // larger count takes the decoder's own reject path (0x4275B0: message invalid). Same instruction lengths.
    //   0x4271D9: 66 83 7F 14 22   cmp word ptr [rdi+0x14], 0x22  ->  0x11
    //   0x4271E8: cmp ax, r15w (r15 = 0x20, 4 bytes, either encoding)  ->  66 83 F8 10  cmp ax, 0x10
    // See docs/limits-players-loops.md "#32 is the membership update message". 32-player mode will raise both again.
    {
        static const uint8_t kExpectedPeerCount[5] = { 0x66, 0x83, 0x7F, 0x14, 0x22 };
        static const uint8_t kPatchedPeerCount[5]  = { 0x66, 0x83, 0x7F, 0x14, 0x11 };
        static const uint8_t kExpectedPlayerCountA[4] = { 0x66, 0x41, 0x3B, 0xC7 };   // cmp ax, r15w (3B /r)
        static const uint8_t kExpectedPlayerCountB[4] = { 0x66, 0x44, 0x39, 0xF8 };   // cmp ax, r15w (39 /r)
        static const uint8_t kPatchedPlayerCount[4]  = { 0x66, 0x83, 0xF8, 0x10 };
        uint8_t* peerAt = reinterpret_cast<uint8_t*>(baseAddress + 0x4271D9);
        uint8_t* playerAt = reinterpret_cast<uint8_t*>(baseAddress + 0x4271E8);
        const bool peerStock = std::memcmp(peerAt, kExpectedPeerCount, sizeof(kExpectedPeerCount)) == 0;
        const bool playerStock = std::memcmp(playerAt, kExpectedPlayerCountA, sizeof(kExpectedPlayerCountA)) == 0 ||
                                 std::memcmp(playerAt, kExpectedPlayerCountB, sizeof(kExpectedPlayerCountB)) == 0;
        if (peerStock && playerStock) {   // both or neither
            DWORD oldProt = 0;
            if (VirtualProtect(peerAt, 0x13, PAGE_EXECUTE_READWRITE, &oldProt)) {
                std::memcpy(peerAt, kPatchedPeerCount, sizeof(kPatchedPeerCount));
                std::memcpy(playerAt, kPatchedPlayerCount, sizeof(kPatchedPlayerCount));
                FlushInstructionCache(GetCurrentProcess(), peerAt, 0x13);
                VirtualProtect(peerAt, 0x13, oldProt, &oldProt);
                NOBLE_INFO("EnginePatches: Membership update decoder now rejects more than 17 peer / 16 player entries "
                           "(RVAs 0x4271D9 / 0x4271E8; stock allowed 34 / 32 into a 17 / 16 buffer).");
            } else {
                NOBLE_WARN("EnginePatches: Failed to VirtualProtect the membership update decoder at RVA 0x4271D9.");
            }
        } else if (std::memcmp(peerAt, kPatchedPeerCount, sizeof(kPatchedPeerCount)) == 0 &&
                   std::memcmp(playerAt, kPatchedPlayerCount, sizeof(kPatchedPlayerCount)) == 0) {
            NOBLE_INFO("EnginePatches: Membership update decoder bounds already patched.");
        } else {
            NOBLE_WARN("EnginePatches: Unexpected bytes in the membership update decoder (RVA 0x4271D9 / 0x4271E8); "
                       "count bounds patch skipped.");
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

    // 5. Hook ai_place (RVA 0x6C2B10) to suppress client-side AI script spawning in multiplayer.
    // In retail Reach, scenario scripts run on both host and client. In multiplayer mods with AI,
    // both host and guest execute (ai_place squad). Because the host also replicates placed actors
    // over the network, guests receive duplicate cloned AI. Hooking ai_place and returning early
    // on non-host clients ensures AI is spawned authoritatively by the host only and synchronized cleanly.
    {
        constexpr uintptr_t kAiPlaceRva = 0x6C2B10;
        uintptr_t aiPlaceAddr = 0;
        uint8_t* staticAddr = reinterpret_cast<uint8_t*>(baseAddress + kAiPlaceRva);
        static const uint8_t kAiPlaceHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x48, 0x8B, 0xEC, 0x48, 0x83 };
        if (std::memcmp(staticAddr, kAiPlaceHead, sizeof(kAiPlaceHead)) == 0) {
            aiPlaceAddr = reinterpret_cast<uintptr_t>(staticAddr);
        } else {
            aiPlaceAddr = SigScanner::FindPattern(
                baseAddress,
                modInfo.SizeOfImage,
                "48 89 5C 24 18 48 89 7C 24 20 55 48 8B EC 48 83"
            );
        }

        if (aiPlaceAddr) {
            if (HookManager::InstallHook(
                reinterpret_cast<void*>(aiPlaceAddr),
                reinterpret_cast<void*>(&Hook_AiPlace),
                reinterpret_cast<void**>(&s_originalAiPlace),
                "haloreach_ai_place"
            )) {
                NOBLE_INFO("EnginePatches: Successfully hooked ai_place at 0x{:X} for multiplayer AI clone suppression.", aiPlaceAddr);
            } else {
                NOBLE_ERROR("EnginePatches: Failed to hook ai_place at 0x{:X}!", aiPlaceAddr);
            }
        } else {
            NOBLE_WARN("EnginePatches: Could not locate ai_place in haloreach.dll.");
        }
    }

    s_isApplied = true;
    return true;
}

} // namespace noble
