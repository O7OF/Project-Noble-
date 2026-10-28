#include "ArmorCustomizer.hpp"
#include "NobleStyle.hpp"
#include "common/Logger.hpp"

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
    std::vector<ObjPreviewLayer> extraLayers;
    bool replaceBaseHelmet = false;
    const char* description = "";
};

struct HelmetSpec {
    const char* name;
    const char* stringId;
    ObjPreviewLayer baseLayer;
    std::vector<HelmetVariantSpec> variants;
    const char* description;
};

const std::vector<HelmetSpec>& GetSpartanHelmets() {
    static const std::vector<HelmetSpec> kHelmets = {
        {
            "MARK V [B]", "default",
            {"-default", {"spartan_helmet_default", "spartan_visor_default", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "default", {}, false,
                 "The Mark V[B] was entering service in standard issue across the UNSC just prior to the fall of Reach."},
                {"UA", "default_b", {{"-flair_helmet_perm1_default__", {}, {}}}, false,
                 "Up-Armored brow plate module for the standard Mark V [B] combat helmet."},
                {"UA/HUL", "default_c", {{"-flair_helmet_perm1_default__", {}, {}}, {"-flair_helmet_perm2_default__", {}, {}}}, false,
                 "Up-Armored brow plate with external Hardened Uplink module for the Mark V [B]."},
            },
            "The Mark V[B] was entering service in standard issue across the UNSC just prior to the fall of Reach."
        },
        {
            "CQC", "cqc",
            {"-player_kurt", {"spartan_helmet_kurt", "spartan_visor_kurt", "spartan_visor_default", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "cqc", {}, false,
                 "Standard-issue close quarters combat variant developed at the UNSC Hoppocken testing facility."},
                {"CBRN", "cqc_b", {{"-flair_helmet_perm1__(spartans_player_kurt_render)", {}, {}}}, false,
                 "Chemical, Biological, Radiological and Nuclear filtration unit for the CQC helmet."},
                {"UA/HUL", "cqc_c", {{"-flair_helmet_perm2__(spartans_player_kurt_render)", {}, {}}}, false,
                 "Up-Armored visor shield with Hardened Uplink module for the CQC helmet."},
            },
            "Standard-issue close quarters combat variant developed at the UNSC Hoppocken testing facility."
        },
        {
            "ODST", "odst",
            {"-player_jai", {"spartan_helmet_jai", "spartan_helmet_jai_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "odst", {}, false,
                 "Many technologies initially developed for MJOLNIR have gained widespread adoption by the ODST."},
                {"UA/CNM", "odst_b", {{"-flair_helmet_perm1__(spartans_player_jai_render)", {}, {}}}, false,
                 "Up-Armored brow plate with Command Network Module for the ODST helmet."},
                {"CBRN/HUL", "odst_c", {{"-flair_helmet_perm1__(spartans_player_jai_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_jai_render)", {}, {}}}, false,
                 "CBRN rebreather and Hardened Uplink package for the ODST helmet."},
            },
            "Many technologies initially developed for MJOLNIR have gained widespread adoption by the ODST."
        },
        {
            "HAZOP", "hazop",
            {"-player_isaac", {"spartan_helmet_isaac", "spartan_visor_isaac", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "hazop", {}, false,
                 "The Mark V[H] variant is environmentally sealed for prolonged operations in hazardous environments."},
                {"CBRN", "hazop_b", {{"-flair_helmet_perm1__(spartans_player_isaac_render)", {}, {}}}, false,
                 "Twin CBRN filtration canisters for extreme hazard zone operations."},
                {"CNM-I", "hazop_c", {{"-flair_helmet_perm1__(spartans_player_isaac_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_isaac_render)", {}, {}}}, false,
                 "Command Network Module and CBRN filtration suite for the HAZOP helmet."},
            },
            "The Mark V[H] variant is environmentally sealed for prolonged operations in hazardous environments."
        },
        {
            "EOD", "eod",
            {"-player_vinh", {"spartan_helmet_vinh", "spartan_visor_vinh", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "eod", {}, false,
                 "Designed specifically to channel the pressure wave of a detonation around the user's head."},
                {"CNM", "eod_b", {{"-flair_helmet_perm1__(spartans_player_vinh_render)", {}, {}}}, false,
                 "Command Network Module fitted to the Explosive Ordnance Disposal helmet."},
                {"UA/HUL", "eod_c", {{"-flair_helmet_perm2__(spartans_player_vinh_render)", {}, {}}}, false,
                 "Up-Armored blast shield with Hardened Uplink module for the EOD helmet."},
            },
            "Designed specifically to channel the pressure wave of a detonation around the user's head."
        },
        {
            "OPERATOR", "operator",
            {"-player_nicole", {"spartan_helmet_nicole", "spartan_helmet_visor_nicole", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "operator", {}, false,
                 "A multi-role variant optimized for specialized vehicle and espionage teams within Asymmetric Action Groups."},
                {"UA/HUL", "operator_b", {{"-flair_helmet_perm1__(spartans_player_nicole_render)", {}, {}}}, false,
                 "Up-Armored brow plate and Hardened Uplink module for the Operator helmet."},
                {"CNM", "operator_c", {{"-flair_helmet_perm1__(spartans_player_nicole_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_nicole_render)", {}, {}}}, false,
                 "Command Network Module and optical rangefinder suite for the Operator helmet."},
            },
            "A multi-role variant optimized for specialized vehicle and espionage teams within Asymmetric Action Groups."
        },
        {
            "GRENADIER", "grenadier",
            {"-player_joshua", {"spartan_jorge_helmet", "spartan_joshua_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "grenadier", {}, false,
                 "Tested early in the program's history, the Grenadier firmware was the basis for the Mark V's shield emitters."},
                {"UA", "grenadier_b", {{"-flair_helmet_perm1__(spartans_player_joshua_render)", {}, {}}}, false,
                 "Up-Armored brow reinforcement for the heavy Grenadier helmet."},
                {"UA/FC", "grenadier_c", {{"-flair_helmet_perm1__(spartans_player_joshua_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_joshua_render)", {}, {}}}, false,
                 "Up-Armored brow plate with external Fascia Command module for the Grenadier helmet."},
            },
            "Tested early in the program's history, the Grenadier firmware was the basis for the Mark V's shield emitters."
        },
        {
            "AIR ASSAULT", "air_assault",
            {"-female", {"spartan_helmet_kat_base", "spartan_helmet_kat_rubber", "spartan_helmet_kat_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "air_assault", {}, false,
                 "Adopted by Airborne units as an improvement on the venerable ODST helmet architecture."},
                {"UA/CNM", "air_assault_b", {{"-flair_helmet_kat_perm1", {}, {}}}, false,
                 "Up-Armored brow plate with Command Network Module for the Air Assault helmet."},
                {"FC-I", "air_assault_c", {{"-flair_helmet_kat_perm1", {}, {}}, {"-flair_helmet_kat_perm2", {}, {}}}, false,
                 "Fascia Command and optical uplink attachment for the Air Assault helmet."},
            },
            "Adopted by Airborne units as an improvement on the venerable ODST helmet architecture."
        },
        {
            "SCOUT", "scout",
            {"-jun", {"spartan_helmet_jun", "spartan_helmet_rubberr_jun", "spartan_visor_jun", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "scout", {}, false,
                 "Sharing many components with the Recon variant, the Scout relies more heavily on stealth coatings."},
                {"HU/RS", "scout_b", {{"-flair_helmet_perm1__(spartans_jun_render)", {}, {}}}, false,
                 "Hardened Uplink and Remote Sensor optics for long-range reconnaissance."},
                {"CBRN/CNM", "scout_c", {{"-flair_helmet_perm1__(spartans_jun_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_jun_render)", {}, {}}}, false,
                 "CBRN filtration and Command Network Module for the Scout helmet."},
            },
            "Sharing many components with the Recon variant, the Scout relies more heavily on stealth coatings."
        },
        {
            "EVA", "eva",
            {"-player_kelly", {"spartan_helmet_kelly", "spartan_visor_kelly", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "eva", {}, false,
                 "Created in a zero-g facility above Damascus on Chi Ceti IV, the Mark V[V] was designed to increase exo-atmospheric survivability."},
                {"CNM", "eva_b", {{"-flair_helmet_perm1__(spartans_player_kelly_render)", {}, {}}}, false,
                 "Command Network Module for the Extra-Vehicular Activity helmet."},
                {"UA/HUL[3]", "eva_c", {{"-flair_helmet_perm1__(spartans_player_kelly_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_kelly_render)", {}, {}}}, false,
                 "Up-Armored brow plate and triple Hardened Uplink transceiver for the EVA helmet."},
            },
            "Created in a zero-g facility above Damascus on Chi Ceti IV, the Mark V[V] was designed to increase exo-atmospheric survivability."
        },
        {
            "JFO", "jfo",
            {"-player_adrianna", {"spartan_helmet_adrianna", "spartan_helmet_adrianna_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "jfo", {}, false,
                 "Designed to coordinate orbital fire-support and close air strikes in contested combat zones."},
                {"HUL-I", "jfo_b", {{"-flair_helmet_perm1__(spartans_player_adrianna_render)", {}, {}}}, false,
                 "Internalized Hardened Uplink module for the Joint Fires Observer helmet."},
                {"UA", "jfo_c", {{"-flair_helmet_perm1__(spartans_player_adrianna_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_adrianna_render)", {}, {}}}, false,
                 "Up-Armored blast shield and Hardened Uplink suite for the JFO helmet."},
            },
            "Designed to coordinate orbital fire-support and close air strikes in contested combat zones."
        },
        {
            "COMMANDO", "commando",
            {"-carter", {"spartan_helmet_carter", "spartan_visor_carter", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "commando", {}, false,
                 "Featuring an integrated command and control suite, the Commando variant is favored by Spartan team leaders."},
                {"CBRN/CNM", "commando_b", {{"-flair_helmet_perm1__(spartans_carter_render)", {}, {}}}, false,
                 "CBRN filtration and Command Network Module for the Commando helmet."},
                {"UA/FC-I[2]", "commando_c", {{"-flair_helmet_perm2__(spartans_carter_render)", {}, {}}}, false,
                 "Up-Armored visor sunshield and dual Fascia Command sensors (Noble One configuration)."},
            },
            "Featuring an integrated command and control suite, the Commando variant is favored by Spartan team leaders."
        },
        {
            "EVA [C]", "eva_emile",
            {"-emile", {"spartan_helmet_emile", "spartan_helmet_skull_emile", "spartan_visor_emile", "spartan_visor_li", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "eva_emile", {}, false,
                 "Field-modified EVA variant featuring a hand-carved ballistic polarization dome (Noble Four configuration)."},
                {"CNM", "eva_emile_b", {{"-flair_helmet_perm1_emile__", {}, {}}}, false,
                 "Command Network Module mounted to the EVA [C] helmet."},
                {"UA/HUL[3]", "eva_emile_c", {{"-flair_helmet_perm1_emile__", {}, {}}, {"-flair_helmet_perm2_emile__", {}, {}}}, false,
                 "Up-Armored brow plate and triple Hardened Uplink module on the EVA [C] helmet."},
            },
            "Field-modified EVA variant featuring a hand-carved ballistic polarization dome (Noble Four configuration)."
        },
        {
            "MJOLNIR MK. V", "mark_v",
            {"-player_will", {"spartan_helmet_will", "spartan_helmet_will_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "mark_v", {}, false,
                 "Entered service on November 24, 2541; the Mark V was the first generation of MJOLNIR to incorporate energy shielding."},
                {"CNM", "mark_v_b", {{"-flair_helmet_perm1__(spartans_player_will_render)", {}, {}}}, false,
                 "Command Network Module for the classic MJOLNIR Mark V helmet."},
                {"UA", "mark_v_c", {{"-flair_helmet_perm1__(spartans_player_will_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_will_render)", {}, {}}}, false,
                 "Up-Armored brow plate and Command Network Module for the MJOLNIR Mark V helmet."},
            },
            "Entered service on November 24, 2541; the Mark V was the first generation of MJOLNIR to incorporate energy shielding."
        },
        {
            "PILOT", "pilot",
            {"-player_skull", {"spartan_helmet_skull", "spartan_helmet_skull_armor", "spartan_helmet_skull_rubber", "spartan_helmet_skull_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "pilot", {}, false,
                 "Utilizing the latest in holographic HUD interfaces, the Pilot variant is widespread among UNSC Air Force and Navy crews."},
                {"HUL[3]", "pilot_b", {{"-flair_helmet_perm1__(spartans_player_skull_render)", {}, {}}}, false,
                 "Triple Hardened Uplink module for the Pilot helmet."},
                {"UA/HUL[3]", "pilot_c", {{"-flair_helmet_perm1__(spartans_player_skull_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_skull_render)", {}, {}}}, false,
                 "Up-Armored blast shield and triple Hardened Uplink module for the Pilot helmet."},
                {"HAUNTED", "haunted", {{"-bungie", {}, {}}, {"-flair_helmet_perm2__(spartans_player_skull_render)", {}, {}}}, true,
                 "And when he had opened the fourth seal, I heard the voice of the fourth beast say, Come and see."},
            },
            "Utilizing the latest in holographic HUD interfaces, the Pilot variant is widespread among UNSC Air Force and Navy crews."
        },
        {
            "SECURITY", "security",
            {"-player_linda", {"spartan_helmet_linda", "spartan_helmet_linda_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "security", {}, false,
                 "Manufactured by Misriah Armory on Mars, the Mark V[M] Security variant was designed for shipboard defense."},
                {"UA/HUL", "security_b", {{"-flair_helmet_perm1__(spartans_player_linda_render)", {}, {}}}, false,
                 "Up-Armored brow plate with Hardened Uplink antenna for the Security helmet."},
                {"CBRN/CNM", "security_c", {{"-flair_helmet_perm1__(spartans_player_linda_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_linda_render)", {}, {}}}, false,
                 "CBRN filtration and Command Network Module for the Security helmet."},
            },
            "Manufactured by Misriah Armory on Mars, the Mark V[M] Security variant was designed for shipboard defense."
        },
        {
            "MILITARY POLICE", "mp",
            {"-helmet_base__(spartans_player_mp_render)", {}, {}},
            {
                {"BASE", "mp", {}, false,
                 "Previously an exclusive asset of Beta-5 security forces, the MP variant has entered service with the Army and Air Force."},
                {"CBRN/HU/RS", "mp_b", {{"-flair_helmet_perm1__(spartans_player_mp_render)", {}, {}}}, false,
                 "CBRN rebreather and Hardened Uplink / Remote Sensor package for the Military Police helmet."},
                {"HU/RS/CNM", "mp_c", {{"-flair_helmet_perm1__(spartans_player_mp_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_mp_render)", {}, {}}}, false,
                 "Hardened Uplink, Remote Sensor, and Command Network Module for the Military Police helmet."},
            },
            "Previously an exclusive asset of Beta-5 security forces, the MP variant has entered service with the Army and Air Force."
        },
        {
            "CQB", "cqb",
            {"-player_rosenda", {"spartan_helmet_rosenda", "spartan_visor_rosenda", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "cqb", {}, false,
                 "The Mark V[C] variant improves survivability in close-quarters combat via hardened T-visor plating."},
                {"HU/RS/CNM", "cqb_b", {{"-flair_helmet_perm1__(spartans_player_rosenda_render)", {}, {}}}, false,
                 "Hardened Uplink, Remote Sensor, and Command Network Module for the CQB helmet."},
                {"UA/HUL", "cqb_c", {{"-flair_helmet_perm1__(spartans_player_rosenda_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_rosenda_render)", {}, {}}}, false,
                 "Up-Armored brow plate and Hardened Uplink suite for the CQB helmet."},
            },
            "The Mark V[C] variant improves survivability in close-quarters combat via hardened T-visor plating."
        },
        {
            "RECON", "recon",
            {"-player_malcolm", {"spartan_helmet_malcolm", "spartan_helmet_malcolm_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "recon", {}, false,
                 "An ONI developmental prototype designed to decrease noticeability and silhouette in deep-reconnaissance operations."},
                {"HUL", "recon_b", {{"-flair_helmet_perm1__(spartans_player_malcolm_render)", {}, {}}}, false,
                 "Hardened Uplink transceiver mounted to the ONI Recon helmet."},
                {"UA/HUL[3]", "recon_c", {{"-flair_helmet_perm1__(spartans_player_malcolm_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_malcolm_render)", {}, {}}}, false,
                 "Up-Armored blast shield and triple Hardened Uplink array for the Recon helmet."},
            },
            "An ONI developmental prototype designed to decrease noticeability and silhouette in deep-reconnaissance operations."
        },
        {
            "MJOLNIR MK. VI", "mark_vi",
            {"-player_john117", {"spartan_helmet_john117", "spartan_helmet_john117_visor", "spartan_visor_default", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "mark_vi", {}, false,
                 "Field-tested in the final months of 2552 on Reach prior to full deployment across remaining SPARTAN-II personnel."},
                {"FC-I[2]", "mark_vi_b", {{"-flair_helmet_perm1__(spartans_player_john117_render)", {}, {}}}, false,
                 "Dual internalized Fascia Command sensors for the prototype Mark VI helmet."},
                {"UA/HUL-I", "mark_vi_c", {{"-flair_helmet_perm1__(spartans_player_john117_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_john117_render)", {}, {}}}, false,
                 "Up-Armored brow plate and internalized Hardened Uplink for the prototype Mark VI helmet."},
            },
            "Field-tested in the final months of 2552 on Reach prior to full deployment across remaining SPARTAN-II personnel."
        },
        {
            "GUNGNIR", "gungnir",
            {"-player_sam", {"spartan_helmet_sam", "spartan_helmet_sam_visor", "spartan_helmet_interior", "spartan_helmet_interior_visor"}, {}},
            {
                {"BASE", "gungnir", {}, false,
                 "Created as part of Project GUNGNIR to pair with the M6 Grindell/Galilean Nonlinear Rifle."},
                {"HU/RS", "gungnir_b", {{"-flair_helmet_perm1__(spartans_player_sam_render)", {}, {}}}, false,
                 "Hardened Uplink and Remote Sensor module for the enclosed GUNGNIR helmet."},
                {"CBRN", "gungnir_c", {{"-flair_helmet_perm1__(spartans_player_sam_render)", {}, {}}, {"-flair_helmet_perm2__(spartans_player_sam_render)", {}, {}}}, false,
                 "CBRN filtration and Remote Sensor suite for the GUNGNIR helmet."},
            },
            "Created as part of Project GUNGNIR to pair with the M6 Grindell/Galilean Nonlinear Rifle."
        },
        {
            "AKIS / GRD", "grd",
            {"-dlc2_helmet_hilands", {}, {}},
            {
                {"BASE", "grd_base", {}, false,
                 "High-altitude reconnaissance and assassination prototype developed at the UNSC Highland Mountains facility."},
                {"CBRN", "grd_cbrn", {{"-flair_dlc2_helmet_hilands_perm1", {}, {}}}, false,
                 "CBRN filtration module for the AKIS prototype helmet."},
                {"GRD", "grd_full", {{"-flair_dlc2_helmet_hilands_perm1", {}, {}}, {"-flair_dlc2_helmet_hilands_perm2", {}, {}}}, false,
                 "Full optical rangefinder and CBRN suite for the AKIS / GRD helmet."},
            },
            "High-altitude reconnaissance and assassination prototype developed at the UNSC Highland Mountains facility."
        },
        {
            "MARINER", "mariner",
            {"-dlc2_helmet_cndmnd", {}, {}},
            {
                {"BASE", "mariner", {}, false,
                 "Deep-space orbital salvage and boarding operations helmet developed near the Condemned orbital station."},
                {"OPTREX", "mariner_optrex", {{"-flair_dlc2_helmet_cndmnd_optrx", {}, {}}}, false,
                 "External Optrex sensor array for the Mariner orbital helmet."},
                {"DEFIANT", "mariner_defiant", {{"-dlc2_helmet_cndmnd_dfnt", {}, {}}, {"-flair_dlc2_helmet_cndmnd_dfnt", {}, {}}}, true,
                 "Defiant-pattern reinforced faceplate for the Mariner orbital helmet."},
                {"MISTER CHIEF", "mariner_mister_chief", {{"-dlc2_helmet_cndmnd_mstr", {}, {}}, {"-flair_dlc2_helmet_cndmnd_mstr", {}, {}}}, true,
                 "Custom hand-drawn commemorative faceplate on the Mariner orbital dome."},
            },
            "Deep-space orbital salvage and boarding operations helmet developed near the Condemned orbital station."
        },
        {
            "BELOS", "belos",
            {"-s8_helmet_belos_cerberus", {}, {}},
            {
                {"CERBERUS", "belos_cerberus", {}, false, "Mythic Hellas-inspired Corinthian combat helm."},
                {"TRANSVERSE", "belos_transverse", {{"-s8_helmet_belos_cerberus", {}, {}}, {"-flair_s8_helmet_belos_transverse", {}, {}}}, true, "Belos helm crested with a transverse horsehair plume."},
                {"STHENO", "belos_stheno", {{"-s8_helmet_belos_stheno", {}, {}}, {"-flair_s8_helmet_belos_stheno", {}, {}}}, true, "Gorgon-visaged Belos variant."},
                {"KERATA", "belos_kerata", {{"-s8_helmet_belos_kerata", {}, {}}, {"-flair_s8_helmet_belos_kerata", {}, {}}}, true, "Horned Belos assault helm."},
                {"CORINTHIAN", "belos_corinthian", {{"-s8_helmet_belos_corinthian", {}, {}}, {"-flair_s8_helmet_belos_corinthian", {}, {}}}, true, "High-crested Corinthian Belos helm."},
                {"LEONTOKARDOS", "belos_leontokardos", {{"-s8_helmet_belos_leontokardos", {}, {}}, {"-flair_s8_helmet_belos_leontokardos", {}, {}}}, true, "Lion-hearted Belos champion helm."},
            },
            "Mythic Hellas-inspired MJOLNIR combat helmet from MCC Season 8."
        },
        {
            "BLACKGUARD", "blackguard",
            {"-s8_helmet_blckgrd_base", {}, {}},
            {
                {"BASE", "blackguard_base", {}, false, "Knightly great-helm forged for frontline siege warfare."},
                {"CROWN", "blackguard_crown", {{"-s8_helmet_blckgrd_base", {}, {}}, {"-flair_s8_helmet_blckgrd_crown", {}, {}}}, true, "Crowned Blackguard command helm."},
                {"HOOD", "blackguard_hood", {{"-s8_helmet_blckgrd_hood", {}, {}}, {"-flair_s8_helmet_blckgrd_hood", {}, {}}}, true, "Cloth-hooded Blackguard errant helm."},
                {"SKULL", "blackguard_skull", {{"-s8_helmet_blckgrd_skull", {}, {}}, {"-flair_s8_helmet_blckgrd_skull", {}, {}}}, true, "Death's-head Blackguard dread helm."},
                {"BISHOP", "blackguard_bishop", {{"-s8_helmet_blckgrd_bishop", {}, {}}}, true, "Mitred Blackguard inquisitor helm."},
                {"DRAGON", "blackguard_dragon", {{"-s8_helmet_blckgrd_dragon", {}, {}}, {"-flair_s8_helmet_blckgrd_dragon", {}, {}}}, true, "Wyrm-crested Blackguard helm."},
                {"WIND", "blackguard_wind", {{"-s8_helmet_blckgrd_wind", {}, {}}, {"-flair_s8_helmet_blckgrd_wind", {}, {}}}, true, "Winged Blackguard tournament helm."},
            },
            "Medieval siege-plate inspired MJOLNIR combat helmet from MCC Season 8."
        },
        {
            "DRENGR", "drengr",
            {"-s8_helmet_drengr_base", {}, {}},
            {
                {"BASE", "drengr_base", {}, false, "Norse spectacle-guard war helm built for shock-drop boarding actions."},
                {"TYR", "drengr_tyr", {{"-s8_helmet_drengr_tyr", {}, {}}, {"-flair_s8_helmet_drengr_tyr", {}, {}}}, true, "Tyr-pattern Drengr war helm."},
                {"NARI", "drengr_nari", {{"-s8_helmet_drengr_nari", {}, {}}, {"-flair_s8_helmet_drengr_nari", {}, {}}}, true, "Nari-pattern Drengr war helm."},
                {"THYRA", "drengr_thyra", {{"-s8_helmet_drengr_thyra", {}, {}}, {"-flair_s8_helmet_drengr_thyra", {}, {}}}, true, "Thyra-pattern Drengr war helm."},
                {"DVALINN", "drengr_dvalinn", {{"-s8_helmet_drengr_dvalinn", {}, {}}, {"-flair_s8_helmet_drengr_dvalinn", {}, {}}}, true, "Dvalinn-pattern Drengr war helm."},
                {"VALDEMAR", "drengr_valdemar", {{"-s8_helmet_drengr_valdemar", {}, {}}, {"-flair_s8_helmet_drengr_valdemar", {}, {}}}, true, "Valdemar-pattern Drengr war helm."},
            },
            "Norse-inspired MJOLNIR shock-assault helmet from MCC Season 8."
        },
    };
    return kHelmets;
}

