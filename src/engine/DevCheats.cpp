#include "DevCheats.hpp"
#include "SharedTagCache.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"
#include <cmath>
#include <algorithm>

namespace noble {

bool DevCheats::s_initialized = false;
uintptr_t DevCheats::s_baseAddress = 0;

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

// haloreach.dll+0x1ED320: object_cause_damage
using DamageObjectFn = void(__fastcall*)(uint32_t, uint32_t, float, void*, void*, void*);
static DamageObjectFn s_origDamageObject = nullptr;

static void __fastcall Hook_DamageObject(uint32_t objectDatum, uint32_t marker, float damageAmount, void* a4, void* a5, void* a6) {
    if (DevCheats::IsGodmode()) {
        const uint32_t localUnit = DevCheats::GetLocalPlayerUnitDatum();
        const uint32_t localVeh = DevCheats::GetLocalPlayerVehicleDatum();
        if (objectDatum != 0xFFFFFFFF && (objectDatum == localUnit || (localVeh != 0xFFFFFFFF && objectDatum == localVeh))) {
            return; // GODMODE: Completely block incoming damage
        }
    }
    if (s_origDamageObject) {
        s_origDamageObject(objectDatum, marker, damageAmount, a4, a5, a6);
    }
}

// haloreach.dll+0x47CAA8: object_kill
using ObjectKillFn = void(__fastcall*)(uint32_t, uint32_t, uint8_t, uint32_t);
static ObjectKillFn s_origObjectKill = nullptr;

static void __fastcall Hook_ObjectKill(uint32_t objectDatum, uint32_t flags, uint8_t a3, uint32_t a4) {
    if (DevCheats::IsGodmode()) {
        const uint32_t localUnit = DevCheats::GetLocalPlayerUnitDatum();
        const uint32_t localVeh = DevCheats::GetLocalPlayerVehicleDatum();
        if (objectDatum != 0xFFFFFFFF && (objectDatum == localUnit || (localVeh != 0xFFFFFFFF && objectDatum == localVeh))) {
            return; // GODMODE: Completely block death
        }
    }
    if (s_origObjectKill) {
        s_origObjectKill(objectDatum, flags, a3, a4);
    }
}

// Isolated SEH helpers to avoid C2712
static uint8_t* SafeResolveObject(uintptr_t base, uint32_t datum) {
    if (!base || datum == 0xFFFFFFFF || datum == 0) return nullptr;
    __try {
        uint32_t tlsSlot = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        uintptr_t* tls = reinterpret_cast<uintptr_t*>(__readgsqword(0x58));
        if (!tls) return nullptr;
        uintptr_t threadStorage = tls[tlsSlot];
        if (!threadStorage) return nullptr;
        uintptr_t objectTable = *reinterpret_cast<const uintptr_t*>(threadStorage + 0x10);
        if (!objectTable) return nullptr;
        uintptr_t slots = *reinterpret_cast<const uintptr_t*>(objectTable + 0x50);
        if (!slots) return nullptr;
        uint16_t index = static_cast<uint16_t>(datum & 0xFFFF);
        return *reinterpret_cast<uint8_t**>(slots + index * 24 + 0x10);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

static uint32_t SafeGetLocalUnit(uintptr_t base) {
    if (!base) return 0xFFFFFFFF;
    auto fn = reinterpret_cast<int32_t(__fastcall*)(int32_t)>(base + 0x53EF8);
    if (!fn) return 0xFFFFFFFF;
    __try {
        int32_t unit = fn(0);
        return (unit == -1 || unit == 0) ? 0xFFFFFFFF : static_cast<uint32_t>(unit);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0xFFFFFFFF;
    }
}

static uint32_t SafeGetUltimateParent(uintptr_t base, uint32_t unit) {
    if (!base || unit == 0xFFFFFFFF) return 0xFFFFFFFF;
    auto fn = reinterpret_cast<int32_t(__fastcall*)(int32_t)>(base + 0x473DC0);
    if (!fn) return 0xFFFFFFFF;
    __try {
        int32_t parent = fn(static_cast<int32_t>(unit));
        return (parent == -1 || parent == 0 || static_cast<uint32_t>(parent) == unit) ? 0xFFFFFFFF : static_cast<uint32_t>(parent);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0xFFFFFFFF;
    }
}

static void SafeRestoreShields(uintptr_t base, uint32_t unitDatum) {
    if (!base || unitDatum == 0xFFFFFFFF) return;
    auto fn = reinterpret_cast<void(__fastcall*)(int32_t)>(base + 0x49DD58);
    if (!fn) return;
    __try {
        fn(static_cast<int32_t>(unitDatum));
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

uint8_t* DevCheats::GetObjectByDatum(uint32_t datum) {
    return SafeResolveObject(s_baseAddress, datum);
}

uint32_t DevCheats::GetLocalPlayerUnitDatum() {
    return SafeGetLocalUnit(s_baseAddress);
}

uint32_t DevCheats::GetLocalPlayerVehicleDatum() {
    uint32_t unit = GetLocalPlayerUnitDatum();
    return SafeGetUltimateParent(s_baseAddress, unit);
}

void DevCheats::SetFreecam(bool val) {
    s_freecam.store(val);
    if (!val) {
        s_freecamInit = false;
    }
}

bool DevCheats::IsFreecamActive() {
    return s_freecam.load();
}

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

bool DevCheats::Initialize(HMODULE hReachDll) {
    if (s_initialized || !hReachDll) return s_initialized;

    s_baseAddress = reinterpret_cast<uintptr_t>(hReachDll);

    // 1. Hook object_cause_damage (RVA 0x1ED320)
    void* damageTarget = reinterpret_cast<void*>(s_baseAddress + 0x1ED320);
    if (HookManager::InstallHook(damageTarget, reinterpret_cast<void*>(&Hook_DamageObject),
                                 reinterpret_cast<void**>(&s_origDamageObject), "object_cause_damage")) {
        NOBLE_INFO("DevCheats: Hooked object_cause_damage at RVA 0x1ED320.");
    } else {
        NOBLE_WARN("DevCheats: Failed to install object_cause_damage hook at RVA 0x1ED320.");
    }

    // 2. Hook object_kill (RVA 0x47CAA8)
    void* killTarget = reinterpret_cast<void*>(s_baseAddress + 0x47CAA8);
    if (HookManager::InstallHook(killTarget, reinterpret_cast<void*>(&Hook_ObjectKill),
                                 reinterpret_cast<void**>(&s_origObjectKill), "object_kill")) {
        NOBLE_INFO("DevCheats: Hooked object_kill at RVA 0x47CAA8.");
    } else {
        NOBLE_WARN("DevCheats: Failed to install object_kill hook at RVA 0x47CAA8.");
    }

    BuildCatalog();

    s_initialized = true;
    NOBLE_INFO("DevCheats: Developer Cheats system initialized.");
    return true;
}

void DevCheats::Shutdown() {
    s_initialized = false;
}

void DevCheats::Tick(float dt) {
    if (!s_initialized || dt <= 0.0f) return;

    const uint32_t localUnitDatum = GetLocalPlayerUnitDatum();
    if (localUnitDatum == 0xFFFFFFFF) return;

    uint8_t* biped = GetObjectByDatum(localUnitDatum);
    if (!biped) return;

    // 1. Godmode: keep shields full
    if (s_godmode.load()) {
        SafeRestoreShields(s_baseAddress, localUnitDatum);
    }

    // 2. Super Jump & Super Speed
    float* vel = reinterpret_cast<float*>(biped + 0x68);
    if (s_superSpeed.load() && !s_noclip.load()) {
        const float speedMult = s_speedMultiplier.load();
        const float hSpeed = std::hypot(vel[0], vel[1]);
        if (hSpeed > 0.005f) {
            vel[0] *= (1.0f + (speedMult - 1.0f) * 0.15f);
            vel[1] *= (1.0f + (speedMult - 1.0f) * 0.15f);
        }
    }

    if (s_superJump.load() && !s_noclip.load()) {
        const float jumpMult = s_jumpMultiplier.load();
        if (vel[2] > 0.02f) {
            vel[2] = std::max(vel[2], 0.11f * jumpMult);
        }
    }

    // 3. Noclip / Free Flight
    if (s_noclip.load()) {
        float* pos = reinterpret_cast<float*>(biped + 0x44);
        const float* fwd = reinterpret_cast<const float*>(biped + 0x50);
        const float* up  = reinterpret_cast<const float*>(biped + 0x5C);

        vel[0] = 0.0f;
        vel[1] = 0.0f;
        vel[2] = 0.0f;

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

        const float ncSpeed = s_noclipSpeed.load();
        pos[0] += (fwd[0] * moveFwd + right[0] * moveRight) * ncSpeed * dt;
        pos[1] += (fwd[1] * moveFwd + right[1] * moveRight) * ncSpeed * dt;
        pos[2] += (fwd[2] * moveFwd + moveUp) * ncSpeed * dt;
    }

    // 4. Freecam navigation (Arrow keys / E & Q)
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

        const float fcSpeed = s_freecamSpeed.load();
        s_freecamPos[0] += (s_lastCamFwd[0] * moveFwd - s_lastCamFwd[1] * moveRight) * fcSpeed * dt;
        s_freecamPos[1] += (s_lastCamFwd[1] * moveFwd + s_lastCamFwd[0] * moveRight) * fcSpeed * dt;
        s_freecamPos[2] += (s_lastCamFwd[2] * moveFwd + moveUp) * fcSpeed * dt;
    }

    // 5. Infinite Ammo & Bottomless Clip
    const bool infAmmo = s_infiniteAmmo.load();
    const bool bottomless = s_bottomlessClip.load();
    if (infAmmo || bottomless) {
        for (int slot = 0; slot < 4; ++slot) {
            uint32_t weaponDatum = *reinterpret_cast<const uint32_t*>(biped + 0x350 + slot * 4);
            if (weaponDatum != 0xFFFFFFFF && weaponDatum != 0) {
                uint8_t* weapon = GetObjectByDatum(weaponDatum);
                if (weapon) {
                    if (infAmmo) {
                        *reinterpret_cast<uint32_t*>(weapon + 0x3034) = 999; // reserve ammo
                    }
                    if (bottomless) {
                        *reinterpret_cast<uint32_t*>(weapon + 0x3030) = 99;  // loaded clip rounds
                    }
                }
            }
        }
    }
}

void DevCheats::SpawnItem(const std::string& tagPath) {
    if (tagPath.empty()) return;
    SharedTagCache::QueueSpawn(0xFFFFFFFF, tagPath);
}

const std::vector<SpawnCatalogItem>& DevCheats::GetCatalog() {
    return s_catalog;
}

void DevCheats::BuildCatalog() {
    s_catalog.clear();

    // UNSC Weapons
    s_catalog.push_back({ "Assault Rifle", "UNSC Weapons", "objects/weapons/rifle/assault_rifle/assault_rifle", "Standard issue MA37 automatic rifle." });
    s_catalog.push_back({ "DMR", "UNSC Weapons", "objects/weapons/rifle/dmr/dmr", "Designated Marksman Rifle (M392)." });
    s_catalog.push_back({ "Magnum", "UNSC Weapons", "objects/weapons/pistol/magnum/magnum", "M6G semi-automatic sidearm." });
    s_catalog.push_back({ "Shotgun", "UNSC Weapons", "objects/weapons/shotgun/shotgun", "M45 Tactical 8-gauge shotgun." });
    s_catalog.push_back({ "Sniper Rifle", "UNSC Weapons", "objects/weapons/rifle/sniper_rifle/sniper_rifle", "SRS99D Anti-Materiel sniper rifle." });
    s_catalog.push_back({ "Rocket Launcher", "UNSC Weapons", "objects/weapons/support_high/rocket_launcher/rocket_launcher", "M41 SSR AV/AA twin-tube launcher." });
    s_catalog.push_back({ "Spartan Laser", "UNSC Weapons", "objects/weapons/support_high/spartan_laser/spartan_laser", "M6 G/GNR directed-energy weapon." });
    s_catalog.push_back({ "Grenade Launcher", "UNSC Weapons", "objects/weapons/pistol/grenade_launcher/grenade_launcher", "M319 individual explosive launcher." });
    s_catalog.push_back({ "Target Locator", "UNSC Weapons", "objects/weapons/support_high/target_locator/target_locator", "Orbital bombardment designator." });
    s_catalog.push_back({ "Machine Gun Turret", "UNSC Weapons", "objects/weapons/turret/machinegun_turret/machinegun_turret", "Detachable AIE-486H heavy turret." });

    // Covenant Weapons
    s_catalog.push_back({ "Plasma Pistol", "Covenant Weapons", "objects/weapons/pistol/plasma_pistol/plasma_pistol", "Type-25 directed energy pistol with EMP." });
    s_catalog.push_back({ "Plasma Rifle", "Covenant Weapons", "objects/weapons/rifle/plasma_rifle/plasma_rifle", "Type-25 rapid plasma rifle." });
    s_catalog.push_back({ "Plasma Repeater", "Covenant Weapons", "objects/weapons/rifle/plasma_repeater/plasma_repeater", "Type-51 manual-vent automatic rifle." });
    s_catalog.push_back({ "Needle Rifle", "Covenant Weapons", "objects/weapons/rifle/needle_rifle/needle_rifle", "Type-31 marksman sub-munitions rifle." });
    s_catalog.push_back({ "Needler", "Covenant Weapons", "objects/weapons/pistol/needler/needler", "Type-33 tracking crystalline projector." });
    s_catalog.push_back({ "Energy Sword", "Covenant Weapons", "objects/weapons/melee/energy_sword/energy_sword", "Type-1 plasma melee blade." });
    s_catalog.push_back({ "Gravity Hammer", "Covenant Weapons", "objects/weapons/melee/gravity_hammer/gravity_hammer", "Type-2 kinetic gravity shock hammer." });
    s_catalog.push_back({ "Concussion Rifle", "Covenant Weapons", "objects/weapons/support_high/concussion_rifle/concussion_rifle", "Type-50 explosive blast weapon." });
    s_catalog.push_back({ "Fuel Rod Gun", "Covenant Weapons", "objects/weapons/support_high/fuel_rod_gun/fuel_rod_gun", "Type-33 heavy explosive cannon." });
    s_catalog.push_back({ "Plasma Launcher", "Covenant Weapons", "objects/weapons/support_high/plasma_launcher/plasma_launcher", "Type-52 quad tracking launcher." });
    s_catalog.push_back({ "Focus Rifle", "Covenant Weapons", "objects/weapons/rifle/focus_rifle/focus_rifle", "Type-52 continuous beam sniper." });
    s_catalog.push_back({ "Spiker", "Covenant Weapons", "objects/weapons/pistol/spike_rifle/spike_rifle", "Type-25 brutal projectile carbine." });

    // Equipment & Armor Abilities
    s_catalog.push_back({ "Jetpack", "Armor Abilities", "objects/equipment/jet_pack/jet_pack", "Series 8 personal flight thruster pack." });
    s_catalog.push_back({ "Sprint", "Armor Abilities", "objects/equipment/sprint/sprint", "Burst locomotion accelerator." });
    s_catalog.push_back({ "Armor Lock", "Armor Abilities", "objects/equipment/armor_lock/armor_lock", "Complete invulnerable kinetic shielding." });
    s_catalog.push_back({ "Active Camouflage", "Armor Abilities", "objects/equipment/active_camo/active_camo", "Adaptive light-bending stealth emitter." });
    s_catalog.push_back({ "Drop Shield", "Armor Abilities", "objects/equipment/drop_shield/drop_shield", "Deployable hemispherical energy shield." });
    s_catalog.push_back({ "Evade", "Armor Abilities", "objects/equipment/evade/evade", "Tactical evasive combat roll." });
    s_catalog.push_back({ "Hologram", "Armor Abilities", "objects/equipment/hologram/hologram", "Solid light projection decoy." });

    // Vehicles
    s_catalog.push_back({ "Warthog", "Vehicles", "objects/vehicles/human/warthog/warthog", "M12 LRV all-terrain reconnaissance vehicle." });
    s_catalog.push_back({ "Mongoose", "Vehicles", "objects/vehicles/human/mongoose/mongoose", "M274 light quad utility vehicle." });
    s_catalog.push_back({ "Scorpion", "Vehicles", "objects/vehicles/human/scorpion/scorpion", "M808B heavy battle tank with 90mm cannon." });
    s_catalog.push_back({ "Falcon", "Vehicles", "objects/vehicles/human/falcon/falcon", "UH-144 troop transport / close air support." });
    s_catalog.push_back({ "Ghost", "Vehicles", "objects/vehicles/covenant/ghost/ghost", "Type-32 rapid assault vehicle." });
    s_catalog.push_back({ "Banshee", "Vehicles", "objects/vehicles/covenant/banshee/banshee", "Type-26 aerial attack craft." });
    s_catalog.push_back({ "Wraith", "Vehicles", "objects/vehicles/covenant/wraith/wraith", "Type-26 heavy mortar tank." });
    s_catalog.push_back({ "Flyable Pelican", "Vehicles", "objects/vehicles/human/pelican/pelican", "D77-TC dropship (flyable with wraith_d seat)." });
}

} // namespace noble
