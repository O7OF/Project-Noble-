#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace noble {

class ReachArmory {
public:
    static void Initialize(const std::filesystem::path& dataDir);

    static void SetMenuMode(bool menuMode);
    static bool IsMenuMode();

    static void SetTargetScene(const std::string& sceneName);
    static std::string GetTargetScene();

    static void AddYawRotation(float deltaRadians);
    static void SetYawRotation(float radians);
    static float GetYawRotation();

    // Called every frame on the engine render thread inside HostSlot1_FrameTick.
    static void Tick();

    // True once mainmenu_all.map director scene table is active.
    static bool IsSceneReady();
};

} // namespace noble