struct ShoulderSpec {
    const char* name;
    const char* stringId;
    ObjPreviewLayer leftLayer;
    ObjPreviewLayer rightLayer;
    const char* description;
};

const std::vector<ShoulderSpec>& GetSpartanShoulders() {
    static const std::vector<ShoulderSpec> kShoulders = {
        {"DEFAULT",          "default",       {"", {}, {}}, {"", {}, {}},
         "Standard Mark V [B] low-profile shoulderdeltoid seal without supplemental pauldron plating."},
        {"FJ/PARA",          "fj_para",       {"-flair_shoulder_kat_l", {}, {}}, {"-flair_shoulder_kat_r", {}, {}},
         "Lightweight para-rescue pauldron favored by UNSC Airborne and orbital drop personnel."},
        {"HAZOP",            "hazop",         {"-flair_l_shoulder__(spartans_player_isaac_render)", {}, {}}, {"-flair_r_shoulder__(spartans_player_isaac_render)", {}, {}},
         "Supplemental hazard-operations pauldron sealed against corrosive and radiological particulates."},
        {"JFO",              "jfo",           {"-flair_shoulder_adrianna_l", {}, {}}, {"-flair_shoulder_adrianna_r", {}, {}},
         "Up-armored pauldron housing auxiliary telemetry relays for Joint Fires Observers."},
        {"RECON",            "recon",         {"-flair_l_shoulder__(spartans_player_malcolm_render)", {}, {}, true}, {"-flair_r_shoulder__(spartans_player_malcolm_render)", {}, {}, true},
         "Angular stealth-contoured pauldron designed to minimize radar cross-section."},
        {"UA/MULTI-THREAT",  "multi_threat",  {"-flair_shoulder_l_default__", {}, {}}, {"-flair_shoulder_r_default__", {}, {}},
         "Heavy multi-threat ballistic pauldron designed for sustained frontline firefights."},
        {"JUMP JET",         "jump_jet",      {"-flair_l_shoulder__(spartans_player_will_render)", {}, {}}, {"-flair_r_shoulder__(spartans_player_will_render)", {}, {}},
         "Experimental micro-thruster pauldron designed for zero-g attitude control and high-altitude drops."},
        {"EVA",              "eva",           {"-flair_l_shoulder__(spartans_player_kelly_render)", {}, {}}, {"-flair_r_shoulder__(spartans_player_kelly_render)", {}, {}},
         "Contoured Extra-Vehicular Activity pauldron built for vacuum and orbital operations."},
        {"GUNGNIR",          "gungnir",       {"-flair_l_shoulder__(spartans_player_sam_render)", {}, {}}, {"-flair_r_shoulder__(spartans_player_sam_render)", {}, {}},
         "Heavy slab-sided pauldron engineered under Project GUNGNIR to deflect anti-materiel shrapnel."},
        {"ODST",             "odst",          {"-flair_l_shoulder__(spartans_player_jai_render)", {}, {}, true}, {"-flair_r_shoulder__(spartans_player_jai_render)", {}, {}, true},
         "Drop-shock pauldron adopted across both Orbital Drop Shock Trooper units and Spartan teams."},
        {"UA/BASE SECURITY", "base_security", {"-flair_l_shoulder__(spartans_carter_render)", {}, {}}, {"-flair_r_shoulder__(spartans_carter_render)", {}, {}},
         "Asymmetric reinforced command pauldron issued to ONI and HIGHCOM installation defenders."},
        {"CQC",              "cqc",           {"-player_kurt", {"spartan_shoulder_kurt"}, {}}, {"-flair_r_shoulder__(spartans_player_kurt_render)", {}, {}},
         "Reinforced close-quarters brawling pauldron designed to absorb point-blank impacts."},
        {"OPERATOR",         "operator",      {"-flair_l_shoulder__(spartans_player_nicole_render)", {}, {}}, {"-flair_r_shoulder__(spartans_player_nicole_render)", {}, {}},
         "Compact low-snag pauldron designed for vehicle crews and Asymmetric Action Group operators."},
        {"COMMANDO",         "commando",      {"-flair_l_shoulder_emile__", {}, {}}, {"-flair_r_shoulder__(spartans_player_linda_render)", {}, {}},
         "Heavy tactical command pauldron worn by Spartan-III team leaders."},
        {"GRENADIER",        "grenadier",     {"-flair_joshua_l_shoulder", {}, {}, true}, {"-flair_joshua_r_shoulder", {}, {}, true},
         "Massive up-armored siege pauldron worn by heavy weapons specialists (Noble Five configuration)."},
        {"SNIPER",           "sniper",        {"-flair_l_shoulder__(spartans_jun_render)", {}, {}}, {"-flair_r_shoulder__(spartans_jun_render)", {}, {}},
         "Marksman pauldron fitted with external 14.5x114mm APFSDS anti-materiel ammunition loops."},
        {"MJOLNIR MK. V",    "mark_v",        {"-flair_l_shoulder__(spartans_player_rosenda_render)", {}, {}}, {"-flair_r_shoulder__(spartans_player_rosenda_render)", {}, {}},
         "Classic Mark V fleet-issue pauldron with reinforced upper deltoid ridge."},
        {"SECURITY",         "security",      {"-flair_l_shoulder__(spartans_player_linda_render)", {}, {}}, {"-flair_r_shoulder_emile__", {}, {}},
         "The Supplemental Armor, Pauldron, MJOLNIR (M) variant was originally manufactured in 2528. Right pauldron mounts an M947 Kukri."},
    };
    return kShoulders;
}

