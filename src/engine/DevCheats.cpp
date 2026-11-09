#include "DevCheats.hpp"

#if defined(NOBLE_DEV_BUILD)

#include "SharedTagCache.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"

#include <cmath>
#include <algorithm>
#include <mutex>
#include <unordered_set>

namespace noble {

bool DevCheats::s_initialized = false;
uintptr_t DevCheats::s_baseAddress = 0;

std::atomic<int32_t> DevCheats::s_localUnitDatum{ -1 };
std::atomic<int32_t> DevCheats::s_localVehicleDatum{ -1 };

std::atomic<bool> DevCheats::s_godmode{false};
std::atomic<bool> DevCheats::s_superJump{false};
std::atomic<float> DevCheats::s_jumpMultiplier{3.0f};
std::atomic<bool> DevCheats::s_superSpeed{false};
std::atomic<float> DevCheats::s_speedMultiplier{2.5f};
std::atomic<bool> DevCheats::s_noclip{false};
std::atomic<float> DevCheats::s_noclipSpeed{15.0f};
std::atomic<bool> DevCheats::s_freecam{false};
std::atomic<float> DevCheats::s_freecamSpeed{12.0f};
std::atomic<bool> DevCheats::s_infiniteAmmo{false};
std::atomic<bool> DevCheats::s_bottomlessClip{false};
std::atomic<bool> DevCheats::s_infiniteEquipment{false};

float DevCheats::s_freecamPos[3] = { 0.0f, 0.0f, 0.0f };
float DevCheats::s_lastCamFwd[3] = { 0.0f, 1.0f, 0.0f };
bool DevCheats::s_freecamInit = false;

std::vector<SpawnCatalogItem> DevCheats::s_catalog;

namespace {
// Engine function types
using PlacementNewFn = void(__fastcall*)(void* placement, int32_t tagIndex, int32_t owner, void* extra);
using ObjectNewFn = int32_t(__fastcall*)(void* placement);
using ObjectSetPositionFn = void(__fastcall*)(int32_t object, const float* position, const float* forward, const float* up, uint64_t, uint8_t);
using ObjectRefreshFn = void(__fastcall*)(int32_t object);
using UnitAddWeaponFn = void(__fastcall*)(int32_t unit, int16_t slot, int32_t weapon, int16_t flags);
using ShieldRestoreFn = void(__fastcall*)(int32_t unit);
using GetLocalUnitFn = int32_t(__fastcall*)(int32_t);
using GetUltimateParentFn = int32_t(__fastcall*)(int32_t);
using DamageObjectFn = void(__fastcall*)(uint32_t, uint32_t, float, void*, void*, void*);
using ObjectKillFn = void(__fastcall*)(uint32_t, uint32_t, uint8_t, uint32_t);
using TickFn = uint64_t(__fastcall*)(int32_t, int32_t, int32_t, float, float);

PlacementNewFn s_placementNew = nullptr;
ObjectNewFn s_objectNew = nullptr;
ObjectSetPositionFn s_objectSetPosition = nullptr;
ObjectRefreshFn s_objectRefresh = nullptr;
UnitAddWeaponFn s_unitAddWeapon = nullptr;
ShieldRestoreFn s_shieldRestore = nullptr;
GetLocalUnitFn s_getLocalUnit = nullptr;
GetUltimateParentFn s_getUltimateParent = nullptr;
DamageObjectFn s_origDamageObject = nullptr;
ObjectKillFn s_origObjectKill = nullptr;
TickFn s_origTick = nullptr;

struct ObjectPool {
    uint8_t* header = nullptr;
    uint8_t* records = nullptr;
    int32_t capacity = 0;
};

bool GetObjectPool(uintptr_t base, ObjectPool& pool) {
    if (!base) return false;
    __try {
        const uint32_t tlsSlot = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
        if (!tls || !tls[tlsSlot]) return false;
        uint8_t* header = *reinterpret_cast<uint8_t**>(tls[tlsSlot] + 0x10);
        if (!header || std::strncmp(reinterpret_cast<const char*>(header), "object", 32) != 0) return false;
        if (*reinterpret_cast<const uint32_t*>(header + 0x28) != 0x64407440) return false;
        pool.header = header;
        pool.capacity = *reinterpret_cast<const int32_t*>(header + 0x2C);
        pool.records = *reinterpret_cast<uint8_t**>(header + 0x50);
        return (pool.records != nullptr && pool.capacity > 0);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

struct QueuedAction {
    std::string itemName;
    std::string category;
    std::string tagPath;
    uint32_t tagIndex = 0xFFFFFFFF;
    bool giveToPlayer = false;
};

std::mutex s_actionMutex;
std::vector<QueuedAction> s_actionQueue;
std::string s_lastActionStatus;
std::mutex s_statusMutex;

void SetStatus(const std::string& msg) {
    std::lock_guard<std::mutex> lock(s_statusMutex);
    s_lastActionStatus = msg;
    NOBLE_INFO("DevCheats: {}", msg);
}

int32_t ResolveTagIndex(uintptr_t base, const std::string& tagPath, const std::string& category, const ObjectPool& pool) {
    // 1. Check SharedTagCache donor map
    for (const auto& item : SharedTagCache::GetDonorPaletteItems()) {
        if (!tagPath.empty() && item.tagPath.find(tagPath) != std::string::npos && item.datumIndex != 0xFFFFFFFF) {
            return static_cast<int32_t>(item.datumIndex);
        }
    }

    // 2. Scan active map tagTable (haloreach.dll + 0xC1A600)
    __try {
        uint8_t* tagTable = *reinterpret_cast<uint8_t* const*>(base + 0xC1A600);
        if (tagTable) {
            const int32_t tagCount = std::clamp(*reinterpret_cast<const int32_t*>(tagTable + 0x3FFF8), 0, 0x8000);
            uint32_t wantedMagic = 0;
            if (category == "UNSC Weapons" || category == "Covenant Weapons") wantedMagic = 0x77656170; // 'weap'
            else if (category == "Vehicles") wantedMagic = 0x76656869; // 'vehi'
            else if (category == "Armor Abilities") wantedMagic = 0x65716970; // 'eqip'

            for (int32_t ti = 0; ti < tagCount; ++ti) {
                uint8_t* te = tagTable + static_cast<size_t>(ti) * 8u;
                int16_t groupIdx = *reinterpret_cast<const int16_t*>(te + 0);
                uint16_t salt = *reinterpret_cast<const uint16_t*>(te + 2);
                if (groupIdx < 0 || salt == 0xFFFF) continue;
                uint32_t magic = *reinterpret_cast<const uint32_t*>(tagTable + 0x3FFFC + static_cast<size_t>(groupIdx) * 16u);
                if (wantedMagic && magic == wantedMagic) {
                    return static_cast<int32_t>((static_cast<uint32_t>(salt) << 16) | static_cast<uint32_t>(ti));
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    // 3. Fallback: scan objects currently placed in the pool
    int wantedType = -1;
    if (category == "UNSC Weapons" || category == "Covenant Weapons") wantedType = 2; // weapon
    else if (category == "Vehicles") wantedType = 1; // vehicle
    else if (category == "Armor Abilities") wantedType = 3; // equipment

    if (wantedType != -1) {
        for (int i = 0; i < pool.capacity; ++i) {
            uint8_t type = pool.records[static_cast<size_t>(i) * 24 + 4];
            if (type == wantedType) {
                const uint8_t* obj = *reinterpret_cast<uint8_t* const*>(pool.records + static_cast<size_t>(i) * 24 + 0x10);
                if (obj) {
                    int32_t t = *reinterpret_cast<const int32_t*>(obj);
                    if (t != -1) return t;
                }
            }
        }
    }

    return -1;
}

static void ProcessActions(uintptr_t base, const ObjectPool& pool, int32_t unitDatum, uint8_t* biped) {
    std::vector<QueuedAction> actions;
    {
        std::lock_guard<std::mutex> lock(s_actionMutex);
        if (s_actionQueue.empty()) return;
        actions = std::move(s_actionQueue);
        s_actionQueue.clear();
    }

    float* bipedPos = reinterpret_cast<float*>(biped + 0x44);
    const float* bipedFwd = reinterpret_cast<const float*>(biped + 0x50);

    for (const auto& act : actions) {
        int32_t tagIndex = act.tagIndex;
        if (tagIndex == 0xFFFFFFFF || tagIndex == -1) {
            tagIndex = ResolveTagIndex(base, act.tagPath, act.category, pool);
        }

        if (tagIndex == 0xFFFFFFFF || tagIndex == -1) {
            SetStatus("Tag unavailable in active map: " + act.itemName);
            continue;
        }

        const float spawnDist = (act.category == "Vehicles") ? 5.5f : 2.5f;
        const float spawnPos[3] = {
            bipedPos[0] + bipedFwd[0] * spawnDist,
            bipedPos[1] + bipedFwd[1] * spawnDist,
            bipedPos[2] + 0.3f
        };

        alignas(16) uint8_t placement[0x1A8 + 16];
        std::memset(placement, 0, sizeof(placement));
        if (s_placementNew) {
            s_placementNew(placement, tagIndex, -1, nullptr);
        } else {
            *reinterpret_cast<int32_t*>(placement) = tagIndex;
        }

        std::memcpy(placement + 0x1C, spawnPos, sizeof(float) * 3);
        std::memcpy(placement + 0x28, bipedFwd, sizeof(float) * 3);

        int32_t handle = s_objectNew ? s_objectNew(placement) : -1;
        if (handle != -1) {
            float up[3] = { 0.0f, 0.0f, 1.0f };
            if (s_objectSetPosition) {
                s_objectSetPosition(handle, spawnPos, bipedFwd, up, 0, false);
            }
            if (s_objectRefresh) {
                s_objectRefresh(handle);
            }

            if (act.giveToPlayer && act.category != "Vehicles") {
                if (s_unitAddWeapon && unitDatum != -1) {
                    s_unitAddWeapon(unitDatum, 0, handle, 0);
                }
                SetStatus("Gave " + act.itemName + " directly into your hands!");
            } else {
                SetStatus("Spawned " + act.itemName + " in front of you!");
            }
        } else {
            SetStatus("Engine refused spawn for " + act.itemName);
        }
    }
}

// Hook object_cause_damage (RVA 0x1ED320)
static void __fastcall Hook_DamageObject(uint32_t objectDatum, uint32_t marker, float damageAmount, void* a4, void* a5, void* a6) {
    if (DevCheats::IsGodmode()) {
        const int32_t localUnit = DevCheats::GetLocalPlayerUnitDatum();
        const int32_t localVeh  = DevCheats::GetLocalPlayerVehicleDatum();
        if (objectDatum != 0xFFFFFFFF && (objectDatum == static_cast<uint32_t>(localUnit) || (localVeh != -1 && objectDatum == static_cast<uint32_t>(localVeh)))) {
            return; // GODMODE: Block incoming damage completely
        }
    }
    if (s_origDamageObject) {
        s_origDamageObject(objectDatum, marker, damageAmount, a4, a5, a6);
    }
}

// Hook object_kill (RVA 0x47CAA8)
static void __fastcall Hook_ObjectKill(uint32_t objectDatum, uint32_t flags, uint8_t a3, uint32_t a4) {
    if (DevCheats::IsGodmode()) {
        const int32_t localUnit = DevCheats::GetLocalPlayerUnitDatum();
        const int32_t localVeh  = DevCheats::GetLocalPlayerVehicleDatum();
        if (objectDatum != 0xFFFFFFFF && (objectDatum == static_cast<uint32_t>(localUnit) || (localVeh != -1 && objectDatum == static_cast<uint32_t>(localVeh)))) {
            return; // GODMODE: Block death completely
        }
    }
    if (s_origObjectKill) {
        s_origObjectKill(objectDatum, flags, a3, a4);
    }
}

// Hook user_control_tick (RVA 0x61A38)
static uint64_t __fastcall Hook_UserControlTick(int32_t a, int32_t b, int32_t c, float d, float e) {
    const uint64_t result = s_origTick ? s_origTick(a, b, c, d, e) : 0;
    DevCheats::GameTick(a);
    return result;
}
} // namespace

bool DevCheats::IsGodmode() { return s_godmode.load(); }
void DevCheats::SetGodmode(bool val) { s_godmode.store(val); }

bool DevCheats::IsSuperJump() { return s_superJump.load(); }
void DevCheats::SetSuperJump(bool val) { s_superJump.store(val); }
float DevCheats::GetJumpMultiplier() { return s_jumpMultiplier.load(); }
void DevCheats::SetJumpMultiplier(float val) { s_jumpMultiplier.store(val); }

bool DevCheats::IsSuperSpeed() { return s_superSpeed.load(); }
void DevCheats::SetSuperSpeed(bool val) { s_superSpeed.store(val); }
float DevCheats::GetSpeedMultiplier() { return s_speedMultiplier.load(); }
void DevCheats::SetSpeedMultiplier(float val) { s_speedMultiplier.store(val); }

bool DevCheats::IsNoclip() { return s_noclip.load(); }
void DevCheats::SetNoclip(bool val) { s_noclip.store(val); }
float DevCheats::GetNoclipSpeed() { return s_noclipSpeed.load(); }
void DevCheats::SetNoclipSpeed(float val) { s_noclipSpeed.store(val); }

bool DevCheats::IsFreecam() { return s_freecam.load(); }
void DevCheats::SetFreecam(bool val) {
    s_freecam.store(val);
    if (!val) s_freecamInit = false;
}
float DevCheats::GetFreecamSpeed() { return s_freecamSpeed.load(); }
void DevCheats::SetFreecamSpeed(float val) { s_freecamSpeed.store(val); }

bool DevCheats::IsInfiniteAmmo() { return s_infiniteAmmo.load(); }
void DevCheats::SetInfiniteAmmo(bool val) { s_infiniteAmmo.store(val); }

bool DevCheats::IsBottomlessClip() { return s_bottomlessClip.load(); }
void DevCheats::SetBottomlessClip(bool val) { s_bottomlessClip.store(val); }

bool DevCheats::IsInfiniteEquipment() { return s_infiniteEquipment.load(); }
void DevCheats::SetInfiniteEquipment(bool val) { s_infiniteEquipment.store(val); }

bool DevCheats::IsFreecamActive() { return s_freecam.load(); }
void DevCheats::ApplyFreecam(float* camPos, const float* camFwd) {
    if (!s_freecam.load() || !camPos) return;
    if (!s_freecamInit) {
        s_freecamPos[0] = camPos[0];
        s_freecamPos[1] = camPos[1];
        s_freecamPos[2] = camPos[2];
        s_freecamInit = true;
    }
    if (camFwd) {
        s_lastCamFwd[0] = camFwd[0];
        s_lastCamFwd[1] = camFwd[1];
        s_lastCamFwd[2] = camFwd[2];
    }
    camPos[0] = s_freecamPos[0];
    camPos[1] = s_freecamPos[1];
    camPos[2] = s_freecamPos[2];
}

uint32_t DevCheats::GetLocalPlayerUnitDatum() {
    int32_t u = s_localUnitDatum.load();
    return (u == -1) ? 0xFFFFFFFF : static_cast<uint32_t>(u);
}

uint32_t DevCheats::GetLocalPlayerVehicleDatum() {
    int32_t v = s_localVehicleDatum.load();
    return (v == -1) ? 0xFFFFFFFF : static_cast<uint32_t>(v);
}

uint8_t* DevCheats::GetObjectByDatum(uint32_t datum) {
    if (datum == 0xFFFFFFFF || !s_baseAddress) return nullptr;
    ObjectPool pool;
    if (!GetObjectPool(s_baseAddress, pool)) return nullptr;
    uint16_t idx = static_cast<uint16_t>(datum & 0xFFFF);
    if (idx >= pool.capacity) return nullptr;
    return *reinterpret_cast<uint8_t**>(pool.records + static_cast<size_t>(idx) * 24 + 0x10);
}

void DevCheats::QueueItemAction(const std::string& name, const std::string& category, const std::string& tagPath, uint32_t tagIndex, bool giveToPlayer) {
    std::lock_guard<std::mutex> lock(s_actionMutex);
    s_actionQueue.push_back({ name, category, tagPath, tagIndex, giveToPlayer });
}

std::string DevCheats::GetLastStatus() {
    std::lock_guard<std::mutex> lock(s_statusMutex);
    return s_lastActionStatus;
}

const std::vector<SpawnCatalogItem>& DevCheats::GetCatalog() {
    return s_catalog;
}

std::vector<SpawnCatalogItem> DevCheats::GetActiveMapItems() {
    std::vector<SpawnCatalogItem> items;
    if (!s_baseAddress) return items;

    ObjectPool pool;
    if (!GetObjectPool(s_baseAddress, pool)) return items;

    // Discover items currently placed in the match
    std::unordered_set<int32_t> seenTags;
    for (int i = 0; i < pool.capacity; ++i) {
        uint8_t type = pool.records[static_cast<size_t>(i) * 24 + 4];
        if (type == 1 || type == 2 || type == 3) { // vehicle, weapon, equipment
            const uint8_t* obj = *reinterpret_cast<uint8_t* const*>(pool.records + static_cast<size_t>(i) * 24 + 0x10);
            if (!obj) continue;
            int32_t t = *reinterpret_cast<const int32_t*>(obj);
            if (t == -1 || seenTags.count(t)) continue;
            seenTags.insert(t);

            std::string cat = (type == 1) ? "Vehicles" : (type == 2) ? "Weapons" : "Equipment";
            char label[64];
            snprintf(label, sizeof(label), "Map %s #%d [0x%08X]", cat.c_str(), static_cast<int>(seenTags.size()), static_cast<uint32_t>(t));
            items.push_back({ label, cat, "", static_cast<uint32_t>(t), "Active map loaded entity." });
        }
    }
    return items;
}

bool DevCheats::Initialize(HMODULE hReachDll) {
    if (s_initialized || !hReachDll) return s_initialized;

    s_baseAddress = reinterpret_cast<uintptr_t>(hReachDll);

    // Resolve native engine function pointers
    s_placementNew = reinterpret_cast<PlacementNewFn>(s_baseAddress + 0x49CC24);
    s_objectNew = reinterpret_cast<ObjectNewFn>(s_baseAddress + 0x46D6E0);
    s_objectSetPosition = reinterpret_cast<ObjectSetPositionFn>(s_baseAddress + 0x46CB38);
    s_objectRefresh = reinterpret_cast<ObjectRefreshFn>(s_baseAddress + 0x46C8B4);
    s_unitAddWeapon = reinterpret_cast<UnitAddWeaponFn>(s_baseAddress + 0x48F778);
    s_shieldRestore = reinterpret_cast<ShieldRestoreFn>(s_baseAddress + 0x49DD58);
    s_getLocalUnit = reinterpret_cast<GetLocalUnitFn>(s_baseAddress + 0x53EF8);
    s_getUltimateParent = reinterpret_cast<GetUltimateParentFn>(s_baseAddress + 0x473DC0);

    // 1. Hook user_control_tick (RVA 0x61A38) - Game Thread Tick Execution
    void* tickTarget = reinterpret_cast<void*>(s_baseAddress + 0x61A38);
    if (HookManager::InstallHook(tickTarget, reinterpret_cast<void*>(&Hook_UserControlTick),
                                 reinterpret_cast<void**>(&s_origTick), "user_control_tick")) {
        NOBLE_INFO("DevCheats: Hooked user_control_tick at RVA 0x61A38.");
    } else {
        NOBLE_WARN("DevCheats: Hook user_control_tick already present or failed.");
    }

    // 2. Hook object_cause_damage (RVA 0x1ED320)
    void* damageTarget = reinterpret_cast<void*>(s_baseAddress + 0x1ED320);
    HookManager::InstallHook(damageTarget, reinterpret_cast<void*>(&Hook_DamageObject),
                             reinterpret_cast<void**>(&s_origDamageObject), "object_cause_damage");

    // 3. Hook object_kill (RVA 0x47CAA8)
    void* killTarget = reinterpret_cast<void*>(s_baseAddress + 0x47CAA8);
    HookManager::InstallHook(killTarget, reinterpret_cast<void*>(&Hook_ObjectKill),
                             reinterpret_cast<void**>(&s_origObjectKill), "object_kill");

    BuildCatalog();

    s_initialized = true;
    NOBLE_INFO("DevCheats: Developer Cheats system initialized.");
    return true;
}

void DevCheats::Shutdown() {
    s_initialized = false;
}

void DevCheats::GameTick(int32_t userIndex) {
    if (userIndex != 0 || !s_initialized) return;

    // 1. Retrieve local player biped & vehicle datum indices on the Game Thread
    int32_t unitDatum = -1;
    if (s_getLocalUnit) {
        unitDatum = s_getLocalUnit(0);
    }
    s_localUnitDatum.store(unitDatum);

    int32_t vehDatum = -1;
    if (s_getUltimateParent && unitDatum != -1 && unitDatum != 0) {
        vehDatum = s_getUltimateParent(unitDatum);
        if (vehDatum == unitDatum || vehDatum == 0) vehDatum = -1;
    }
    s_localVehicleDatum.store(vehDatum);

    // 2. Resolve object pool
    ObjectPool pool;
    if (!GetObjectPool(s_baseAddress, pool)) return;

    uint8_t* biped = nullptr;
    if (unitDatum != -1) {
        int idx = unitDatum & 0xFFFF;
        if (idx < pool.capacity) {
            biped = *reinterpret_cast<uint8_t**>(pool.records + static_cast<size_t>(idx) * 24 + 0x10);
        }
    }
    if (!biped) return;

    // 3. Apply Godmode
    if (s_godmode.load()) {
        if (s_shieldRestore) {
            s_shieldRestore(unitDatum);
            if (vehDatum != -1) s_shieldRestore(vehDatum);
        }
        __try {
            int16_t healthOffset = *reinterpret_cast<const int16_t*>(biped + 0x186);
            if (healthOffset > 0 && healthOffset < 0x2000) {
                uint8_t* healthComp = biped + healthOffset;
                *reinterpret_cast<float*>(healthComp + 0x10) = 1.0f;
                *reinterpret_cast<float*>(healthComp + 0x14) = 1.0f;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // 4. Super Jump & Super Speed
    float* vel = reinterpret_cast<float*>(biped + 0x68);
    if (s_superSpeed.load() && !s_noclip.load()) {
        const float hSpeed = std::hypot(vel[0], vel[1]);
        if (hSpeed > 0.005f) {
            const float mult = s_speedMultiplier.load();
            vel[0] *= (1.0f + (mult - 1.0f) * 0.20f);
            vel[1] *= (1.0f + (mult - 1.0f) * 0.20f);
        }
    }

    if (s_superJump.load() && !s_noclip.load()) {
        if (vel[2] > 0.015f) {
            vel[2] = std::max(vel[2], 0.12f * s_jumpMultiplier.load());
        }
    }

    // 5. Noclip (Ghost Flight)
    if (s_noclip.load()) {
        float* pos = reinterpret_cast<float*>(biped + 0x44);
        const float* fwd = reinterpret_cast<const float*>(biped + 0x50);
        const float* up  = reinterpret_cast<const float*>(biped + 0x5C);

        vel[0] = 0.0f; vel[1] = 0.0f; vel[2] = 0.0f;

        const float right[3] = {
            fwd[1] * up[2] - fwd[2] * up[1],
            fwd[2] * up[0] - fwd[0] * up[2],
            fwd[0] * up[1] - fwd[1] * up[0]
        };

        float moveFwd = 0.0f, moveRight = 0.0f, moveUp = 0.0f;
        if (GetAsyncKeyState('W') & 0x8000) moveFwd += 1.0f;
        if (GetAsyncKeyState('S') & 0x8000) moveFwd -= 1.0f;
        if (GetAsyncKeyState('D') & 0x8000) moveRight += 1.0f;
        if (GetAsyncKeyState('A') & 0x8000) moveRight -= 1.0f;
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) moveUp += 1.0f;
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) || (GetAsyncKeyState('C') & 0x8000)) moveUp -= 1.0f;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            moveFwd *= 2.5f; moveRight *= 2.5f; moveUp *= 2.5f;
        }

        const float speed = s_noclipSpeed.load() * (1.0f / 60.0f);
        pos[0] += (fwd[0] * moveFwd + right[0] * moveRight) * speed;
        pos[1] += (fwd[1] * moveFwd + right[1] * moveRight) * speed;
        pos[2] += (fwd[2] * moveFwd + moveUp) * speed;
    }

    // 6. Freecam Pan
    if (s_freecam.load()) {
        float moveFwd = 0.0f, moveRight = 0.0f, moveUp = 0.0f;
        if (GetAsyncKeyState(VK_UP) & 0x8000) moveFwd += 1.0f;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) moveFwd -= 1.0f;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) moveRight += 1.0f;
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) moveRight -= 1.0f;
        if ((GetAsyncKeyState(VK_PRIOR) & 0x8000) || (GetAsyncKeyState('E') & 0x8000)) moveUp += 1.0f;
        if ((GetAsyncKeyState(VK_NEXT) & 0x8000) || (GetAsyncKeyState('Q') & 0x8000)) moveUp -= 1.0f;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            moveFwd *= 2.5f; moveRight *= 2.5f; moveUp *= 2.5f;
        }

        const float speed = s_freecamSpeed.load() * (1.0f / 60.0f);
        s_freecamPos[0] += (s_lastCamFwd[0] * moveFwd - s_lastCamFwd[1] * moveRight) * speed;
        s_freecamPos[1] += (s_lastCamFwd[1] * moveFwd + s_lastCamFwd[0] * moveRight) * speed;
        s_freecamPos[2] += (s_lastCamFwd[2] * moveFwd + moveUp) * speed;
    }

    // 7. Infinite Ammo & Bottomless Clip
    const bool infAmmo = s_infiniteAmmo.load();
    const bool bottomless = s_bottomlessClip.load();
    if (infAmmo || bottomless) {
        for (int slot = 0; slot < 4; ++slot) {
            uint32_t wDatum = *reinterpret_cast<const uint32_t*>(biped + 0x350 + slot * 4);
            if (wDatum != 0xFFFFFFFF && wDatum != 0) {
                int wIdx = wDatum & 0xFFFF;
                if (wIdx < pool.capacity) {
                    uint8_t* weapon = *reinterpret_cast<uint8_t**>(pool.records + static_cast<size_t>(wIdx) * 24 + 0x10);
                    if (weapon) {
                        if (infAmmo) *reinterpret_cast<uint32_t*>(weapon + 0x3034) = 999;
                        if (bottomless) *reinterpret_cast<uint32_t*>(weapon + 0x3030) = 99;
                        *reinterpret_cast<float*>(weapon + 0x3038) = 1.0f; // battery
                        *reinterpret_cast<float*>(weapon + 0x303C) = 0.0f; // heat
                    }
                }
            }
        }
    }

    // 8. Infinite Equipment Energy
    if (s_infiniteEquipment.load()) {
        uint32_t eqDatum = *reinterpret_cast<const uint32_t*>(biped + 0x3E8);
        if (eqDatum != 0xFFFFFFFF && eqDatum != 0) {
            int eqIdx = eqDatum & 0xFFFF;
            if (eqIdx < pool.capacity) {
                uint8_t* equip = *reinterpret_cast<uint8_t**>(pool.records + static_cast<size_t>(eqIdx) * 24 + 0x10);
                if (equip) {
                    *reinterpret_cast<float*>(equip + 0x3030) = 1.0f;
                    *reinterpret_cast<float*>(equip + 0x3034) = 1.0f;
                }
            }
        }
    }

    // 9. Process Queued Spawn & Give actions
    ProcessActions(s_baseAddress, pool, unitDatum, biped);
}

void DevCheats::BuildCatalog() {
    s_catalog.clear();

    // UNSC Weapons
    s_catalog.push_back({ "Assault Rifle", "UNSC Weapons", "objects/weapons/rifle/assault_rifle/assault_rifle", 0xFFFFFFFF, "Standard issue MA37 automatic rifle." });
    s_catalog.push_back({ "DMR", "UNSC Weapons", "objects/weapons/rifle/dmr/dmr", 0xFFFFFFFF, "Designated Marksman Rifle (M392)." });
    s_catalog.push_back({ "Magnum", "UNSC Weapons", "objects/weapons/pistol/magnum/magnum", 0xFFFFFFFF, "M6G semi-automatic sidearm." });
    s_catalog.push_back({ "Shotgun", "UNSC Weapons", "objects/weapons/shotgun/shotgun", 0xFFFFFFFF, "M45 Tactical 8-gauge shotgun." });
    s_catalog.push_back({ "Sniper Rifle", "UNSC Weapons", "objects/weapons/rifle/sniper_rifle/sniper_rifle", 0xFFFFFFFF, "SRS99D Anti-Materiel sniper rifle." });
    s_catalog.push_back({ "Rocket Launcher", "UNSC Weapons", "objects/weapons/support_high/rocket_launcher/rocket_launcher", 0xFFFFFFFF, "M41 SSR AV/AA twin-tube launcher." });
    s_catalog.push_back({ "Spartan Laser", "UNSC Weapons", "objects/weapons/support_high/spartan_laser/spartan_laser", 0xFFFFFFFF, "M6 G/GNR directed-energy weapon." });
    s_catalog.push_back({ "Grenade Launcher", "UNSC Weapons", "objects/weapons/pistol/grenade_launcher/grenade_launcher", 0xFFFFFFFF, "M319 individual explosive launcher." });
    s_catalog.push_back({ "Target Locator", "UNSC Weapons", "objects/weapons/support_high/target_locator/target_locator", 0xFFFFFFFF, "Orbital bombardment designator." });
    s_catalog.push_back({ "Machine Gun Turret", "UNSC Weapons", "objects/weapons/turret/machinegun_turret/machinegun_turret", 0xFFFFFFFF, "Detachable AIE-486H heavy turret." });

    // Covenant Weapons
    s_catalog.push_back({ "Plasma Pistol", "Covenant Weapons", "objects/weapons/pistol/plasma_pistol/plasma_pistol", 0xFFFFFFFF, "Type-25 directed energy pistol with EMP." });
    s_catalog.push_back({ "Plasma Rifle", "Covenant Weapons", "objects/weapons/rifle/plasma_rifle/plasma_rifle", 0xFFFFFFFF, "Type-25 rapid plasma rifle." });
    s_catalog.push_back({ "Plasma Repeater", "Covenant Weapons", "objects/weapons/rifle/plasma_repeater/plasma_repeater", 0xFFFFFFFF, "Type-51 manual-vent automatic rifle." });
    s_catalog.push_back({ "Needle Rifle", "Covenant Weapons", "objects/weapons/rifle/needle_rifle/needle_rifle", 0xFFFFFFFF, "Type-31 marksman sub-munitions rifle." });
    s_catalog.push_back({ "Needler", "Covenant Weapons", "objects/weapons/pistol/needler/needler", 0xFFFFFFFF, "Type-33 tracking crystalline projector." });
    s_catalog.push_back({ "Energy Sword", "Covenant Weapons", "objects/weapons/melee/energy_sword/energy_sword", 0xFFFFFFFF, "Type-1 plasma melee blade." });
    s_catalog.push_back({ "Gravity Hammer", "Covenant Weapons", "objects/weapons/melee/gravity_hammer/gravity_hammer", 0xFFFFFFFF, "Type-2 kinetic gravity shock hammer." });
    s_catalog.push_back({ "Concussion Rifle", "Covenant Weapons", "objects/weapons/support_high/concussion_rifle/concussion_rifle", 0xFFFFFFFF, "Type-50 explosive blast weapon." });
    s_catalog.push_back({ "Fuel Rod Gun", "Covenant Weapons", "objects/weapons/support_high/fuel_rod_gun/fuel_rod_gun", 0xFFFFFFFF, "Type-33 heavy explosive cannon." });
    s_catalog.push_back({ "Plasma Launcher", "Covenant Weapons", "objects/weapons/support_high/plasma_launcher/plasma_launcher", 0xFFFFFFFF, "Type-52 quad tracking launcher." });
    s_catalog.push_back({ "Focus Rifle", "Covenant Weapons", "objects/weapons/rifle/focus_rifle/focus_rifle", 0xFFFFFFFF, "Type-52 continuous beam sniper." });
    s_catalog.push_back({ "Spiker", "Covenant Weapons", "objects/weapons/pistol/spike_rifle/spike_rifle", 0xFFFFFFFF, "Type-25 brutal projectile carbine." });

    // Equipment & Armor Abilities
    s_catalog.push_back({ "Jetpack", "Armor Abilities", "objects/equipment/jet_pack/jet_pack", 0xFFFFFFFF, "Series 8 personal flight thruster pack." });
    s_catalog.push_back({ "Sprint", "Armor Abilities", "objects/equipment/sprint/sprint", 0xFFFFFFFF, "Burst locomotion accelerator." });
    s_catalog.push_back({ "Armor Lock", "Armor Abilities", "objects/equipment/armor_lock/armor_lock", 0xFFFFFFFF, "Complete invulnerable kinetic shielding." });
    s_catalog.push_back({ "Active Camouflage", "Armor Abilities", "objects/equipment/active_camo/active_camo", 0xFFFFFFFF, "Adaptive light-bending stealth emitter." });
    s_catalog.push_back({ "Drop Shield", "Armor Abilities", "objects/equipment/drop_shield/drop_shield", 0xFFFFFFFF, "Deployable hemispherical energy shield." });
    s_catalog.push_back({ "Evade", "Armor Abilities", "objects/equipment/evade/evade", 0xFFFFFFFF, "Tactical evasive combat roll." });
    s_catalog.push_back({ "Hologram", "Armor Abilities", "objects/equipment/hologram/hologram", 0xFFFFFFFF, "Solid light projection decoy." });

    // Vehicles
    s_catalog.push_back({ "Warthog", "Vehicles", "objects/vehicles/human/warthog/warthog", 0xFFFFFFFF, "M12 LRV all-terrain reconnaissance vehicle." });
    s_catalog.push_back({ "Mongoose", "Vehicles", "objects/vehicles/human/mongoose/mongoose", 0xFFFFFFFF, "M274 light quad utility vehicle." });
    s_catalog.push_back({ "Scorpion", "Vehicles", "objects/vehicles/human/scorpion/scorpion", 0xFFFFFFFF, "M808B heavy battle tank with 90mm cannon." });
    s_catalog.push_back({ "Falcon", "Vehicles", "objects/vehicles/human/falcon/falcon", 0xFFFFFFFF, "UH-144 troop transport / close air support." });
    s_catalog.push_back({ "Ghost", "Vehicles", "objects/vehicles/covenant/ghost/ghost", 0xFFFFFFFF, "Type-32 rapid assault vehicle." });
    s_catalog.push_back({ "Banshee", "Vehicles", "objects/vehicles/covenant/banshee/banshee", 0xFFFFFFFF, "Type-26 aerial attack craft." });
    s_catalog.push_back({ "Wraith", "Vehicles", "objects/vehicles/covenant/wraith/wraith", 0xFFFFFFFF, "Type-26 heavy mortar tank." });
    s_catalog.push_back({ "Flyable Pelican", "Vehicles", "objects/vehicles/human/pelican/pelican", 0x95e82c96, "D77-TC dropship (flyable with wraith_d seat)." });
}

} // namespace noble

#endif // defined(NOBLE_DEV_BUILD)
