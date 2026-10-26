#pragma once

#include <windows.h>
#include <MinHook.h>
#include <unordered_map>
#include <string>

namespace noble {

class HookManager {
public:
    static bool Initialize();
    static void Shutdown();

    // Creates and immediately enables a function hook via MinHook
    static bool InstallHook(void* target, void* detour, void** original, const std::string& name = "");

    // Disables and removes a hook
    static bool RemoveHook(void* target);

    // Enable or disable all managed hooks
    static bool EnableAll();
    static bool DisableAll();

private:
    static bool s_isInitialized;
    static std::unordered_map<void*, std::string> s_installedHooks;
};

} // namespace noble
