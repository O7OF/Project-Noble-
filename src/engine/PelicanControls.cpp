#include "PelicanControls.hpp"

#if defined(NOBLE_DEV_BUILD)
#include "common/Logger.hpp"

#include <intrin.h>
#endif

namespace noble::pelican {

#if defined(NOBLE_DEV_BUILD)
namespace {

// RVAs / offsets in haloreach.dll 1.3528.0.0 (same build as SpawnTest / DevCheats).
constexpr uintptr_t kTlsIndexRva = 0xC17B18;          // TLS slot of the engine's game-state root
constexpr uintptr_t kGetLocalUnitRva = 0x53EF8;       // player_mapping_get_unit_by_output_user(0)
// object_function_set's script evaluator (RVA 0x19AAB4) clamps the value to [0, 1] and stores it at
// (*(float**)(root + 0x3C0))[index], index 0..3. The Pelican's "_scripted_doors" tags read index 3.
constexpr size_t kScriptedFunctionsRootOffset = 0x3C0;
constexpr int kPelicanDoorFunction = 3;
constexpr int kToggleKey = 'X';

using GetLocalUnitFn = int32_t(__fastcall*)(int32_t userIndex);

bool s_keyDown = false;
ULONGLONG s_lastToggle = 0;

uint8_t* GameStateRoot(uintptr_t base) {
    const uint32_t slot = *reinterpret_cast<const uint32_t*>(base + kTlsIndexRva);
    auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
    return tls ? tls[slot] : nullptr;
}

// Unit handle -> object data (object pool at root + 0x10, 24-byte records, data pointer at +0x10).
uint8_t* ObjectData(uint8_t* root, int32_t handle) {
    if (handle == -1) return nullptr;
    uint8_t* header = *reinterpret_cast<uint8_t**>(root + 0x10);
    if (!header) return nullptr;
    const int32_t capacity = *reinterpret_cast<const int32_t*>(header + 0x2C);
    uint8_t* records = *reinterpret_cast<uint8_t**>(header + 0x50);
    const int index = handle & 0xFFFF;
    if (!records || index >= capacity) return nullptr;
    const uint16_t salt = *reinterpret_cast<const uint16_t*>(records + static_cast<size_t>(index) * 24);
    if (salt != static_cast<uint16_t>(static_cast<uint32_t>(handle) >> 16)) return nullptr;
    return *reinterpret_cast<uint8_t**>(records + static_cast<size_t>(index) * 24 + 0x10);
}

// Returns the new value (0 or 1), or -1 if the local player is not in a vehicle / state not ready.
int ToggleDoors(uintptr_t base) {
    __try {
        uint8_t* root = GameStateRoot(base);
        if (!root) return -1;
        auto getLocalUnit = reinterpret_cast<GetLocalUnitFn>(base + kGetLocalUnitRva);
        const int32_t unit = getLocalUnit(0);
        uint8_t* body = ObjectData(root, unit);
        if (!body) return -1;
        const int32_t vehicle = *reinterpret_cast<const int32_t*>(body + 0x14);   // parent object
        if (vehicle == -1) return -1;
        float* functions = *reinterpret_cast<float**>(root + kScriptedFunctionsRootOffset);
        if (!functions) return -1;
        const float next = functions[kPelicanDoorFunction] > 0.5f ? 0.0f : 1.0f;
        functions[kPelicanDoorFunction] = next;
        return next > 0.5f ? 1 : 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }
}

} // namespace
#endif

void GameTick([[maybe_unused]] uintptr_t base) {
#if defined(NOBLE_DEV_BUILD)
    if (!base) return;
    const bool down = (GetAsyncKeyState(kToggleKey) & 0x8000) != 0 && GetForegroundWindow() != nullptr;
    if (down && !s_keyDown && GetTickCount64() - s_lastToggle > 400) {
        s_lastToggle = GetTickCount64();
        const int state = ToggleDoors(base);
        if (state == 1) NOBLE_INFO("Pelican: X, ramp + landing gear OPEN (scripted function 3 = 1)");
        else if (state == 0) NOBLE_INFO("Pelican: X, ramp + landing gear CLOSED (scripted function 3 = 0)");
        else NOBLE_INFO("Pelican: X ignored (not in a vehicle)");
    }
    s_keyDown = down;
#endif
}

} // namespace noble::pelican