struct MultiLayerSpec {
    const char* name;
    const char* stringId;
    std::vector<ObjPreviewLayer> layers;
    bool hasProstheticArm = false;
    const char* description = "";
};

const std::vector<MultiLayerSpec>& GetSpartanChests() {
    static const std::vector<MultiLayerSpec> kChests = {
        {"DEFAULT", "default", {}, false,
         "Standard Mark V [B] titanium-A battle plate with integrated fusion pack."},
        {"HP/HALO", "hp_halo", {
            {"-flair_carter_gear_pack", {}, {}},
            {"-flair_carter_gear_pack_b", {}, {}},
        }, false, "High-Altitude Low-Opening jump harness and supplemental oxygen/telemetry rig."},
        {"UA/COUNTERASSAULT", "ua_counterassault", {
            {"-flair_chest_perm1_default__", {}, {}},
        }, false, "Up-Armored sternum and abdominal blast plating for close-assault operations."},
        {"TACTICAL/LRP", "tactical_lrp", {
            {"-flair_chest_perm2_default__", {}, {}},
        }, false, "Long-Range Patrol chest rig carrying three field-accessible ammunition and ration pouches."},
        {"UA/ODST", "ua_odst", {
            {"-flair_chest__(spartans_player_jai_render)", {}, {}},
        }, false, "Orbital Drop Shock Trooper composite chest plate adapted for MJOLNIR Mark V."},
        {"TACTICAL/RECON", "tactical_recon", {
            {"-flair_chest__(spartans_player_isaac_render)", {}, {}},
        }, false, "Extended reconnaissance chest rig with four sealed magazine and field-gear pouches."},
        {"COLLAR/GRENADIER", "collar_grenadier", {
            {"-flair_joshua_torso", {}, {}},
        }, false, "Heavy ballistic gorget and upper-thorax shield generator housing."},
        {"TACTICAL/PATROL", "tactical_patrol", {
            {"-flair_chest__(spartans_jun_render)", {}, {}},
            {"-flair_backpack__(spartans_jun_render)", {}, {}},
            {"-jun", {"spartan_jun_hood", "spartan_chest_knife"}, {}},
        }, false, "Sniper patrol webbing with camo ghillie hood, combat knife, and long-range rucksack (Noble Three)."},
        {"ASSAULT/COMMANDO", "assault_commando", {
            {"-flair_chest_piece__(spartans_carter_render)", {}, {}},
            {"-flair_ammopack_back__(spartans_carter_render)", {}, {}},
            {"-flair_ammopack_front__(spartans_carter_render)", {}, {}},
        }, false, "Command assault harness with shoulder-mounted comms antenna and rear ammo pack (Noble One)."},
        {"ASSAULT/SAPPER", "assault_sapper", {
            {"-flair_mesh_chest_grenades_emile", {}, {}},
            {"-flair_mesh_grenade_belt_emile_torso", {}, {}},
            {"-flair_mesh_grenade_belt_emile_pelvis", {}, {}},
            {"-flair_mesh_grenade_belt_emile_back", {}, {}},
        }, false, "Combat engineer webbing loaded with bandolier and 40mm grenade belt (Noble Four)."},
        {"COLLAR/BREACHER", "collar_breacher", {
            {"-flair_chest_plate_emile", {}, {}},
            {"-flair_mesh_grenade_belt_emile_torso", {}, {}},
            {"-flair_mesh_grenade_belt_emile_pelvis", {}, {}},
            {"-flair_mesh_grenade_belt_emile_back", {}, {}},
        }, false, "Heavy breaching collar paired with a full waist belt of 40mm high-explosive shells."},
        {"HP/PARAFOIL", "hp_parafoil", {
            {"-flair_chest__(spartans_player_skull_render)", {}, {}},
        }, false, "High-altitude parafoil harness and chest-mounted flight stabilization pack."},
        {"UA/BASE SECURITY", "ua_base_security", {
            {"-flair_chest_kat", {}, {}},
        }, false, "Reinforced security cuirass designed to deflect small-arms and plasma bolts."},
        {"COLLAR/GRENADIER [UA]", "collar_grenadier_ua", {
            {"-flair_joshua_torso", {}, {}},
            {"-flair_joshua_neck_piece", {}, {}},
            {"-flair_joshua_hip_plates", {}, {}},
        }, false, "Full heavy weapons specialist armor suite with armored blast collar and hip plates (Noble Five)."},
        {"UA/MULTI-THREAT", "ua_multi_threat", {
            {"-flair_chest_plate_emile", {}, {}},
            {"-flair_pistol_packs__(spartans_player_malcolm_render)", {}, {}},
        }, false, "Up-Armored chest plate with twin chest-mounted tactical ammunition modules."},
        {"UA/MULTI-THREAT [W]", "ua_multi_threat_w", {
            {"-flair_chest_plate_emile", {}, {}},
            {"-flair_pistol_packs__(spartans_player_malcolm_render)", {}, {}},
            {"-flair_mesh_grenade_belt_emile_torso", {}, {}},
            {"-flair_mesh_grenade_belt_emile_pelvis", {}, {}},
            {"-flair_mesh_grenade_belt_emile_back", {}, {}},
        }, false, "Up-Armored Multi-Threat cuirass combined with a full 40mm grenade waist belt."},
        {"UA/BASE SECURITY [W]", "ua_base_security_w", {
            {"-flair_chest_kat", {}, {}},
        }, true, "Up-Armored Base Security cuirass paired with a trans-humeral prosthetic right arm (Noble Two)."},
        {"TACTICAL/PATROL [R]", "tactical_patrol_r", {
            {"-flair_chest__(spartans_jun_render)", {}, {}},
            {"-flair_backpack__(spartans_jun_render)", {}, {}},
            {"-jun", {"spartan_jun_hood", "spartan_chest_knife"}, {}},
        }, true, "Tactical/Patrol sniper harness paired with a cybernetic right arm."},
        {"TACTICAL/LRP [R]", "tactical_lrp_r", {
            {"-flair_chest_perm2_default__", {}, {}},
        }, true, "Long-Range Patrol chest webbing paired with a cybernetic right arm."},
        {"COLLAR/GRENADIER [UA] [R]", "collar_grenadier_ua_r", {
            {"-flair_joshua_torso", {}, {}},
            {"-flair_joshua_neck_piece", {}, {}},
            {"-flair_joshua_hip_plates", {}, {}},
        }, true, "Collar/Grenadier [UA] heavy chest plate paired with a cybernetic right arm."},
        {"COLLAR/BREACHER [R]", "collar_breacher_r", {
            {"-flair_chest_plate_emile", {}, {}},
            {"-flair_mesh_grenade_belt_emile_torso", {}, {}},
            {"-flair_mesh_grenade_belt_emile_pelvis", {}, {}},
            {"-flair_mesh_grenade_belt_emile_back", {}, {}},
        }, true, "Collar/Breacher armor and 40mm grenade belt paired with a cybernetic right arm."},
        {"ASSAULT/SAPPER [R]", "assault_sapper_r", {
            {"-flair_mesh_chest_grenades_emile", {}, {}},
            {"-flair_mesh_grenade_belt_emile_torso", {}, {}},
            {"-flair_mesh_grenade_belt_emile_pelvis", {}, {}},
            {"-flair_mesh_grenade_belt_emile_back", {}, {}},
        }, true, "Assault/Sapper grenade bandolier paired with a cybernetic right arm."},
        {"HP/PARAFOIL [R]", "hp_parafoil_r", {
            {"-flair_chest__(spartans_player_skull_render)", {}, {}},
        }, true, "HP/Parafoil jump harness paired with a cybernetic right arm."},
        {"UA/MULTI-THREAT [R]", "ua_multi_threat_r", {
            {"-flair_chest_plate_emile", {}, {}},
            {"-flair_pistol_packs__(spartans_player_malcolm_render)", {}, {}},
        }, true, "UA/Multi-Threat chest armor paired with a cybernetic right arm."},
        {"CONDEMNED", "condemned", {
            {"-dlc2_chest_cndmnd", {}, {}},
        }, false, "Heavy orbital salvage chest rig from the Condemned development series."},
        {"HIGHLANDS", "highlands", {
            {"-dlc2_chest_hilands", {}, {}},
        }, false, "Highland Mountains training facility composite chest harness."},
    };
    return kChests;
}

