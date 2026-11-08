#include "ForgePatches.hpp"
#include "SigScanner.hpp"
#include "engine/SharedTagCache.hpp"
#include "common/Logger.hpp"
#include <psapi.h>

namespace noble {

bool ForgePatches::s_isApplied = false;
bool ForgePatches::s_unlimitedBudget = true;
bool ForgePatches::s_unlockedRotation = true;

[[maybe_unused]] static bool WriteProtectedMemory(void* destination, const void* source, size_t size) {
    DWORD oldProtect = 0;
    if (!VirtualProtect(destination, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    memcpy(destination, source, size);
    VirtualProtect(destination, size, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), destination, size);
    return true;
}

bool ForgePatches::PlaceObject(const std::string& tagPath, int tagIndex) {
    NOBLE_INFO("ForgePatches: placement request for '{}' (tagIndex: {})", tagPath, tagIndex);
    SharedTagCache::QueueSpawn(tagIndex >= 0 ? static_cast<uint32_t>(tagIndex) : 0xFFFFFFFF, tagPath);
    return true;
}

bool ForgePatches::Apply(HMODULE hReachDll) {
    if (!hReachDll) {
        NOBLE_ERROR("ForgePatches: Invalid haloreach.dll handle provided.");
        return false;
    }

    uintptr_t baseAddress = reinterpret_cast<uintptr_t>(hReachDll);
    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hReachDll, &modInfo, sizeof(modInfo))) {
        NOBLE_ERROR("ForgePatches: Failed to query haloreach.dll module bounds.");
        return false;
    }

    NOBLE_INFO("ForgePatches: Applying haloreach.dll Forge patches (AI spawning)...");

    // 1. Forge Edit Mode Abort Check (RVA 0x692CA5)
    // In retail haloreach.dll, when an entity is spawned, the engine checks cmp byte ptr [rax+1], 0
    // and aborts if the player is not actively holding/grabbing an object in Monitor edit mode
    // (0F 84 9A 00 00 00: jz +0x9A directly to exit). NOPing this 6-byte jump allows spawning
    // anytime directly from custom menus or hotkeys!
    {
        static const uint8_t kExpectedEditJz[6] = { 0x0F, 0x84, 0x9A, 0x00, 0x00, 0x00 };
        static const uint8_t kNopPatch[6]       = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        constexpr uintptr_t kStaticEditRva = 0x692CA5;

        uint8_t* patchAddr = nullptr;
        uint8_t* rvaAddr = reinterpret_cast<uint8_t*>(baseAddress + kStaticEditRva);
        if (std::memcmp(rvaAddr, kExpectedEditJz, sizeof(kExpectedEditJz)) == 0) {
            patchAddr = rvaAddr;
            NOBLE_INFO("ForgePatches: Located Forge edit-mode check at static RVA 0x{:X}.", kStaticEditRva);
        } else if (std::memcmp(rvaAddr, kNopPatch, sizeof(kNopPatch)) == 0) {
            NOBLE_INFO("ForgePatches: Forge edit-mode check at RVA 0x{:X} is already patched.", kStaticEditRva);
        } else {
            uintptr_t match = SigScanner::FindPattern(
                baseAddress,
                modInfo.SizeOfImage,
                "80 78 01 00 0F 84 9A 00 00 00 33 D2"
            );
            if (match != 0) {
                patchAddr = reinterpret_cast<uint8_t*>(match + 4);
                NOBLE_INFO("ForgePatches: Located Forge edit-mode check via pattern scan at RVA 0x{:X}.", match - baseAddress + 4);
            }
        }

        if (patchAddr) {
            if (WriteProtectedMemory(patchAddr, kNopPatch, sizeof(kNopPatch))) {
                NOBLE_INFO("ForgePatches: Successfully patched Forge edit-mode check (jz -> 6x NOP).");
            } else {
                NOBLE_ERROR("ForgePatches: Failed to write edit-mode patch at 0x{:p}!", static_cast<void*>(patchAddr));
            }
        }
    }

    // 2. Forge AI Biped / Vehicle Spawn Patch (RVA 0x692CB5)
    // In retail haloreach.dll, when an entity is spawned from the Forge menu, the engine tests
    // [rax] & 1 and aborts with a jump (0F 84 8A 00 00 00: jz +0x8A) directly to the function
    // exit epilogue if the entity is an AI biped/vehicle or non-standard palette object.
    // NOPing this 6-byte jump (90 90 90 90 90 90) enables spawning AI (Spartans, Elites, Grunts,
    // Brutes, Hunters, Marines, Drones, custom vehicles) right through the Forge menu!
    {
        static const uint8_t kExpectedJz[6]  = { 0x0F, 0x84, 0x8A, 0x00, 0x00, 0x00 };
        static const uint8_t kNopPatch[6]    = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        constexpr uintptr_t kStaticRva = 0x692CB5;

        uint8_t* patchAddr = nullptr;

        // Verify static RVA first (instant check)
        uint8_t* rvaAddr = reinterpret_cast<uint8_t*>(baseAddress + kStaticRva);
        if (std::memcmp(rvaAddr, kExpectedJz, sizeof(kExpectedJz)) == 0) {
            patchAddr = rvaAddr;
            NOBLE_INFO("ForgePatches: Located Forge AI spawn check at static RVA 0x{:X}.", kStaticRva);
        } else if (std::memcmp(rvaAddr, kNopPatch, sizeof(kNopPatch)) == 0) {
            NOBLE_INFO("ForgePatches: Forge AI spawn check at RVA 0x{:X} is already patched (modded haloreach.dll).", kStaticRva);
            s_isApplied = true;
        } else {
            // Fallback: Signature scan in case of minor MCC patch/shift
            uintptr_t match = SigScanner::FindPattern(
                baseAddress,
                modInfo.SizeOfImage,
                "F6 00 01 0F 84 8A 00 00 00 F3 0F 10"
            );
            if (match != 0) {
                patchAddr = reinterpret_cast<uint8_t*>(match + 3); // Skip "F6 00 01"
                NOBLE_INFO("ForgePatches: Located Forge AI spawn check via pattern scan at RVA 0x{:X}.", match - baseAddress + 3);
            } else {
                uintptr_t nopMatch = SigScanner::FindPattern(
                    baseAddress,
                    modInfo.SizeOfImage,
                    "F6 00 01 90 90 90 90 90 90 F3 0F 10"
                );
                if (nopMatch != 0) {
                    NOBLE_INFO("ForgePatches: Forge AI spawn check is already patched at RVA 0x{:X}.", nopMatch - baseAddress + 3);
                    s_isApplied = true;
                } else {
                    NOBLE_WARN("ForgePatches: Could not locate Forge AI spawn check in haloreach.dll.");
                }
            }
        }

        if (patchAddr) {
            if (WriteProtectedMemory(patchAddr, kNopPatch, sizeof(kNopPatch))) {
                NOBLE_INFO("ForgePatches: Successfully patched Forge AI spawn check (jz -> 6x NOP). AI spawning from Forge menu ENABLED.");
                s_isApplied = true;
            } else {
                NOBLE_ERROR("ForgePatches: Failed to write memory at 0x{:p}!", static_cast<void*>(patchAddr));
            }
        }
    }

    return s_isApplied;
}

} // namespace noble
