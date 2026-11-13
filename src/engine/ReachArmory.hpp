#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace noble {

struct ReachArmorySelection {
    // Preferred species for in-game spawning (0 = Spartan, 1 = Elite)
    bool isElite = false;
    // Species currently previewed on the Armory pedestal (can be toggled independently or with Preferred Species)
    bool previewElite = false;
    bool isFemale = false;

    // Integer indices for in-game profile / appearance table
    int primaryColorIndex = 1;    // 0..29 (Steel)
    int secondaryColorIndex = 10; // 0..29 (Green/Brick)
    int accentColorIndex = 14;    // 0..29 (Cyan; the third/detail colour)
    int lightsColorIndex = 14;    // 0..29 (4th slot: lights / illumination)
    int visorColorIndex = 0;      // 0..4
    int armorEffectIndex = 0;     // 0..6
    int firefightVoiceIndex = 0;  // 0..15
    int eliteSuitIndex = 0;       // 0..10 (the Armory's elite set presets)
    std::string serviceTag = "N06";
    // Emblem, in Reach's own indices (2026-11-11): foreground 0..79, background 0..52 (0 = Blank), colours 0..29,
    // flags bit 0 = toggled (hide the secondary layer). Written into the game's emblem slot by ApplyLocalEmblem.
    int emblemForeground = 1;
    int emblemBackground = 0;
    int emblemFlags = 0;
    std::array<int, 3> emblemColors = { 2, 2, 16 };   // primary, secondary, background

    // Spartan permutation string_id names (matching mainmenu_all.string_ids.csv)
    std::string helmet = "default";
    std::string chest = "chest_blank";
    std::string leftShoulder = "left_shoulder_blank";
    std::string rightShoulder = "right_shoulder_blank";
    std::string hands = "default";       // "default" or "robot"
    std::string wrist = "wrist_blank";
    std::string utility = "utility_blank";
    std::string knees = "knees_default";
    std::string armorEffect = "default"; // "default", "birthday_party", "heart_attack", "pestilence", "inclement_weather", "flames", "blue_flames"
    std::string visor = "default";
    std::string voice = "noble_six";

    // Elite suit string_id name ("minor", "spec_ops", "space", "ultra", "zealot", "general", "story_elite", "officer")
    std::string eliteSuit = "minor";
    // Elite per-piece permutation names for the menu map's Elite model regions (elite_helmet, elite_left_shoulder,
    // elite_right_shoulder, elite_chest, elite_forearms, elite_legs).
    std::string eliteHelmet = "minor";
    std::string eliteLeftShoulder = "minor";
    std::string eliteRightShoulder = "minor";
    std::string eliteChest = "minor";
    std::string eliteArms = "default";
    std::string eliteLegs = "minor";
    int eliteArmorEffectIndex = 0;
    std::string eliteArmorEffect = "default";

    // Linear RGB colors [0..1]
    std::array<float, 3> primaryRgb = { 0.100f, 0.100f, 0.100f };
    std::array<float, 3> secondaryRgb = { 0.100f, 0.100f, 0.100f };
    std::array<float, 3> accentRgb = { 0.471f, 0.788f, 0.808f };   // third/detail colour (Cyan)
    std::array<float, 3> lightsRgb = { 0.106f, 0.839f, 0.529f };   // 4th slot: lights / illumination
    std::array<float, 3> visorRgb = { 0.69f, 0.25f, 0.02f };
    // True when the colour was picked on the wheel instead of from Reach's 30-colour palette. Only those
    // colours have to be forced onto the game (see SyncMatchLiveBipeds); palette colours travel in the
    // player's appearance record and the engine already renders every player's own choice.
    bool primaryCustom = false;
    bool secondaryCustom = false;
    // Marine / ODST pieces (2026-11-13): applied to the local player's biped in matches where a mod makes the player a
    // Marine (Reach's objects\characters\marine model). {region, permutation} pairs; empty = keep the mod's pieces.
    std::vector<std::pair<std::string, std::string>> marinePieces;
    bool marineColors = false;   // apply marinePrimaryRgb / marineSecondaryRgb to change colours 0 / 1
    bool previewMarine = false;  // Armory MARINE tab: show marine_armory (rr_elite = 2) instead of the Spartan
    // Female Marine (2026-11-13): pairs for objects\characters\marine_female; marineFemale = the Marine tab's gender is
    // female (preview shows marine_f_armory, rr_elite = 3; in matches used on a female-marine body).
    std::vector<std::pair<std::string, std::string>> femalePieces;
    bool marineFemale = false;
    std::array<float, 3> marinePrimaryRgb = { 0.3f, 0.3f, 0.3f };
    std::array<float, 3> marineSecondaryRgb = { 0.3f, 0.3f, 0.3f };
    std::array<float, 3> marineThirdRgb = { 0.3f, 0.3f, 0.3f };    // change colour slot 2
    std::array<float, 3> marineFourthRgb = { 0.3f, 0.3f, 0.3f };   // change colour slot 3
    bool accentCustom = false;
    bool lightsCustom = false;
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

    // Resolves a Blam! string_id name from mainmenu_all.string_ids.csv (returns 0 if not found).
    static uint32_t ResolveStringId(const std::string& name);

    // Called every frame on the engine render thread inside HostSlot1_FrameTick.
    static void Tick();
    static void NotifyEngineTick();
    static void SyncMatchLiveBipeds();
    // Marine / ODST pieces on the local player's Marine biped (mods). Called every match frame.
    static void SyncMarine();
    // Flashlight (2026-11-13; PepperMan's Nexus 2507 lights merged into spartans.biped): L toggles the local unit's
    // "open" state like hs unit_open / unit_close. Only maps built with that biped show a light.
    static void TickFlashlight();
    // Slot 48 reports every map file it resolves; the scenario map's string ids are read from it.
    static void NoteMapPath(const std::string& path);

    // True once the Armory map's HS GLOBALS table is active.
    static bool IsSceneReady();
    // True once the menu map's script is running and Noble's first armour apply has gone through (plus a short
    // settle so the engine has drawn the dressed character). Until then the Armory shows its loading state:
    // IsSceneReady() turns true as soon as the map's memory exists, while the script's values are still garbage.
    static bool IsCharacterPresentable();
    // Milliseconds since IsCharacterPresentable() turned true (0 while it is false); used to fade the model in.
    static uint64_t CharacterPresentableForMs();
    // True for a moment around a character rebuild (armour effect change) or species switch: the engine shows the
    // new biped for a frame or two in default colours / before it is posed, so the Armory keeps showing the last
    // finished picture until the change has settled.
    static bool IsStageHeld();
    // Hold the Armory picture for a quarter second from now (the game reacting to a profile/appearance update can
    // show the character from another camera or pose for a single frame).
    static void HoldStageBriefly();

    // True when the player has an armour pack map installed (data\armor\*.map + .rrreloc).
    static bool HasArmoryMap();
    // True only when data\menumaps\mainmenu_all.map (the custom Armory scene) is present. The Elite
    // pack alone does not change the stock menu map's cameras, so framing must not follow HasArmoryMap.
    static bool HasCustomMenuMap();

    // Dev tool: the pmcg (region=permutation) names for an Armory item that have no string id in the
    // loaded map. Empty = every piece the item changes can be resolved. Used by -armory-validate.
    static std::vector<std::string> MissingPermutationsFor(const std::string& list, const std::string& item);

    // True when the engine is actively displaying an Armory pedestal/customization scene.
    static bool IsArmoryScene();
};

} // namespace noble