const std::vector<MultiLayerSpec>& GetSpartanWrists() {
    static const std::vector<MultiLayerSpec> kWrists = {
        {"DEFAULT", "default", {}, false,
         "Standard Mark V [B] vambrace without external forearm attachments."},
        {"UA/BUCKLER", "ua_buckler", {
            {"-flair_forearm__(spartans_carter_render)", {}, {}},
        }, false, "Up-Armored forearm deflector shield for parrying close-quarters melee strikes."},
        {"UA/BRACER", "ua_bracer", {
            {"-flair_gauntlet_kat", {}, {}},
        }, false, "Reinforced forearm bracer protecting wrist actuators and suit seals."},
        {"TACTICAL/TACPAD", "tactical_tacpad", {
            {"-flair_tricorder_kat", {}, {}},
        }, false, "Ruggedized forearm Tactical Data Pad providing real-time battlefield telemetry."},
        {"TACTICAL/UGPS", "tactical_ugps", {
            {"-flair_wrist__(spartans_player_kelly_render)", {}, {}},
        }, false, "Universal Global Positioning System wrist terminal for orbital and terrestrial navigation."},
        {"ASSAULT/BREACHER", "assault_breacher", {
            {"-flair_shotgun_reloader__(spartans_player_rosenda_render)", {}, {}},
        }, false, "Forearm-mounted 8-gauge tactical shotgun shell caddy for rapid CQB reloading."},
    };
    return kWrists;
}

