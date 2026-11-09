#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <atomic>

#if defined(NOBLE_DEV_BUILD)

namespace noble {

struct SpawnCatalogItem {
    std::string name;
    std::string category;
    std::string tagPath;
    uint32_t tagIndex = 0xFFFFFFFF;
    std::string description;
};

class DevCheats {
public:
    static bool Initialize(HMODULE hReachDll);
    static void Shutdown();

    // Game tick execution - runs on haloreach.dll GAME THREAD inside Hook_Tick (RVA 0x61A38)
    static void GameTick(int32_t userIndex);

    // Freecam hook handler - executed inside CameraThirdPerson camera update
    static bool IsFreecamActive();
    static void ApplyFreecam(float* camPos, const float* camFwd);

    // Cheat toggles
    static bool IsGodmode();
    static void SetGodmode(bool val);

    static bool IsSuperJump();
    static void SetSuperJump(bool val);
    static float GetJumpMultiplier();
    static void SetJumpMultiplier(float val);

    static bool IsSuperSpeed();
    static void SetSuperSpeed(bool val);
    static float GetSpeedMultiplier();
    static void SetSpeedMultiplier(float val);

    static bool IsNoclip();
    static void SetNoclip(bool val);
    static float GetNoclipSpeed();
    static void SetNoclipSpeed(float val);

    static bool IsFreecam();
    static void SetFreecam(bool val);
    static float GetFreecamSpeed();
    static void SetFreecamSpeed(float val);

    static bool IsInfiniteAmmo();
    static void SetInfiniteAmmo(bool val);

    static bool IsBottomlessClip();
    static void SetBottomlessClip(bool val);

    static bool IsInfiniteEquipment();
    static void SetInfiniteEquipment(bool val);

    // Spawning & Giving
    static void QueueItemAction(const std::string& name, const std::string& category, const std::string& tagPath, uint32_t tagIndex, bool giveToPlayer);
    static const std::vector<SpawnCatalogItem>& GetCatalog();
    static std::vector<SpawnCatalogItem> GetActiveMapItems();

    static std::string GetLastStatus();

    // Memory helpers
    static uint32_t GetLocalPlayerUnitDatum();
    static uint32_t GetLocalPlayerVehicleDatum();
    static uint8_t* GetObjectByDatum(uint32_t datum);

private:
    static bool s_initialized;
    static uintptr_t s_baseAddress;

    static std::atomic<int32_t> s_localUnitDatum;
    static std::atomic<int32_t> s_localVehicleDatum;

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

#else // !defined(NOBLE_DEV_BUILD) - Release build stubs (zero dev code compiled in)

namespace noble {

struct SpawnCatalogItem {
    std::string name;
    std::string category;
    std::string tagPath;
    uint32_t tagIndex = 0xFFFFFFFF;
    std::string description;
};

class DevCheats {
public:
    static bool Initialize(HMODULE) { return false; }
    static void Shutdown() {}
    static void GameTick(int32_t) {}
    static constexpr bool IsFreecamActive() { return false; }
    static void ApplyFreecam(float*, const float*) {}
    static constexpr bool IsGodmode() { return false; }
    static void SetGodmode(bool) {}
    static constexpr bool IsSuperJump() { return false; }
    static void SetSuperJump(bool) {}
    static constexpr float GetJumpMultiplier() { return 1.0f; }
    static void SetJumpMultiplier(float) {}
    static constexpr bool IsSuperSpeed() { return false; }
    static void SetSuperSpeed(bool) {}
    static constexpr float GetSpeedMultiplier() { return 1.0f; }
    static void SetSpeedMultiplier(float) {}
    static constexpr bool IsNoclip() { return false; }
    static void SetNoclip(bool) {}
    static constexpr float GetNoclipSpeed() { return 15.0f; }
    static void SetNoclipSpeed(float) {}
    static constexpr bool IsFreecam() { return false; }
    static void SetFreecam(bool) {}
    static constexpr float GetFreecamSpeed() { return 12.0f; }
    static void SetFreecamSpeed(float) {}
    static constexpr bool IsInfiniteAmmo() { return false; }
    static void SetInfiniteAmmo(bool) {}
    static constexpr bool IsBottomlessClip() { return false; }
    static void SetBottomlessClip(bool) {}
    static constexpr bool IsInfiniteEquipment() { return false; }
    static void SetInfiniteEquipment(bool) {}
    static void QueueItemAction(const std::string&, const std::string&, const std::string&, uint32_t, bool) {}
    static const std::vector<SpawnCatalogItem>& GetCatalog() {
        static const std::vector<SpawnCatalogItem> s_empty;
        return s_empty;
    }
    static std::vector<SpawnCatalogItem> GetActiveMapItems() { return {}; }
    static std::string GetLastStatus() { return ""; }
    static constexpr uint32_t GetLocalPlayerUnitDatum() { return 0xFFFFFFFF; }
    static constexpr uint32_t GetLocalPlayerVehicleDatum() { return 0xFFFFFFFF; }
    static uint8_t* GetObjectByDatum(uint32_t) { return nullptr; }
};

} // namespace noble

#endif
