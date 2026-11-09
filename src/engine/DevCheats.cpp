#include "DevCheats.hpp"

#if defined(NOBLE_DEV_BUILD)

#include "SharedTagCache.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"

#include <cmath>
#include <algorithm>
#include <mutex>
#include <unordered_set>
#include <vector>
#include <string>

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
float DevCheats::s_lastCamFwd[3] = { 1.0f, 0.0f, 0.0f };
bool DevCheats::s_freecamInit = false;

std::vector<SpawnCatalogItem> DevCheats::s_catalog;

namespace {

// RVAs in haloreach.dll (Tyler's verified offsets)
constexpr uintptr_t kPlacementNewRva    = 0x49CC24;  // object_placement_data_new(placement*, tag, owner, ptr)
constexpr uintptr_t kObjectNewRva       = 0x46D6E0;  // object_new(placement*) -> object handle or -1
constexpr uintptr_t kTlsIndexRva        = 0xC17B18;  // TLS slot of the engine's game-state root
constexpr uintptr_t kGetLocalUnitRva    = 0x53EF8;   // player_mapping_get_unit_by_output_user(0)
constexpr uintptr_t kUnitAddWeaponRva   = 0x48F778;  // (unit, slot, weapon, flags)
constexpr uintptr_t kShieldRestoreRva   = 0x49DD58;  // (unit)
constexpr uintptr_t kDamageObjectRva    = 0x1ED320;  // object_cause_damage
constexpr uintptr_t kObjectKillRva      = 0x47CAA8;  // object_kill
constexpr uintptr_t kTagTableRva        = 0xC1A600;  // loaded tag table root
constexpr size_t    kPlacementSize      = 0x1A8;

using PlacementNewFn   = void(__fastcall*)(void* placement, int32_t tagIndex, int32_t owner, void* extra);
using ObjectNewFn      = int32_t(__fastcall*)(void* placement);
using GetLocalUnitFn   = int32_t(__fastcall*)(int32_t userIndex);
using UnitAddWeaponFn  = void(__fastcall*)(int32_t unit, int16_t slot, int32_t weapon, int16_t flags);
using ShieldRestoreFn  = void(__fastcall*)(int32_t unit);
using DamageObjectFn   = void(__fastcall*)(uint32_t, uint32_t, float, void*, void*, void*);
using ObjectKillFn     = void(__fastcall*)(uint32_t, uint32_t, uint8_t, uint32_t);

PlacementNewFn   s_placementNew     = nullptr;
ObjectNewFn      s_objectNew        = nullptr;
GetLocalUnitFn   s_getLocalUnit     = nullptr;
UnitAddWeaponFn  s_unitAddWeapon    = nullptr;
ShieldRestoreFn  s_shieldRestore    = nullptr;
DamageObjectFn   s_origDamageObject = nullptr;
ObjectKillFn     s_origObjectKill   = nullptr;

struct ObjectPool {
    uint8_t* header = nullptr;
    uint8_t* records = nullptr;
    const uint32_t* bitmap = nullptr;
    int32_t capacity = 0;
};

bool GetObjectPool(uintptr_t base, ObjectPool& pool) {
    if (!base) return false;
    __try {
        const uint32_t tlsSlot = *reinterpret_cast<const uint32_t*>(base + kTlsIndexRva);
        auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
        if (!tls || !tls[tlsSlot]) return false;
        uint8_t* root = tls[tlsSlot];

        uint8_t* header = *reinterpret_cast<uint8_t**>(root + 0x10);
        if (!header || std::strncmp(reinterpret_cast<const char*>(header), "object", 32) != 0) return false;
        if (*reinterpret_cast<const uint32_t*>(header + 0x28) != 0x64407440) return false;

        pool.header = header;
        pool.capacity = *reinterpret_cast<const int32_t*>(header + 0x2C);
        pool.records = *reinterpret_cast<uint8_t**>(header + 0x50);
        pool.bitmap = reinterpret_cast<const uint32_t*>(header + *reinterpret_cast<const int64_t*>(header + 0x68));
        return (pool.records != nullptr && pool.capacity > 0 && pool.bitmap != nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

inline bool SlotUsed(const ObjectPool& pool, int i) {
    return (pool.bitmap[i >> 5] >> (i & 31)) & 1;
}

inline uint8_t* ObjectData(const ObjectPool& pool, int32_t handle) {
    if (handle == -1) return nullptr;
    const int index = handle & 0xFFFF;
    if (index >= pool.capacity || !SlotUsed(pool, index)) return nullptr;
    const uint16_t salt = *reinterpret_cast<const uint16_t*>(pool.records + static_cast<size_t>(index) * 24);
    if (salt != static_cast<uint16_t>(static_cast<uint32_t>(handle) >> 16)) return nullptr;
    return *reinterpret_cast<uint8_t**>(pool.records + static_cast<size_t>(index) * 24 + 0x10);
}

struct PlayerState {
    int32_t unitHandle = -1;
    int32_t vehicleHandle = -1;
    uint8_t* biped = nullptr;
    uint8_t* vehicle = nullptr;
    float pos[3] = {};
    float fwd[3] = {};
    float vel[3] = {};
};

// Robust multi-tier local player discovery (works in Campaign, Firefight, Forge, and Multiplayer)
bool GetLocalPlayer(uintptr_t base, const ObjectPool& pool, PlayerState& out) {
    if (!base) return false;
    __try {
        const uint32_t tlsSlot = *reinterpret_cast<const uint32_t*>(base + kTlsIndexRva);
        auto** tls = reinterpret_cast<uint8_t**>(__readgsqword(0x58));
        if (!tls || !tls[tlsSlot]) return false;
        uint8_t* root = tls[tlsSlot];

        // Tier 1: Try s_getLocalUnit(0) (player_mapping_get_unit_by_output_user)
        if (s_getLocalUnit) {
            int32_t u = s_getLocalUnit(0);
            if (u != -1 && u != 0) {
                uint8_t* body = ObjectData(pool, u);
                if (body) {
                    out.unitHandle = u;
                    out.biped = body;
                    std::memcpy(out.pos, body + 0x44, sizeof(float) * 3);
                    std::memcpy(out.fwd, body + 0x50, sizeof(float) * 3);
                    std::memcpy(out.vel, body + 0x68, sizeof(float) * 3);
                    int32_t parent = *reinterpret_cast<const int32_t*>(body + 0x14);
                    if (parent != -1) {
                        out.vehicleHandle = parent;
                        out.vehicle = ObjectData(pool, parent);
                    }
                    return true;
                }
            }
        }

        // Tier 2: Check players pool (root + 0x18, 0x490-byte records)
        uint8_t* plyHeader = *reinterpret_cast<uint8_t**>(root + 0x18);
        if (plyHeader && std::strncmp(reinterpret_cast<const char*>(plyHeader), "players", 7) == 0) {
            const int32_t cap = *reinterpret_cast<const int32_t*>(plyHeader + 0x2C);
            const uint8_t* records = *reinterpret_cast<uint8_t**>(plyHeader + 0x50);
            const auto* bitmap = reinterpret_cast<const uint32_t*>(plyHeader + *reinterpret_cast<const int64_t*>(plyHeader + 0x68));
            for (int i = 0; records && i < cap; ++i) {
                if (!((bitmap[i >> 5] >> (i & 31)) & 1)) continue;
                int32_t unitHandle = *reinterpret_cast<const int32_t*>(records + static_cast<size_t>(i) * 0x490 + 0x28);
                if (unitHandle == -1 || unitHandle == 0) continue;
                uint8_t* body = ObjectData(pool, unitHandle);
                if (!body) continue;

                out.unitHandle = unitHandle;
                out.biped = body;
                std::memcpy(out.pos, body + 0x44, sizeof(float) * 3);
                std::memcpy(out.fwd, body + 0x50, sizeof(float) * 3);
                std::memcpy(out.vel, body + 0x68, sizeof(float) * 3);

                int32_t parent = *reinterpret_cast<const int32_t*>(body + 0x14);
                if (parent != -1) {
                    out.vehicleHandle = parent;
                    out.vehicle = ObjectData(pool, parent);
                }
                return true;
            }
        }

        // Tier 3: Search object pool for placed bipeds (Noble Six / Spartan)
        for (int i = 0; i < pool.capacity; ++i) {
            if (!SlotUsed(pool, i)) continue;
            uint8_t type = pool.records[static_cast<size_t>(i) * 24 + 4];
            if (type == 0) { // biped
                uint8_t* body = *reinterpret_cast<uint8_t**>(pool.records + static_cast<size_t>(i) * 24 + 0x10);
                if (!body) continue;
                float p[3];
                std::memcpy(p, body + 0x44, sizeof(p));
                if (p[0] != 0.0f || p[1] != 0.0f || p[2] != 0.0f) {
                    uint16_t salt = *reinterpret_cast<const uint16_t*>(pool.records + static_cast<size_t>(i) * 24);
                    out.unitHandle = static_cast<int32_t>((static_cast<uint32_t>(salt) << 16) | static_cast<uint32_t>(i));
                    out.biped = body;
                    std::memcpy(out.pos, p, sizeof(p));
                    std::memcpy(out.fwd, body + 0x50, sizeof(float) * 3);
                    std::memcpy(out.vel, body + 0x68, sizeof(float) * 3);
                    return true;
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    return false;
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

int32_t ResolveTagIndex(uintptr_t base, const std::string& tagPath, const std::string& category, uint32_t tagIndex, const ObjectPool& pool) {
    if (tagIndex != 0xFFFFFFFF && tagIndex != 0) return static_cast<int32_t>(tagIndex);

    // 1. Check SharedTagCache donor map
    for (const auto& item : SharedTagCache::GetDonorPaletteItems()) {
        if (!tagPath.empty() && item.tagPath.find(tagPath) != std::string::npos && item.datumIndex != 0xFFFFFFFF) {
            return static_cast<int32_t>(item.datumIndex);
        }
    }

    // 2. Scan active object pool for any object of this category (Tyler's PickReference logic)
    int wantedType = (category == "Vehicles") ? 1 :
                     (category == "UNSC Weapons" || category == "Covenant Weapons") ? 2 :
                     (category == "Armor Abilities") ? 3 : -1;

    for (int i = 0; i < pool.capacity; ++i) {
        if (!SlotUsed(pool, i)) continue;
        uint8_t type = pool.records[static_cast<size_t>(i) * 24 + 4];
        if (type == wantedType) {
            const uint8_t* obj = *reinterpret_cast<uint8_t* const*>(pool.records + static_cast<size_t>(i) * 24 + 0x10);
            if (obj) {
                int32_t t = *reinterpret_cast<const int32_t*>(obj);
                if (t != -1) return t;
            }
        }
    }

    // 3. Scan active map tagTable (haloreach.dll + 0xC1A600)
    __try {
        uint8_t* tagTable = *reinterpret_cast<uint8_t* const*>(base + kTagTableRva);
        if (tagTable) {
            const int32_t tagCount = std::clamp(*reinterpret_cast<const int32_t*>(tagTable + 0x3FFF8), 0, 0x8000);
            uint32_t wantedMagic = 0;
            if (wantedType == 2) wantedMagic = 0x77656170; // 'weap'
            else if (wantedType == 1) wantedMagic = 0x76656869; // 'vehi'
            else if (wantedType == 3) wantedMagic = 0x65716970; // 'eqip'

            for (int32_t ti = 0; ti < tagCount; ++ti) {
                uint8_t* te = tagTable + static_cast<size_t>(ti) * 8u;
                int16_t groupIdx = *reinterpret_cast<const int16_t*>(te + 0);
                uint16_t salt = *reinterpret_cast<const uint16_t*>(te + 2);
                if (groupIdx < 0 || salt == 0xFFFF) continue;
                uint32_t magic = *reinterpret_cast<const uint32_t*>(tagTable + 0x3FFFC + static_cast<size_t>(groupIdx) * 16u);
                if (magic == wantedMagic) {
                    return static_cast<int32_t>((static_cast<uint32_t>(salt) << 16) | static_cast<uint32_t>(ti));
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    return -1;
}

static void ProcessActions(uintptr_t base, const ObjectPool& pool, const PlayerState& player) {
    std::vector<QueuedAction> actions;
    {
        std::lock_guard<std::mutex> lock(s_actionMutex);
        if (s_actionQueue.empty()) return;
        actions = std::move(s_actionQueue);
        s_actionQueue.clear();
    }

    for (const auto& act : actions) {
        int32_t tagIndex = ResolveTagIndex(base, act.tagPath, act.category, act.tagIndex, pool);
        if (tagIndex == -1 || tagIndex == 0xFFFFFFFF) {
            SetStatus("Tag unavailable in active match: " + act.itemName);
            continue;
        }

        // Spawn directly in front of the player
        const float spawnDist = act.giveToPlayer ? 0.8f : ((act.category == "Vehicles") ? 5.0f : ((act.category == "Characters") ? 3.0f : 2.5f));
        const float spawnZ    = act.giveToPlayer ? 0.4f : 0.3f;
        const float spawnPos[3] = {
            player.pos[0] + player.fwd[0] * spawnDist,
            player.pos[1] + player.fwd[1] * spawnDist,
            player.pos[2] + spawnZ
        };

        alignas(16) uint8_t placement[kPlacementSize + 16];
        std::memset(placement, 0, sizeof(placement));
        if (s_placementNew) {
            s_placementNew(placement, tagIndex, -1, nullptr);
        } else {
            *reinterpret_cast<int32_t*>(placement) = tagIndex;
        }

        std::memcpy(placement + 0x1C, spawnPos, sizeof(float) * 3);
        if (act.category == "Characters") {
            const float faceBack[3] = { -player.fwd[0], -player.fwd[1], 0.0f };
            std::memcpy(placement + 0x28, faceBack, sizeof(float) * 3);
        } else {
            std::memcpy(placement + 0x28, player.fwd, sizeof(float) * 3);
        }

        int32_t handle = s_objectNew ? s_objectNew(placement) : -1;
        if (handle != -1) {
            NOBLE_INFO("DevCheats: s_objectNew created handle 0x{:08X} for '{}' (tag 0x{:08X}) at ({:.1f}, {:.1f}, {:.1f})",
                handle, act.itemName, static_cast<uint32_t>(tagIndex), spawnPos[0], spawnPos[1], spawnPos[2]);

            if (act.giveToPlayer && act.category != "Vehicles" && act.category != "Characters" && act.category != "Props / Scenery") {
                if (s_unitAddWeapon && player.unitHandle != -1) {
                    s_unitAddWeapon(player.unitHandle, 0, handle, 0);
                }
                SetStatus("Gave " + act.itemName + " to your hands!");
            } else {
                SetStatus("Spawned " + act.itemName + " in front of you!");
            }
        } else {
            SetStatus("Engine refused spawn for " + act.itemName + " (tag 0x" + std::to_string(tagIndex) + ")");
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
    return ObjectData(pool, static_cast<int32_t>(datum));
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

static void ReadEquippedTagsSEH(const uint8_t* biped, const ObjectPool& pool, int32_t* primary, int32_t* secondary) {
    __try {
        const int32_t w0 = *reinterpret_cast<const int32_t*>(biped + 0x2C4);
        const int32_t w1 = *reinterpret_cast<const int32_t*>(biped + 0x2C8);
        if (w0 != -1) {
            const uint8_t* obj = ObjectData(pool, w0);
            if (obj) *primary = *reinterpret_cast<const int32_t*>(obj);
        }
        if (w1 != -1) {
            const uint8_t* obj = ObjectData(pool, w1);
            if (obj) *secondary = *reinterpret_cast<const int32_t*>(obj);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

std::vector<SpawnCatalogItem> DevCheats::GetActiveMapItems() {
    std::vector<SpawnCatalogItem> items;
    if (!s_baseAddress) return items;

    ObjectPool pool;
    if (!GetObjectPool(s_baseAddress, pool)) return items;

    PlayerState player{};
    const bool havePlayer = GetLocalPlayer(s_baseAddress, pool, player);
    int32_t primaryWeapTag = -1;
    int32_t secondaryWeapTag = -1;
    if (havePlayer && player.biped) {
        ReadEquippedTagsSEH(player.biped, pool, &primaryWeapTag, &secondaryWeapTag);
    }

    auto getFriendlyName = [&](int32_t tag, uint8_t type, int indexInCat) -> std::pair<std::string, std::string> {
        if (tag == primaryWeapTag && primaryWeapTag != -1) {
            return { "Equipped Primary Weapon", "Currently held in your primary weapon slot." };
        }
        if (tag == secondaryWeapTag && secondaryWeapTag != -1) {
            return { "Equipped Secondary Weapon", "Currently held in your secondary weapon slot." };
        }

        // Known tags from engine and donor maps
        switch (static_cast<uint32_t>(tag)) {
        case 0x96022DF2: return { "Assault Rifle (MA37)", "Standard UNSC individual combat weapon system." };
        case 0x95e82c96: return { "Flyable Pelican", "D77-TC dropship (flyable vehicle)." };
        case 0x96102cb9: return { "Spartan AI Character", "Noble Team / Spartan biped actor." };
        case 0x98952f09: return { "Marine / Trooper AI", "UNSC Army infantry trooper biped." };
        case 0x98ad2f21: return { "ODST AI", "Orbital Drop Shock Trooper biped." };
        case 0x9ddc3336: return { "Grunt AI (Unggoy)", "Covenant infantry biped." };
        case 0x9f5f34b0: return { "Jackal AI (Kig-Yar)", "Covenant shield scout biped." };
        case 0x9f6b34bc: return { "Skirmisher AI", "Covenant shock infantry biped." };
        case 0xa24f370e: return { "Elite AI (Sangheili)", "Covenant warrior biped." };
        case 0xa37c3828: return { "Hunter AI (Mgalekgolo)", "Heavy armored Covenant pair biped." };
        case 0xa48e3927: return { "Drone AI (Yanme'e)", "Aerial Covenant swarm biped." };
        case 0xad234074: return { "Elite Pinky AI", "High-rank Sangheili biped." };
        case 0xa26c372b: return { "Elite Schism AI", "Spec-Ops Sangheili biped." };
        case 0x98992f0d: return { "Deviant Marine AI", "Specialist UNSC trooper biped." };
        case 0x98342ea9: return { "Spartan Ret AI", "Support Spartan biped." };
        default: break;
        }

        std::string catName = "Item";
        if (type == 2) catName = "Weapon";
        else if (type == 1) catName = "Vehicle";
        else if (type == 3) catName = "Equipment";
        else if (type == 0) catName = "Character";
        else if (type == 6 || type == 10) catName = "Prop";

        char buf[64];
        snprintf(buf, sizeof(buf), "Active %s #%d [0x%08X]", catName.c_str(), indexInCat, static_cast<unsigned int>(tag));
        char desc[128];
        snprintf(desc, sizeof(desc), "Live %s tag found in current match object pool.", catName.c_str());
        return { std::string(buf), std::string(desc) };
    };

    std::unordered_set<int32_t> seenTags;
    std::vector<SpawnCatalogItem> weapons;
    std::vector<SpawnCatalogItem> vehicles;
    std::vector<SpawnCatalogItem> equipment;
    std::vector<SpawnCatalogItem> characters;
    std::vector<SpawnCatalogItem> props;

    int weapCount = 0, vehCount = 0, eqCount = 0, charCount = 0, propCount = 0;

    for (int i = 0; i < pool.capacity; ++i) {
        if (!SlotUsed(pool, i)) continue;
        const uint8_t type = pool.records[static_cast<size_t>(i) * 24 + 4];
        if (type == 0 || type == 1 || type == 2 || type == 3 || type == 6 || type == 10) {
            const uint8_t* obj = *reinterpret_cast<uint8_t* const*>(pool.records + static_cast<size_t>(i) * 24 + 0x10);
            if (!obj) continue;
            const int32_t t = *reinterpret_cast<const int32_t*>(obj);
            if (t == -1 || seenTags.count(t)) continue;
            seenTags.insert(t);

            if (type == 2) {
                weapCount++;
                auto [name, desc] = getFriendlyName(t, type, weapCount);
                if (t == primaryWeapTag) {
                    weapons.insert(weapons.begin(), { name, "Weapons", "", static_cast<uint32_t>(t), desc });
                } else if (t == secondaryWeapTag) {
                    auto it = (weapons.empty() || weapons.front().name != "Equipped Primary Weapon") ? weapons.begin() : weapons.begin() + 1;
                    weapons.insert(it, { name, "Weapons", "", static_cast<uint32_t>(t), desc });
                } else {
                    weapons.push_back({ name, "Weapons", "", static_cast<uint32_t>(t), desc });
                }
            } else if (type == 1) {
                vehCount++;
                auto [name, desc] = getFriendlyName(t, type, vehCount);
                vehicles.push_back({ name, "Vehicles", "", static_cast<uint32_t>(t), desc });
            } else if (type == 3) {
                eqCount++;
                auto [name, desc] = getFriendlyName(t, type, eqCount);
                equipment.push_back({ name, "Equipment", "", static_cast<uint32_t>(t), desc });
            } else if (type == 0) {
                charCount++;
                auto [name, desc] = getFriendlyName(t, type, charCount);
                characters.push_back({ name, "Characters", "", static_cast<uint32_t>(t), desc });
            } else if (type == 6 || type == 10) {
                propCount++;
                auto [name, desc] = getFriendlyName(t, type, propCount);
                props.push_back({ name, "Props / Scenery", "", static_cast<uint32_t>(t), desc });
            }
        }
    }

    items.insert(items.end(), weapons.begin(), weapons.end());
    items.insert(items.end(), vehicles.begin(), vehicles.end());
    items.insert(items.end(), equipment.begin(), equipment.end());
    items.insert(items.end(), characters.begin(), characters.end());
    items.insert(items.end(), props.begin(), props.end());

    return items;
}

bool DevCheats::Initialize(HMODULE hReachDll) {
    if (s_initialized || !hReachDll) return s_initialized;

    s_baseAddress = reinterpret_cast<uintptr_t>(hReachDll);

    // Resolve native engine function pointers
    s_placementNew      = reinterpret_cast<PlacementNewFn>(s_baseAddress + kPlacementNewRva);
    s_objectNew         = reinterpret_cast<ObjectNewFn>(s_baseAddress + kObjectNewRva);
    s_getLocalUnit      = reinterpret_cast<GetLocalUnitFn>(s_baseAddress + kGetLocalUnitRva);
    s_unitAddWeapon     = reinterpret_cast<UnitAddWeaponFn>(s_baseAddress + kUnitAddWeaponRva);
    s_shieldRestore     = reinterpret_cast<ShieldRestoreFn>(s_baseAddress + kShieldRestoreRva);

    // 1. Hook object_cause_damage (RVA 0x1ED320)
    void* damageTarget = reinterpret_cast<void*>(s_baseAddress + kDamageObjectRva);
    HookManager::InstallHook(damageTarget, reinterpret_cast<void*>(&Hook_DamageObject),
                             reinterpret_cast<void**>(&s_origDamageObject), "object_cause_damage");

    // 2. Hook object_kill (RVA 0x47CAA8)
    void* killTarget = reinterpret_cast<void*>(s_baseAddress + kObjectKillRva);
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

void DevCheats::GameTick([[maybe_unused]] int32_t userIndex) {
    if (!s_initialized || !s_baseAddress) return;

    ObjectPool pool;
    if (!GetObjectPool(s_baseAddress, pool)) return;

    PlayerState player{};
    const bool havePlayer = GetLocalPlayer(s_baseAddress, pool, player);

    s_localUnitDatum.store(player.unitHandle);
    s_localVehicleDatum.store(player.vehicleHandle);

    if (havePlayer && player.biped) {
        // 1. Apply Godmode
        if (s_godmode.load()) {
            if (s_shieldRestore && player.unitHandle != -1) {
                s_shieldRestore(player.unitHandle);
                if (player.vehicleHandle != -1) s_shieldRestore(player.vehicleHandle);
            }
            __try {
                // Direct shield & health write
                *reinterpret_cast<float*>(player.biped + 0xDC) = 1.0f; // shield
                *reinterpret_cast<float*>(player.biped + 0xE0) = 1.0f; // health

                // Health component (if present)
                int16_t healthOffset = *reinterpret_cast<const int16_t*>(player.biped + 0x186);
                if (healthOffset > 0 && healthOffset < 0x2000) {
                    uint8_t* healthComp = player.biped + healthOffset;
                    *reinterpret_cast<float*>(healthComp + 0x10) = 1.0f;
                    *reinterpret_cast<float*>(healthComp + 0x14) = 1.0f;
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        // 2. Super Jump & Super Speed
        float* vel = reinterpret_cast<float*>(player.biped + 0x68);
        if (s_superSpeed.load() && !s_noclip.load() && !s_freecam.load()) {
            float moveFwd = 0.0f, moveRight = 0.0f;
            if (GetAsyncKeyState('W') & 0x8000) moveFwd += 1.0f;
            if (GetAsyncKeyState('S') & 0x8000) moveFwd -= 1.0f;
            if (GetAsyncKeyState('D') & 0x8000) moveRight += 1.0f;
            if (GetAsyncKeyState('A') & 0x8000) moveRight -= 1.0f;

            if (moveFwd != 0.0f || moveRight != 0.0f) {
                float fwdLen = std::hypot(player.fwd[0], player.fwd[1]);
                if (fwdLen > 0.001f) {
                    const float fwd[2]   = { player.fwd[0] / fwdLen, player.fwd[1] / fwdLen };
                    const float right[2] = { fwd[1], -fwd[0] };
                    const float mult = s_speedMultiplier.load();
                    const float boost = (mult - 1.0f) * 0.12f;

                    float* pos = reinterpret_cast<float*>(player.biped + 0x44);
                    pos[0] += (fwd[0] * moveFwd + right[0] * moveRight) * boost;
                    pos[1] += (fwd[1] * moveFwd + right[1] * moveRight) * boost;

                    vel[0] = (fwd[0] * moveFwd + right[0] * moveRight) * (0.15f * mult);
                    vel[1] = (fwd[1] * moveFwd + right[1] * moveRight) * (0.15f * mult);
                }
            }
        }

        if (s_superJump.load() && !s_noclip.load() && !s_freecam.load()) {
            if ((GetAsyncKeyState(VK_SPACE) & 0x8000) || vel[2] > 0.010f) {
                vel[2] = (std::max)(vel[2], 0.16f * s_jumpMultiplier.load());
            }
        }

        // 3. Noclip (Ghost Flight)
        if (s_noclip.load()) {
            float* pos = reinterpret_cast<float*>(player.biped + 0x44);
            const float* fwd = reinterpret_cast<const float*>(player.biped + 0x50);
            const float* up  = reinterpret_cast<const float*>(player.biped + 0x5C);

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

        // 4. Freecam Pan: freeze player biped so legs/body do not walk away!
        if (s_freecam.load()) {
            vel[0] = 0.0f; vel[1] = 0.0f; vel[2] = 0.0f;

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

            const float speed = s_freecamSpeed.load() * (1.0f / 60.0f);
            float fwdLen = std::hypot(s_lastCamFwd[0], s_lastCamFwd[1]);
            if (fwdLen < 0.001f) fwdLen = 1.0f;
            const float hFwd[2]   = { s_lastCamFwd[0] / fwdLen, s_lastCamFwd[1] / fwdLen };
            const float hRight[2] = { hFwd[1], -hFwd[0] };

            s_freecamPos[0] += (hFwd[0] * moveFwd + hRight[0] * moveRight) * speed;
            s_freecamPos[1] += (hFwd[1] * moveFwd + hRight[1] * moveRight) * speed;
            s_freecamPos[2] += (s_lastCamFwd[2] * moveFwd + moveUp) * speed;
        }

        // 5. Infinite Ammo & Bottomless Clip
        if (s_infiniteAmmo.load() || s_bottomlessClip.load()) {
            __try {
                // Weapons inventory in unit record (slots at unit + 0x2C4, active index at +0x2C0)
                int16_t activeSlot = *reinterpret_cast<const int16_t*>(player.biped + 0x2C0);
                if (activeSlot >= 0 && activeSlot < 4) {
                    int32_t weapHandle = *reinterpret_cast<const int32_t*>(player.biped + 0x2C4 + static_cast<size_t>(activeSlot) * 4);
                    if (weapHandle != -1) {
                        uint8_t* weap = ObjectData(pool, weapHandle);
                        if (weap) {
                            if (s_bottomlessClip.load()) {
                                *reinterpret_cast<int16_t*>(weap + 0x3030) = 999; // magazine ammo
                            }
                            if (s_infiniteAmmo.load()) {
                                *reinterpret_cast<int16_t*>(weap + 0x3034) = 999; // reserve ammo
                            }
                        }
                    }
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        // 6. Process Item Spawn & Give requests
        ProcessActions(s_baseAddress, pool, player);
    }
}

void DevCheats::BuildCatalog() {
    s_catalog.clear();

    // UNSC Weapons
    s_catalog.push_back({ "Assault Rifle", "UNSC Weapons", "objects/weapons/rifle/assault_rifle/assault_rifle", 0xFFFFFFFF, "MA37 Individual Combat Weapon System." });
    s_catalog.push_back({ "DMR", "UNSC Weapons", "objects/weapons/rifle/dmr/dmr", 0xFFFFFFFF, "M392 Designated Marksman Rifle." });
    s_catalog.push_back({ "Magnum", "UNSC Weapons", "objects/weapons/pistol/magnum/magnum", 0xFFFFFFFF, "M6G semi-automatic sidearm." });
    s_catalog.push_back({ "Sniper Rifle", "UNSC Weapons", "objects/weapons/rifle/sniper_rifle/sniper_rifle", 0xFFFFFFFF, "SRS99D Anti-Materiel 14.5mm sniper." });
    s_catalog.push_back({ "Rocket Launcher", "UNSC Weapons", "objects/weapons/support_high/rocket_launcher/rocket_launcher", 0xFFFFFFFF, "M41 SSR AV/AA twin rocket launcher." });
    s_catalog.push_back({ "Shotgun", "UNSC Weapons", "objects/weapons/rifle/shotgun/shotgun", 0xFFFFFFFF, "M45 Tactical 8-gauge shotgun." });
    s_catalog.push_back({ "Grenade Launcher", "UNSC Weapons", "objects/weapons/support_low/grenade_launcher/grenade_launcher", 0xFFFFFFFF, "M319 Individual 40mm launcher." });
    s_catalog.push_back({ "Spartan Laser", "UNSC Weapons", "objects/weapons/support_high/spartan_laser/spartan_laser", 0xFFFFFFFF, "W/AV M6 G/GNR directed energy beam." });

    // Covenant Weapons
    s_catalog.push_back({ "Plasma Pistol", "Covenant Weapons", "objects/weapons/pistol/plasma_pistol/plasma_pistol", 0xFFFFFFFF, "Type-25 Directed Energy Pistol." });
    s_catalog.push_back({ "Plasma Rifle", "Covenant Weapons", "objects/weapons/rifle/plasma_rifle/plasma_rifle", 0xFFFFFFFF, "Type-25 Directed Energy Rifle." });
    s_catalog.push_back({ "Needler", "Covenant Weapons", "objects/weapons/pistol/needler/needler", 0xFFFFFFFF, "Type-33 Guided Munitions launcher." });
    s_catalog.push_back({ "Energy Sword", "Covenant Weapons", "objects/weapons/melee/energy_sword/energy_sword", 0xFFFFFFFF, "Type-1 close combat energy blade." });
    s_catalog.push_back({ "Gravity Hammer", "Covenant Weapons", "objects/weapons/melee/gravity_hammer/gravity_hammer", 0xFFFFFFFF, "Type-2 Energy Weapon / Fist of Rukt." });
    s_catalog.push_back({ "Fuel Rod Gun", "Covenant Weapons", "objects/weapons/support_high/flak_cannon/flak_cannon", 0xFFFFFFFF, "Type-33 heavy explosive assault cannon." });
    s_catalog.push_back({ "Concussion Rifle", "Covenant Weapons", "objects/weapons/support_low/concussion_rifle/concussion_rifle", 0xFFFFFFFF, "Type-50 explosive plasma propellant." });
    s_catalog.push_back({ "Plasma Repeater", "Covenant Weapons", "objects/weapons/rifle/plasma_repeater/plasma_repeater", 0xFFFFFFFF, "Type-51 manual venting plasma rifle." });
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