const std::vector<MultiLayerSpec>& GetSpartanUtilities() {
    static const std::vector<MultiLayerSpec> kUtilities = {
        {"DEFAULT", "default", {}, false,
         "Standard Mark V [B] thigh armor plate without external utility attachments."},
        {"UA/CHOBHAM", "ua_chobham", {
            {"-flair_thigh_adrianna", {}, {}},
        }, false, "Composite Chobham reactive thigh armor plate for improved ballistic protection."},
        {"TACTICAL/HARD CASE", "tactical_hard_case", {
            {"-flair_joshua_thigh", {}, {}},
        }, false, "Armored thigh hard-case designed to transport sensitive optical or data cores."},
        {"UA/NXRA", "ua_nxra", {
            {"-flair_thigh__(spartans_player_skull_render)", {}, {}},
        }, false, "Non-Explosive Reactive Armor thigh plate engineered to dissipate plasma splash damage."},
        {"TACTICAL/TRAUMA KIT", "tactical_trauma_kit", {
            {"-flair_toolkit_kat", {}, {}},
        }, false, "Field trauma kit stocked with biofoam injectors and emergency suit sealants."},
        {"TACTICAL/SOFT CASE", "tactical_soft_case", {
            {"-flair_mesh_thigh_pouch_emile", {}, {}},
        }, false, "Canvas drop-leg utility pouch for extra magazines and field charges."},
        {"HYDRATION", "hydration", {
            {"-flask", {}, {}},
        }, false, "Hip-mounted stainless steel field canteen for extended campaigns."},
    };
    return kUtilities;
}

const std::vector<MultiLayerSpec>& GetSpartanKneeGuards() {
    static const std::vector<MultiLayerSpec> kKnees = {
        {"DEFAULT", "default", {}, false,
         "Standard Mark V [B] articulated poleyn knee guards."},
        {"FJ/PARA", "fj_para", {
            {"-flair_kneepad_kat_l_mesh1", {}, {}},
            {"-flair_kneepad_kat_l_mesh2", {}, {}},
            {"-flair_kneepad_kat_r_mesh1", {}, {}},
            {"-flair_kneepad_kat_r_mesh2", {}, {}},
        }, false, "Shock-absorbing para-drop knee guards designed for high-velocity combat landings."},
        {"GUNGNIR", "gungnir", {
            {"-flair_knee_adrianna_l", {}, {}},
            {"-flair_knee_adrianna_r", {}, {}},
        }, false, "Heavy angular knee armor developed under Project GUNGNIR."},
        {"GRENADIER", "grenadier", {
            {"-flair_joshua_l_knee", {}, {}},
            {"-flair_joshua_r_knee", {}, {}},
        }, false, "Up-armored siege knee guards built to brace heavy support weapons."},
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
    {"AUNTIE DOT",     "dot",       " Noble Team Dumb AI 'Auntie Dot' tactical status callouts."},
}};

