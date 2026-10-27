#pragma once

#include <windows.h>
#include <cstdint>

namespace noble {

class EnginePatches {
public:
    // Applies engine stability patches to haloreach.dll (e.g. whitespace underflow fix)
    static bool Apply(HMODULE hReachDll);

    static bool IsApplied() { return s_isApplied; }

private:
    static bool s_isApplied;
};

} // namespace noble
