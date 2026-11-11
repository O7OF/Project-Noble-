#include "ForgePlacement.hpp"
#include "engine/BlamEngine.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"

#include <cstdint>
#include <cstring>

namespace noble {
namespace {

using ObjectNewFn = int32_t(__fastcall*)(void* placement);
ObjectNewFn s_origObjectNew = nullptr;
int s_logged = 0;

int32_t __fastcall HookObjectNew(void* placement) {
    const int32_t result = s_origObjectNew ? s_origObjectNew(placement) : -1;
    if (!placement) return result;

    BlamEngine* engine = BlamEngine::Active();
    if (!engine || !engine->IsForgeMode()) return result;
    if (s_logged > 200) return result;

    __try {
        const int32_t tag = *reinterpret_cast<const int32_t*>(placement);
        float position[3] = {};
        float facing[3] = {};
        std::memcpy(position, static_cast<const uint8_t*>(placement) + 0x1C, sizeof(position));
        std::memcpy(facing, static_cast<const uint8_t*>(placement) + 0x28, sizeof(facing));
        ++s_logged;
        NOBLE_INFO("ForgeCapture: object_new tag={} pos=({:.2f},{:.2f},{:.2f}) facing=({:.2f},{:.2f},{:.2f}) -> {}",
                   tag, position[0], position[1], position[2], facing[0], facing[1], facing[2], result);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return result;
}

} // namespace

void InstallForgePlacementWatch(HMODULE reachModule) {
    if (s_origObjectNew || !reachModule) return;
    void* target = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(reachModule) + 0x46D6E0);
    if (!HookManager::InstallHook(target, reinterpret_cast<void*>(&HookObjectNew),
                                  reinterpret_cast<void**>(&s_origObjectNew), "forge_object_new")) {
        NOBLE_WARN("ForgeCapture: could not hook object_new");
        return;
    }
    NOBLE_INFO("ForgeCapture: object_new hooked (RVA 0x46D6E0); placements in Forge are logged");
}

} // namespace noble
