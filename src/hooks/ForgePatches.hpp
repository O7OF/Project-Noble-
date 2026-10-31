#pragma once

#include <windows.h>
#include <cstdint>
#include <string>

namespace noble {

class ForgePatches {
public:
    // Applies in-memory patches to haloreach.dll for unlimited Forge budget and object limits
    static bool Apply(HMODULE hReachDll);

    static bool IsApplied() { return s_isApplied; }

    // Client-side palette placement: resolves tagPath and asks the engine to create the object.
    // Returns false until the engine-side create call is wired, so the UI can say so honestly.
    static bool PlaceObject(const std::string& tagPath);
    static void SetUnlimitedBudget(bool enabled) { s_unlimitedBudget = enabled; }
    static void SetUnlockedRotation(bool enabled) { s_unlockedRotation = enabled; }

private:
    static bool s_isApplied;
    static bool s_unlimitedBudget;
    static bool s_unlockedRotation;
};

} // namespace noble
