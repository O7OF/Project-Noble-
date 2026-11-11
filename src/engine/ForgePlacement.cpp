#include "ForgePlacement.hpp"
#include "engine/BlamEngine.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

namespace noble {
namespace {

using ObjectNewFn = int32_t(__fastcall*)(void* placement);
ObjectNewFn s_origObjectNew = nullptr;
std::mutex s_placementsMutex;
std::vector<mvar::WorldPlacement> s_placements;
int s_logged = 0;

bool ReadPlacementFields(void* placement, int32_t& tag, float position[3], float facing[3]) {
    __try {
        tag = *reinterpret_cast<const int32_t*>(placement);
        std::memcpy(position, static_cast<const uint8_t*>(placement) + 0x1C, sizeof(float) * 3);
        std::memcpy(facing, static_cast<const uint8_t*>(placement) + 0x28, sizeof(float) * 3);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

int32_t __fastcall HookObjectNew(void* placement) {
    const int32_t result = s_origObjectNew ? s_origObjectNew(placement) : -1;
    if (!placement) return result;

    BlamEngine* engine = BlamEngine::Active();
    if (!engine || !engine->IsForgeMode()) return result;
    if (s_logged > 5000) return result;

    int32_t tag = 0;
    float position[3] = {};
    float facing[3] = {};
    if (ReadPlacementFields(placement, tag, position, facing)) {
        {
            std::lock_guard<std::mutex> lock(s_placementsMutex);
            mvar::WorldPlacement entry;
            entry.tag = static_cast<uint32_t>(tag);
            std::memcpy(entry.position, position, sizeof(position));
            std::memcpy(entry.forward, facing, sizeof(facing));
            s_placements.push_back(entry);
        }
        ++s_logged;
        // The editor recreates its ghost/preview object every frame with the same tag and position; only a tag change
        // is a real new placement, and only then is the raw placement struct worth dumping.
        static int32_t s_lastTag = 0;
        if (tag != s_lastTag) {
            s_lastTag = tag;
            char hex[0x80 * 2 + 1] = {};
            const uint8_t* bytes = static_cast<const uint8_t*>(placement);
            for (int i = 0; i < 0x80; ++i) std::snprintf(hex + i * 2, 3, "%02x", bytes[i]);
            NOBLE_INFO("ForgeCapture: #{} object_new tag={} pos=({:.2f},{:.2f},{:.2f}) facing=({:.2f},{:.2f},{:.2f}) -> {}",
                       s_logged, tag, position[0], position[1], position[2], facing[0], facing[1], facing[2], result);
            NOBLE_INFO("ForgeCapture: #{} bytes {}", s_logged, hex);
        }
    }
    return result;
}

} // namespace

std::vector<mvar::WorldPlacement> TakeForgePlacements() {
    std::lock_guard<std::mutex> lock(s_placementsMutex);
    std::vector<mvar::WorldPlacement> out = std::move(s_placements);
    s_placements.clear();
    return out;
}

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
