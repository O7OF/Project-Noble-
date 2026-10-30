#include "ArmorCustomizer.hpp"
#include "common/Settings.hpp"
#include "NobleStyle.hpp"
#include "UiArt.hpp"
#include "career/Career.hpp"
#include "common/Logger.hpp"
#include "engine/ReachArmory.hpp"
#include "engine/ReachHost.hpp"

#include <windows.h>
#include <shellapi.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cfloat>
#include <random>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>

namespace noble {

namespace {

float SrgbToLinear(float c) {
    c = std::clamp(c, 0.0f, 1.0f);
    return (c <= 0.04045f) ? (c / 12.92f) : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

// Authentic 30-color Halo: Reach palette (verified against MCC Data/UI/playercustomization.xml)
constexpr std::array<ReachColorEntry, 30> kReachColors = {{
    {"Steel",     0.349f, 0.349f, 0.349f},
    {"Silver",    0.608f, 0.608f, 0.608f},
    {"White",     0.847f, 0.847f, 0.847f},
    {"Brown",     0.427f, 0.310f, 0.220f},
    {"Tan",       0.620f, 0.494f, 0.376f},
    {"Khaki",     0.780f, 0.671f, 0.541f},
    {"Sage",      0.345f, 0.408f, 0.302f},
    {"Olive",     0.490f, 0.569f, 0.400f},
    {"Drab",      0.643f, 0.725f, 0.533f},
    {"Forest",    0.165f, 0.369f, 0.212f},
    {"Green",     0.267f, 0.545f, 0.314f},
    {"Sea Foam",  0.467f, 0.729f, 0.506f},
    {"Teal",      0.165f, 0.431f, 0.459f},
    {"Aqua",      0.278f, 0.608f, 0.635f},
    {"Cyan",      0.471f, 0.788f, 0.808f},
    {"Blue",      0.176f, 0.302f, 0.514f},
    {"Cobalt",    0.290f, 0.455f, 0.694f},
    {"Ice",       0.518f, 0.678f, 0.875f},
    {"Violet",    0.310f, 0.227f, 0.498f},
    {"Orchid",    0.459f, 0.365f, 0.678f},
    {"Lavender",  0.651f, 0.565f, 0.855f},
    {"Maroon",    0.467f, 0.161f, 0.200f},
    {"Brick",     0.678f, 0.251f, 0.278f},
    {"Rose",      0.863f, 0.478f, 0.494f},
    {"Rust",      0.557f, 0.231f, 0.133f},
    {"Coral",     0.784f, 0.376f, 0.239f},
    {"Peach",     0.941f, 0.588f, 0.451f},
    {"Gold",      0.639f, 0.439f, 0.141f},
    {"Yellow",    0.859f, 0.651f, 0.220f},
    {"Pale",      0.961f, 0.816f, 0.447f},
}};

struct VisorEntry {
    const char* name;
    const char* stringId;
    float r, g, b;
    const char* description;
};

constexpr std::array<VisorEntry, 5> kVisorColors = {{
    {"DEFAULT", "default", 0.85f, 0.53f, 0.14f,
     "Standard-issue solar/laser reflective gold-amber coating applied to MJOLNIR Mk. V visors."},
    {"SILVER",  "silver",  0.78f, 0.83f, 0.90f,
     "High-reflectivity silver mirror coating favored for low-glare urban and orbital operations."},
    {"BLUE",    "blue",    0.18f, 0.54f, 0.95f,
     "Cobalt-tinted optical coating tuned for high-contrast target acquisition in low-light engagements."},
    {"BLACK",   "black",   0.11f, 0.13f, 0.16f,
     "Polarized obsidian stealth coating that conceals the operator's gaze completely."},
    {"GOLD",    "gold",    0.98f, 0.78f, 0.18f,
     "Pure 24-karat metallic gold deposition reserved for veteran Spartan command personnel."},
}};

struct HelmetVariantSpec {
    const char* name;
    const char* stringId;
    const char* description = "";
};

struct HelmetSpec {
    const char* name;
    const char* stringId;
    const char* legacyGroup;
    std::vector<HelmetVariantSpec> variants;
    const char* description;
};

// All helmet string_ids verified against data/menumaps/mainmenu_all.string_ids.csv (IDs 12473..12537 & 8905..8927)
const std::vector<HelmetSpec>& GetSpartanHelmets() {
    static const std::vector<HelmetSpec> kHelmets = {
        {
            "MARK V [B]", "default", "-default",
            {
                {"BASE", "default", "The Mark V[B] was entering service in standard issue across the UNSC just prior to the fall of Reach."},
                {"UA", "default_a", "Up-Armored brow plate module for the standard Mark V [B] combat helmet."},
                {"UA/HUL", "default_b", "Up-Armored brow plate with external Hardened Uplink module for the Mark V [B]."},
            },
            "The Mark V[B] was entering service in standard issue across the UNSC just prior to the fall of Reach."
        },
        {
            "CQC", "rosenda", "-player_kurt",
            {
                {"BASE", "rosenda", "Standard-issue close quarters combat variant developed at the UNSC Hoppocken testing facility."},
                {"CBRN", "rosenda_a", "Chemical, Biological, Radiological and Nuclear filtration unit for the CQC helmet."},
                {"UA/HUL", "rosenda_b", "Up-Armored visor shield with Hardened Uplink module for the CQC helmet."},
            },
            "Standard-issue close quarters combat variant developed at the UNSC Hoppocken testing facility."
        },
        {
            "ODST", "malcolm", "-player_jai",
            {
                {"BASE", "malcolm", "Many technologies initially developed for MJOLNIR have gained widespread adoption by the ODST."},
                {"UA/CNM", "malcolm_a", "Up-Armored brow plate with Command Network Module for the ODST helmet."},
                {"CBRN/HUL", "malcolm_b", "CBRN rebreather and Hardened Uplink package for the ODST helmet."},
            },
            "Many technologies initially developed for MJOLNIR have gained widespread adoption by the ODST."
        },
        {
            "HAZOP", "skull", "-player_isaac",
            {
                {"BASE", "skull", "The Mark V[H] variant is environmentally sealed for prolonged operations in hazardous environments."},
                {"CBRN/HUL", "skull_a", "Twin CBRN filtration canisters and a Hardened Uplink for extreme hazard zone operations."},
                {"CNM-I", "skull_b", "Command Network Module and CBRN filtration suite for the HAZOP helmet."},
            },
            "The Mark V[H] variant is environmentally sealed for prolonged operations in hazardous environments."
        },
        {
            "EOD", "vinh", "-player_vinh",
            {
                {"BASE", "vinh", "Designed specifically to channel the pressure wave of a detonation around the user's head."},
                {"CNM", "vinh_a", "Command Network Module fitted to the Explosive Ordnance Disposal helmet."},
                {"UA/HUL", "vinh_b", "Up-Armored blast shield with Hardened Uplink module for the EOD helmet."},
            },
            "Designed specifically to channel the pressure wave of a detonation around the user's head."
        },
        {
            "OPERATOR", "isaac", "-player_nicole",
            {
                {"BASE", "isaac", "A multi-role variant optimized for specialized vehicle and espionage teams within Asymmetric Action Groups."},
                {"UA/HUL", "isaac_a", "Up-Armored brow plate and Hardened Uplink module for the Operator helmet."},
                {"CNM", "isaac_b", "Command Network Module and optical rangefinder suite for the Operator helmet."},
            },
            "A multi-role variant optimized for specialized vehicle and espionage teams within Asymmetric Action Groups."
        },
        {
            "GRENADIER", "joshua", "-player_joshua",
            {
                {"BASE", "joshua", "Tested early in the program's history, the Grenadier firmware was the basis for the Mark V's shield emitters."},
                {"UA", "joshua_a", "Up-Armored brow reinforcement for the heavy Grenadier helmet."},
                {"UA/FC", "joshua_b", "Up-Armored brow plate with external Fascia Command module for the Grenadier helmet."},
            },
            "Tested early in the program's history, the Grenadier firmware was the basis for the Mark V's shield emitters."
        },
        {
            "AIR ASSAULT", "female", "-female",
            {
                {"BASE", "female", "Adopted by Airborne units as an improvement on the venerable ODST helmet architecture."},
                {"UA/CNM", "female_a", "Up-Armored brow plate with Command Network Module for the Air Assault helmet."},
                {"FC-I", "female_b", "Fascia Command and optical uplink attachment for the Air Assault helmet."},
            },
            "Adopted by Airborne units as an improvement on the venerable ODST helmet architecture."
        },
        {
            "SCOUT", "jun", "-jun",
            {
                {"BASE", "jun", "Sharing many components with the Recon variant, the Scout relies more heavily on stealth coatings."},
                {"HU/RS", "jun_a", "Hardened Uplink and Remote Sensor optics for long-range reconnaissance."},
                {"CBRN/CNM", "jun_b", "CBRN filtration and Command Network Module for the Scout helmet."},
            },
            "Sharing many components with the Recon variant, the Scout relies more heavily on stealth coatings."
        },
        {
            "EVA", "player_li", "-player_kelly",
            {
                {"BASE", "player_li", "Created in a zero-g facility above Damascus on Chi Ceti IV, the Mark V[V] was designed to increase exo-atmospheric survivability."},
                {"CNM", "player_li_a", "Command Network Module for the Extra-Vehicular Activity helmet."},
                {"UA/HUL[3]", "player_li_b", "Up-Armored brow plate and triple Hardened Uplink transceiver for the EVA helmet."},
            },
            "Created in a zero-g facility above Damascus on Chi Ceti IV, the Mark V[V] was designed to increase exo-atmospheric survivability."
        },
        {
            "JFO", "nicole", "-player_adrianna",
            {
                {"BASE", "nicole", "Designed to coordinate orbital fire-support and close air strikes in contested combat zones."},
                {"HUL-I", "nicole_a", "Internalized Hardened Uplink module for the Joint Fires Observer helmet."},
                {"UA", "nicole_b", "Up-Armored blast shield and Hardened Uplink suite for the JFO helmet."},
            },
            "Designed to coordinate orbital fire-support and close air strikes in contested combat zones."
        },
        {
            "COMMANDO", "carter", "-carter",
            {
                {"BASE", "carter", "Featuring an integrated command and control suite, the Commando variant is favored by Spartan team leaders."},
                {"CBRN/CNM", "carter_a", "CBRN filtration and Command Network Module for the Commando helmet."},
                {"UA/FC-I[2]", "carter_b", "Up-Armored visor sunshield and dual Fascia Command sensors (Noble One configuration)."},
            },
            "Featuring an integrated command and control suite, the Commando variant is favored by Spartan team leaders."
        },
        {
            "EVA [C]", "emile", "-emile",
            {
                {"BASE", "emile", "Field-modified EVA variant featuring a hand-carved ballistic polarization dome (Noble Four configuration)."},
                {"CNM", "emile_a", "Command Network Module mounted to the EVA [C] helmet."},
                {"UA/HUL[3]", "emile_b", "Up-Armored brow plate and triple Hardened Uplink module on the EVA [C] helmet."},
            },
            "Field-modified EVA variant featuring a hand-carved ballistic polarization dome (Noble Four configuration)."
        },
        {
            "MJOLNIR MK. V", "john117", "-player_will",
            {
                {"BASE", "john117", "Entered service on November 24, 2541; the Mark V was the first generation of MJOLNIR to incorporate energy shielding."},
                {"CNM", "john117_a", "Command Network Module for the classic MJOLNIR Mark V helmet."},
                {"UA", "john117_b", "Up-Armored brow plate and Command Network Module for the MJOLNIR Mark V helmet."},
            },
            "Entered service on November 24, 2541; the Mark V was the first generation of MJOLNIR to incorporate energy shielding."
        },
        {
            "PILOT", "sam", "-player_skull",
            {
                {"BASE", "sam", "Utilizing the latest in holographic HUD interfaces, the Pilot variant is widespread among UNSC Air Force and Navy crews."},
                {"HUL[3]", "sam_a", "Triple Hardened Uplink module for the Pilot helmet."},
                {"UA/HUL[3]", "sam_b", "Up-Armored blast shield and triple Hardened Uplink module for the Pilot helmet."},
                {"HAUNTED", "sam_c", "And when he had opened the fourth seal, I heard the voice of the fourth beast say, Come and see."},
            },
            "Utilizing the latest in holographic HUD interfaces, the Pilot variant is widespread among UNSC Air Force and Navy crews."
        },
        {
            "SECURITY", "linda", "-player_linda",
            {
                {"BASE", "linda", "Manufactured by Misriah Armory on Mars, the Mark V[M] Security variant was designed for shipboard defense."},
                {"UA/HUL", "linda_a", "Up-Armored brow plate with Hardened Uplink antenna for the Security helmet."},
                {"CBRN/CNM", "linda_b", "CBRN filtration and Command Network Module for the Security helmet."},
            },
            "Manufactured by Misriah Armory on Mars, the Mark V[M] Security variant was designed for shipboard defense."
        },
        {
            "MILITARY POLICE", "adrianna", "-helmet_base__(spartans_player_mp_render)",
            {
                {"BASE", "adrianna", "Previously an exclusive asset of Beta-5 security forces, the MP variant has entered service with the Army and Air Force."},
                {"CBRN/HU/RS", "adrianna_a", "CBRN rebreather and Hardened Uplink / Remote Sensor package for the Military Police helmet."},
                {"HU/RS/CNM", "adrianna_b", "Hardened Uplink, Remote Sensor, and Command Network Module for the Military Police helmet."},
            },
            "Previously an exclusive asset of Beta-5 security forces, the MP variant has entered service with the Army and Air Force."
        },
        {
            "CQB", "kurt", "-player_rosenda",
            {
                {"BASE", "kurt", "The Mark V[C] variant improves survivability in close-quarters combat via hardened T-visor plating."},
                {"HU/RS/CNM", "kurt_a", "Hardened Uplink, Remote Sensor, and Command Network Module for the CQB helmet."},
                {"UA/HUL", "kurt_b", "Up-Armored brow plate and Hardened Uplink suite for the CQB helmet."},
            },
            "The Mark V[C] variant improves survivability in close-quarters combat via hardened T-visor plating."
        },
        {
            "RECON", "jai", "-player_malcolm",
            {
                {"BASE", "jai", "An ONI developmental prototype designed to decrease noticeability and silhouette in deep-reconnaissance operations."},
                {"HUL-I", "jai_a", "Hardened Uplink transceiver mounted to the ONI Recon helmet."},
                {"UA/HUL[3]", "jai_b", "Up-Armored blast shield and triple Hardened Uplink array for the Recon helmet."},
            },
            "An ONI developmental prototype designed to decrease noticeability and silhouette in deep-reconnaissance operations."
        },
        {
            "MJOLNIR MK. VI", "will", "-player_john117",
            {
                {"BASE", "will", "Field-tested in the final months of 2552 on Reach prior to full deployment across remaining SPARTAN-II personnel."},
                {"FC-I[2]", "will_a", "Dual internalized Fascia Command sensors for the prototype Mark VI helmet."},
                {"UA/HUL-I", "will_b", "Up-Armored brow plate and internalized Hardened Uplink for the prototype Mark VI helmet."},
            },
            "Field-tested in the final months of 2552 on Reach prior to full deployment across remaining SPARTAN-II personnel."
        },
        {
            "GUNGNIR", "kelly", "-player_sam",
            {
                {"BASE", "kelly", "Created as part of Project GUNGNIR to pair with the M6 Grindell/Galilean Nonlinear Rifle."},
                {"HU/RS", "kelly_a", "Hardened Uplink and Remote Sensor module for the enclosed GUNGNIR helmet."},
                {"CBRN", "kelly_b", "CBRN filtration and Remote Sensor suite for the GUNGNIR helmet."},
            },
            "Created as part of Project GUNGNIR to pair with the M6 Grindell/Galilean Nonlinear Rifle."
        },
        {
            "AKIS", "hilands", "-dlc2_helmet_hilands",
            {
                // MCC's four AKIS items in MCC order: Base, GRD, CBRN, GRD/CBRN (names as on Halopedia).
                {"BASE", "hilands", "High-altitude reconnaissance and assassination prototype developed at the UNSC Highland Mountains facility."},
                {"GRD", "hilands_b", "GRD optical rangefinder module for the AKIS helmet."},
                {"CBRN", "hilands_a", "CBRN filtration module for the AKIS prototype helmet."},
                {"GRD/CBRN", "hilands_c", "GRD rangefinder combined with the CBRN filtration suite for the AKIS helmet."},
            },
            "High-altitude reconnaissance and assassination prototype developed at the UNSC Highland Mountains facility."
        },
    };
    return kHelmets;
}

struct ShoulderSpec {
    const char* name;
    const char* leftStringId;
    const char* rightStringId;
    const char* legacyLeftGroup;
    const char* legacyRightGroup;
    const char* description;
};

// Verified against mainmenu_all.string_ids.csv IDs 12538..12573
const std::vector<ShoulderSpec>& GetSpartanShoulders() {
    static const std::vector<ShoulderSpec> kShoulders = {
        {"DEFAULT",          "left_shoulder_blank", "right_shoulder_blank", "", "",
         "Standard Mark V [B] low-profile shoulder deltoid seal without supplemental pauldron plating."},
        {"FJ/PARA",          "left_shoulder_1",     "right_shoulder_1",     "-flair_shoulder_kat_l", "-flair_shoulder_kat_r",
         "Lightweight para-rescue pauldron favored by UNSC Airborne and orbital drop personnel."},
        {"HAZOP",            "left_shoulder_2",     "right_shoulder_2",     "-flair_l_shoulder__(spartans_player_isaac_render)", "-flair_r_shoulder__(spartans_player_isaac_render)",
         "Supplemental hazard-operations pauldron sealed against corrosive and radiological particulates."},
        {"JFO",              "left_shoulder_3",     "right_shoulder_3",     "-flair_shoulder_adrianna_l", "-flair_shoulder_adrianna_r",
         "Up-armored pauldron housing auxiliary telemetry relays for Joint Fires Observers."},
        {"RECON",            "left_shoulder_4",     "right_shoulder_4",     "-flair_l_shoulder__(spartans_player_malcolm_render)", "-flair_r_shoulder__(spartans_player_malcolm_render)",
         "Angular stealth-contoured pauldron designed to minimize radar cross-section."},
        {"UA/MULTI-THREAT",  "left_shoulder_5",     "right_shoulder_5",     "-flair_shoulder_l_default__", "-flair_shoulder_r_default__",
         "Heavy multi-threat ballistic pauldron designed for sustained frontline firefights."},
        {"JUMP JET",         "left_shoulder_6",     "right_shoulder_6",     "-flair_l_shoulder__(spartans_player_will_render)", "-flair_r_shoulder__(spartans_player_will_render)",
         "Experimental micro-thruster pauldron designed for zero-g attitude control and high-altitude drops."},
        {"EVA",              "left_shoulder_7",     "right_shoulder_7",     "-flair_l_shoulder__(spartans_player_kelly_render)", "-flair_r_shoulder__(spartans_player_kelly_render)",
         "Contoured Extra-Vehicular Activity pauldron built for vacuum and orbital operations."},
        {"GUNGNIR",          "left_shoulder_8",     "right_shoulder_8",     "-flair_l_shoulder__(spartans_player_sam_render)", "-flair_r_shoulder__(spartans_player_sam_render)",
         "Heavy slab-sided pauldron engineered under Project GUNGNIR to deflect anti-materiel shrapnel."},
        {"ODST",             "left_shoulder_9",     "right_shoulder_9",     "-flair_l_shoulder__(spartans_player_jai_render)", "-flair_r_shoulder__(spartans_player_jai_render)",
         "Drop-shock pauldron adopted across both Orbital Drop Shock Trooper units and Spartan teams."},
        {"UA/BASE SECURITY", "left_shoulder_10",    "right_shoulder_10",    "-flair_l_shoulder__(spartans_carter_render)", "-flair_r_shoulder__(spartans_carter_render)",
         "Asymmetric reinforced command pauldron issued to ONI and HIGHCOM installation defenders."},
        {"CQC",              "left_shoulder_kurt",  "right_shoulder_kurt",  "-player_kurt", "-flair_r_shoulder__(spartans_player_kurt_render)",
         "Reinforced close-quarters brawling pauldron designed to absorb point-blank impacts."},
        {"OPERATOR",         "left_shoulder_11",    "right_shoulder_11",    "-flair_l_shoulder__(spartans_player_nicole_render)", "-flair_r_shoulder__(spartans_player_nicole_render)",
         "Compact low-snag pauldron designed for vehicle crews and Asymmetric Action Group operators."},
        {"COMMANDO",         "left_shoulder_12",    "right_shoulder_12",    "-flair_l_shoulder_emile__", "-flair_r_shoulder__(spartans_player_linda_render)",
         "Heavy tactical command pauldron worn by Spartan-III team leaders."},
        {"GRENADIER",        "left_shoulder_13",    "right_shoulder_13",    "-flair_joshua_l_shoulder", "-flair_joshua_r_shoulder",
         "Massive up-armored siege pauldron worn by heavy weapons specialists (Noble Five configuration)."},
        {"SNIPER",           "left_shoulder_14",    "right_shoulder_14",    "-flair_l_shoulder__(spartans_jun_render)", "-flair_r_shoulder__(spartans_jun_render)",
         "Marksman pauldron fitted with external 14.5x114mm APFSDS anti-materiel ammunition loops."},
        {"MJOLNIR MK. V",    "left_shoulder_15",    "right_shoulder_15",    "-flair_l_shoulder__(spartans_player_rosenda_render)", "-flair_r_shoulder__(spartans_player_rosenda_render)",
         "Classic Mark V fleet-issue pauldron with reinforced upper deltoid ridge."},
        {"SECURITY",         "left_shoulder_16",    "right_shoulder_16",    "-flair_l_shoulder__(spartans_player_linda_render)", "-flair_r_shoulder_emile__",
         "The Supplemental Armor, Pauldron, MJOLNIR (M) variant was originally manufactured in 2528. Right pauldron mounts an M947 Kukri."},
    };
    return kShoulders;
}

struct MultiLayerSpec {
    const char* name;
    const char* stringId;
    const char* legacyGroup;
    bool hasProstheticArm = false;
    const char* description = "";
};

// Verified against mainmenu_all.string_ids.csv IDs 12574..12599 & 8902..8903
const std::vector<MultiLayerSpec>& GetSpartanChests() {
    static const std::vector<MultiLayerSpec> kChests = {
        {"DEFAULT", "chest_blank", "", false,
         "Standard Mark V [B] titanium-A battle plate with integrated fusion pack."},
        {"HP/HALO", "chest_1", "-flair_carter_gear_pack", false,
         "High-Altitude Low-Opening jump harness and supplemental oxygen/telemetry rig."},
        {"UA/COUNTERASSAULT", "chest_2", "-flair_chest_perm1_default__", false,
         "Up-Armored sternum and abdominal blast plating for close-assault operations."},
        {"TACTICAL/LRP", "chest_3", "-flair_chest_perm2_default__", false,
         "Long-Range Patrol chest rig carrying three field-accessible ammunition and ration pouches."},
        {"UA/ODST", "chest_4", "-flair_chest__(spartans_player_jai_render)", false,
         "Orbital Drop Shock Trooper composite chest plate adapted for MJOLNIR Mark V."},
        {"TACTICAL/RECON", "chest_5", "-flair_chest__(spartans_player_isaac_render)", false,
         "Extended reconnaissance chest rig with four sealed magazine and field-gear pouches."},
        {"COLLAR/GRENADIER", "chest_6", "-flair_joshua_torso", false,
         "Heavy ballistic gorget and upper-thorax shield generator housing."},
        {"TACTICAL/PATROL", "chest_7", "-flair_chest__(spartans_jun_render)", false,
         "Sniper patrol webbing with camo ghillie hood, combat knife, and long-range rucksack (Noble Three)."},
        {"COLLAR/BREACHER", "chest_8", "-flair_chest_plate_emile", false,
         "Heavy breaching collar paired with a full waist belt of 40mm high-explosive shells."},
        {"ASSAULT/SAPPER", "chest_9", "-flair_mesh_chest_grenades_emile", false,
         "Combat engineer webbing loaded with bandolier and 40mm grenade belt (Noble Four)."},
        {"ASSAULT/COMMANDO", "chest_10", "-flair_chest_piece__(spartans_carter_render)", false,
         "Command assault harness with shoulder-mounted comms antenna and rear ammo pack (Noble One)."},
        {"HP/PARAFOIL", "chest_11", "-flair_chest__(spartans_player_skull_render)", false,
         "High-altitude parafoil harness and chest-mounted flight stabilization pack."},
        {"COLLAR/GRENADIER [UA]", "chest_12", "-flair_joshua_torso", false,
         "Full heavy weapons specialist armor suite with armored blast collar and hip plates (Noble Five)."},
        {"UA/MULTI-THREAT [W]", "chest_wp_1", "-flair_chest_plate_emile", false,
         "Up-Armored Multi-Threat cuirass combined with a full 40mm grenade waist belt."},
        {"UA/BASE SECURITY [W]", "chest_wp_2", "-flair_chest_kat", true,
         "Up-Armored Base Security cuirass paired with a trans-humeral prosthetic right arm (Noble Two)."},
        {"COLLAR/BREACHER [R]", "chest_robo_1", "-flair_chest_plate_emile", true,
         "Collar/Breacher armor and 40mm grenade belt paired with a cybernetic right arm."},
        {"HP/PARAFOIL [R]", "chest_robo_2", "-flair_chest__(spartans_player_skull_render)", true,
         "HP/Parafoil jump harness paired with a cybernetic right arm."},
        {"ASSAULT/SAPPER [R]", "chest_robo_3", "-flair_mesh_chest_grenades_emile", true,
         "Assault/Sapper grenade bandolier paired with a cybernetic right arm."},
        {"UA/MULTI-THREAT", "chest_13", "-flair_chest_plate_emile", false,
         "Up-Armored chest plate with twin chest-mounted tactical ammunition modules."},
        {"UA/BASE SECURITY", "chest_kurt", "-flair_chest_kat", false,
         "Reinforced security cuirass designed to deflect small-arms and plasma bolts."},
        {"DEFAULT [R]", "chest_default_roboarm", "", true,
         "Standard Mark V [B] battle plate paired with a cybernetic right arm."},
        {"UA/COUNTERASSAULT [R]", "chest_uacounterassault_roboarm", "-flair_chest_perm1_default__", true,
         "UA/Counterassault chest armor paired with a cybernetic right arm."},
        {"TACTICAL/RECON [R]", "chest_tacticalrecon_roboarm", "-flair_chest__(spartans_player_isaac_render)", true,
         "Tactical/Recon chest webbing paired with a cybernetic right arm."},
        {"TACTICAL/PATROL [R]", "chest_tacticalpatrol_roboarm", "-flair_chest__(spartans_jun_render)", true,
         "Tactical/Patrol sniper harness paired with a cybernetic right arm."},
        {"COLLAR/GRENADIER [UA] [R]", "chest_collargrenadierua_roboarm", "-flair_joshua_torso", true,
         "Collar/Grenadier [UA] heavy chest plate paired with a cybernetic right arm."},
    };
    return kChests;
}

// Verified against mainmenu_all.string_ids.csv IDs 12600..12605
const std::vector<MultiLayerSpec>& GetSpartanWrists() {
    static const std::vector<MultiLayerSpec> kWrists = {
        {"DEFAULT", "wrist_blank", "", false,
         "Standard Mark V [B] vambrace without external forearm attachments."},
        {"UA/BUCKLER", "wrist_1", "-flair_forearm__(spartans_carter_render)", false,
         "Up-Armored forearm deflector shield for parrying close-quarters melee strikes."},
        {"UA/BRACER", "wrist_2", "-flair_gauntlet_kat", false,
         "Reinforced forearm bracer protecting wrist actuators and suit seals."},
        {"TACTICAL/TACPAD", "wrist_3", "-flair_tricorder_kat", false,
         "Ruggedized forearm Tactical Data Pad providing real-time battlefield telemetry."},
        {"TACTICAL/UGPS", "wrist_5", "-flair_wrist__(spartans_player_kelly_render)", false,
         "Universal Global Positioning System wrist terminal for orbital and terrestrial navigation."},
        {"ASSAULT/BREACHER", "wrist_4", "-flair_shotgun_reloader__(spartans_player_rosenda_render)", false,
         "Forearm-mounted 8-gauge tactical shotgun shell caddy for rapid CQB reloading."},
    };
    return kWrists;
}

// Verified against mainmenu_all.string_ids.csv IDs 12609..12615
const std::vector<MultiLayerSpec>& GetSpartanUtilities() {
    static const std::vector<MultiLayerSpec> kUtilities = {
        {"DEFAULT", "utility_blank", "", false,
         "Standard Mark V [B] thigh armor plate without external utility attachments."},
        {"UA/CHOBHAM", "utility_1", "-flair_thigh_adrianna", false,
         "Composite Chobham reactive thigh armor plate for improved ballistic protection."},
        {"TACTICAL/HARD CASE", "utility_2", "-flair_joshua_thigh", false,
         "Armored thigh hard-case designed to transport sensitive optical or data cores."},
        {"UA/NxRA", "utility_3", "-flair_thigh__(spartans_player_skull_render)", false,
         "Non-Explosive Reactive Armor thigh plate engineered to dissipate plasma splash damage."},
        {"TACTICAL/TRAUMA KIT", "utility_4", "-flair_toolkit_kat", false,
         "Field trauma kit stocked with biofoam injectors and emergency suit sealants."},
        {"TACTICAL/SOFT CASE", "utility_5", "-flair_mesh_thigh_pouch_emile", false,
         "Canvas drop-leg utility pouch for extra magazines and field charges."},
        {"HYDRATION", "utility_6", "-flask", false,
         "Hip-mounted stainless steel field canteen for extended campaigns."},
    };
    return kUtilities;
}

// Verified against mainmenu_all.string_ids.csv IDs 12620..12623
const std::vector<MultiLayerSpec>& GetSpartanKneeGuards() {
    static const std::vector<MultiLayerSpec> kKnees = {
        {"DEFAULT", "knees_default", "", false,
         "Standard Mark V [B] articulated poleyn knee guards."},
        {"FJ/PARA", "knees_1", "-flair_kneepad_kat_l_mesh1", false,
         "Shock-absorbing para-drop knee guards designed for high-velocity combat landings."},
        {"GUNGNIR", "knees_2", "-flair_knee_adrianna_l", false,
         "Heavy angular knee armor developed under Project GUNGNIR."},
        {"GRENADIER", "knees_3", "-flair_joshua_l_knee", false,
         "Up-armored siege knee guards built to brace heavy support weapons."},
    };
    return kKnees;
}

struct SimpleOption {
    const char* name;
    const char* stringId;
    const char* description;
};

constexpr std::array<SimpleOption, 7> kArmorEffects = {{
    {"NONE",              "default",           "Standard combat configuration without external armor particle effects."},
    {"BIRTHDAY PARTY",    "birthday_party",    "Celebratory confetti and cheering crowd audio upon headshot."},
    {"HEART ATTACK",      "heart_attack",      "Cupid's arrows and floating hearts trail the operator in combat."},
    {"PESTILENCE",        "pestilence",        "Avirulent swarm and toxic miasma billow from the armor seals."},
    {"INCLEMENT WEATHER", "inclement_weather", "Localized electrical storm and lightning arcs crackle across the MJOLNIR shell."},
    {"LEGENDARY",         "flames",            "Legendary solar-plasma flames wreathe the operator's helmet."},
    {"ETERNAL",           "blue_flames",       "Bungie Eternal cobalt-plasma flames wreathe the operator's helmet."},
}};

// Reach's own 12 Firefight voices (MCC playercustomization.xml). The ODST-only voices
// (Dare, Dutch, Mickey, Romeo) were removed: Reach has no voice files for them.
constexpr std::array<SimpleOption, 12> kFirefightVoices = {{
    {"NOBLE SIX",      "noble_six", "Default Spartan-B312 combat vocalizations for Firefight."},
    {"CORTANA",        "cortana",   "CTN 0452-9 AI construct combat callouts for Firefight."},
    {"JOHN-117",       "chief",     "Master Chief Petty Officer John-117 combat callouts for Firefight."},
    {"SGTMAJ JOHNSON", "johnson",   "Sergeant Major Avery Junior Johnson combat callouts for Firefight."},
    {"GY. SGT BUCK",   "buck",      "Gunnery Sergeant Edward Buck ODST combat callouts for Firefight."},
    {"GYSGT. STACKER", "stacker",   "Gunnery Sergeant Pete Stacker UNSC Marine combat callouts."},
    {"CDR. CARTER",    "carter",    "Commander Carter-A259 (Noble One) combat callouts for Firefight."},
    {"LCDR. KAT",      "kat",       "Lieutenant Commander Catherine-B320 (Noble Two) combat callouts."},
    {"WO. JUN",        "jun",       "Warrant Officer Jun-A266 (Noble Three) combat callouts."},
    {"WO. EMILE",      "emile",     "Warrant Officer Emile-A239 (Noble Four) combat callouts."},
    {"CWO. JORGE",     "jorge",     "Chief Warrant Officer Jorge-052 (Noble Five) combat callouts."},
    {"AUNTIE DOT",     "dot",       "Noble Team Dumb AI 'Auntie Dot' tactical status callouts."},
}};

struct EliteSetSpec {
    const char* name;
    const char* stringId;
    const char* description;
};

// Verified against mainmenu_all.string_ids.csv IDs 2398, 2421..2426, 12624
constexpr std::array<EliteSetSpec, 9> kEliteSets = {{
    {"MINOR", "minor",
     "Standard-issue Sangheili Minor combat harness worn by newly blooded frontline warriors."},
    {"SPEC OPS", "spec_ops",
     "Streamlined Special Operations combat harness tuned for active camouflage and silent infiltration."},
    {"RANGER", "space",
     "Vacuum-sealed Sangheili Ranger exo-atmospheric harness with integrated thruster pack."},
    {"ULTRA", "ultra",
     "Command harness of the Sangheili Evocati, featuring high-capacity energy shielding and enclosed helm."},
    {"ZEALOT", "zealot",
     "Ornate crimson-forged harness of the Ministry of Fervent Intercession's Zealot strike teams."},
    {"GENERAL", "general",
     "Heavily crested golden battle harness worn by high-ranking Sangheili ground commanders."},
    {"FIELD MARSHAL", "story_elite",
     "Supreme tactical command harness worn by Sangheili Field Marshals leading Covenant armies."},
    {"OFFICER", "officer",
     "Distinguished Sangheili Officer harness featuring sweeping cranial fins and reinforced pauldrons."},
    {"HONOR GUARD", "honor_guard",
     "Ceremonial and protective combat harness of the High Council's elite Honor Guard detachment."},
}};

// Per-piece Elite options: the regions and permutations of the Elite model in mainmenu_all.map
// (render model regions elite_helmet / elite_left_shoulder / elite_right_shoulder / elite_chest / elite_forearms /
// elite_legs). The first 8 entries of every list follow kEliteSets, so an armor-set index is also a piece index.
struct ElitePieceSpec {
    const char* name;
    const char* stringId;
    const char* description;
};

enum ElitePiece { kEliteHelmet = 0, kEliteLeftShoulder, kEliteRightShoulder, kEliteChest, kEliteArms, kEliteLegs, kElitePieceCount };

const std::vector<ElitePieceSpec>& GetElitePieces(int piece) {
    static const std::array<std::vector<ElitePieceSpec>, kElitePieceCount> lists = [] {
        std::array<std::vector<ElitePieceSpec>, kElitePieceCount> l;
        static const char* kExtraNote = "Extra Elite piece (from the Elite armour pack); mix it with any set.";
        for (int p = 0; p < kElitePieceCount; ++p) {
            for (const auto& set : kEliteSets) {
                const bool minorArms = p == kEliteArms && std::strcmp(set.stringId, "minor") == 0;
                l[p].push_back({ set.name, minorArms ? "default" : set.stringId, set.description });
            }
        }
        l[kEliteHelmet].push_back({ "COUNCIL", "council", kExtraNote });
        l[kEliteHelmet].push_back({ "OSSOONA", "ossoona", kExtraNote });
        l[kEliteHelmet].push_back({ "RANGER (H2A)", "h2a_space_helmet", kExtraNote });
        l[kEliteLeftShoulder].push_back({ "SCHISM", "schism", kExtraNote });
        l[kEliteLeftShoulder].push_back({ "COMBAT", "combat", kExtraNote });
        l[kEliteLeftShoulder].push_back({ "H2A", "h2a_l_shoulder", kExtraNote });
        l[kEliteRightShoulder].push_back({ "SCHISM", "schism", kExtraNote });
        l[kEliteRightShoulder].push_back({ "COMBAT", "combat", kExtraNote });
        l[kEliteRightShoulder].push_back({ "H2A", "h2a_r_shoulder", kExtraNote });
        l[kEliteLegs].push_back({ "HONOR GUARD (ROBE)", "honor_guard_legs", kExtraNote });
        l[kEliteLegs].push_back({ "H2A", "h2a_legs", kExtraNote });
        return l;
    }();
    return lists[std::clamp(piece, 0, kElitePieceCount - 1)];
}

const char* ElitePieceId(int piece, int index) {
    const auto& list = GetElitePieces(piece);
    return list[std::clamp(index, 0, static_cast<int>(list.size()) - 1)].stringId;
}

// Elite unlocks (Tyler, 2026-10-28): Elites unlock by rank only (no cR, like Reach), a whole set or extra piece
// group at once; every piece of an unlocked set can then be mixed freely. Minor (and its "default" arms) is free.
// Ranks are names from Career::Ranks(). "-armory-unlock-all" unlocks everything (testing).
struct EliteUnlock {
    const char* id;     // piece / set string id (as in GetElitePieces / kEliteSets)
    const char* rank;
};
constexpr EliteUnlock kEliteUnlocks[] = {
    { "spec_ops", "Corporal" },          { "space", "Sergeant" },              { "officer", "Sergeant Grade 2" },
    { "ultra", "Warrant Officer" },      { "zealot", "Captain" },              { "general", "Major" },
    { "story_elite", "Lt. Colonel" },    { "ossoona", "Major Grade 3" },       { "schism", "Commander Grade 2" },
    { "combat", "Commander Grade 2" },   { "h2a_space_helmet", "Colonel Grade 1" },
    { "h2a_l_shoulder", "Colonel Grade 1" }, { "h2a_r_shoulder", "Colonel Grade 1" }, { "h2a_legs", "Colonel Grade 1" },
    { "council", "Brigadier" },          { "honor_guard", "General" },         { "honor_guard_legs", "General" },
};

// Rank needed for an Elite item: index into Career::Ranks() (0 = from the start) and its name.
int EliteUnlockRank(const char* id, const char** rankName = nullptr) {
    if (rankName) *rankName = nullptr;
    if (!id) return 0;
    for (const auto& u : kEliteUnlocks) {
        if (std::strcmp(u.id, id) != 0) continue;
        const auto& ranks = Career::Ranks();
        for (size_t i = 0; i < ranks.size(); ++i) {
            if (std::strcmp(ranks[i].name, u.rank) == 0) {
                if (rankName) *rankName = ranks[i].name;
                return static_cast<int>(i);
            }
        }
        return 0;
    }
    return 0;
}

bool UnlockAllArg() {
    if (Career::ArmoryUnlockAll()) return true;
    static const bool v = [] {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        bool found = false;
        for (int i = 1; argv && i < argc; ++i) if (_wcsicmp(argv[i], L"-armory-unlock-all") == 0) found = true;
        if (argv) LocalFree(argv);
        return found;
    }();
    return v;
}

// The player's rank (career.json read at most every 2 s).
int PlayerRankIndex() {
    static int rank = 0;
    static ULONGLONG next = 0;
    const ULONGLONG now = GetTickCount64();
    if (now >= next) {
        rank = Career::RankIndex(Career::Get().credits);
        next = now + 2000;
    }
    return rank;
}

bool EliteLocked(const char* id) {
    return !UnlockAllArg() && EliteUnlockRank(id) > PlayerRankIndex();
}



// Spartan Armory catalog: picture, rank and cR for every Spartan option (Reach's Armory prices from Halopedia,
// docs/career/armory_overrides.json for the items Reach didn't sell). Generated by tools/armory/build_spartan_catalog.py.
struct SpartanItem {
    const char* slot;     // "helmet", "helmet attachment", "left shoulder", "right shoulder", "chest", "wrist",
                          // "utility", "knees", "visor", "armor effect", "firefight voice"
    const char* id;       // permutation string id (as in the Armory lists)
    const char* picDir;   // under data/ui/reach ("" = no picture)
    const char* pic;
    const char* rank;     // Career::Ranks() name ("" = from the start)
    int64_t credits;
};
constexpr SpartanItem kSpartanItems[] = {
#include "SpartanArmoryCatalog.inc"
};

const SpartanItem* FindSpartanItem(const char* slot, const char* id) {
    for (const auto& it : kSpartanItems)
        if (std::strcmp(it.slot, slot) == 0 && std::strcmp(it.id, id) == 0) return &it;
    return nullptr;
}

int RankIndexByName(const char* name) {
    if (!name || !*name) return 0;
    const auto& ranks = Career::Ranks();
    for (size_t i = 0; i < ranks.size(); ++i) if (std::strcmp(ranks[i].name, name) == 0) return static_cast<int>(i);
    return 0;
}

std::string WithCommas(int64_t v) {
    std::string d = std::to_string(v), out;
    for (size_t i = 0; i < d.size(); ++i) {
        if (i && (d.size() - i) % 3 == 0) out += ',';
        out += d[i];
    }
    return out;
}

} // namespace

std::filesystem::path ArmorCustomizer::ResolveDefaultProfilePath() {
    std::filesystem::path dataDir = ReachHost::Get().GetDataDir();
    if (dataDir.empty()) {
        wchar_t exeBuf[MAX_PATH] = {};
        if (GetModuleFileNameW(nullptr, exeBuf, MAX_PATH)) {
            dataDir = std::filesystem::path(exeBuf).parent_path() / "data";
        } else {
            dataDir = "data";
        }
    }
    std::error_code ec;
    std::filesystem::create_directories(dataDir, ec);
    return dataDir / "appearance.json";
}

ArmorCustomizer::ArmorCustomizer() {
    m_profilePath = ResolveDefaultProfilePath();

    // If a newer profile was written to <exe_dir>/appearance.json or <exe_dir>/data/sandbox/appearance.json
    // before path unification, migrate it into <exe_dir>/data/appearance.json and remove the stale duplicates.
    wchar_t exeBuf[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, exeBuf, MAX_PATH)) {
        const std::filesystem::path exeDir = std::filesystem::path(exeBuf).parent_path();
        const std::filesystem::path sandboxPath = exeDir / "data" / "sandbox" / "appearance.json";
        const std::filesystem::path rootPath    = exeDir / "appearance.json";
        std::error_code ec;
        if (std::filesystem::exists(sandboxPath, ec)) {
            if (!std::filesystem::exists(m_profilePath, ec) ||
                std::filesystem::last_write_time(sandboxPath, ec) > std::filesystem::last_write_time(m_profilePath, ec)) {
                std::filesystem::copy_file(sandboxPath, m_profilePath, std::filesystem::copy_options::overwrite_existing, ec);
            }
            std::filesystem::remove(sandboxPath, ec);
        }
        if (std::filesystem::exists(rootPath, ec) && rootPath != m_profilePath) {
            if (!std::filesystem::exists(m_profilePath, ec)) {
                std::filesystem::copy_file(rootPath, m_profilePath, std::filesystem::copy_options::overwrite_existing, ec);
            }
            std::filesystem::remove(rootPath, ec);
        }
    }

