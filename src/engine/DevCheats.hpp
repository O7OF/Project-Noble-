#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>
#include <windows.h>

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

  // Game tick execution - runs on haloreach.dll GAME THREAD inside Hook_Tick
  // (RVA 0x61A38)
  static void GameTick(int32_t userIndex);

  // Freecam hook handler - executed inside CameraThirdPerson camera update
  static bool IsFreecamActive();
  static void ApplyFreecam(float *camPos, const float *camFwd);

  // Cheat toggles
  static bool IsGodmode();
  static void SetGodmode(bool val);
  // Kills the local player on the engine thread (next GameTick). The pause
  // menu's CHANGE TEAM uses it so a team switch behaves like Reach's: you die
  // and respawn on the new side, instead of walking around with the old team's
  // colours until the next death.
  static void RequestLocalSuicide();

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
  static void QueueItemAction(const std::string &name,
                              const std::string &category,
                              const std::string &tagPath, uint32_t tagIndex,
                              bool giveToPlayer);
  static const std::vector<SpawnCatalogItem> &GetCatalog();
  static std::vector<SpawnCatalogItem> GetActiveMapItems();

  static std::string GetLastStatus();

  // Memory helpers
  static uint32_t GetLocalPlayerUnitDatum();
  static uint32_t GetLocalPlayerVehicleDatum();
  static uint8_t *GetObjectByDatum(uint32_t datum);

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