struct EliteSetSpec {
    const char* name;
    const char* stringId;
    const char* helmetGroup;
    const char* chestGroup;
    const char* leftShoulderGroup;
    const char* rightShoulderGroup;
    const char* armsGroup;
    const char* legsGroup;
    const char* extraFlairGroup;
    const char* description;
};

constexpr std::array<EliteSetSpec, 8> kEliteSets = {{
    {"MINOR", "minor",
     "helmet_minor", "chest_minor", "left_shoulder_minor", "right_shoulder_minor", "arms_minor", "legs_minor", "",
     "Standard-issue Sangheili Minor combat harness worn by newly blooded frontline warriors."},
    {"SPEC OPS", "spec_ops",
     "helmet_spec_ops", "chest_spec_ops", "left_shoulder_spec_ops", "right_shoulder_spec_ops", "arms_spec_ops", "legs_spec_ops", "",
     "Streamlined Special Operations combat harness tuned for active camouflage and silent infiltration."},
    {"RANGER", "space",
     "helmet_ranger", "chest_ranger", "left_shoulder_ranger", "right_shoulder_ranger", "arms_ranger", "legs_ranger", "flair_jetpack_ranger",
     "Vacuum-sealed Sangheili Ranger exo-atmospheric harness with integrated thruster pack."},
    {"ULTRA", "ultra",
     "helmet_ultra", "chest_ultra", "left_shoulder_ultra", "right_shoulder_ultra", "arms_ultra", "legs_ultra", "",
     "Command harness of the Sangheili Evocati, featuring high-capacity energy shielding and enclosed helm."},
    {"ZEALOT", "zealot",
     "helmet_zealot", "chest_zealot", "left_shoulder_zealot", "right_shoulder_zealot", "arms_zealot", "legs_zealot", "",
     " Ornate crimson-forged harness of the Ministry of Fervent Intercession's Zealot strike teams."},
    {"GENERAL", "general",
     "helmet_general", "chest_general", "left_shoulder_general", "right_shoulder_general", "arms_general", "legs_general", "",
     "Heavily crested golden battle harness worn by high-ranking Sangheili ground commanders."},
    {"FIELD MARSHAL", "field_marshall",
     "helmet_field_marshall", "chest_field_marshall", "left_shoulder_field_marshall", "right_shoulder_field_marshall", "arms_general", "legs_general", "",
     "Supreme tactical command harness worn by Sangheili Field Marshals leading Covenant armies."},
    {"OFFICER", "officer",
     "helmet_officer", "chest_officer", "left_shoulder_officer", "right_shoulder_officer", "arms_officer", "legs_officer", "",
     "Distinguished Sangheili Officer harness featuring sweeping cranial fins and reinforced pauldrons."},
}};

std::filesystem::path FindArmorAssetPath(const std::filesystem::path& relativePath) {
    std::error_code ec;
    const std::filesystem::path cwdCandidate = std::filesystem::current_path(ec) / relativePath;
    if (std::filesystem::is_regular_file(cwdCandidate, ec)) return cwdCandidate;

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

} // namespace

ArmorCustomizer::ArmorCustomizer() {
    m_profilePath = "appearance.json";
    LoadProfile(m_profilePath);
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

    m_profile.spartanHelmetGroup = h.baseLayer.groupName;
    m_profile.spartanChestGroup = chests[m_profile.chestIndex].layers.empty() ? "" : chests[m_profile.chestIndex].layers.front().groupName;
    m_profile.spartanLeftShoulderGroup = shoulders[m_profile.leftShoulderIndex].leftLayer.groupName;
    m_profile.spartanRightShoulderGroup = shoulders[m_profile.rightShoulderIndex].rightLayer.groupName;
    m_profile.spartanWristGroup = wrists[m_profile.wristIndex].layers.empty() ? "" : wrists[m_profile.wristIndex].layers.front().groupName;
    m_profile.spartanKneesGroup = knees[m_profile.kneeGuardsIndex].layers.empty() ? "" : knees[m_profile.kneeGuardsIndex].layers.front().groupName;
}

void ArmorCustomizer::MigrateFromLegacyFields() {
    const auto& helmets = GetSpartanHelmets();
    if (!m_profile.spartanHelmetGroup.empty() && m_profile.helmetIndex == 0) {
        for (size_t i = 0; i < helmets.size(); ++i) {
            if (helmets[i].baseLayer.groupName == m_profile.spartanHelmetGroup) {
                m_profile.helmetIndex = static_cast<int>(i);
                break;
            }
            for (size_t v = 0; v < helmets[i].variants.size(); ++v) {
                for (const auto& l : helmets[i].variants[v].extraLayers) {
                    if (l.groupName == m_profile.spartanHelmetGroup) {
                        m_profile.helmetIndex = static_cast<int>(i);
                        m_profile.helmetVariantIndex = static_cast<int>(v);
                        break;
                    }
                }
            }
        }
    }
    const auto& shoulders = GetSpartanShoulders();
    if (!m_profile.spartanLeftShoulderGroup.empty() && m_profile.leftShoulderIndex == 0) {
        for (size_t i = 0; i < shoulders.size(); ++i) {
            if (shoulders[i].leftLayer.groupName == m_profile.spartanLeftShoulderGroup) {
                m_profile.leftShoulderIndex = static_cast<int>(i);
                break;
            }
        }
    }
    if (!m_profile.spartanRightShoulderGroup.empty() && m_profile.rightShoulderIndex == 0) {
        for (size_t i = 0; i < shoulders.size(); ++i) {
            if (shoulders[i].rightLayer.groupName == m_profile.spartanRightShoulderGroup) {
                m_profile.rightShoulderIndex = static_cast<int>(i);
                break;
            }
        }
    }
    const auto& chests = GetSpartanChests();
    if (!m_profile.spartanChestGroup.empty() && m_profile.chestIndex == 0) {
        for (size_t i = 0; i < chests.size(); ++i) {
            for (const auto& l : chests[i].layers) {
                if (l.groupName == m_profile.spartanChestGroup) {
                    m_profile.chestIndex = static_cast<int>(i);
                    break;
                }
            }
        }
    }
}

bool ArmorCustomizer::LoadProfile(const std::filesystem::path& path) {
    m_profilePath = path;
    if (!std::filesystem::exists(path)) {
        SyncLegacyFields();
        return false;
    }

    try {
        std::ifstream f(path);
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
        NOBLE_INFO("ArmorCustomizer: Loaded profile from '{}' (Tag: {})", path.string(), m_profile.serviceTag);
        return true;
    } catch (const std::exception& e) {
        NOBLE_WARN("ArmorCustomizer: Failed to parse '{}': {}", path.string(), e.what());
        SyncLegacyFields();
        return false;
    }
}

bool ArmorCustomizer::SaveProfile(const std::filesystem::path& path) {
    SyncLegacyFields();
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
            {"left_shoulder_id", shoulders[m_profile.leftShoulderIndex].stringId},
            {"right_shoulder_id", shoulders[m_profile.rightShoulderIndex].stringId},
            {"chest_id", chests[m_profile.chestIndex].stringId},
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

        std::ofstream f(path);
        if (!f.is_open()) return false;
        f << j.dump(2);
        m_dirty = false;
        NOBLE_INFO("ArmorCustomizer: Saved profile to '{}'", path.string());
        return true;
    } catch (const std::exception& e) {
        NOBLE_ERROR("ArmorCustomizer: Failed to save '{}': {}", path.string(), e.what());
        return false;
    }
}