    LoadProfile(m_profilePath);
    m_previewSpecies = m_profile.species;
}

void ArmorCustomizer::SyncLegacyFields() {
    const auto& helmets = GetSpartanHelmets();
    const auto& shoulders = GetSpartanShoulders();
    const auto& chests = GetSpartanChests();
    const auto& wrists = GetSpartanWrists();
    const auto& knees = GetSpartanKneeGuards();

    m_profile.helmetIndex = std::clamp(m_profile.helmetIndex, 0, static_cast<int>(helmets.size()) - 1);
    const auto& h = helmets[m_profile.helmetIndex];
    m_profile.helmetVariantIndex = std::clamp(m_profile.helmetVariantIndex, 0, static_cast<int>(h.variants.size()) - 1);
    m_profile.leftShoulderIndex = std::clamp(m_profile.leftShoulderIndex, 0, static_cast<int>(shoulders.size()) - 1);
    m_profile.rightShoulderIndex = std::clamp(m_profile.rightShoulderIndex, 0, static_cast<int>(shoulders.size()) - 1);
    m_profile.chestIndex = std::clamp(m_profile.chestIndex, 0, static_cast<int>(chests.size()) - 1);
    m_profile.wristIndex = std::clamp(m_profile.wristIndex, 0, static_cast<int>(wrists.size()) - 1);
    m_profile.utilityIndex = std::clamp(m_profile.utilityIndex, 0, static_cast<int>(GetSpartanUtilities().size()) - 1);
    m_profile.visorColorIndex = std::clamp(m_profile.visorColorIndex, 0, static_cast<int>(kVisorColors.size()) - 1);
    m_profile.kneeGuardsIndex = std::clamp(m_profile.kneeGuardsIndex, 0, static_cast<int>(knees.size()) - 1);
    m_profile.armorEffectIndex = std::clamp(m_profile.armorEffectIndex, 0, static_cast<int>(kArmorEffects.size()) - 1);
    m_profile.firefightVoiceIndex = std::clamp(m_profile.firefightVoiceIndex, 0, static_cast<int>(kFirefightVoices.size()) - 1);
    m_profile.eliteArmorSet = std::clamp(m_profile.eliteArmorSet, 0, static_cast<int>(kEliteSets.size()) - 1);

    m_profile.spartanHelmetGroup = h.legacyGroup;
    m_profile.spartanChestGroup = chests[m_profile.chestIndex].legacyGroup;
    m_profile.spartanLeftShoulderGroup = shoulders[m_profile.leftShoulderIndex].legacyLeftGroup;
    m_profile.spartanRightShoulderGroup = shoulders[m_profile.rightShoulderIndex].legacyRightGroup;
    m_profile.spartanWristGroup = wrists[m_profile.wristIndex].legacyGroup;
    m_profile.spartanKneesGroup = knees[m_profile.kneeGuardsIndex].legacyGroup;
}

void ArmorCustomizer::MigrateFromLegacyFields() {
    const auto& helmets = GetSpartanHelmets();
    if (!m_profile.spartanHelmetGroup.empty() && m_profile.helmetIndex == 0) {
        for (size_t i = 0; i < helmets.size(); ++i) {
            if (m_profile.spartanHelmetGroup == helmets[i].legacyGroup) {
                m_profile.helmetIndex = static_cast<int>(i);
                break;
            }
        }
    }
    const auto& shoulders = GetSpartanShoulders();
    if (!m_profile.spartanLeftShoulderGroup.empty() && m_profile.leftShoulderIndex == 0) {
        for (size_t i = 0; i < shoulders.size(); ++i) {
            if (m_profile.spartanLeftShoulderGroup == shoulders[i].legacyLeftGroup) {
                m_profile.leftShoulderIndex = static_cast<int>(i);
                break;
            }
        }
    }
    if (!m_profile.spartanRightShoulderGroup.empty() && m_profile.rightShoulderIndex == 0) {
        for (size_t i = 0; i < shoulders.size(); ++i) {
            if (m_profile.spartanRightShoulderGroup == shoulders[i].legacyRightGroup) {
                m_profile.rightShoulderIndex = static_cast<int>(i);
                break;
            }
        }
    }
    const auto& chests = GetSpartanChests();
    if (!m_profile.spartanChestGroup.empty() && m_profile.chestIndex == 0) {
        for (size_t i = 0; i < chests.size(); ++i) {
            if (m_profile.spartanChestGroup == chests[i].legacyGroup) {
                m_profile.chestIndex = static_cast<int>(i);
                break;
            }
        }
    }
}


void ArmorCustomizer::SetPreferredSpecies(PlayerSpecies species) {
    m_profile.species = species;
    m_previewSpecies = species;
    m_focusedRow = 0;
    m_dirty = true;
    SaveProfile(m_profilePath);
    SyncToReachArmory();
}


void ArmorCustomizer::UpdateArmoryCamera() const {
    if (!ReachArmory::IsMenuMode()) return;
    // Test-only: "-armory-scene-index <n>" forces one of the menu map's camera scenes so the per-piece close-ups can
    // be identified by capture. No effect in normal runs.
    {
        static const std::string forced = [] {
            std::string out;
            int argc = 0;
            LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
            for (int i = 1; argv && i + 1 < argc; ++i) {
                if (_wcsicmp(argv[i], L"-armory-scene-index") == 0) {
                    for (const wchar_t* c = argv[i + 1]; c && *c; ++c) out.push_back(static_cast<char>(*c));
                }
            }
            if (argv) LocalFree(argv);
            return out;
        }();
        if (!forced.empty()) {
            ReachArmory::SetTargetScene("index:" + forced);
            return;
        }
    }
    if (m_previewSpecies == PlayerSpecies::Elite) {
        ReachArmory::SetTargetScene("armory_elite");
        return;
    }
    // Camera scenes in mainmenu_all.map: "armory" (Reach's own 3/4 view), "c_full_spartan" (whole body) and
    // the per-slot close-ups ("c_helmet", "c_chest", ...). Mode 2 (default) follows the slot being edited.
    if (m_cameraMode == 1) ReachArmory::SetTargetScene("c_full_spartan");
    else if (m_cameraMode == 2) ReachArmory::SetTargetScene(m_focusScene.empty() ? "c_full_spartan" : m_focusScene.c_str());
    else ReachArmory::SetTargetScene("armory");
}


namespace {
// Applying the appearance makes the engine re-instantiate the Spartan: the palette flashes back to default and the
// armory idle animation restarts (which reads as "he threw the DMR away"). Holding an arrow key used to trigger that
// on every repeat, so changes are coalesced and applied at most every 250 ms.
ReachArmorySelection s_pendingSelection{};
bool s_pushPending = false;
ULONGLONG s_lastChangeTick = 0;
ULONGLONG s_lastApplyTick = 0;
} // namespace

void ArmorCustomizer::SyncToReachArmory() const {
    const auto& helmets = GetSpartanHelmets();
    const auto& shoulders = GetSpartanShoulders();
    const auto& chests = GetSpartanChests();
    const auto& wrists = GetSpartanWrists();
    const auto& utilities = GetSpartanUtilities();
    const auto& knees = GetSpartanKneeGuards();

    const int hIdx = std::clamp(m_profile.helmetIndex, 0, static_cast<int>(helmets.size()) - 1);
    const auto& hSpec = helmets[hIdx];
    const int hvIdx = std::clamp(m_profile.helmetVariantIndex, 0, static_cast<int>(hSpec.variants.size()) - 1);
    const auto& hvSpec = hSpec.variants[hvIdx];

    const int lsIdx = std::clamp(m_profile.leftShoulderIndex, 0, static_cast<int>(shoulders.size()) - 1);
    const int rsIdx = std::clamp(m_profile.rightShoulderIndex, 0, static_cast<int>(shoulders.size()) - 1);
    const int cIdx = std::clamp(m_profile.chestIndex, 0, static_cast<int>(chests.size()) - 1);
    const auto& cSpec = chests[cIdx];
    const int wIdx = std::clamp(m_profile.wristIndex, 0, static_cast<int>(wrists.size()) - 1);
    const int uIdx = std::clamp(m_profile.utilityIndex, 0, static_cast<int>(utilities.size()) - 1);
    const int kIdx = std::clamp(m_profile.kneeGuardsIndex, 0, static_cast<int>(knees.size()) - 1);
    const int aeIdx = std::clamp(m_profile.armorEffectIndex, 0, static_cast<int>(kArmorEffects.size()) - 1);
    const int fvIdx = std::clamp(m_profile.firefightVoiceIndex, 0, static_cast<int>(kFirefightVoices.size()) - 1);
    const int elIdx = std::clamp(m_profile.eliteArmorSet, 0, static_cast<int>(kEliteSets.size()) - 1);
    const int vcIdx = std::clamp(m_profile.visorColorIndex, 0, static_cast<int>(kVisorColors.size()) - 1);
    const int priIdx = std::clamp(m_profile.primaryColor, 0, 29);
    const int secIdx = std::clamp(m_profile.secondaryColor, 0, 29);

    const auto& pCol = kReachColors[priIdx];
    const auto& sCol = kReachColors[secIdx];
    const auto& vCol = kVisorColors[vcIdx];

    ReachArmorySelection sel;
    sel.isElite = (m_profile.species == PlayerSpecies::Elite);
    sel.previewElite = (m_previewSpecies == PlayerSpecies::Elite);
    sel.isFemale = (m_profile.bodyType == BodyType::Female);
    sel.primaryColorIndex = priIdx;
    sel.secondaryColorIndex = secIdx;
    sel.visorColorIndex = vcIdx;
    sel.armorEffectIndex = aeIdx;
    sel.firefightVoiceIndex = fvIdx;
    sel.eliteSuitIndex = elIdx;
    sel.serviceTag = m_profile.serviceTag;

    sel.helmet = hvSpec.stringId;
    sel.chest = cSpec.stringId;
    sel.leftShoulder = shoulders[lsIdx].leftStringId;
    sel.rightShoulder = shoulders[rsIdx].rightStringId;
    sel.wrist = wrists[wIdx].stringId;
    sel.utility = utilities[uIdx].stringId;
    sel.knees = knees[kIdx].stringId;
    sel.hands = cSpec.hasProstheticArm ? "robot" : "default";
    sel.armorEffect = kArmorEffects[aeIdx].stringId;
    sel.visor = vCol.stringId;
    sel.voice = kFirefightVoices[fvIdx].stringId;
    sel.eliteSuit = kEliteSets[elIdx].stringId;
    sel.eliteHelmet = ElitePieceId(kEliteHelmet, m_profile.eliteHelmetIndex);
    sel.eliteLeftShoulder = ElitePieceId(kEliteLeftShoulder, m_profile.eliteLeftShoulderIndex);
    sel.eliteRightShoulder = ElitePieceId(kEliteRightShoulder, m_profile.eliteRightShoulderIndex);
    sel.eliteChest = ElitePieceId(kEliteChest, m_profile.eliteChestIndex);
    sel.eliteArms = ElitePieceId(kEliteArms, m_profile.eliteArmsIndex);
    sel.eliteLegs = ElitePieceId(kEliteLegs, m_profile.eliteLegsIndex);
    const int elAeIdx = std::clamp(m_profile.eliteArmorEffectIndex, 0, static_cast<int>(kArmorEffects.size()) - 1);
    sel.eliteArmorEffectIndex = elAeIdx;
    sel.eliteArmorEffect = kArmorEffects[elAeIdx].stringId;

    sel.primaryRgb = { SrgbToLinear(pCol.r), SrgbToLinear(pCol.g), SrgbToLinear(pCol.b) };
    sel.secondaryRgb = { SrgbToLinear(sCol.r), SrgbToLinear(sCol.g), SrgbToLinear(sCol.b) };
    sel.visorRgb = { SrgbToLinear(vCol.r), SrgbToLinear(vCol.g), SrgbToLinear(vCol.b) };

    // Coalesce: apply at once when the previous application is older than 250 ms, otherwise keep it pending and let
    // the per-frame flush in Render pick it up. Holding an arrow key then costs one engine write, not one per repeat.
    s_pendingSelection = sel;
    s_pushPending = true;
    s_lastChangeTick = GetTickCount64();
    if (s_lastChangeTick - s_lastApplyTick >= 250) {
        s_pushPending = false;
        s_lastApplyTick = s_lastChangeTick;
        ReachArmory::SetSelection(s_pendingSelection);
        UpdateArmoryCamera();
    }
}

bool ArmorCustomizer::LoadProfile(const std::filesystem::path& path) {
    m_profilePath = path.empty() ? ResolveDefaultProfilePath() : path;
    if (!std::filesystem::exists(m_profilePath)) {
        SyncLegacyFields();
        m_previewSpecies = m_profile.species;
        SyncToReachArmory();
        SaveProfile(m_profilePath);
        return true;
    }

    try {
        std::ifstream f(m_profilePath);
        if (!f.is_open()) return false;

        nlohmann::json j;
        f >> j;

        if (j.contains("service_tag") && j["service_tag"].is_string())
            m_profile.serviceTag = j["service_tag"].get<std::string>();
        if (j.contains("species") && j["species"].is_number_integer())
            m_profile.species = static_cast<PlayerSpecies>(std::clamp(j["species"].get<int>(), 0, 1));
        if (j.contains("body_type") && j["body_type"].is_number_integer())
            m_profile.bodyType = static_cast<BodyType>(std::clamp(j["body_type"].get<int>(), 0, 1));
        if (j.contains("primary_color") && j["primary_color"].is_number_integer())
            m_profile.primaryColor = std::clamp(j["primary_color"].get<int>(), 0, 29);
        if (j.contains("secondary_color") && j["secondary_color"].is_number_integer())
            m_profile.secondaryColor = std::clamp(j["secondary_color"].get<int>(), 0, 29);

        if (j.contains("spartan") && j["spartan"].is_object()) {
            const auto& sp = j["spartan"];
            if (sp.contains("helmet") && sp["helmet"].is_string())
                m_profile.spartanHelmetGroup = sp["helmet"].get<std::string>();
            if (sp.contains("chest") && sp["chest"].is_string())
                m_profile.spartanChestGroup = sp["chest"].get<std::string>();
            if (sp.contains("left_shoulder") && sp["left_shoulder"].is_string())
                m_profile.spartanLeftShoulderGroup = sp["left_shoulder"].get<std::string>();
            if (sp.contains("right_shoulder") && sp["right_shoulder"].is_string())
                m_profile.spartanRightShoulderGroup = sp["right_shoulder"].get<std::string>();
            if (sp.contains("wrist") && sp["wrist"].is_string())
                m_profile.spartanWristGroup = sp["wrist"].get<std::string>();
            if (sp.contains("knees") && sp["knees"].is_string())
                m_profile.spartanKneesGroup = sp["knees"].get<std::string>();

            if (sp.contains("helmet_index") && sp["helmet_index"].is_number_integer())
                m_profile.helmetIndex = sp["helmet_index"].get<int>();
            if (sp.contains("helmet_variant_index") && sp["helmet_variant_index"].is_number_integer())
                m_profile.helmetVariantIndex = sp["helmet_variant_index"].get<int>();
            // The helmet id wins over the variant index (variant lists can be reordered, e.g. GRD).
            if (sp.contains("helmet_id") && sp["helmet_id"].is_string()) {
                const std::string helmetId = sp["helmet_id"].get<std::string>();
                const auto& helmetList = GetSpartanHelmets();
                if (m_profile.helmetIndex >= 0 && m_profile.helmetIndex < static_cast<int>(helmetList.size())) {
                    const auto& variants = helmetList[m_profile.helmetIndex].variants;
                    for (size_t v = 0; v < variants.size(); ++v) {
                        if (helmetId == variants[v].stringId) m_profile.helmetVariantIndex = static_cast<int>(v);
                    }
                }
            }
            if (sp.contains("left_shoulder_index") && sp["left_shoulder_index"].is_number_integer())
                m_profile.leftShoulderIndex = sp["left_shoulder_index"].get<int>();
            if (sp.contains("right_shoulder_index") && sp["right_shoulder_index"].is_number_integer())
                m_profile.rightShoulderIndex = sp["right_shoulder_index"].get<int>();
            if (sp.contains("chest_index") && sp["chest_index"].is_number_integer())
                m_profile.chestIndex = sp["chest_index"].get<int>();
            if (sp.contains("wrist_index") && sp["wrist_index"].is_number_integer())
                m_profile.wristIndex = sp["wrist_index"].get<int>();
            if (sp.contains("utility_index") && sp["utility_index"].is_number_integer())
                m_profile.utilityIndex = sp["utility_index"].get<int>();
            if (sp.contains("visor_color_index") && sp["visor_color_index"].is_number_integer())
                m_profile.visorColorIndex = sp["visor_color_index"].get<int>();
            if (sp.contains("knee_guards_index") && sp["knee_guards_index"].is_number_integer())
                m_profile.kneeGuardsIndex = sp["knee_guards_index"].get<int>();
            if (sp.contains("armor_effect_index") && sp["armor_effect_index"].is_number_integer())
                m_profile.armorEffectIndex = sp["armor_effect_index"].get<int>();
            if (sp.contains("firefight_voice_index") && sp["firefight_voice_index"].is_number_integer())
                m_profile.firefightVoiceIndex = sp["firefight_voice_index"].get<int>();
            // The voice id wins over the index: files saved with the old 16-voice list have other indices.
            if (sp.contains("voice_id") && sp["voice_id"].is_string()) {
                const std::string voiceId = sp["voice_id"].get<std::string>();
                m_profile.firefightVoiceIndex = 0;   // removed ODST voices -> Noble Six
                for (size_t v = 0; v < kFirefightVoices.size(); ++v) {
                    if (voiceId == kFirefightVoices[v].stringId) m_profile.firefightVoiceIndex = static_cast<int>(v);
                }
            }
        }

        if (j.contains("elite") && j["elite"].is_object()) {
            const auto& el = j["elite"];
            if (el.contains("armor_set") && el["armor_set"].is_number_integer()) {
                m_profile.eliteArmorSet = std::clamp(el["armor_set"].get<int>(), 0, static_cast<int>(kEliteSets.size()) - 1);
                m_profile.eliteHelmetIndex = m_profile.eliteArmorSet;
                m_profile.eliteLeftShoulderIndex = m_profile.eliteArmorSet;
                m_profile.eliteRightShoulderIndex = m_profile.eliteArmorSet;
                m_profile.eliteChestIndex = m_profile.eliteArmorSet;
                m_profile.eliteArmsIndex = m_profile.eliteArmorSet;
                m_profile.eliteLegsIndex = m_profile.eliteArmorSet;
            }
            auto maxOf = [](int piece) { return static_cast<int>(GetElitePieces(piece).size()) - 1; };
            if (el.contains("helmet_index") && el["helmet_index"].is_number_integer())
                m_profile.eliteHelmetIndex = std::clamp(el["helmet_index"].get<int>(), 0, maxOf(kEliteHelmet));
            if (el.contains("left_shoulder_index") && el["left_shoulder_index"].is_number_integer())
                m_profile.eliteLeftShoulderIndex = std::clamp(el["left_shoulder_index"].get<int>(), 0, maxOf(kEliteLeftShoulder));
            if (el.contains("right_shoulder_index") && el["right_shoulder_index"].is_number_integer())
                m_profile.eliteRightShoulderIndex = std::clamp(el["right_shoulder_index"].get<int>(), 0, maxOf(kEliteRightShoulder));
            if (el.contains("chest_index") && el["chest_index"].is_number_integer())
                m_profile.eliteChestIndex = std::clamp(el["chest_index"].get<int>(), 0, maxOf(kEliteChest));
            if (el.contains("arms_index") && el["arms_index"].is_number_integer())
                m_profile.eliteArmsIndex = std::clamp(el["arms_index"].get<int>(), 0, maxOf(kEliteArms));
            if (el.contains("legs_index") && el["legs_index"].is_number_integer())
                m_profile.eliteLegsIndex = std::clamp(el["legs_index"].get<int>(), 0, maxOf(kEliteLegs));
            if (el.contains("armor_effect_index") && el["armor_effect_index"].is_number_integer())
                m_profile.eliteArmorEffectIndex = std::clamp(el["armor_effect_index"].get<int>(), 0, static_cast<int>(kArmorEffects.size()) - 1);
            else if (el.contains("armor_effect_id") && el["armor_effect_id"].is_string()) {
                const std::string fxId = el["armor_effect_id"].get<std::string>();
                for (size_t a = 0; a < kArmorEffects.size(); ++a)
                    if (fxId == kArmorEffects[a].stringId) m_profile.eliteArmorEffectIndex = static_cast<int>(a);
            }
        }

        if (j.contains("nameplate_index") && j["nameplate_index"].is_number_integer())
            m_profile.nameplateIndex = j["nameplate_index"].get<int>();
        if (j.contains("emblem_index") && j["emblem_index"].is_number_integer())
            m_profile.emblemIndex = j["emblem_index"].get<int>();

        MigrateFromLegacyFields();
        SyncLegacyFields();
        m_previewSpecies = m_profile.species;
        SyncToReachArmory();
        NOBLE_INFO("ArmorCustomizer: Loaded profile from '{}' (Tag: {}, PreferredSpecies: {})",
                   m_profilePath.string(), m_profile.serviceTag,
                   m_profile.species == PlayerSpecies::Elite ? "Elite" : "Spartan");
        return true;
    } catch (const std::exception& e) {
        NOBLE_WARN("ArmorCustomizer: Failed to parse '{}': {}", m_profilePath.string(), e.what());
        SyncLegacyFields();
        m_previewSpecies = m_profile.species;
        SyncToReachArmory();
        return false;
    }
}

bool ArmorCustomizer::SaveProfile(const std::filesystem::path& path) {
    const std::filesystem::path targetPath = path.empty() ? ResolveDefaultProfilePath() : path;
    m_profilePath = targetPath;
    SyncLegacyFields();
    SyncToReachArmory();
    try {
        const auto& helmets = GetSpartanHelmets();
        const auto& shoulders = GetSpartanShoulders();
        const auto& chests = GetSpartanChests();
        const auto& wrists = GetSpartanWrists();
        const auto& utilities = GetSpartanUtilities();
        const auto& knees = GetSpartanKneeGuards();

        const auto& h = helmets[m_profile.helmetIndex];
        const auto& hv = h.variants[m_profile.helmetVariantIndex];

        nlohmann::json j;
        j["service_tag"] = m_profile.serviceTag;
        j["species"] = static_cast<int>(m_profile.species);
        j["body_type"] = static_cast<int>(m_profile.bodyType);
        j["primary_color"] = m_profile.primaryColor;
        j["secondary_color"] = m_profile.secondaryColor;

        j["spartan"] = {
            {"helmet", m_profile.spartanHelmetGroup},
            {"chest", m_profile.spartanChestGroup},
            {"left_shoulder", m_profile.spartanLeftShoulderGroup},
            {"right_shoulder", m_profile.spartanRightShoulderGroup},
            {"wrist", m_profile.spartanWristGroup},
            {"knees", m_profile.spartanKneesGroup},
            {"helmet_index", m_profile.helmetIndex},
            {"helmet_variant_index", m_profile.helmetVariantIndex},
            {"left_shoulder_index", m_profile.leftShoulderIndex},
            {"right_shoulder_index", m_profile.rightShoulderIndex},
            {"chest_index", m_profile.chestIndex},
            {"wrist_index", m_profile.wristIndex},
            {"utility_index", m_profile.utilityIndex},
            {"visor_color_index", m_profile.visorColorIndex},
            {"knee_guards_index", m_profile.kneeGuardsIndex},
            {"armor_effect_index", m_profile.armorEffectIndex},
            {"firefight_voice_index", m_profile.firefightVoiceIndex},
            {"helmet_id", hv.stringId},
            {"left_shoulder_id", shoulders[m_profile.leftShoulderIndex].leftStringId},
            {"right_shoulder_id", shoulders[m_profile.rightShoulderIndex].rightStringId},
            {"chest_id", chests[m_profile.chestIndex].stringId},
            {"hands_id", chests[m_profile.chestIndex].hasProstheticArm ? "robot" : "default"},
            {"wrist_id", wrists[m_profile.wristIndex].stringId},
            {"utility_id", utilities[m_profile.utilityIndex].stringId},
            {"visor_id", kVisorColors[m_profile.visorColorIndex].stringId},
            {"knees_id", knees[m_profile.kneeGuardsIndex].stringId},
            {"armor_effect_id", kArmorEffects[m_profile.armorEffectIndex].stringId},
            {"voice_id", kFirefightVoices[m_profile.firefightVoiceIndex].stringId}
        };

        j["elite"] = {
            {"armor_set", m_profile.eliteArmorSet},
            {"armor_set_id", kEliteSets[std::clamp(m_profile.eliteArmorSet, 0, static_cast<int>(kEliteSets.size()) - 1)].stringId},
            {"helmet_index", m_profile.eliteHelmetIndex},
            {"left_shoulder_index", m_profile.eliteLeftShoulderIndex},
            {"right_shoulder_index", m_profile.eliteRightShoulderIndex},
            {"chest_index", m_profile.eliteChestIndex},
            {"arms_index", m_profile.eliteArmsIndex},
            {"legs_index", m_profile.eliteLegsIndex},
            {"helmet_id", ElitePieceId(kEliteHelmet, m_profile.eliteHelmetIndex)},
            {"left_shoulder_id", ElitePieceId(kEliteLeftShoulder, m_profile.eliteLeftShoulderIndex)},
            {"right_shoulder_id", ElitePieceId(kEliteRightShoulder, m_profile.eliteRightShoulderIndex)},
            {"chest_id", ElitePieceId(kEliteChest, m_profile.eliteChestIndex)},
            {"arms_id", ElitePieceId(kEliteArms, m_profile.eliteArmsIndex)},
            {"legs_id", ElitePieceId(kEliteLegs, m_profile.eliteLegsIndex)},
            {"armor_effect_index", m_profile.eliteArmorEffectIndex},
            {"armor_effect_id", kArmorEffects[std::clamp(m_profile.eliteArmorEffectIndex, 0, static_cast<int>(kArmorEffects.size()) - 1)].stringId}
        };

        j["nameplate_index"] = m_profile.nameplateIndex;
        j["emblem_index"] = m_profile.emblemIndex;

        std::error_code ec;
        if (targetPath.has_parent_path()) {
            std::filesystem::create_directories(targetPath.parent_path(), ec);
        }
        std::ofstream f(targetPath);
        if (!f.is_open()) return false;
        f << j.dump(2);
        m_dirty = false;
        NOBLE_INFO("ArmorCustomizer: Saved profile to '{}' (PreferredSpecies: {}, EliteSet: {})",
                   targetPath.string(),
                   m_profile.species == PlayerSpecies::Elite ? "Elite" : "Spartan",
                   kEliteSets[m_profile.eliteArmorSet].stringId);
        return true;
    } catch (const std::exception& e) {
        NOBLE_ERROR("ArmorCustomizer: Failed to save '{}': {}", targetPath.string(), e.what());
        return false;
    }
}

namespace {

// The OBJ exports live outside the repo (git-ignored data\armor\, never committed): looked up next to the exe
// and up to three folders above it.
std::filesystem::path FindArmorAssetPath(const std::filesystem::path& relativePath) {
    std::error_code ec;
    wchar_t exePath[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) > 0) {
        std::filesystem::path dir = std::filesystem::path(exePath).parent_path();
        for (int depth = 0; depth < 4 && !dir.empty(); ++depth) {
            const std::filesystem::path candidate = dir / relativePath;
            if (std::filesystem::is_regular_file(candidate, ec)) return candidate;
            dir = dir.parent_path();
        }
    }
    return relativePath;
}

// Camera framing per slot for the OBJ preview: {zoom, look-at height}. 1.0 = whole body.
struct Framing { float zoom, focusY, focusX; };
Framing FramingFor(const std::string& scene, int cameraMode, bool elite) {
    // The preview scales the body to a fixed size around its centre (about the hips); the camera sees
    // +-0.86/zoom up and down. Measured from a capture: head ~+1.7, shoulders ~+1.3, chest ~+0.9,
    // thighs ~-0.5, knees ~-1.05, feet ~-1.95.
    // focusX moves the camera sideways: the left shoulder pans left, the right shoulder right, and the left-arm
    // pieces (wrist, utility) follow their side. This is what the engine's single shoulder camera cannot do.
    if (cameraMode == 1 || elite) return { 0.41f, 0.0f, 0.0f };
    if (cameraMode == 0) return { 0.52f, 0.40f, 0.0f };
    if (scene == "c_helmet") return { 1.15f, 1.05f, 0.0f };
    if (scene == "c_left_shoulder") return { 1.05f, 0.85f, -0.55f };
    if (scene == "c_right_shoulder") return { 1.05f, 0.85f, 0.55f };
    if (scene == "c_chest") return { 0.90f, 0.45f, 0.0f };
    if (scene == "c_wrist") return { 0.62f, 0.55f, -0.45f };
    if (scene == "c_utility") return { 0.95f, -0.55f, -0.30f };
    if (scene == "c_knees") return { 0.90f, -1.00f, 0.0f };
    return { 0.41f, 0.0f, 0.0f };
}

} // namespace

