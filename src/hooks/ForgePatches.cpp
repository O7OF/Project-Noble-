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
    (void)hReachDll;
    s_isApplied = false;
    NOBLE_INFO("ForgePatches: Disabled per launch configuration.");
    return true;
}

} // namespace noble