void ArmorCustomizer::BuildActiveLayers(std::vector<ObjPreviewLayer>& layers, std::string& referenceGroup) const {
    layers.clear();

    if (m_profile.species == PlayerSpecies::Spartan) {
        referenceGroup = "-default";
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

        const int cIdx = std::clamp(m_profile.chestIndex, 0, static_cast<int>(chests.size()) - 1);
        const auto& cSpec = chests[cIdx];

        // 1. Base Spartan Body (with embedded helmet materials excluded so our selected helmet renders cleanly)
        if (cSpec.hasProstheticArm) {
            // Group -kat is the Spartan body with Kat's prosthetic right arm
            ObjPreviewLayer bodyLayer;
            bodyLayer.groupName = "-kat";
            bodyLayer.excludedMaterials = {
                "spartan_helmet_kat_base",
                "spartan_helmet_kat_rubber",
                "spartan_helmet_kat_visor",
                "spartan_helmet_interior",
                "spartan_helmet_interior_visor"
            };
            layers.push_back(std::move(bodyLayer));
        } else if (m_profile.bodyType == BodyType::Female) {
            ObjPreviewLayer bodyLayer;
            bodyLayer.groupName = "-female";
            bodyLayer.excludedMaterials = {
                "spartan_helmet_kat_base",
                "spartan_helmet_kat_rubber",
                "spartan_helmet_kat_visor",
                "spartan_helmet_interior",
                "spartan_helmet_interior_visor"
            };
            layers.push_back(std::move(bodyLayer));
        } else {
            ObjPreviewLayer bodyLayer;
            bodyLayer.groupName = "-default";
            bodyLayer.excludedMaterials = {
                "spartan_helmet_default",
                "spartan_visor_default",
                "spartan_helmet_interior",
                "spartan_helmet_interior_visor"
            };
            layers.push_back(std::move(bodyLayer));
        }

        // 2. Helmet + Helmet Variant
        if (!hvSpec.replaceBaseHelmet) {
            layers.push_back(hSpec.baseLayer);
        }
        for (const auto& extra : hvSpec.extraLayers) {
            layers.push_back(extra);
        }

        // 3. Chest Layers
        for (const auto& cl : cSpec.layers) {
            layers.push_back(cl);
        }

        // 4. Left & Right Shoulders
        const int lsIdx = std::clamp(m_profile.leftShoulderIndex, 0, static_cast<int>(shoulders.size()) - 1);
        if (!shoulders[lsIdx].leftLayer.groupName.empty()) {
            layers.push_back(shoulders[lsIdx].leftLayer);
        }
        const int rsIdx = std::clamp(m_profile.rightShoulderIndex, 0, static_cast<int>(shoulders.size()) - 1);
        if (!shoulders[rsIdx].rightLayer.groupName.empty()) {
            layers.push_back(shoulders[rsIdx].rightLayer);
        }

        // 5. Wrist
        const int wIdx = std::clamp(m_profile.wristIndex, 0, static_cast<int>(wrists.size()) - 1);
        for (const auto& wl : wrists[wIdx].layers) {
            layers.push_back(wl);
        }

        // 6. Utility (Thigh)
        const int uIdx = std::clamp(m_profile.utilityIndex, 0, static_cast<int>(utilities.size()) - 1);
        for (const auto& ul : utilities[uIdx].layers) {
            layers.push_back(ul);
        }

        // 7. Knee Guards (both Left & Right!)
        const int kIdx = std::clamp(m_profile.kneeGuardsIndex, 0, static_cast<int>(knees.size()) - 1);
        for (const auto& kl : knees[kIdx].layers) {
            layers.push_back(kl);
        }
    } else {
        referenceGroup = "body";
        layers.push_back({"body", {}, {}});

        const int maxEl = static_cast<int>(kEliteSets.size()) - 1;
        const auto& hSet = kEliteSets[std::clamp(m_profile.eliteHelmetIndex, 0, maxEl)];
        const auto& cSet = kEliteSets[std::clamp(m_profile.eliteChestIndex, 0, maxEl)];
        const auto& lsSet = kEliteSets[std::clamp(m_profile.eliteLeftShoulderIndex, 0, maxEl)];
        const auto& rsSet = kEliteSets[std::clamp(m_profile.eliteRightShoulderIndex, 0, maxEl)];
        const auto& aSet = kEliteSets[std::clamp(m_profile.eliteArmsIndex, 0, maxEl)];
        const auto& lSet = kEliteSets[std::clamp(m_profile.eliteLegsIndex, 0, maxEl)];

        layers.push_back({hSet.helmetGroup, {}, {}});
        layers.push_back({cSet.chestGroup, {}, {}});
        layers.push_back({lsSet.leftShoulderGroup, {}, {}});
        layers.push_back({rsSet.rightShoulderGroup, {}, {}});
        layers.push_back({aSet.armsGroup, {}, {}});
        layers.push_back({lSet.legsGroup, {}, {}});
        if (cSet.extraFlairGroup && cSet.extraFlairGroup[0] != '\0') {
            layers.push_back({cSet.extraFlairGroup, {}, {}});
        }
    }
}

void ArmorCustomizer::RenderCharacterPreview(ID3D11Device* device, ID3D11DeviceContext* context,
                                             float width, float height, bool fullBleed) {
    if (!device || !context) return;

    if (!m_previewInitialized) {
        m_spartanPreview.Initialize(device);
        m_elitePreview.Initialize(device);
        m_spartanPreview.LoadModel(FindArmorAssetPath("data/armor/spartans/spartans.obj"), m_spartanLoadError);
        m_elitePreview.LoadModel(FindArmorAssetPath("data/armor/elite/elite.obj"), m_eliteLoadError);
        m_previewInitialized = true;
    }

    ObjModelPreview& activePreview = (m_profile.species == PlayerSpecies::Spartan) ? m_spartanPreview : m_elitePreview;
    const std::string& activeError = (m_profile.species == PlayerSpecies::Spartan) ? m_spartanLoadError : m_eliteLoadError;

    if (!activePreview.IsLoaded()) {
        ImGui::SetCursorPos(ImVec2(width * 0.1f, height * 0.45f));
        ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.35f, 1.0f), "Preview model unavailable: %s", activeError.c_str());
        return;
    }

    const auto& pCol = kReachColors[std::clamp(m_profile.primaryColor, 0, 29)];
    const auto& sCol = kReachColors[std::clamp(m_profile.secondaryColor, 0, 29)];
    const auto& vCol = kVisorColors[std::clamp(m_profile.visorColorIndex, 0, static_cast<int>(kVisorColors.size()) - 1)];

    ObjPreviewColors colors;
    colors.primary = {pCol.r, pCol.g, pCol.b, 1.0f};
    colors.secondary = {sCol.r, sCol.g, sCol.b, 1.0f};
    colors.detail = {0.15f, 0.16f, 0.18f, 1.0f};
    colors.visor = {vCol.r, vCol.g, vCol.b, 1.0f};

    std::vector<ObjPreviewLayer> layers;
    std::string referenceGroup;
    BuildActiveLayers(layers, referenceGroup);

    if (m_autoRotate) {
        m_previewYaw += ImGui::GetIO().DeltaTime * 0.35f;
    }

    // Automatically frame full body when Knee Guards or Legs row is highlighted, otherwise use camera toggle
    const bool kneeRowFocused = (m_profile.species == PlayerSpecies::Spartan && m_focusedRow == 8) ||
                                (m_profile.species == PlayerSpecies::Elite && m_focusedRow == 6);
    const bool useFullBody = m_fullBodyCamera || kneeRowFocused;

    ObjPreviewCamera cam{};
    cam.referenceGroup = referenceGroup;
    cam.pitchRadians = m_previewPitch;
    cam.zoom = useFullBody ? 1.02f : m_previewZoom;
    cam.focusYOffset = useFullBody ? -0.02f : m_previewFocusY;
    cam.focusXOffset = 0.0f;
    cam.transparentBackground = fullBleed;

    activePreview.Render(context, m_previewYaw, layers, colors, &cam);

    if (auto* srv = activePreview.GetPreviewTexture()) {
        const float rtAspect = static_cast<float>(activePreview.GetWidth()) / static_cast<float>(activePreview.GetHeight());
        float drawW = width;
        float drawH = drawW / rtAspect;
        if (drawH < height) {
            drawH = height;
            drawW = drawH * rtAspect;
        }
        const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        const float offsetX = (width - drawW) * 0.5f;
        const float offsetY = (height - drawH) * 0.5f;
        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<ImTextureID>(srv),
            ImVec2(cursorPos.x + offsetX, cursorPos.y + offsetY),
            ImVec2(cursorPos.x + offsetX + drawW, cursorPos.y + offsetY + drawH)
        );

        ImGui::InvisibleButton("##character_preview_drag", ImVec2(width, height));
        if (ImGui::IsItemActive() && (ImGui::IsMouseDragging(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Right))) {
            m_previewYaw -= ImGui::GetIO().MouseDelta.x * 0.011f;
            m_previewPitch = std::clamp(m_previewPitch + ImGui::GetIO().MouseDelta.y * 0.005f, -0.35f, 0.35f);
        }
        if (ImGui::IsItemHovered()) {
            const float wheel = ImGui::GetIO().MouseWheel;
            if (std::abs(wheel) > 0.01f) {
                m_fullBodyCamera = false;
                m_previewZoom = std::clamp(m_previewZoom + wheel * 0.12f, 0.90f, 2.30f);
                m_previewFocusY = std::clamp((m_previewZoom - 1.0f) * 0.28f, -0.05f, 0.30f);
            }
        }
    }
}