void ArmorCustomizer::RenderCharacterPreview(ID3D11Device* device, ID3D11DeviceContext* context,
                                             float width, float height, bool /*fullBleed*/) {
    (void)device;
    (void)context;
    const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Reach's own Armory scene draws the Spartan - the shipped in-game model. Noble has no stand-in: the OBJ re-exports
    // in data\armor are a different mesh and must not be used (Tyler, 2026-10-31).
    static const bool menuMapPresent = std::filesystem::exists(ReachHost::Get().GetDataDir() / "menumaps" / "mainmenu_all.map");
    if (!ReachArmory::IsSceneReady()) {
        const std::string msg = menuMapPresent
            ? std::string("LOADING ARMORY") + std::string(static_cast<size_t>(static_cast<int>(ImGui::GetTime() * 2.0) % 4), '.')
            : std::string("NO 3D PREVIEW: data\\menumaps\\mainmenu_all.map is missing");
        const ImVec2 ts = ImGui::CalcTextSize(msg.c_str());
        dl->AddText(ImVec2(cursorPos.x + std::max(0.0f, (width - ts.x) * 0.5f), cursorPos.y + height * 0.45f),
                    IM_COL32(143, 216, 255, 220), msg.c_str());
    }

    if (m_autoRotate) ReachArmory::AddYawRotation(ImGui::GetIO().DeltaTime * 0.45f);
    ImGui::InvisibleButton("##character_preview_drag", ImVec2(width, height));
    if (ImGui::IsItemActive() &&
        (ImGui::IsMouseDragging(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Right))) {
        ReachArmory::AddYawRotation(-ImGui::GetIO().MouseDelta.x * 0.0085f);
    }
    if (ImGui::IsItemHovered()) {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (std::abs(wheel) > 0.01f) {
            if (wheel > 0.0f) m_cameraMode = 2;
            else m_cameraMode = (m_cameraMode == 2) ? 1 : 0;
            UpdateArmoryCamera();
        }
    }
}

