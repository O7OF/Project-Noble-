#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <atomic>

namespace noble {

struct SpawnCatalogItem {
    std::string name;
    std::string category;
    std::string tagPath;
    std::string description;
};

class DevCheats {
public:
    static bool Initialize(HMODULE hReachDll);
    static void Shutdown();

    // Simulation tick - executed on haloreach.dll simulation thread inside FillLocalInput
    static void Tick(float dt);

    // Freecam hook handler - executed inside CameraThirdPerson camera update
    static bool IsFreecamActive();
    static void ApplyFreecam(float* camPos, const float* camFwd);

    // Cheat toggles
    static bool IsGodmode() { return s_godmode.load(); }
    static void SetGodmode(bool val) { s_godmode.store(val); }

    static bool IsSuperJump() { return s_superJump.load(); }
    static void SetSuperJump(bool val) { s_superJump.store(val); }
    static float GetJumpMultiplier() { return s_jumpMultiplier.load(); }
    static void SetJumpMultiplier(float val) { s_jumpMultiplier.store(val); }

    static bool IsSuperSpeed() { return s_superSpeed.load(); }
    static void SetSuperSpeed(bool val) { s_superSpeed.store(val); }
    static float GetSpeedMultiplier() { return s_speedMultiplier.load(); }
    static void SetSpeedMultiplier(float val) { s_speedMultiplier.store(val); }

    static bool IsNoclip() { return s_noclip.load(); }
    static void SetNoclip(bool val) { s_noclip.store(val); }
    static float GetNoclipSpeed() { return s_noclipSpeed.load(); }
    static void SetNoclipSpeed(float val) { s_noclipSpeed.store(val); }

    static bool IsFreecam() { return s_freecam.load(); }
    static void SetFreecam(bool val);
    static float GetFreecamSpeed() { return s_freecamSpeed.load(); }
    static void SetFreecamSpeed(float val) { s_freecamSpeed.store(val); }

    static bool IsInfiniteAmmo() { return s_infiniteAmmo.load(); }
    static void SetInfiniteAmmo(bool val) { s_infiniteAmmo.store(val); }

    static bool IsBottomlessClip() { return s_bottomlessClip.load(); }
    static void SetBottomlessClip(bool val) { s_bottomlessClip.store(val); }

    static bool IsInfiniteEquipment() { return s_infiniteEquipment.load(); }
    static void SetInfiniteEquipment(bool val) { s_infiniteEquipment.store(val); }

    // Spawning
    static void SpawnItem(const std::string& tagPath);
    static const std::vector<SpawnCatalogItem>& GetCatalog();

    // Memory helpers
    static uint32_t GetLocalPlayerUnitDatum();
    static uint32_t GetLocalPlayerVehicleDatum();
    static uint8_t* GetObjectByDatum(uint32_t datum);

private:
    static bool s_initialized;
    static uintptr_t s_baseAddress;

    static std::atomic<bool> s_godmode;
    static std::atomic<bool> s_superJump;
    static std::atomic<float> s_jumpMultiplier;
    static std::atomic<bool> s_superSpeed;
    static std::atomic<float> s_speedMultiplier;
    static std::atomic<bool> s_noclip;
    static std::atomic<float> s_noclipSpeed;
    static std::atomic<bool> s_freecam;
    static std::atomic<float> s_freecamSpeed;
    static std::atomic<bool> s_infiniteAmmo;
    static std::atomic<bool> s_bottomlessClip;
    static std::atomic<bool> s_infiniteEquipment;

    static float s_freecamPos[3];
    static float s_lastCamFwd[3];
    static bool s_freecamInit;

    static std::vector<SpawnCatalogItem> s_catalog;
    static void BuildCatalog();
};

} // namespace noble
