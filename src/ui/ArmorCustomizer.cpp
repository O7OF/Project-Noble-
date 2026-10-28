#include "ArmorCustomizer.hpp"
#include "NobleStyle.hpp"
#include "common/Logger.hpp"
#include "engine/ReachArmory.hpp"
#include "engine/ReachHost.hpp"

#include <windows.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <algorithm>
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
            "CQC", "kurt", "-player_kurt",
            {
                {"BASE", "kurt", "Standard-issue close quarters combat variant developed at the UNSC Hoppocken testing facility."},
                {"CBRN", "kurt_a", "Chemical, Biological, Radiological and Nuclear filtration unit for the CQC helmet."},
                {"UA/HUL", "kurt_b", "Up-Armored visor shield with Hardened Uplink module for the CQC helmet."},
            },
            "Standard-issue close quarters combat variant developed at the UNSC Hoppocken testing facility."
        },
        {
            "ODST", "jai", "-player_jai",
            {
                {"BASE", "jai", "Many technologies initially developed for MJOLNIR have gained widespread adoption by the ODST."},
                {"UA/CNM", "jai_a", "Up-Armored brow plate with Command Network Module for the ODST helmet."},
                {"CBRN/HUL", "jai_b", "CBRN rebreather and Hardened Uplink package for the ODST helmet."},
            },
            "Many technologies initially developed for MJOLNIR have gained widespread adoption by the ODST."
        },
        {
            "HAZOP", "isaac", "-player_isaac",
            {
                {"BASE", "isaac", "The Mark V[H] variant is environmentally sealed for prolonged operations in hazardous environments."},
                {"CBRN", "isaac_a", "Twin CBRN filtration canisters for extreme hazard zone operations."},
                {"CNM-I", "isaac_b", "Command Network Module and CBRN filtration suite for the HAZOP helmet."},
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
            "OPERATOR", "nicole", "-player_nicole",
            {
                {"BASE", "nicole", "A multi-role variant optimized for specialized vehicle and espionage teams within Asymmetric Action Groups."},
                {"UA/HUL", "nicole_a", "Up-Armored brow plate and Hardened Uplink module for the Operator helmet."},
                {"CNM", "nicole_b", "Command Network Module and optical rangefinder suite for the Operator helmet."},
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
            "EVA", "kelly", "-player_kelly",
            {
                {"BASE", "kelly", "Created in a zero-g facility above Damascus on Chi Ceti IV, the Mark V[V] was designed to increase exo-atmospheric survivability."},
                {"CNM", "kelly_a", "Command Network Module for the Extra-Vehicular Activity helmet."},
                {"UA/HUL[3]", "kelly_b", "Up-Armored brow plate and triple Hardened Uplink transceiver for the EVA helmet."},
            },
            "Created in a zero-g facility above Damascus on Chi Ceti IV, the Mark V[V] was designed to increase exo-atmospheric survivability."
        },
        {
            "JFO", "adrianna", "-player_adrianna",
            {
                {"BASE", "adrianna", "Designed to coordinate orbital fire-support and close air strikes in contested combat zones."},
                {"HUL-I", "adrianna_a", "Internalized Hardened Uplink module for the Joint Fires Observer helmet."},
                {"UA", "adrianna_b", "Up-Armored blast shield and Hardened Uplink suite for the JFO helmet."},
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
            "MJOLNIR MK. V", "will", "-player_will",
            {
                {"BASE", "will", "Entered service on November 24, 2541; the Mark V was the first generation of MJOLNIR to incorporate energy shielding."},
                {"CNM", "will_a", "Command Network Module for the classic MJOLNIR Mark V helmet."},
                {"UA", "will_b", "Up-Armored brow plate and Command Network Module for the MJOLNIR Mark V helmet."},
            },
            "Entered service on November 24, 2541; the Mark V was the first generation of MJOLNIR to incorporate energy shielding."
        },
        {
            "PILOT", "skull", "-player_skull",
            {
                {"BASE", "skull", "Utilizing the latest in holographic HUD interfaces, the Pilot variant is widespread among UNSC Air Force and Navy crews."},
                {"HUL[3]", "skull_a", "Triple Hardened Uplink module for the Pilot helmet."},
                {"UA/HUL[3]", "skull_b", "Up-Armored blast shield and triple Hardened Uplink module for the Pilot helmet."},
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
            "MILITARY POLICE", "player_li", "-helmet_base__(spartans_player_mp_render)",
            {
                {"BASE", "player_li", "Previously an exclusive asset of Beta-5 security forces, the MP variant has entered service with the Army and Air Force."},
                {"CBRN/HU/RS", "player_li_a", "CBRN rebreather and Hardened Uplink / Remote Sensor package for the Military Police helmet."},
                {"HU/RS/CNM", "player_li_b", "Hardened Uplink, Remote Sensor, and Command Network Module for the Military Police helmet."},
            },
            "Previously an exclusive asset of Beta-5 security forces, the MP variant has entered service with the Army and Air Force."
        },
        {
            "CQB", "rosenda", "-player_rosenda",
            {
                {"BASE", "rosenda", "The Mark V[C] variant improves survivability in close-quarters combat via hardened T-visor plating."},
                {"HU/RS/CNM", "rosenda_a", "Hardened Uplink, Remote Sensor, and Command Network Module for the CQB helmet."},
                {"UA/HUL", "rosenda_b", "Up-Armored brow plate and Hardened Uplink suite for the CQB helmet."},
            },
            "The Mark V[C] variant improves survivability in close-quarters combat via hardened T-visor plating."
        },
        {
            "RECON", "malcolm", "-player_malcolm",
            {
                {"BASE", "malcolm", "An ONI developmental prototype designed to decrease noticeability and silhouette in deep-reconnaissance operations."},
                {"HUL", "malcolm_a", "Hardened Uplink transceiver mounted to the ONI Recon helmet."},
                {"UA/HUL[3]", "malcolm_b", "Up-Armored blast shield and triple Hardened Uplink array for the Recon helmet."},
            },
            "An ONI developmental prototype designed to decrease noticeability and silhouette in deep-reconnaissance operations."
        },
        {
            "MJOLNIR MK. VI", "john117", "-player_john117",
            {
                {"BASE", "john117", "Field-tested in the final months of 2552 on Reach prior to full deployment across remaining SPARTAN-II personnel."},
                {"FC-I[2]", "john117_a", "Dual internalized Fascia Command sensors for the prototype Mark VI helmet."},
                {"UA/HUL-I", "john117_b", "Up-Armored brow plate and internalized Hardened Uplink for the prototype Mark VI helmet."},
            },
            "Field-tested in the final months of 2552 on Reach prior to full deployment across remaining SPARTAN-II personnel."
        },
        {
            "GUNGNIR", "sam", "-player_sam",
            {
                {"BASE", "sam", "Created as part of Project GUNGNIR to pair with the M6 Grindell/Galilean Nonlinear Rifle."},
                {"HU/RS", "sam_a", "Hardened Uplink and Remote Sensor module for the enclosed GUNGNIR helmet."},
                {"CBRN", "sam_b", "CBRN filtration and Remote Sensor suite for the GUNGNIR helmet."},
            },
            "Created as part of Project GUNGNIR to pair with the M6 Grindell/Galilean Nonlinear Rifle."
        },
        {
            "AKIS / GRD", "hilands", "-dlc2_helmet_hilands",
            {
                {"BASE", "hilands", "High-altitude reconnaissance and assassination prototype developed at the UNSC Highland Mountains facility."},
                {"CBRN", "hilands_a", "CBRN filtration module for the AKIS prototype helmet."},
                {"GRD", "hilands_b", "Full optical rangefinder and CBRN suite for the AKIS / GRD helmet."},
            },
            "High-altitude reconnaissance and assassination prototype developed at the UNSC Highland Mountains facility."
        },
        {
            "MARINER", "cndmnd", "-dlc2_helmet_cndmnd",
            {
                {"BASE", "cndmnd", "Deep-space orbital salvage and boarding operations helmet developed near the Condemned orbital station."},
                {"OPTREX", "cndmnd_a", "External Optrex sensor array for the Mariner orbital helmet."},
                {"DEFIANT", "cndmnd_b", "Defiant-pattern reinforced faceplate for the Mariner orbital helmet."},
                {"MISTER CHIEF", "cndmnd_c", "Custom hand-drawn commemorative faceplate on the Mariner orbital dome."},
            },
            "Deep-space orbital salvage and boarding operations helmet developed near the Condemned orbital station."
        },
        {
            "BELOS", "s8_helmet_belos_cerberus", "-s8_helmet_belos_cerberus",
            {
                {"CERBERUS", "s8_helmet_belos_cerberus", "Mythic Hellas-inspired Corinthian combat helm."},
                {"TRANSVERSE", "s8_helmet_belos_transverse", "Belos helm crested with a transverse horsehair plume."},
                {"STHENO", "s8_helmet_belos_stheno", "Gorgon-visaged Belos variant."},
                {"KERATA", "s8_helmet_belos_kerata", "Horned Belos assault helm."},
                {"CORINTHIAN", "s8_helmet_belos_corinthian", "High-crested Corinthian Belos helm."},
                {"LEONTOKARDOS", "s8_helmet_belos_leontokardos", "Lion-hearted Belos champion helm."},
            },
            "Mythic Hellas-inspired MJOLNIR combat helmet from MCC Season 8."
        },
        {
            "BLACKGUARD", "s8_helmet_blackguard_base", "-s8_helmet_blckgrd_base",
            {
                {"BASE", "s8_helmet_blackguard_base", "Knightly great-helm forged for frontline siege warfare."},
                {"CROWN", "s8_helmet_blackguard_crown", "Crowned Blackguard command helm."},
                {"VISAGE", "s8_helmet_blackguard_visage", "Visored Blackguard dread helm."},
                {"BISHOP", "s8_helmet_blackguard_bishop", "Mitred Blackguard inquisitor helm."},
                {"DRAGON", "s8_helmet_blackguard_dragon", "Wyrm-crested Blackguard helm."},
                {"WIND", "s8_helmet_blackguard_wind", "Winged Blackguard tournament helm."},
            },
            "Medieval siege-plate inspired MJOLNIR combat helmet from MCC Season 8."
        },
        {
            "DRENGR", "s8_helmet_drengr_base", "-s8_helmet_drengr_base",
            {
                {"BASE", "s8_helmet_drengr_base", "Norse spectacle-guard war helm built for shock-drop boarding actions."},
                {"TYR", "s8_helmet_drengr_tyr", "Tyr-pattern Drengr war helm."},
                {"NARI", "s8_helmet_drengr_nari", "Nari-pattern Drengr war helm."},
                {"THYRA", "s8_helmet_drengr_thyra", "Thyra-pattern Drengr war helm."},
                {"DVALINN", "s8_helmet_drengr_dvalinn", "Dvalinn-pattern Drengr war helm."},
                {"VALDEMAR", "s8_helmet_drengr_valdemar", "Valdemar-pattern Drengr war helm."},
            },
            "Norse-inspired MJOLNIR shock-assault helmet from MCC Season 8."
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
        {"UA/COUNTERASSAULT [R]", "chest_2_roboarm", "-flair_chest_perm1_default__", true,
         "UA/Counterassault chest armor paired with a cybernetic right arm."},
        {"TACTICAL/RECON [R]", "chest_5_roboarm", "-flair_chest__(spartans_player_isaac_render)", true,
         "Tactical/Recon chest webbing paired with a cybernetic right arm."},
        {"TACTICAL/PATROL [R]", "chest_7_roboarm", "-flair_chest__(spartans_jun_render)", true,
         "Tactical/Patrol sniper harness paired with a cybernetic right arm."},
        {"COLLAR/GRENADIER [UA] [R]", "chest_12_roboarm", "-flair_joshua_torso", true,
         "Collar/Grenadier [UA] heavy chest plate paired with a cybernetic right arm."},
        {"CONDEMNED", "dlc2_chest_cndmnd", "-dlc2_chest_cndmnd", false,
         "Heavy orbital salvage chest rig from the Condemned development series."},
        {"HIGHLANDS", "dlc2_chest_hilands", "-dlc2_chest_hilands", false,
         "Highland Mountains training facility composite chest harness."},
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
        {"TACTICAL/UGPS", "wrist_4", "-flair_wrist__(spartans_player_kelly_render)", false,
         "Universal Global Positioning System wrist terminal for orbital and terrestrial navigation."},
        {"ASSAULT/BREACHER", "wrist_5", "-flair_shotgun_reloader__(spartans_player_rosenda_render)", false,
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
    {"FLAMES",            "flames",            "Legendary solar-plasma flames wreathe the operator's helmet."},
    {"BLUE FLAMES",       "blue_flames",       "Bungie Eternal cobalt-plasma flames wreathe the operator's helmet."},
}};

constexpr std::array<SimpleOption, 16> kFirefightVoices = {{
    {"NOBLE SIX",      "noble_six", "Default Spartan-B312 combat vocalizations for Firefight."},
    {"CORTANA",        "cortana",   "CTN 0452-9 AI construct combat callouts for Firefight."},
    {"JOHN-117",       "chief",     "Master Chief Petty Officer John-117 combat callouts for Firefight."},
    {"SGTMAJ JOHNSON", "johnson",   "Sergeant Major Avery Junior Johnson combat callouts for Firefight."},
    {"GY. SGT BUCK",   "buck",      "Gunnery Sergeant Edward Buck ODST combat callouts for Firefight."},
    {"CAPT. DARE",     "dare",      "ONI Captain Veronica Dare combat callouts for Firefight."},
    {"CPL. DUTCH",     "dutch",     "Corporal Taylor 'Dutch' Miles ODST combat callouts for Firefight."},
    {"PFC. MICKEY",    "mickey",    "Private First Class Michael 'Mickey' Crespo ODST callouts."},
    {"LCPL. ROMEO",    "romeo",     "Lance Corporal Kojo 'Romeo' Agu ODST combat callouts."},
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
constexpr std::array<EliteSetSpec, 8> kEliteSets = {{
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
    {"FIELD MARSHAL", "zealot_commander",
     "Supreme tactical command harness worn by Sangheili Field Marshals leading Covenant armies."},
    {"OFFICER", "officer",
     "Distinguished Sangheili Officer harness featuring sweeping cranial fins and reinforced pauldrons."},
}};

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

void ArmorCustomizer::UpdateArmoryCamera() const {
    if (!ReachArmory::IsMenuMode()) return;

    if (m_previewSpecies == PlayerSpecies::Elite) {
        ReachArmory::SetTargetScene("armory_elite");
        return;
    }

    // Spartan camera scene selection in mainmenu_all.map:
    //   m_cameraMode == 0: "armory" (Reach's native Armory 3/4 view positioned on the right half of the screen)
    //   m_cameraMode == 1: "c_full_spartan" (Full-body Spartan view)
    //   m_cameraMode == 2: Slot-specific close-up director scenes ("c_helmet", "c_chest", etc.)
    if (m_cameraMode == 1) {
        ReachArmory::SetTargetScene("c_full_spartan");
        return;
    }
    if (m_cameraMode == 2) {
        // Row 0 is Preferred Species; Spartan armor rows start at 1
        switch (m_focusedRow) {
        case 1:
        case 2:
        case 8: // Helmet, Helmet Variant, Visor Color
            ReachArmory::SetTargetScene("c_helmet");
            return;
        case 3: // Left Shoulder
            ReachArmory::SetTargetScene("c_left_shoulder");
            return;
        case 4: // Right Shoulder
            ReachArmory::SetTargetScene("c_right_shoulder");
            return;
        case 5: // Chest
            ReachArmory::SetTargetScene("c_chest");
            return;
        case 6: // Wrist
            ReachArmory::SetTargetScene("c_wrist");
            return;
        case 7: // Utility
            ReachArmory::SetTargetScene("c_utility");
            return;
        case 9: // Knee Guards
            ReachArmory::SetTargetScene("c_knees");
            return;
        default:
            ReachArmory::SetTargetScene("c_full_spartan");
            return;
        }
    }

    ReachArmory::SetTargetScene("armory");
}

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

    sel.primaryRgb = { SrgbToLinear(pCol.r), SrgbToLinear(pCol.g), SrgbToLinear(pCol.b) };
    sel.secondaryRgb = { SrgbToLinear(sCol.r), SrgbToLinear(sCol.g), SrgbToLinear(sCol.b) };
    sel.visorRgb = { SrgbToLinear(vCol.r), SrgbToLinear(vCol.g), SrgbToLinear(vCol.b) };

    ReachArmory::SetSelection(sel);
    UpdateArmoryCamera();
}

bool ArmorCustomizer::LoadProfile(const std::filesystem::path& path) {
    m_profilePath = path.empty() ? ResolveDefaultProfilePath() : path;
    if (!std::filesystem::exists(m_profilePath)) {
        SyncLegacyFields();
        m_previewSpecies = m_profile.species;
        SyncToReachArmory();
        return false;
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
            const int maxEl = static_cast<int>(kEliteSets.size()) - 1;
            if (el.contains("helmet_index") && el["helmet_index"].is_number_integer())
                m_profile.eliteHelmetIndex = std::clamp(el["helmet_index"].get<int>(), 0, maxEl);
            if (el.contains("left_shoulder_index") && el["left_shoulder_index"].is_number_integer())
                m_profile.eliteLeftShoulderIndex = std::clamp(el["left_shoulder_index"].get<int>(), 0, maxEl);
            if (el.contains("right_shoulder_index") && el["right_shoulder_index"].is_number_integer())
                m_profile.eliteRightShoulderIndex = std::clamp(el["right_shoulder_index"].get<int>(), 0, maxEl);
            if (el.contains("chest_index") && el["chest_index"].is_number_integer())
                m_profile.eliteChestIndex = std::clamp(el["chest_index"].get<int>(), 0, maxEl);
            if (el.contains("arms_index") && el["arms_index"].is_number_integer())
                m_profile.eliteArmsIndex = std::clamp(el["arms_index"].get<int>(), 0, maxEl);
            if (el.contains("legs_index") && el["legs_index"].is_number_integer())
                m_profile.eliteLegsIndex = std::clamp(el["legs_index"].get<int>(), 0, maxEl);
        }

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
            {"legs_index", m_profile.eliteLegsIndex}
        };

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

void ArmorCustomizer::RenderCharacterPreview(ID3D11Device* device, ID3D11DeviceContext* context,
                                             float width, float height, bool fullBleed) {
    (void)device;
    (void)context;
    (void)fullBleed;

    // The live Halo: Reach Blam! engine (`haloreach.dll`) renders the 3D Armory Spartan / Elite
    // scene (`mainmenu_all.map`) directly into the swap chain backbuffer behind this transparent viewport.
    const ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    if (!ReachArmory::IsSceneReady()) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const char* msg = "LOADING HALO: REACH ARMORY SCENE...";
        const ImVec2 ts = ImGui::CalcTextSize(msg);
        dl->AddText(ImVec2(cursorPos.x + (width - ts.x) * 0.5f, cursorPos.y + height * 0.45f),
                    IM_COL32(143, 216, 255, 220), msg);
    }

    if (m_autoRotate) {
        ReachArmory::AddYawRotation(ImGui::GetIO().DeltaTime * 0.45f);
    }

    ImGui::InvisibleButton("##character_preview_drag", ImVec2(width, height));
    if (ImGui::IsItemActive() &&
        (ImGui::IsMouseDragging(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Right))) {
        ReachArmory::AddYawRotation(-ImGui::GetIO().MouseDelta.x * 0.0085f);
    }
    if (ImGui::IsItemHovered()) {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (std::abs(wheel) > 0.01f) {
            if (wheel > 0.0f) {
                m_cameraMode = 2; // Zoom to currently focused armor slot camera
            } else {
                m_cameraMode = (m_cameraMode == 2) ? 0 : 1; // Zoom out to Armory or Full Body camera
            }
            UpdateArmoryCamera();
        }
    }
}

void ArmorCustomizer::Render(ID3D11Device* device, ID3D11DeviceContext* context) {
    SyncLegacyFields();
    if (!m_syncedInitial) {
        m_syncedInitial = true;
        SyncToReachArmory();
    } else {
        UpdateArmoryCamera();
    }

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float s = std::max(0.75f, avail.y / 640.0f);

    // Keyboard / Gamepad rotation shortcuts (Q/E or Right Stick)
    if (!ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyDown(ImGuiKey_Q) || ImGui::IsKeyDown(ImGuiKey_GamepadRStickLeft)) {
            ReachArmory::AddYawRotation(ImGui::GetIO().DeltaTime * 1.8f);
        }
        if (ImGui::IsKeyDown(ImGuiKey_E) || ImGui::IsKeyDown(ImGuiKey_GamepadRStickRight)) {
            ReachArmory::AddYawRotation(-ImGui::GetIO().DeltaTime * 1.8f);
        }
    }

    // -------------------------------------------------------------------------
    // 1. RIGHT SIDE: LIVE IN-ENGINE 3D CHARACTER VIEWPORT (haloreach.dll)
    // -------------------------------------------------------------------------
    const float leftColWidth = std::clamp(avail.x * 0.41f, 360.0f * s, 520.0f * s);
    const float previewX = origin.x + leftColWidth - 20.0f * s;
    const float previewW = avail.x - (leftColWidth - 20.0f * s);
    const float previewH = avail.y;

    ImGui::SetCursorScreenPos(ImVec2(previewX, origin.y));
    RenderCharacterPreview(device, context, previewW, previewH, true);

    // -------------------------------------------------------------------------
    // 2. TOP SUB-HEADER BAR: SPARTAN/ELITE TABS + PREFERRED SPECIES BUTTON + BODY/CAMERA
    // -------------------------------------------------------------------------
    const float barY = origin.y + 4.0f * s;
    float pillX = origin.x;

    auto drawModePill = [&](const char* label, bool active, float w, bool highlightGold = false) -> bool {
        const ImVec2 p0(pillX, barY);
        const ImVec2 p1(pillX + w, barY + 28.0f * s);
        ImGui::SetCursorScreenPos(p0);
        const bool clicked = ImGui::InvisibleButton(label, ImVec2(w, 28.0f * s));
        const bool hovered = ImGui::IsItemHovered();

        const ImU32 bgCol = highlightGold
                          ? (hovered ? IM_COL32(112, 82, 22, 230) : IM_COL32(78, 56, 16, 205))
                          : active ? IM_COL32(34, 88, 136, 210)
                          : hovered ? IM_COL32(28, 52, 82, 180)
                                    : IM_COL32(14, 24, 38, 150);
        dl->AddRectFilled(p0, p1, bgCol, 2.0f);
        if (highlightGold) {
            dl->AddLine(ImVec2(p0.x, p1.y - 2.0f), ImVec2(p1.x, p1.y - 2.0f), IM_COL32(255, 205, 85, 255), 2.5f);
        } else if (active) {
            dl->AddLine(ImVec2(p0.x, p1.y - 2.0f), ImVec2(p1.x, p1.y - 2.0f), IM_COL32(95, 205, 255, 255), 2.5f);
        }
        const ImVec2 ts = ImGui::CalcTextSize(label);
        dl->AddText(ImVec2(p0.x + (w - ts.x) * 0.5f, p0.y + (28.0f * s - ts.y) * 0.5f),
                    (active || highlightGold) ? IM_COL32(255, 255, 255, 255) : IM_COL32(175, 195, 215, 220), label);
        pillX += w + 6.0f * s;
        return clicked;
    };

    if (drawModePill("SPARTAN", m_previewSpecies == PlayerSpecies::Spartan, 90.0f * s)) {
        m_previewSpecies = PlayerSpecies::Spartan;
        m_profile.species = PlayerSpecies::Spartan;
        m_focusedRow = 0;
        m_dirty = true;
        SaveProfile(m_profilePath);
    }
    if (drawModePill("ELITE", m_previewSpecies == PlayerSpecies::Elite, 78.0f * s)) {
        m_previewSpecies = PlayerSpecies::Elite;
        m_profile.species = PlayerSpecies::Elite;
        m_focusedRow = 0;
        m_dirty = true;
        SaveProfile(m_profilePath);
    }

    const char* prefLabel = (m_profile.species == PlayerSpecies::Elite)
                          ? "PREFERRED SPECIES: ELITE"
                          : "PREFERRED SPECIES: SPARTAN";
    if (drawModePill(prefLabel, true, 210.0f * s, true)) {
        m_profile.species = (m_profile.species == PlayerSpecies::Spartan)
                          ? PlayerSpecies::Elite
                          : PlayerSpecies::Spartan;
        m_previewSpecies = m_profile.species;
        m_dirty = true;
        SaveProfile(m_profilePath);
    }

    if (m_previewSpecies == PlayerSpecies::Spartan) {
        if (drawModePill(m_profile.bodyType == BodyType::Male ? "BODY: LIGHT/MALE" : "BODY: HEAVY/FEMALE", false, 142.0f * s)) {
            m_profile.bodyType = (m_profile.bodyType == BodyType::Male) ? BodyType::Female : BodyType::Male;
            m_dirty = true;
            SaveProfile(m_profilePath);
        }
        const char* camLabel = (m_cameraMode == 0) ? "CAM: ARMORY"
                             : (m_cameraMode == 1) ? "CAM: FULL BODY"
                                                   : "CAM: SLOT ZOOM";
        if (drawModePill(camLabel, m_cameraMode == 2, 122.0f * s)) {
            m_cameraMode = (m_cameraMode + 1) % 3;
            UpdateArmoryCamera();
        }
    }

    // -------------------------------------------------------------------------
    // 3. LEFT COLUMN: MCC-STYLE TRANSLUCENT CUSTOMIZATION TABLE ROWS
    // -------------------------------------------------------------------------
    int preferredSpeciesInt = static_cast<int>(m_profile.species);

    struct RowDescriptor {
        const char* category;
        std::string currentValue;
        const char* itemTitle;
        const char* itemDesc;
        int* indexPtr;
        int count;
        bool isColorRow = false;
        bool isServiceTagRow = false;
        bool isPreferredSpeciesRow = false;
    };

    const char* prefValueStr = (m_profile.species == PlayerSpecies::Elite) ? "ELITE" : "SPARTAN";
    const char* prefDescStr  = (m_profile.species == PlayerSpecies::Elite)
        ? "Selects Sangheili (Elite) as your preferred playable species for Multiplayer, Forge, and Custom Games."
        : "Selects UNSC SPARTAN-III as your preferred playable species for Multiplayer, Forge, and Custom Games.";

    std::vector<RowDescriptor> rows;
    rows.push_back({"Preferred Species", prefValueStr, prefValueStr, prefDescStr, &preferredSpeciesInt, 2, false, false, true});

    if (m_previewSpecies == PlayerSpecies::Spartan) {
        const auto& helmets = GetSpartanHelmets();
        const auto& shoulders = GetSpartanShoulders();
        const auto& chests = GetSpartanChests();
        const auto& wrists = GetSpartanWrists();
        const auto& utilities = GetSpartanUtilities();
        const auto& knees = GetSpartanKneeGuards();

        const auto& h = helmets[m_profile.helmetIndex];
        const auto& hv = h.variants[m_profile.helmetVariantIndex];
        const auto& ls = shoulders[m_profile.leftShoulderIndex];
        const auto& rs = shoulders[m_profile.rightShoulderIndex];
        const auto& ch = chests[m_profile.chestIndex];
        const auto& wr = wrists[m_profile.wristIndex];
        const auto& ut = utilities[m_profile.utilityIndex];
        const auto& vc = kVisorColors[m_profile.visorColorIndex];
        const auto& kn = knees[m_profile.kneeGuardsIndex];
        const auto& ae = kArmorEffects[m_profile.armorEffectIndex];
        const auto& fv = kFirefightVoices[m_profile.firefightVoiceIndex];

        rows.push_back({"Helmet",          h.name,  h.name,  h.description,  &m_profile.helmetIndex,         static_cast<int>(helmets.size())});
        rows.push_back({"Helmet Variant",  hv.name, hv.name, hv.description, &m_profile.helmetVariantIndex,  static_cast<int>(h.variants.size())});
        rows.push_back({"Left Shoulder",   ls.name, ls.name, ls.description, &m_profile.leftShoulderIndex,   static_cast<int>(shoulders.size())});
        rows.push_back({"Right Shoulder",  rs.name, rs.name, rs.description, &m_profile.rightShoulderIndex,  static_cast<int>(shoulders.size())});
        rows.push_back({"Chest",           ch.name, ch.name, ch.description, &m_profile.chestIndex,          static_cast<int>(chests.size())});
        rows.push_back({"Wrist",           wr.name, wr.name, wr.description, &m_profile.wristIndex,          static_cast<int>(wrists.size())});
        rows.push_back({"Utility",         ut.name, ut.name, ut.description, &m_profile.utilityIndex,        static_cast<int>(utilities.size())});
        rows.push_back({"Visor Color",     vc.name, vc.name, vc.description, &m_profile.visorColorIndex,     static_cast<int>(kVisorColors.size())});
        rows.push_back({"Knee Guards",     kn.name, kn.name, kn.description, &m_profile.kneeGuardsIndex,     static_cast<int>(knees.size())});
        rows.push_back({"Armor Effect",    ae.name, ae.name, ae.description, &m_profile.armorEffectIndex,    static_cast<int>(kArmorEffects.size())});
        rows.push_back({"Firefight Voice", fv.name, fv.name, fv.description, &m_profile.firefightVoiceIndex, static_cast<int>(kFirefightVoices.size())});
        rows.push_back({"CHANGE COLORS",   "",      "ARMOR COLORS", "Customize primary and secondary MJOLNIR armor plate coloration.", nullptr, 0, true, false});
        rows.push_back({"SERVICE TAG",     m_profile.serviceTag, "SERVICE TAG", "Four-character alphanumeric UNSC identification tag.", nullptr, 0, false, true});
    } else {
        const int maxEl = static_cast<int>(kEliteSets.size());
        const auto& preset = kEliteSets[std::clamp(m_profile.eliteArmorSet, 0, maxEl - 1)];
        const auto& ae = kArmorEffects[m_profile.armorEffectIndex];
        const auto& fv = kFirefightVoices[m_profile.firefightVoiceIndex];

        rows.push_back({"Armor Set",       preset.name, preset.name, preset.description, &m_profile.eliteArmorSet,          maxEl});
        rows.push_back({"Armor Effect",    ae.name,     ae.name,     ae.description,     &m_profile.armorEffectIndex,       static_cast<int>(kArmorEffects.size())});
        rows.push_back({"Firefight Voice", fv.name,     fv.name,     fv.description,     &m_profile.firefightVoiceIndex,    static_cast<int>(kFirefightVoices.size())});
        rows.push_back({"CHANGE COLORS",   "",          "ELITE HARNESS COLORS", "Customize primary and secondary Sangheili combat harness coloration.", nullptr, 0, true, false});
        rows.push_back({"SERVICE TAG",     m_profile.serviceTag, "SERVICE TAG", "Four-character alphanumeric identification tag.", nullptr, 0, false, true});
    }

    const int rowCount = static_cast<int>(rows.size());
    m_focusedRow = std::clamp(m_focusedRow, 0, rowCount - 1);

    // Keyboard/Gamepad Up/Down/Left/Right navigation when modal isn't open
    if (!m_showColorModal && !ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown)) {
            m_focusedRow = (m_focusedRow + 1) % rowCount;
            UpdateArmoryCamera();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp)) {
            m_focusedRow = (m_focusedRow - 1 + rowCount) % rowCount;
            UpdateArmoryCamera();
        }
    }

    auto applyRowDelta = [&](int rowIndex, int delta) {
        RowDescriptor& r = rows[rowIndex];
        if (!r.indexPtr || r.count <= 0) return;
        *r.indexPtr = (*r.indexPtr + delta + r.count) % r.count;
        if (r.isPreferredSpeciesRow) {
            m_profile.species = static_cast<PlayerSpecies>(std::clamp(preferredSpeciesInt, 0, 1));
            m_previewSpecies = m_profile.species;
        } else if (m_previewSpecies == PlayerSpecies::Spartan && rowIndex == 1) {
            // Reset helmet variant to BASE when switching base helmet
            m_profile.helmetVariantIndex = 0;
        } else if (m_previewSpecies == PlayerSpecies::Elite && rowIndex == 1) {
            // Changing Elite Armor Set updates all Elite armor slots
            m_profile.eliteHelmetIndex = m_profile.eliteArmorSet;
            m_profile.eliteLeftShoulderIndex = m_profile.eliteArmorSet;
            m_profile.eliteRightShoulderIndex = m_profile.eliteArmorSet;
            m_profile.eliteChestIndex = m_profile.eliteArmorSet;
            m_profile.eliteArmsIndex = m_profile.eliteArmorSet;
            m_profile.eliteLegsIndex = m_profile.eliteArmorSet;
        }
        m_dirty = true;
        SaveProfile(m_profilePath);
    };

    if (!m_showColorModal && !ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft)) {
            applyRowDelta(m_focusedRow, -1);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight)) {
            applyRowDelta(m_focusedRow, +1);
        }
        if ((ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown)) && rows[m_focusedRow].isColorRow) {
            m_showColorModal = true;
        }
    }

    const float tableTop = origin.y + 42.0f * s;
    const float rowH = std::min(34.0f * s, (avail.y - 54.0f * s) / static_cast<float>(rowCount));
    const float catWidth = leftColWidth * 0.43f;
    const float valWidth = leftColWidth - catWidth;

    for (int i = 0; i < rowCount; ++i) {
        RowDescriptor& r = rows[i];
        const float y0 = tableTop + i * rowH;
        const float y1 = y0 + rowH - 1.0f;
        const ImVec2 rowMin(origin.x, y0);
        const ImVec2 rowMax(origin.x + leftColWidth, y1);
        const ImVec2 valMin(origin.x + catWidth, y0);
        const ImVec2 valMax(origin.x + leftColWidth, y1);

        ImGui::PushID(i);
        ImGui::SetCursorScreenPos(rowMin);
        const bool rowClicked = ImGui::InvisibleButton("##row_hit", ImVec2(leftColWidth, rowH - 1.0f));
        const bool rowHovered = ImGui::IsItemHovered();
        if (rowHovered && (ImGui::GetIO().MouseDelta.x != 0.0f || ImGui::GetIO().MouseDelta.y != 0.0f)) {
            if (m_focusedRow != i) {
                m_focusedRow = i;
                UpdateArmoryCamera();
            }
        }
        const bool selected = (m_focusedRow == i);

        // MCC translucent row background + active cyan gradient + left cyan vertical bar
        if (selected) {
            dl->AddRectFilledMultiColor(
                rowMin, rowMax,
                IM_COL32(28, 78, 128, 210),
                IM_COL32(18, 48, 84, 150),
                IM_COL32(18, 48, 84, 150),
                IM_COL32(28, 78, 128, 210)
            );
            dl->AddRectFilled(ImVec2(rowMin.x, rowMin.y), ImVec2(rowMin.x + 4.0f * s, rowMax.y), IM_COL32(95, 215, 255, 255));
        } else {
            dl->AddRectFilled(rowMin, rowMax, IM_COL32(10, 18, 28, 130));
            dl->AddRectFilled(valMin, valMax, IM_COL32(14, 24, 38, 155));
        }
        // Subtle 1px horizontal divider
        dl->AddLine(ImVec2(rowMin.x, rowMax.y), ImVec2(rowMax.x, rowMax.y), IM_COL32(120, 165, 210, 38));
        dl->AddLine(ImVec2(valMin.x, rowMin.y), ImVec2(valMin.x, rowMax.y), IM_COL32(120, 165, 210, 28));

        // Category label on the left
        const ImVec2 catTextSize = ImGui::CalcTextSize(r.category);
        dl->AddText(
            ImVec2(rowMin.x + 14.0f * s, y0 + (rowH - catTextSize.y) * 0.5f),
            selected ? IM_COL32(240, 248, 255, 255) : IM_COL32(168, 188, 210, 225),
            r.category
        );

        if (r.isColorRow) {
            const auto& pCol = kReachColors[std::clamp(m_profile.primaryColor, 0, 29)];
            const auto& sCol = kReachColors[std::clamp(m_profile.secondaryColor, 0, 29)];
            const float swatchSz = rowH * 0.64f;
            const float sy = y0 + (rowH - swatchSz) * 0.5f;
            const float sx1 = valMin.x + valWidth * 0.5f - swatchSz - 4.0f * s;
            const float sx2 = valMin.x + valWidth * 0.5f + 4.0f * s;

            dl->AddRectFilled(ImVec2(sx1, sy), ImVec2(sx1 + swatchSz, sy + swatchSz),
                              ImGui::ColorConvertFloat4ToU32(ImVec4(pCol.r, pCol.g, pCol.b, 1.0f)));
            dl->AddRect(ImVec2(sx1, sy), ImVec2(sx1 + swatchSz, sy + swatchSz), IM_COL32(230, 240, 255, 210), 0.0f, 0, 1.5f);

            dl->AddRectFilled(ImVec2(sx2, sy), ImVec2(sx2 + swatchSz, sy + swatchSz),
                              ImGui::ColorConvertFloat4ToU32(ImVec4(sCol.r, sCol.g, sCol.b, 1.0f)));
            dl->AddRect(ImVec2(sx2, sy), ImVec2(sx2 + swatchSz, sy + swatchSz), IM_COL32(230, 240, 255, 210), 0.0f, 0, 1.5f);

            if (rowClicked) {
                m_showColorModal = !m_showColorModal;
            }
        } else if (r.isServiceTagRow) {
            ImGui::SetCursorScreenPos(ImVec2(valMin.x + valWidth * 0.25f, y0 + (rowH - 22.0f * s) * 0.5f));
            ImGui::SetNextItemWidth(valWidth * 0.50f);
            char tagBuf[8] = {};
            std::snprintf(tagBuf, sizeof(tagBuf), "%s", m_profile.serviceTag.c_str());
            if (ImGui::InputText("##service_tag_in", tagBuf, 5, ImGuiInputTextFlags_CharsUppercase)) {
                m_profile.serviceTag = tagBuf;
                m_dirty = true;
                SaveProfile(m_profilePath);
            }
        } else {
            // Centered uppercase value + < > arrows when selected
            const ImVec2 valTextSize = ImGui::CalcTextSize(r.currentValue.c_str());
            dl->AddText(
                ImVec2(valMin.x + (valWidth - valTextSize.x) * 0.5f, y0 + (rowH - valTextSize.y) * 0.5f),
                selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(165, 185, 205, 215),
                r.currentValue.c_str()
            );

            if (selected) {
                const float cy = y0 + rowH * 0.5f;
                const float lx = valMin.x + 16.0f * s;
                const float rx = valMax.x - 16.0f * s;
                const float chSz = 5.5f * s;
                dl->AddLine(ImVec2(lx + chSz, cy - chSz), ImVec2(lx, cy), IM_COL32(235, 245, 255, 245), 1.8f);
                dl->AddLine(ImVec2(lx, cy), ImVec2(lx + chSz, cy + chSz), IM_COL32(235, 245, 255, 245), 1.8f);
                dl->AddLine(ImVec2(rx - chSz, cy - chSz), ImVec2(rx, cy), IM_COL32(235, 245, 255, 245), 1.8f);
                dl->AddLine(ImVec2(rx, cy), ImVec2(rx - chSz, cy + chSz), IM_COL32(235, 245, 255, 245), 1.8f);
            }

            if (rowClicked) {
                m_focusedRow = i;
                UpdateArmoryCamera();
                const float mouseX = ImGui::GetIO().MousePos.x;
                if (mouseX >= valMin.x && mouseX < valMin.x + valWidth * 0.35f) {
                    applyRowDelta(i, -1);
                } else if (mouseX >= valMin.x + valWidth * 0.35f) {
                    applyRowDelta(i, +1);
                }
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                m_focusedRow = i;
                UpdateArmoryCamera();
                applyRowDelta(i, -1);
            }
        }
        ImGui::PopID();
    }

    // -------------------------------------------------------------------------
    // 4. BOTTOM-RIGHT ITEM TITLE & LORE DESCRIPTION OVERLAY (MCC STYLE)
    // -------------------------------------------------------------------------
    const RowDescriptor& activeRow = rows[m_focusedRow];
    const float infoX = origin.x + leftColWidth + 46.0f * s;
    const float infoY = origin.y + avail.y - 112.0f * s;
    const float infoMaxW = std::max(260.0f * s, origin.x + avail.x - infoX - 24.0f * s);

    // Subtle dark gradient backing behind bottom-right text for readability over legs
    dl->AddRectFilledMultiColor(
        ImVec2(infoX - 18.0f * s, infoY - 12.0f * s),
        ImVec2(origin.x + avail.x, origin.y + avail.y),
        IM_COL32(6, 10, 16, 0),
        IM_COL32(6, 10, 16, 0),
        IM_COL32(6, 10, 16, 210),
        IM_COL32(6, 10, 16, 195)
    );

    ImFont* heroFont = NobleStyle::HeroFont() ? NobleStyle::HeroFont() : ImGui::GetFont();
    ImFont* bodyFont = ImGui::GetFont();
    dl->AddText(heroFont, 32.0f * s, ImVec2(infoX + 2.0f, infoY + 2.0f), IM_COL32(0, 0, 0, 200), activeRow.itemTitle);
    dl->AddText(heroFont, 32.0f * s, ImVec2(infoX, infoY), IM_COL32(245, 250, 255, 255), activeRow.itemTitle);

    dl->AddText(bodyFont, 16.0f * s, ImVec2(infoX, infoY + 38.0f * s), IM_COL32(100, 185, 235, 245),
                activeRow.itemDesc, nullptr, infoMaxW);

    char subInfo[128];
    if (activeRow.indexPtr && activeRow.count > 0) {
        std::snprintf(subInfo, sizeof(subInfo), "[UNLOCKED] %s (%d / %d)  |  Drag character or press Q/E to rotate, Scroll to zoom",
                      activeRow.category, (*activeRow.indexPtr) + 1, activeRow.count);
    } else {
        std::snprintf(subInfo, sizeof(subInfo), "Click row to configure  |  Drag character or press Q/E to rotate, Scroll to zoom");
    }
    dl->AddText(bodyFont, 14.5f * s, ImVec2(infoX, infoY + 78.0f * s), IM_COL32(215, 225, 238, 220), subInfo);

    // -------------------------------------------------------------------------
    // 5. CHANGE COLORS POPUP PANEL (30 AUTHENTIC REACH COLORS)
    // -------------------------------------------------------------------------
    if (m_showColorModal) {
        const float modalW = 420.0f * s;
        const float modalH = 310.0f * s;
        const ImVec2 modalPos(origin.x + leftColWidth + 24.0f * s, origin.y + 42.0f * s);
        ImGui::SetNextWindowPos(modalPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(modalW, modalH), ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.07f, 0.12f, 0.96f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.32f, 0.72f, 0.96f, 0.85f));
        if (ImGui::Begin("CHANGE COLORS##ReachColorModal", &m_showColorModal,
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
            auto renderColorGrid = [&](const char* title, int& colorIdx) {
                const auto& cur = kReachColors[std::clamp(colorIdx, 0, 29)];
                ImGui::TextColored(ImVec4(0.65f, 0.85f, 1.0f, 1.0f), "%s: %s", title, cur.name);
                ImGui::PushID(title);
                const float sw = 28.0f * s;
                for (int c = 0; c < static_cast<int>(kReachColors.size()); ++c) {
                    if (c % 10 != 0) ImGui::SameLine(0.0f, 5.0f * s);
                    ImGui::PushID(c);
                    const auto& col = kReachColors[c];
                    const bool picked = (colorIdx == c);
                    if (picked) {
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.5f);
                    }
                    if (ImGui::ColorButton("##c", ImVec4(col.r, col.g, col.b, 1.0f),
                                           ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, ImVec2(sw, sw))) {
                        colorIdx = c;
                        m_dirty = true;
                        SaveProfile(m_profilePath);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("%s", col.name);
                    }
                    if (picked) {
                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor();
                    }
                    ImGui::PopID();
                }
                ImGui::PopID();
            };

            renderColorGrid("PRIMARY COLOR", m_profile.primaryColor);
            ImGui::Dummy(ImVec2(0.0f, 8.0f * s));
            renderColorGrid("SECONDARY COLOR", m_profile.secondaryColor);
            ImGui::Dummy(ImVec2(0.0f, 8.0f * s));
            if (ImGui::Button("CLOSE", ImVec2(110.0f * s, 28.0f * s)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                m_showColorModal = false;
            }
        }
        ImGui::End();
        ImGui::PopStyleColor(2);
    }
}

} // namespace noble