namespace {

// One option in a picker grid.
struct PickOption {
    std::string name;
    std::string description;
    ImU32 swatch = 0;     // colour tiles
    const char* art = nullptr;   // picture tiles: stem of data\ui\reach\elite_sets\<art>.png (Elite armour sets)
    bool locked = false;         // rank not reached yet
    const char* lockRank = nullptr;
    const char* artDir = nullptr;   // picture folder under data/ui/reach (nullptr = "elite_sets")
    bool forSale = false;        // rank reached, not bought yet (Spartan items with a cR price)
    int64_t price = 0;
    std::string buyKey;          // "<slot>:<permutation id>" in career.json
};

// Picture, rank lock and price of a Spartan option (from kSpartanItems). Rank not reached -> locked;
// rank reached but not bought (cR price > 0) -> for sale. "-armory-unlock-all" skips both.
void SpartanArmoryItem(PickOption& o, const char* slot, const char* id) {
    const SpartanItem* it = FindSpartanItem(slot, id);
    if (!it) return;
    if (it->pic[0]) { o.art = it->pic; o.artDir = it->picDir; }
    o.buyKey = std::string(slot) + ":" + id;
    if (UnlockAllArg()) return;
    const int need = RankIndexByName(it->rank);
    if (need > PlayerRankIndex()) {
        o.locked = true;
        o.lockRank = Career::Ranks()[static_cast<size_t>(need)].name;
        o.description += std::string(o.description.empty() ? "" : " ") + "Unlocks at " + o.lockRank + ".";
    } else if (it->credits > 0 && !Career::OwnsArmoryItem(o.buyKey)) {
        o.forSale = true;
        o.price = it->credits;
        o.description += std::string(o.description.empty() ? "" : " ") + "Costs " + WithCommas(it->credits) + " cR.";
    }
}

static void LockEliteOption(PickOption& o, const char* id) {
    const char* rank = nullptr;
    if (EliteUnlockRank(id, &rank) == 0 || !rank) return;
    o.lockRank = rank;
    o.locked = EliteLocked(id);
    o.description += std::string(o.description.empty() ? "" : " ") + "Unlocks at " + rank + ".";
}

enum class RowKind { List, Colour, Tag };

// One slot in the left list (Armory v2, spec 4.6).
struct SlotRow {
    const char* group;    // CHARACTER / ARMOR / COLORS / EFFECTS / IDENTITY
    const char* label;
    RowKind kind = RowKind::List;
    int* index = nullptr;
    std::vector<PickOption> options;
    const char* scene = "c_full_spartan";   // camera close-up while this slot is edited
};

ImU32 ColourU32(float r, float g, float b) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.0f));
}

