#include "ForgePatches.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>

namespace noble {

bool ForgePatches::s_isApplied = false;
bool ForgePatches::s_unlimitedBudget = true;
bool ForgePatches::s_unlockedRotation = true;

static bool WriteProtectedMemory(void* destination, const void* source, size_t size) {
    DWORD oldProtect = 0;
    if (!VirtualProtect(destination, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    memcpy(destination, source, size);
    VirtualProtect(destination, size, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), destination, size);
    return true;
}

bool ForgePatches::Apply(HMODULE hReachDll) {
    if (!hReachDll) {
        NOBLE_ERROR("ForgePatches: Invalid haloreach.dll handle provided.");
        return false;
    }

    if (s_isApplied) {
        return true;
    }

    NOBLE_INFO("ForgePatches: Scanning haloreach.dll for Forge budget and object restrictions...");

    MODULEINFO modInfo{};
    if (!GetModuleInformation(GetCurrentProcess(), hReachDll, &modInfo, sizeof(modInfo))) {
        NOBLE_ERROR("ForgePatches: Failed to query haloreach.dll module bounds.");
        return false;
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    size_t size = modInfo.SizeOfImage;

    // Pattern for Forge Budget Check & Subtract
    // 29 ?? ?? 83 ?? ?? 7C (sub [reg+offset], reg; cmp [reg+offset], 0; jl ...)
    // Patch SUB instruction to NOPs (90 90 90) or test to bypass budget subtraction
    std::string_view sigBudgetCheck = "29 51 04 83 79 04 00";
    uintptr_t budgetMatch = SigScanner::FindPattern(base, size, sigBudgetCheck);
    if (budgetMatch) {
        uint8_t nops[3] = { 0x90, 0x90, 0x90 }; // NOP out SUB [rcx+4], edx
        if (WriteProtectedMemory(reinterpret_cast<void*>(budgetMatch), nops, sizeof(nops))) {
            NOBLE_INFO("ForgePatches: Forge unlimited budget patch active at 0x{:X}", budgetMatch);
        }
    }

    // Pattern for Forge Max Objects Cap Check (CMP reg, 650 = 0x28A)
    // 81 ?? 8A 02 00 00 7D (cmp reg, 650; jge ...)
    std::string_view sigObjectCap = "81 F9 8A 02 00 00";
    uintptr_t objectCapMatch = SigScanner::FindPattern(base, size, sigObjectCap);
    if (objectCapMatch) {
        // Expand cap from 650 to 65,535 (0xFFFF)
        uint32_t newCap = 0xFFFF;
        if (WriteProtectedMemory(reinterpret_cast<void*>(objectCapMatch + 2), &newCap, sizeof(newCap))) {
            NOBLE_INFO("ForgePatches: Forge max object cap expanded (650 -> 65535) at 0x{:X}", objectCapMatch);
        }
    }

    s_isApplied = true;
    NOBLE_INFO("ForgePatches: Engine Forge enhancements successfully applied.");
    return true;
}

} // namespace noble