void ArmorCustomizer::Render(ID3D11Device* device, ID3D11DeviceContext* context) {
    SyncLegacyFields();

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float s = std::max(0.75f, avail.y / 640.0f);

    // Keyboard / Gamepad rotation shortcuts (Q/E or Right Stick)
    if (!ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyDown(ImGuiKey_Q) || ImGui::IsKeyDown(ImGuiKey_GamepadRStickLeft)) {
            m_previewYaw += ImGui::GetIO().DeltaTime * 1.8f;
        }
        if (ImGui::IsKeyDown(ImGuiKey_E) || ImGui::IsKeyDown(ImGuiKey_GamepadRStickRight)) {
            m_previewYaw -= ImGui::GetIO().DeltaTime * 1.8f;
        }
    }

    // -------------------------------------------------------------------------
    // 1. RIGHT SIDE: LARGE MCC-STYLE 3D CHARACTER PREVIEW
    // -------------------------------------------------------------------------
    const float leftColWidth = std::clamp(avail.x * 0.41f, 360.0f * s, 520.0f * s);
    const float previewX = origin.x + leftColWidth - 20.0f * s;
    const float previewW = avail.x - (leftColWidth - 20.0f * s);
    const float previewH = avail.y;

    ImGui::SetCursorScreenPos(ImVec2(previewX, origin.y));
    RenderCharacterPreview(device, context, previewW, previewH, true);

    // -------------------------------------------------------------------------
    // 2. TOP SUB-HEADER BAR: SPECIES / BODY TYPE / SERVICE TAG / CAMERA CONTROLS
    // -------------------------------------------------------------------------
    const float barY = origin.y + 4.0f * s;
    float pillX = origin.x;

    auto drawModePill = [&](const char* label, bool active, float w) -> bool {
        const ImVec2 p0(pillX, barY);
        const ImVec2 p1(pillX + w, barY + 28.0f * s);
        ImGui::SetCursorScreenPos(p0);
        const bool clicked = ImGui::InvisibleButton(label, ImVec2(w, 28.0f * s));
        const bool hovered = ImGui::IsItemHovered();

        const ImU32 bgCol = active ? IM_COL32(34, 88, 136, 210)
                          : hovered ? IM_COL32(28, 52, 82, 180)
                                    : IM_COL32(14, 24, 38, 150);
        dl->AddRectFilled(p0, p1, bgCol, 2.0f);
        if (active) {
            dl->AddLine(ImVec2(p0.x, p1.y - 2.0f), ImVec2(p1.x, p1.y - 2.0f), IM_COL32(95, 205, 255, 255), 2.5f);
        }
        const ImVec2 ts = ImGui::CalcTextSize(label);
        dl->AddText(ImVec2(p0.x + (w - ts.x) * 0.5f, p0.y + (28.0f * s - ts.y) * 0.5f),
                    active ? IM_COL32(255, 255, 255, 255) : IM_COL32(175, 195, 215, 220), label);
        pillX += w + 6.0f * s;
        return clicked;
    };

    if (drawModePill("SPARTAN", m_profile.species == PlayerSpecies::Spartan, 96.0f * s)) {
        m_profile.species = PlayerSpecies::Spartan;
        m_focusedRow = 0;
        m_dirty = true;
        SaveProfile(m_profilePath);
    }
    if (drawModePill("ELITE", m_profile.species == PlayerSpecies::Elite, 84.0f * s)) {
        m_profile.species = PlayerSpecies::Elite;
        m_focusedRow = 0;
        m_dirty = true;
        SaveProfile(m_profilePath);
    }
    if (m_profile.species == PlayerSpecies::Spartan) {
        if (drawModePill(m_profile.bodyType == BodyType::Male ? "BODY: LIGHT/MALE" : "BODY: HEAVY/FEMALE", false, 148.0f * s)) {
            m_profile.bodyType = (m_profile.bodyType == BodyType::Male) ? BodyType::Female : BodyType::Male;
            m_dirty = true;
            SaveProfile(m_profilePath);
        }
    }
    if (drawModePill(m_fullBodyCamera ? "CAM: FULL BODY" : "CAM: PORTRAIT", false, 124.0f * s)) {
        m_fullBodyCamera = !m_fullBodyCamera;
        m_previewZoom = m_fullBodyCamera ? 1.02f : 1.38f;
        m_previewFocusY = m_fullBodyCamera ? -0.02f : 0.12f;
    }

    // -------------------------------------------------------------------------
    // 3. LEFT COLUMN: MCC-STYLE TRANSLUCENT CUSTOMIZATION TABLE ROWS
    // -------------------------------------------------------------------------
    struct RowDescriptor {
        const char* category;
        std::string currentValue;
        const char* itemTitle;
        const char* itemDesc;
        int* indexPtr;
        int count;
        bool isColorRow = false;
        bool isServiceTagRow = false;
    };

    std::vector<RowDescriptor> rows;
    if (m_profile.species == PlayerSpecies::Spartan) {
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
        const auto& eh = kEliteSets[std::clamp(m_profile.eliteHelmetIndex, 0, maxEl - 1)];
        const auto& els = kEliteSets[std::clamp(m_profile.eliteLeftShoulderIndex, 0, maxEl - 1)];
        const auto& ers = kEliteSets[std::clamp(m_profile.eliteRightShoulderIndex, 0, maxEl - 1)];
        const auto& ech = kEliteSets[std::clamp(m_profile.eliteChestIndex, 0, maxEl - 1)];
        const auto& ear = kEliteSets[std::clamp(m_profile.eliteArmsIndex, 0, maxEl - 1)];
        const auto& elg = kEliteSets[std::clamp(m_profile.eliteLegsIndex, 0, maxEl - 1)];
        const auto& ae = kArmorEffects[m_profile.armorEffectIndex];
        const auto& fv = kFirefightVoices[m_profile.firefightVoiceIndex];

        rows.push_back({"Armor Set",       preset.name, preset.name, preset.description, &m_profile.eliteArmorSet,          maxEl});
        rows.push_back({"Helmet",          eh.name,     eh.name,     eh.description,     &m_profile.eliteHelmetIndex,       maxEl});
        rows.push_back({"Left Shoulder",   els.name,    els.name,    els.description,    &m_profile.eliteLeftShoulderIndex,  maxEl});
        rows.push_back({"Right Shoulder",  ers.name,    ers.name,    ers.description,    &m_profile.eliteRightShoulderIndex, maxEl});
        rows.push_back({"Chest",           ech.name,    ech.name,    ech.description,    &m_profile.eliteChestIndex,        maxEl});
        rows.push_back({"Arms / Gauntlets",ear.name,    ear.name,    ear.description,    &m_profile.eliteArmsIndex,         maxEl});
        rows.push_back({"Legs / Greaves",  elg.name,    elg.name,    elg.description,    &m_profile.eliteLegsIndex,         maxEl});
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
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp)) {
            m_focusedRow = (m_focusedRow - 1 + rowCount) % rowCount;
        }
    }

    auto applyRowDelta = [&](int rowIndex, int delta) {
        RowDescriptor& r = rows[rowIndex];
        if (!r.indexPtr || r.count <= 0) return;
        *r.indexPtr = (*r.indexPtr + delta + r.count) % r.count;
        if (m_profile.species == PlayerSpecies::Spartan && rowIndex == 0) {
            // Reset helmet variant to BASE when switching base helmet
            m_profile.helmetVariantIndex = 0;
        } else if (m_profile.species == PlayerSpecies::Elite && rowIndex == 0) {
            // Changing Elite Armor Set updates all 6 Elite armor slots
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
    const float rowH = std::min(36.0f * s, (avail.y - 54.0f * s) / static_cast<float>(rowCount));
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
            m_focusedRow = i;
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
                const float mouseX = ImGui::GetIO().MousePos.x;
                if (mouseX >= valMin.x && mouseX < valMin.x + valWidth * 0.35f) {
                    applyRowDelta(i, -1);
                } else if (mouseX >= valMin.x + valWidth * 0.35f) {
                    applyRowDelta(i, +1);
                }
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
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
        std::snprintf(subInfo, sizeof(subInfo), "[UNLOCKED] %s (%d / %d)  |  Drag character to rotate, Scroll to zoom",
                      activeRow.category, (*activeRow.indexPtr) + 1, activeRow.count);
    } else {
        std::snprintf(subInfo, sizeof(subInfo), "Click row to configure  |  Drag character to rotate, Scroll to zoom");
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