void TextAt(ImDrawList* dl, ImFont* f, float px, ImVec2 p, ImU32 col, const char* t, float wrap = 0.0f) {
    dl->AddText(f ? f : ImGui::GetFont(), px, p, col, t, nullptr, wrap);
}

float TextW(ImFont* f, float px, const char* t) {
    return (f ? f : ImGui::GetFont())->CalcTextSizeA(px, FLT_MAX, 0.0f, t).x;
}

} // namespace

void ArmorCustomizer::Render(ID3D11Device* device, ID3D11DeviceContext* context) {
    SyncLegacyFields();
    if (!m_syncedInitial) {
        m_syncedInitial = true;
        SyncToReachArmory();
    } else {
        // Apply a coalesced appearance change once the keys have been quiet for 250 ms.
        if (s_pushPending && GetTickCount64() - s_lastChangeTick >= 250) {
            s_pushPending = false;
            s_lastApplyTick = GetTickCount64();
            ReachArmory::SetSelection(s_pendingSelection);
        }
        UpdateArmoryCamera();
    }

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float s = std::max(0.75f, avail.y / 640.0f);
    ImFont* titleFont = NobleStyle::TitleFont();
    ImFont* smallFont = NobleStyle::SmallFont();
    ImFont* bodyFont = ImGui::GetFont();
    const ImU32 kAccentCol = IM_COL32(143, 216, 255, 255);
    const ImU32 kDimCol = IM_COL32(160, 178, 196, 255);
    const ImU32 kWhiteCol = IM_COL32(240, 246, 252, 255);

    // Q / E or the right stick turn the model.
    if (!ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyDown(ImGuiKey_Q) || ImGui::IsKeyDown(ImGuiKey_GamepadRStickLeft)) {
            ReachArmory::AddYawRotation(ImGui::GetIO().DeltaTime * 1.8f);
            m_objYaw += ImGui::GetIO().DeltaTime * 1.8f;
        }
        if (ImGui::IsKeyDown(ImGuiKey_E) || ImGui::IsKeyDown(ImGuiKey_GamepadRStickRight)) {
            ReachArmory::AddYawRotation(-ImGui::GetIO().DeltaTime * 1.8f);
            m_objYaw -= ImGui::GetIO().DeltaTime * 1.8f;
        }
    }

    // ---- Layout: slot list | picker | model (Reach's Armory camera puts the model on the right).
    const float listW = std::clamp(avail.x * 0.22f, 250.0f * s, 330.0f * s);
    const float gap = 14.0f * s;
    const float pickX = origin.x + listW + gap;
    const float pickW = std::clamp(avail.x * 0.30f, 330.0f * s, 470.0f * s);
    const float previewX = pickX + pickW + gap;
    const float previewW = std::max(100.0f, origin.x + avail.x - previewX);

    ImGui::SetCursorScreenPos(ImVec2(previewX, origin.y));
    RenderCharacterPreview(device, context, previewW, avail.y, true);

    // ---- Slot rows for the species being edited.
    int speciesInt = static_cast<int>(m_profile.species);
    int bodyInt = static_cast<int>(m_profile.bodyType);
    std::vector<SlotRow> rows;
    {
        SlotRow r{ "CHARACTER", "Species", RowKind::List, &speciesInt };
        r.options = { { "SPARTAN", "UNSC SPARTAN-III. Used in multiplayer, Forge and custom games." },
                      { "ELITE", "Sangheili Elite. Used in multiplayer, Forge and custom games." } };
        rows.push_back(r);
    }



    if (m_previewSpecies == PlayerSpecies::Spartan) {
        {
            SlotRow r{ "CHARACTER", "Body", RowKind::List, &bodyInt };
            r.options = { { "MALE", "Male Spartan body." }, { "FEMALE", "Female Spartan body." } };
            rows.push_back(r);
        }
        auto addList = [&](const char* label, int* idx, const char* scene, auto&& fill) {
            SlotRow r{ "ARMOR", label, RowKind::List, idx };
            r.scene = scene;
            fill(r.options);
            rows.push_back(std::move(r));
        };
        auto spartanItem = [](PickOption& o, const char* slot, const char* id) { SpartanArmoryItem(o, slot, id); };
        const auto& helmets = GetSpartanHelmets();
        addList("Helmet", &m_profile.helmetIndex, "c_helmet", [&](std::vector<PickOption>& o) {
            for (const auto& h : helmets) { o.push_back({ h.name, h.description }); spartanItem(o.back(), "helmet", h.stringId); }
        });
        addList("Helmet Variant", &m_profile.helmetVariantIndex, "c_helmet", [&](std::vector<PickOption>& o) {
            const auto& h = helmets[std::clamp(m_profile.helmetIndex, 0, static_cast<int>(helmets.size()) - 1)];
            for (const auto& v : h.variants) {
                o.push_back({ v.name, v.description });
                // The base variant is the helmet itself.
                if (std::strcmp(v.stringId, h.stringId) == 0) spartanItem(o.back(), "helmet", h.stringId);
                else spartanItem(o.back(), "helmet attachment", v.stringId);
            }
        });
        const auto& shoulders = GetSpartanShoulders();
        addList("Left Shoulder", &m_profile.leftShoulderIndex, "c_left_shoulder", [&](std::vector<PickOption>& o) {
            for (const auto& x : shoulders) { o.push_back({ x.name, x.description }); spartanItem(o.back(), "left shoulder", x.leftStringId); }
        });
        addList("Right Shoulder", &m_profile.rightShoulderIndex, "c_right_shoulder", [&](std::vector<PickOption>& o) {
            for (const auto& x : shoulders) { o.push_back({ x.name, x.description }); spartanItem(o.back(), "right shoulder", x.rightStringId); }
        });
        auto multi = [&](const std::vector<MultiLayerSpec>& list, const char* slot) {
            return [&list, slot, &spartanItem](std::vector<PickOption>& o) {
                for (const auto& x : list) { o.push_back({ x.name, x.description }); spartanItem(o.back(), slot, x.stringId); }
            };
        };
        addList("Chest", &m_profile.chestIndex, "c_chest", multi(GetSpartanChests(), "chest"));
        addList("Wrist", &m_profile.wristIndex, "c_wrist", multi(GetSpartanWrists(), "wrist"));
        addList("Utility", &m_profile.utilityIndex, "c_utility", multi(GetSpartanUtilities(), "utility"));
        addList("Knee Guards", &m_profile.kneeGuardsIndex, "c_knees", multi(GetSpartanKneeGuards(), "knees"));
        addList("Visor Color", &m_profile.visorColorIndex, "c_helmet", [&](std::vector<PickOption>& o) {
            for (const auto& v : kVisorColors) { o.push_back({ v.name, v.description, ColourU32(v.r, v.g, v.b) }); spartanItem(o.back(), "visor", v.stringId); }
        });
    } else {
        // Locked items can't stay equipped (e.g. a look saved before the rank locks): they fall back to Minor.
        {
            bool reset = false;
            const int set = std::clamp(m_profile.eliteArmorSet, 0, static_cast<int>(kEliteSets.size()) - 1);
            if (EliteLocked(kEliteSets[set].stringId)) { m_profile.eliteArmorSet = 0; reset = true; }
            int* pieces[kElitePieceCount] = { &m_profile.eliteHelmetIndex, &m_profile.eliteLeftShoulderIndex,
                &m_profile.eliteRightShoulderIndex, &m_profile.eliteChestIndex, &m_profile.eliteArmsIndex, &m_profile.eliteLegsIndex };
            for (int p = 0; p < kElitePieceCount; ++p)
                if (EliteLocked(ElitePieceId(p, *pieces[p]))) { *pieces[p] = 0; reset = true; }
            if (reset) {
                NOBLE_INFO("ArmorCustomizer: locked Elite items in the saved look were put back to Minor (rank {})",
                           Career::Ranks()[static_cast<size_t>(PlayerRankIndex())].name);
                m_dirty = true;
                SaveProfile(m_profilePath);
            }
        }
        SlotRow r{ "ARMOR", "Armor Set", RowKind::List, &m_profile.eliteArmorSet };
        for (const auto& e : kEliteSets) {
            r.options.push_back({ e.name, e.description, 0, e.stringId });
            LockEliteOption(r.options.back(), e.stringId);
        }
        rows.push_back(r);
        // Single pieces (any mix, plus the extra Elite pieces). Picking an Armor Set resets them.
        auto addPiece = [&](const char* label, int* idx, int piece) {
            SlotRow p{ "ARMOR", label, RowKind::List, idx };
            for (const auto& e : GetElitePieces(piece)) {
                p.options.push_back({ e.name, e.description });
                LockEliteOption(p.options.back(), e.stringId);
            }
            rows.push_back(std::move(p));
        };
        addPiece("Helmet", &m_profile.eliteHelmetIndex, kEliteHelmet);
        addPiece("Left Shoulder", &m_profile.eliteLeftShoulderIndex, kEliteLeftShoulder);
        addPiece("Right Shoulder", &m_profile.eliteRightShoulderIndex, kEliteRightShoulder);
        addPiece("Chest", &m_profile.eliteChestIndex, kEliteChest);
        addPiece("Arms", &m_profile.eliteArmsIndex, kEliteArms);
        addPiece("Legs", &m_profile.eliteLegsIndex, kEliteLegs);
    }
    {
        SlotRow p{ "COLORS", "Primary", RowKind::Colour, &m_profile.primaryColor };
        SlotRow q{ "COLORS", "Secondary", RowKind::Colour, &m_profile.secondaryColor };
        for (const auto& c : kReachColors) {
            p.options.push_back({ c.name, "", ColourU32(c.r, c.g, c.b) });
            q.options.push_back({ c.name, "", ColourU32(c.r, c.g, c.b) });
        }
        rows.push_back(p);
        rows.push_back(q);
    }



    if (m_previewSpecies == PlayerSpecies::Spartan) {
        SlotRow r{ "EFFECTS & VOICE", "Armor Effect", RowKind::List, &m_profile.armorEffectIndex };
        r.scene = "c_helmet";
        for (const auto& e : kArmorEffects) {
            r.options.push_back({ e.name, e.description });
            SpartanArmoryItem(r.options.back(), "armor effect", e.stringId);
        }
        rows.push_back(r);
        SlotRow v{ "EFFECTS & VOICE", "Firefight Voice", RowKind::List, &m_profile.firefightVoiceIndex };
        for (const auto& e : kFirefightVoices) {
            v.options.push_back({ e.name, e.description });
            SpartanArmoryItem(v.options.back(), "firefight voice", e.stringId);
        }
        rows.push_back(v);
    } else {
        SlotRow r{ "EFFECTS & VOICE", "Armor Effect", RowKind::List, &m_profile.eliteArmorEffectIndex };
        r.scene = "c_helmet";
        for (const auto& e : kArmorEffects) {
            r.options.push_back({ e.name, e.description });
            LockEliteOption(r.options.back(), e.stringId);
        }
        rows.push_back(r);
    }

    {
        const auto& nameplates = UiArt::GetNameplateCatalog();
        SlotRow npRow{ "IDENTITY", "Nameplate", RowKind::List, &m_profile.nameplateIndex };
        for (const auto& np : nameplates) {
            npRow.options.push_back({ np.name.c_str(), "MCC Halo player nameplate banner." });
        }
        rows.push_back(std::move(npRow));

        const auto& emblems = UiArt::GetEmblemCatalog();
        SlotRow embRow{ "IDENTITY", "Emblem", RowKind::List, &m_profile.emblemIndex };
        for (const auto& em : emblems) {
            embRow.options.push_back({ em.name.c_str(), "Halo player identity emblem icon." });
        }
        rows.push_back(std::move(embRow));

        rows.push_back({ "IDENTITY", "Service Tag", RowKind::Tag });
    }

    const int rowCount = static_cast<int>(rows.size());
    m_focusedRow = std::clamp(m_focusedRow, 0, rowCount - 1);

    // Items that are locked or not bought can't stay equipped (e.g. a look saved before the locks): back to the
    // slot's first option (the default piece).



    if (m_previewSpecies == PlayerSpecies::Spartan) {
        bool reset = false;
        for (SlotRow& r : rows) {
            if (!r.index || r.options.empty() || r.index == &speciesInt || r.index == &bodyInt) continue;
            const int v = std::clamp(*r.index, 0, static_cast<int>(r.options.size()) - 1);
            if (r.options[v].locked || r.options[v].forSale) {
                NOBLE_INFO("ArmorCustomizer: {} '{}' isn't unlocked yet; back to '{}'", r.label, r.options[v].name, r.options[0].name);
                *r.index = 0;
                if (r.index == &m_profile.helmetIndex) m_profile.helmetVariantIndex = 0;
                reset = true;
            }
        }
        if (reset) {
            m_dirty = true;
            SaveProfile(m_profilePath);
        }
    }

    // An item clicked that can't be equipped yet (rank not reached, or not bought): it is tried on in the 3D
    // preview, and the info panel shows its rank or offers BUY.
    static int s_buyRow = -1, s_buyValue = -1;

    // Picks an option for a row: saves, updates the model, keeps dependent fields right.
    // bought = true right after a purchase (the option was built as "for sale" this frame).
    auto choose = [&](int rowIndex, int value, bool bought = false) {
        SlotRow& r = rows[rowIndex];
        if (!r.index || r.options.empty()) return;
        value = std::clamp(value, 0, static_cast<int>(r.options.size()) - 1);
        if (r.options[value].locked || (r.options[value].forSale && !bought)) {   // try it on, don't equip
            s_buyRow = rowIndex;
            s_buyValue = value;
            return;
        }
        s_buyRow = -1;
        if (*r.index == value && r.index != &speciesInt && r.index != &bodyInt) return;
        *r.index = value;
        if (r.index == &speciesInt) {
            m_profile.species = static_cast<PlayerSpecies>(std::clamp(speciesInt, 0, 1));
            m_previewSpecies = m_profile.species;
            m_focusedRow = 0;
        } else if (r.index == &bodyInt) {
            m_profile.bodyType = static_cast<BodyType>(std::clamp(bodyInt, 0, 1));
        } else if (r.index == &m_profile.helmetIndex) {
            m_profile.helmetVariantIndex = 0;   // a new helmet starts on its base variant
        } else if (r.index == &m_profile.eliteArmorSet) {
            m_profile.eliteHelmetIndex = m_profile.eliteLeftShoulderIndex = m_profile.eliteRightShoulderIndex =
                m_profile.eliteChestIndex = m_profile.eliteArmsIndex = m_profile.eliteLegsIndex = m_profile.eliteArmorSet;
        }
        m_dirty = true;
        SaveProfile(m_profilePath);
    };
    auto focus = [&](int rowIndex) {
        if (m_focusedRow == rowIndex) return;
        m_focusedRow = rowIndex;
        s_buyRow = -1;
        m_pickerScrollToEquipped = true;
    };

    // Keys / d-pad: up/down picks the slot, left/right steps through its options.
    if (!ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown)) focus((m_focusedRow + 1) % rowCount);
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp)) focus((m_focusedRow - 1 + rowCount) % rowCount);
        SlotRow& fr = rows[m_focusedRow];
        if (fr.index && !fr.options.empty()) {
            const int n = static_cast<int>(fr.options.size());
            // Steps to the next option that isn't locked.
            auto step = [&](int dir) {
                int v = *fr.index;
                for (int k = 0; k < n; ++k) {
                    v = (v + dir + n) % n;
                    if (!fr.options[v].locked && !fr.options[v].forSale) { choose(m_focusedRow, v); break; }
                }
                m_pickerScrollToEquipped = true;
            };
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickLeft)) step(-1);
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight) || ImGui::IsKeyPressed(ImGuiKey_GamepadLStickRight)) step(1);
        }
    }
    // "-armory-focus <label>" (test only): start on a named slot so the per-slot camera can be captured.
    {
        static bool s_applied = false;
        if (!s_applied) {
            s_applied = true;
            int argc = 0;
            LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
            std::wstring want;
            for (int i = 1; argv && i + 1 < argc; ++i) {
                if (_wcsicmp(argv[i], L"-armory-focus") == 0) want = argv[i + 1];
            }
            if (argv) LocalFree(argv);
            if (!want.empty()) {
                for (int i = 0; i < rowCount; ++i) {
                    std::wstring label(rows[i].label, rows[i].label + std::strlen(rows[i].label));
                    if (label.find(want) != std::wstring::npos) { focus(i); break; }
                }
            }
        }
    }
    m_focusScene = m_previewSpecies == PlayerSpecies::Spartan ? rows[m_focusedRow].scene : "armory_elite";


    // ---- Left: grouped slot list.
    {
        ImGui::SetCursorScreenPos(origin);
        ImGui::BeginChild("##armory_slots", ImVec2(listW, avail.y), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground);

        // Preferred Species Switcher
        {
            const float btnW = (listW - 12.0f * s) * 0.5f;
            const float btnH = 28.0f * s;
            const bool isSpartan = (m_profile.species == PlayerSpecies::Spartan);
            const bool isElite = (m_profile.species == PlayerSpecies::Elite);

            ImGui::PushStyleColor(ImGuiCol_Button, isSpartan ? ImVec4(0.20f, 0.45f, 0.70f, 0.85f) : ImVec4(0.10f, 0.14f, 0.18f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_Border, isSpartan ? ImVec4(0.55f, 0.85f, 1.0f, 1.0f) : ImVec4(0.25f, 0.35f, 0.45f, 0.4f));
            if (ImGui::Button("SPARTAN", ImVec2(btnW, btnH))) {
                SetPreferredSpecies(PlayerSpecies::Spartan);
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, isElite ? ImVec4(0.65f, 0.30f, 0.15f, 0.85f) : ImVec4(0.10f, 0.14f, 0.18f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_Border, isElite ? ImVec4(1.0f, 0.65f, 0.35f, 1.0f) : ImVec4(0.25f, 0.35f, 0.45f, 0.4f));
            if (ImGui::Button("ELITE", ImVec2(btnW, btnH))) {
                SetPreferredSpecies(PlayerSpecies::Elite);
            }
            ImGui::PopStyleColor(2);

            ImGui::Dummy(ImVec2(0, 4.0f * s));
        }

        ImDrawList* ldl = ImGui::GetWindowDrawList();
        const float rowH = 30.0f * s;
        const char* lastGroup = nullptr;
        for (int i = 0; i < rowCount; ++i) {
            SlotRow& r = rows[i];
            if (!lastGroup || std::strcmp(lastGroup, r.group) != 0) {
                lastGroup = r.group;
                ImGui::Dummy(ImVec2(0, lastGroup == rows[0].group && i == 0 ? 0.0f : 6.0f * s));
                const ImVec2 hp = ImGui::GetCursorScreenPos();
                TextAt(ldl, smallFont, 13.0f * s, hp, IM_COL32(150, 190, 220, 255), r.group);
                const float tw = TextW(smallFont, 13.0f * s, r.group);
                ldl->AddLine(ImVec2(hp.x + tw + 8.0f * s, hp.y + 7.0f * s), ImVec2(hp.x + listW - 6.0f * s, hp.y + 7.0f * s), IM_COL32(255, 255, 255, 40));
                ImGui::Dummy(ImVec2(listW, 20.0f * s));
            }
            const ImVec2 p0 = ImGui::GetCursorScreenPos();
            const ImVec2 p1(p0.x + listW - 6.0f * s, p0.y + rowH);
            ImGui::PushID(i);
            const bool clicked = ImGui::InvisibleButton("##slot", ImVec2(listW - 6.0f * s, rowH));
            const bool hovered = ImGui::IsItemHovered();
            ImGui::PopID();
            if (clicked) focus(i);
            const bool selected = m_focusedRow == i;
            if (selected) {
                ldl->AddRectFilledMultiColor(p0, p1, IM_COL32(60, 120, 170, 150), IM_COL32(60, 120, 170, 30),
                                             IM_COL32(60, 120, 170, 30), IM_COL32(60, 120, 170, 150));
                ldl->AddRectFilled(p0, ImVec2(p0.x + 3.0f * s, p1.y), kAccentCol);
            } else if (hovered) {
                ldl->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 14));
            }
            const float ty = p0.y + rowH * 0.5f - 9.0f * s;
            TextAt(ldl, bodyFont, 17.0f * s, ImVec2(p0.x + 12.0f * s, ty), selected ? kWhiteCol : kDimCol, r.label);
            // Current value on the right (a swatch for colours).
            const float vr = p1.x - 8.0f * s;
            if (r.kind == RowKind::Colour && r.index) {
                const ImU32 c = r.options[std::clamp(*r.index, 0, 29)].swatch;
                ldl->AddRectFilled(ImVec2(vr - 18.0f * s, p0.y + 7.0f * s), ImVec2(vr, p1.y - 7.0f * s), c);
                ldl->AddRect(ImVec2(vr - 18.0f * s, p0.y + 7.0f * s), ImVec2(vr, p1.y - 7.0f * s), IM_COL32(230, 240, 255, 160));
            } else {
                std::string value = r.kind == RowKind::Tag ? m_profile.serviceTag
                                  : (r.index && !r.options.empty()) ? r.options[std::clamp(*r.index, 0, static_cast<int>(r.options.size()) - 1)].name : "";
                const float maxW = listW * 0.48f;
                while (value.size() > 3 && TextW(smallFont, 14.0f * s, value.c_str()) > maxW) value = value.substr(0, value.size() - 4) + "...";
                TextAt(ldl, smallFont, 14.0f * s, ImVec2(vr - TextW(smallFont, 14.0f * s, value.c_str()), ty + 2.0f * s),
                       selected ? kAccentCol : IM_COL32(130, 150, 170, 255), value.c_str());
            }
        }
        ImGui::EndChild();
    }

    // ---- Middle: picker for the focused slot.
    SlotRow& row = rows[m_focusedRow];
    const ImVec2 k0(pickX, origin.y), k1(pickX + pickW, origin.y + avail.y);
    dl->AddRectFilled(k0, k1, IM_COL32(8, 14, 22, 190), 3.0f * s);
    dl->AddRect(k0, k1, IM_COL32(170, 220, 255, 45), 3.0f * s);
    const float pad = 14.0f * s;
    float y = origin.y + pad;
    std::string heading = row.label;
    for (char& c : heading) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    TextAt(dl, titleFont, 26.0f * s, ImVec2(pickX + pad, y), kWhiteCol, heading.c_str());
    y += 34.0f * s;
    int hoveredOption = -1;

    if (row.kind == RowKind::Tag) {
        TextAt(dl, bodyFont, 16.0f * s, ImVec2(pickX + pad, y), kDimCol,
               "Your service tag, up to 4 letters or numbers. Shown above your head in game.", pickW - pad * 2.0f);
        y += 48.0f * s;
        ImGui::SetCursorScreenPos(ImVec2(pickX + pad, y));
        ImGui::SetNextItemWidth(160.0f * s);
        char tagBuf[8] = {};
        std::snprintf(tagBuf, sizeof(tagBuf), "%s", m_profile.serviceTag.c_str());
        if (ImGui::InputText("##service_tag_in", tagBuf, 5, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank)) {
            m_profile.serviceTag = tagBuf;
            m_dirty = true;
            SaveProfile(m_profilePath);
        }
    } else if (row.index && !row.options.empty()) {
        const int n = static_cast<int>(row.options.size());
        const int equipped = std::clamp(*row.index, 0, n - 1);
        // "Equipped: X" and a RANDOM button.
        const std::string eq = "EQUIPPED: " + row.options[equipped].name;
        TextAt(dl, smallFont, 15.0f * s, ImVec2(pickX + pad, y + 6.0f * s), kAccentCol, eq.c_str());
        {
            const float bw = 96.0f * s, bh = 28.0f * s;
            const ImVec2 b0(k1.x - pad - bw, y), b1(b0.x + bw, y + bh);
            ImGui::SetCursorScreenPos(b0);
            const bool clicked = ImGui::InvisibleButton("##random", ImVec2(bw, bh));
            const bool hov = ImGui::IsItemHovered();
            dl->AddRectFilled(b0, b1, hov ? IM_COL32(60, 74, 88, 240) : IM_COL32(26, 34, 44, 230), 2.0f * s);
            dl->AddRect(b0, b1, IM_COL32(170, 220, 255, hov ? 160 : 70), 2.0f * s);
            TextAt(dl, smallFont, 14.0f * s, ImVec2(b0.x + (bw - TextW(smallFont, 14.0f * s, "RANDOM")) * 0.5f, b0.y + 6.0f * s), kWhiteCol, "RANDOM");
            if (clicked && n > 1) {
                static std::mt19937 rng{ std::random_device{}() };
                std::vector<int> open;   // unlocked options other than the equipped one
                for (int i = 0; i < n; ++i) if (i != equipped && !row.options[i].locked && !row.options[i].forSale) open.push_back(i);
                if (!open.empty())
                    choose(m_focusedRow, open[std::uniform_int_distribution<size_t>(0, open.size() - 1)(rng)]);
                m_pickerScrollToEquipped = true;
            }
        }
        y += 40.0f * s;

        // Grid of options (colours as swatches, 10 hues x 3 shades like Reach).
        const float infoH = 120.0f * s;
        const float gridH = std::max(80.0f * s, k1.y - y - infoH - pad);
        ImGui::SetCursorScreenPos(ImVec2(pickX + pad, y));
        ImGui::BeginChild("##armory_picker", ImVec2(pickW - pad * 2.0f, gridH), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground);
        ImDrawList* gdl = ImGui::GetWindowDrawList();
        const float gw = ImGui::GetContentRegionAvail().x;
        const bool colourGrid = row.kind == RowKind::Colour;
        const bool isNameplate = std::strcmp(row.label, "Nameplate") == 0;
        const bool isEmblem = std::strcmp(row.label, "Emblem") == 0;
        const int cols = colourGrid ? 10 : isEmblem ? 4 : isNameplate ? 1 : 2;
        const float cg = 6.0f * s;
        const float tileW = (gw - cg * (cols - 1)) / cols;
        // Picture tiles (Elite armour sets) when at least the equipped option's picture is there.
        const char* eqArt = row.options[equipped].art;
        auto artDir = [](const PickOption& o) { return o.artDir ? o.artDir : "elite_sets"; };
        const bool artGrid = !colourGrid && !isNameplate && !isEmblem && eqArt && UiArt::ReachArt(artDir(row.options[equipped]), eqArt);
        const float tileH = colourGrid ? tileW : isEmblem ? tileW : isNameplate ? 56.0f * s : artGrid ? tileW * 1.15f : 46.0f * s;
        // Grid origin in screen space, taken once (it already includes the scroll offset).
        const ImVec2 base = ImGui::GetCursorScreenPos();
        // Tiles you can use come first, then ones for sale, then locked ones (each group keeps the list order).
        std::vector<int> tileOf(static_cast<size_t>(n));
        {
            int next = 0;
            for (int group = 0; group < 3; ++group)
                for (int i = 0; i < n; ++i) {
                    const PickOption& o = row.options[i];
                    const int g = o.locked ? 2 : o.forSale ? 1 : 0;
                    if (g == group) tileOf[static_cast<size_t>(i)] = next++;
                }
        }
        for (int i = 0; i < n; ++i) {
            // Colours: column = hue, row = shade (the palette is stored hue by hue, 3 shades each).
            const int slot = colourGrid ? (i % 3) * 10 + i / 3 : tileOf[static_cast<size_t>(i)];
            const int c = slot % cols, r = slot / cols;
            ImGui::SetCursorScreenPos(ImVec2(base.x + c * (tileW + cg), base.y + r * (tileH + cg)));
            const ImVec2 t0 = ImGui::GetCursorScreenPos(), t1(t0.x + tileW, t0.y + tileH);
            ImGui::PushID(i);
            const bool clicked = ImGui::InvisibleButton("##opt", ImVec2(tileW, tileH));
            const bool hov = ImGui::IsItemHovered();
            ImGui::PopID();
            if (hov) hoveredOption = i;
            if (clicked) choose(m_focusedRow, i);
            const bool isEq = i == equipped;
            const bool isTry = s_buyRow == m_focusedRow && s_buyValue == i;   // tried on, not equipped
            if (isEq && m_pickerScrollToEquipped) {
                ImGui::SetScrollHereY(0.4f);
                m_pickerScrollToEquipped = false;
            }
            if (colourGrid) {
                gdl->AddRectFilled(t0, t1, row.options[i].swatch, 2.0f * s);
                gdl->AddRect(t0, t1, isEq ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 255, 255, hov ? 160 : 40), 2.0f * s, 0, isEq ? 2.5f * s : 1.0f);
            } else if (isNameplate) {
                gdl->AddRectFilled(t0, t1, isEq ? IM_COL32(34, 70, 104, 235) : (hov ? IM_COL32(40, 54, 70, 235) : IM_COL32(20, 28, 38, 220)), 2.0f * s);
                const auto& catalog = UiArt::GetNameplateCatalog();
                if (static_cast<size_t>(i) < catalog.size()) {
                    const UiTexture npTex = UiArt::Nameplate(catalog[i].id);
                    if (npTex) {
                        UiArt::DrawContain(gdl, npTex, ImVec2(t0.x + 4.0f * s, t0.y + 4.0f * s), ImVec2(t1.x - 4.0f * s, t1.y - 4.0f * s),
                                           (isEq || hov) ? IM_COL32_WHITE : IM_COL32(220, 220, 220, 255));
                    }
                }
                gdl->AddRect(t0, t1, isEq ? kAccentCol : isTry ? IM_COL32(255, 196, 92, 255) : IM_COL32(170, 220, 255, hov ? 140 : 40),
                             2.0f * s, 0, (isEq || isTry) ? 1.6f * s : 1.0f);
            } else if (isEmblem) {
                gdl->AddRectFilled(t0, t1, isEq ? IM_COL32(34, 70, 104, 235) : (hov ? IM_COL32(40, 54, 70, 235) : IM_COL32(20, 28, 38, 220)), 2.0f * s);
                const auto& catalog = UiArt::GetEmblemCatalog();
                if (static_cast<size_t>(i) < catalog.size()) {
                    const UiTexture embTex = UiArt::Emblem(catalog[i].id);
                    if (embTex) {
                        UiArt::DrawContain(gdl, embTex, ImVec2(t0.x + 4.0f * s, t0.y + 4.0f * s), ImVec2(t1.x - 4.0f * s, t1.y - 4.0f * s),
                                           (isEq || hov) ? IM_COL32_WHITE : IM_COL32(220, 220, 220, 255));
                    }
                }
                gdl->AddRect(t0, t1, isEq ? kAccentCol : isTry ? IM_COL32(255, 196, 92, 255) : IM_COL32(170, 220, 255, hov ? 140 : 40),
                             2.0f * s, 0, (isEq || isTry) ? 1.6f * s : 1.0f);
            } else if (artGrid) {
                gdl->AddRectFilled(t0, t1, isEq ? IM_COL32(34, 70, 104, 235) : (hov ? IM_COL32(40, 54, 70, 235) : IM_COL32(20, 28, 38, 220)), 2.0f * s);
                const float nameH = 30.0f * s;
                if (row.options[i].art) {
                    const UiTexture pic = UiArt::ReachArt(artDir(row.options[i]), row.options[i].art);
                    if (pic) UiArt::DrawContain(gdl, pic, ImVec2(t0.x + 4.0f * s, t0.y + 4.0f * s), ImVec2(t1.x - 4.0f * s, t1.y - nameH),
                                                row.options[i].locked ? IM_COL32(70, 70, 70, 255)
                                                : (isEq || hov) ? IM_COL32_WHITE : IM_COL32(205, 205, 205, 255));
                }
                gdl->AddRectFilled(ImVec2(t0.x, t1.y - nameH), t1, IM_COL32(8, 12, 18, 200), 2.0f * s);
                const std::string& name = row.options[i].name;
                TextAt(gdl, bodyFont, 16.0f * s, ImVec2(t0.x + (tileW - TextW(bodyFont, 16.0f * s, name.c_str())) * 0.5f, t1.y - nameH + 6.0f * s),
                       row.options[i].locked ? IM_COL32(120, 130, 140, 255) : isEq ? kWhiteCol : IM_COL32(200, 214, 228, 255), name.c_str());
                if (row.options[i].forSale) {   // price in the picture's corner
                    const std::string price = WithCommas(row.options[i].price) + " cR";
                    const float pw = TextW(smallFont, 13.0f * s, price.c_str()) + 10.0f * s;
                    gdl->AddRectFilled(ImVec2(t0.x + 4.0f * s, t0.y + 4.0f * s), ImVec2(t0.x + 4.0f * s + pw, t0.y + 22.0f * s), IM_COL32(8, 12, 18, 210), 2.0f * s);
                    TextAt(gdl, smallFont, 13.0f * s, ImVec2(t0.x + 9.0f * s, t0.y + 6.0f * s), IM_COL32(255, 196, 92, 255), price.c_str());
                }
                if (row.options[i].locked) {   // "LOCKED / <rank>" over the darkened picture
                    const std::string rank = row.options[i].lockRank ? row.options[i].lockRank : "";
                    const float my = t0.y + (tileH - nameH) * 0.5f;
                    TextAt(gdl, smallFont, 14.0f * s, ImVec2(t0.x + (tileW - TextW(smallFont, 14.0f * s, "LOCKED")) * 0.5f, my - 16.0f * s),
                           IM_COL32(255, 196, 92, 255), "LOCKED");
                    TextAt(gdl, smallFont, 13.0f * s, ImVec2(t0.x + (tileW - TextW(smallFont, 13.0f * s, rank.c_str())) * 0.5f, my + 2.0f * s),
                           IM_COL32(230, 230, 230, 255), rank.c_str());
                }
                gdl->AddRect(t0, t1, isEq ? kAccentCol : isTry ? IM_COL32(255, 196, 92, 255) : IM_COL32(170, 220, 255, hov ? 140 : 40),
                             2.0f * s, 0, (isEq || isTry) ? 1.6f * s : 1.0f);
            } else {
                gdl->AddRectFilled(t0, t1, isEq ? IM_COL32(34, 70, 104, 235) : (hov ? IM_COL32(40, 54, 70, 235) : IM_COL32(20, 28, 38, 220)), 2.0f * s);
                gdl->AddRect(t0, t1, isEq ? kAccentCol : isTry ? IM_COL32(255, 196, 92, 255) : IM_COL32(170, 220, 255, hov ? 140 : 40),
                             2.0f * s, 0, (isEq || isTry) ? 1.6f * s : 1.0f);
                float tx = t0.x + 10.0f * s;
                if (row.options[i].swatch) {
                    gdl->AddRectFilled(ImVec2(tx, t0.y + 12.0f * s), ImVec2(tx + 18.0f * s, t1.y - 12.0f * s), row.options[i].swatch, 2.0f * s);
                    tx += 26.0f * s;
                }
                std::string name = row.options[i].name;
                const bool locked = row.options[i].locked || row.options[i].forSale;
                const float maxW = t1.x - tx - 26.0f * s;
                if (locked) {   // name on top, "LOCKED - <rank>" or "<price> cR" under it
                    while (name.size() > 3 && TextW(bodyFont, 15.0f * s, name.c_str()) > maxW) name = name.substr(0, name.size() - 4) + "...";
                    TextAt(gdl, bodyFont, 15.0f * s, ImVec2(tx, t0.y + 5.0f * s), IM_COL32(120, 130, 140, 255), name.c_str());
                    const std::string lock = row.options[i].forSale ? WithCommas(row.options[i].price) + " cR"
                        : std::string("LOCKED - ") + (row.options[i].lockRank ? row.options[i].lockRank : "");
                    TextAt(gdl, smallFont, 12.0f * s, ImVec2(tx, t1.y - 18.0f * s), IM_COL32(255, 196, 92, 230), lock.c_str());
                } else {
                    while (name.size() > 3 && TextW(bodyFont, 16.0f * s, name.c_str()) > maxW) name = name.substr(0, name.size() - 4) + "...";
                    TextAt(gdl, bodyFont, 16.0f * s, ImVec2(tx, t0.y + tileH * 0.5f - 9.0f * s), isEq ? kWhiteCol : IM_COL32(200, 214, 228, 255), name.c_str());
                }
            }
            if (isEq) {   // equipped check in the corner
                const ImVec2 cc(t1.x - 11.0f * s, t0.y + 11.0f * s);
                gdl->AddCircleFilled(cc, 8.0f * s, kAccentCol);
                gdl->AddLine(ImVec2(cc.x - 4.0f * s, cc.y), ImVec2(cc.x - 1.0f * s, cc.y + 3.0f * s), IM_COL32(10, 20, 30, 255), 2.0f * s);
                gdl->AddLine(ImVec2(cc.x - 1.0f * s, cc.y + 3.0f * s), ImVec2(cc.x + 4.0f * s, cc.y - 3.0f * s), IM_COL32(10, 20, 30, 255), 2.0f * s);
            }
        }
        const int lastSlot = colourGrid ? 29 : n - 1;   // tileOf covers 0..n-1
        ImGui::SetCursorScreenPos(ImVec2(base.x, base.y + (lastSlot / cols + 1) * (tileH + cg)));
        ImGui::Dummy(ImVec2(1, 1));
        ImGui::EndChild();
        m_pickerScrollToEquipped = false;

        // Name and description of the hovered (or equipped, or about-to-be-bought) option.
        const bool pending = s_buyRow == m_focusedRow && s_buyValue >= 0 && s_buyValue < n;
        const int shown = hoveredOption >= 0 ? hoveredOption : pending ? s_buyValue : equipped;
        const float iy = k1.y - infoH;
        dl->AddLine(ImVec2(pickX + pad, iy), ImVec2(k1.x - pad, iy), IM_COL32(255, 255, 255, 40));
        TextAt(dl, titleFont, 20.0f * s, ImVec2(pickX + pad, iy + 10.0f * s), kWhiteCol, row.options[shown].name.c_str());
        const char* desc = row.options[shown].description.empty() ? (colourGrid ? "One of Reach's 30 armor colors." : "")
                                                                  : row.options[shown].description.c_str();
        TextAt(dl, bodyFont, 15.0f * s, ImVec2(pickX + pad, iy + 38.0f * s), kDimCol, desc, pickW - pad * 2.0f);
        const std::string count = std::to_string(shown + 1) + " / " + std::to_string(n) +
            (shown == equipped ? "   EQUIPPED"
             : row.options[shown].locked ? std::string("   PREVIEW - unlocks at ") + (row.options[shown].lockRank ? row.options[shown].lockRank : "a higher rank")
             : row.options[shown].forSale ? std::string(pending && shown == s_buyValue ? "   PREVIEW - buy to equip" : "   Click to preview")
             : std::string("   Click to equip"));
        TextAt(dl, smallFont, 14.0f * s, ImVec2(pickX + pad, k1.y - 26.0f * s), IM_COL32(130, 150, 170, 255), count.c_str());

        // BUY for an item that was clicked but isn't bought yet. cR balance = earned - spent (rank follows earned).
        if (pending && row.options[s_buyValue].forSale) {
            const PickOption& o = row.options[s_buyValue];
            const int64_t balance = Career::Balance();
            const bool afford = balance >= o.price;
            const std::string label = afford ? "BUY  " + WithCommas(o.price) + " cR" : "NEED " + WithCommas(o.price) + " cR";
            const float bw = std::max(170.0f * s, TextW(smallFont, 15.0f * s, label.c_str()) + 30.0f * s), bh = 30.0f * s;
            const ImVec2 b0(k1.x - pad - bw, k1.y - 36.0f * s), b1(b0.x + bw, b0.y + bh);
            ImGui::SetCursorScreenPos(b0);
            const bool clicked = ImGui::InvisibleButton("##buy", ImVec2(bw, bh));
            const bool hov = ImGui::IsItemHovered();
            dl->AddRectFilled(b0, b1, !afford ? IM_COL32(40, 40, 44, 230) : hov ? IM_COL32(150, 110, 40, 245) : IM_COL32(110, 80, 30, 235), 2.0f * s);
            dl->AddRect(b0, b1, afford ? IM_COL32(255, 196, 92, hov ? 255 : 170) : IM_COL32(120, 120, 120, 120), 2.0f * s);
            TextAt(dl, smallFont, 15.0f * s, ImVec2(b0.x + (bw - TextW(smallFont, 15.0f * s, label.c_str())) * 0.5f, b0.y + 6.0f * s),
                   afford ? kWhiteCol : IM_COL32(150, 150, 150, 255), label.c_str());
            const std::string bal = "Balance: " + WithCommas(balance) + " cR";
            TextAt(dl, smallFont, 13.0f * s, ImVec2(b0.x, b0.y - 18.0f * s), IM_COL32(200, 214, 228, 220), bal.c_str());
            if (clicked && afford && Career::BuyArmoryItem(o.buyKey, o.price)) {
                choose(m_focusedRow, s_buyValue, true);
            }
        }
    }

    // ---- Try-on: the 3D scene shows the clicked piece that can't be equipped yet; the saved look is unchanged.
    {
        static int s_shownRow = -1, s_shownValue = -1;
        int wantRow = -1, wantValue = -1;
        if (s_buyRow >= 0 && s_buyRow < rowCount && rows[s_buyRow].index && s_buyValue >= 0 &&
            s_buyValue < static_cast<int>(rows[s_buyRow].options.size()) && s_buyValue != *rows[s_buyRow].index) {
            wantRow = s_buyRow;
            wantValue = s_buyValue;
        }
        if (wantRow != s_shownRow || wantValue != s_shownValue) {
            s_shownRow = wantRow;
            s_shownValue = wantValue;
            if (wantRow >= 0) {
                int* field = rows[wantRow].index;
                const AppearanceProfile saved = m_profile;
                *field = wantValue;
                if (field == &m_profile.helmetIndex) m_profile.helmetVariantIndex = 0;   // a helmet is tried on with its base
                if (field == &m_profile.eliteArmorSet)   // an Elite set is tried on whole
                    m_profile.eliteHelmetIndex = m_profile.eliteLeftShoulderIndex = m_profile.eliteRightShoulderIndex =
                        m_profile.eliteChestIndex = m_profile.eliteArmsIndex = m_profile.eliteLegsIndex = wantValue;
                SyncToReachArmory();
                m_profile = saved;
            } else {
                SyncToReachArmory();   // back to the equipped look
            }
        }
    }

    // ---- Over the model: camera hint.
    const char* cam = m_cameraMode == 2 ? "CLOSE-UP" : m_cameraMode == 1 ? "FULL BODY" : "ARMORY VIEW";
    const std::string hint = std::string("Camera: ") + cam + "   |   Drag or Q/E to turn, wheel to zoom";
    TextAt(dl, smallFont, 14.0f * s, ImVec2(previewX + 16.0f * s, origin.y + avail.y - 24.0f * s), IM_COL32(200, 214, 228, 220), hint.c_str());
}

} // namespace noble
