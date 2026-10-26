#include "HookManager.hpp"
#include "common/Logger.hpp"

namespace noble {

bool HookManager::s_isInitialized = false;
std::unordered_map<void*, std::string> HookManager::s_installedHooks;

bool HookManager::Initialize() {
    if (s_isInitialized) return true;

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK) {
        NOBLE_ERROR("Failed to initialize MinHook! Status: {}", static_cast<int>(status));
        return false;
    }

    s_isInitialized = true;
    NOBLE_INFO("MinHook detour framework initialized successfully.");
    return true;
}

void HookManager::Shutdown() {
    if (!s_isInitialized) return;

    NOBLE_INFO("Shutting down HookManager and unhooking all functions...");
    DisableAll();
    MH_Uninitialize();
    s_installedHooks.clear();
    s_isInitialized = false;
}

bool HookManager::InstallHook(void* target, void* detour, void** original, const std::string& name) {
    if (!s_isInitialized) {
        if (!Initialize()) return false;
    }

    NOBLE_INFO("Calling MH_CreateHook for '{}'...", name);
    spdlog::default_logger()->flush();

    MH_STATUS status = MH_CreateHook(target, detour, original);
    NOBLE_INFO("MH_CreateHook returned {} for '{}'", static_cast<int>(status), name);
    spdlog::default_logger()->flush();

    if (status != MH_OK) {
        NOBLE_ERROR("MH_CreateHook failed for '{}' at 0x{:X}. Status: {}", 
            name, reinterpret_cast<uintptr_t>(target), static_cast<int>(status));
        spdlog::default_logger()->flush();
        return false;
    }

    NOBLE_INFO("Calling MH_EnableHook for '{}'...", name);
    spdlog::default_logger()->flush();

    status = MH_EnableHook(target);
    NOBLE_INFO("MH_EnableHook returned {} for '{}'", static_cast<int>(status), name);
    spdlog::default_logger()->flush();
    if (status != MH_OK) {
        NOBLE_ERROR("MH_EnableHook failed for '{}' at 0x{:X}. Status: {}", 
            name, reinterpret_cast<uintptr_t>(target), static_cast<int>(status));
        spdlog::default_logger()->flush();
        MH_RemoveHook(target);
        return false;
    }

    s_installedHooks[target] = name.empty() ? "unnamed_hook" : name;
    NOBLE_INFO("Hook '{}' installed at 0x{:X} -> 0x{:X}", 
        s_installedHooks[target], reinterpret_cast<uintptr_t>(target), reinterpret_cast<uintptr_t>(detour));
    spdlog::default_logger()->flush();

    return true;
}

bool HookManager::RemoveHook(void* target) {
    if (!s_isInitialized || !target) return false;

    MH_STATUS status = MH_DisableHook(target);
    if (status != MH_OK) {
        NOBLE_WARN("MH_DisableHook failed for target 0x{:X}", reinterpret_cast<uintptr_t>(target));
    }

    status = MH_RemoveHook(target);
    s_installedHooks.erase(target);
    return (status == MH_OK);
}

bool HookManager::EnableAll() {
    if (!s_isInitialized) return false;
    return (MH_EnableHook(MH_ALL_HOOKS) == MH_OK);
}

bool HookManager::DisableAll() {
    if (!s_isInitialized) return false;
    return (MH_DisableHook(MH_ALL_HOOKS) == MH_OK);
}

} // namespace noble
