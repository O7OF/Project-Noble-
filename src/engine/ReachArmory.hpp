#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

namespace noble {

struct ReachArmorySelection {
    bool isElite = false;
    bool isFemale = false;

    // Spartan permutation string_id names (matching mainmenu_all.string_ids.csv)
    std::string helmet = "default";
    std::string chest = "default";
    std::string leftShoulder = "default";
    std::string rightShoulder = "default";
    std::string hands = "default";      // "default" or "robot"
    std::string wrist = "default";
    std::string utility = "default";
    std::string knees = "default";
    std::string armorEffect = "default"; // "default", "birthday_party", "heart_attack", "pestilence", "inclement_weather", "flames", "blue_flames"

    // Elite suit string_id name ("minor", "spec_ops", "space", "ultra", "zealot", "general", "field_marshall", "officer")
    std::string eliteSuit = "minor";

    // Linear RGB colors [0..1]
    std::array<float, 3> primaryRgb = { 0.15294118f, 0.15294118f, 0.15294118f };
    std::array<float, 3> secondaryRgb = { 0.15294118f, 0.15294118f, 0.15294118f };
    std::array<float, 3> visorRgb = { 1.0f, 0.45f, 0.05f };
};

class ReachArmory {
public:
    static void Initialize(const std::filesystem::path& dataDir);

    static void SetMenuMode(bool menuMode);
    static bool IsMenuMode();

    static void SetTargetScene(const std::string& sceneName);
    static std::string GetTargetScene();

    static void SetSelection(const ReachArmorySelection& sel);
    static ReachArmorySelection GetSelection();

    static void AddYawRotation(float deltaRadians);
    static void SetYawRotation(float radians);
    static float GetYawRotation();

    // Called every frame on the engine render thread inside HostSlot1_FrameTick.
    static void Tick();

    // True once mainmenu_all.map director scene table is active.
    static bool IsSceneReady();
};

} // namespace noble
