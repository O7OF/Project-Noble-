#include "ArmoryPreviewLayers.hpp"

#include <algorithm>
#include <array>
#include <cctype>

namespace noble {

namespace {

struct PvHelmetVariant {
    const char* name;
    const char* stringId;
    std::vector<ObjPreviewLayer> extraLayers;
    bool replaceBaseHelmet = false;
    const char* description = "";
};

struct PvHelmet {
    const char* name;
    const char* stringId;
    ObjPreviewLayer baseLayer;
    std::vector<PvHelmetVariant> variants;
    const char* description;
};

const std::vector<PvHelmet>& PvHelmets() {
    static const std::vector<PvHelmet> kHelmets = {
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

struct PvShoulder {
    const char* name;
    const char* stringId;
    ObjPreviewLayer leftLayer;
    ObjPreviewLayer rightLayer;
    const char* description;
};

const std::vector<PvShoulder>& PvShoulders() {
    static const std::vector<PvShoulder> kShoulders = {
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

struct PvMulti {
    const char* name;
    const char* stringId;
    std::vector<ObjPreviewLayer> layers;
    bool hasProstheticArm = false;
    const char* description = "";
};

const std::vector<PvMulti>& PvChests() {
    static const std::vector<PvMulti> kChests = {
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

const std::vector<PvMulti>& PvWrists() {
    static const std::vector<PvMulti> kWrists = {
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

const std::vector<PvMulti>& PvUtilities() {
    static const std::vector<PvMulti> kUtilities = {
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

const std::vector<PvMulti>& PvKnees() {
    static const std::vector<PvMulti> kKnees = {
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


struct PvEliteSet {
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

constexpr std::array<PvEliteSet, 8> kPvEliteSets = {{
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


std::string Lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

template <typename T>
const T* ByName(const std::vector<T>& list, const std::string& name) {
    const std::string want = Lower(name);
    for (const T& x : list) {
        if (Lower(x.name) == want) return &x;
    }
    return nullptr;
}

} // namespace

void BuildArmoryPreviewLayers(const ArmoryLook& look, std::vector<ObjPreviewLayer>& layers, std::string& referenceGroup) {
    layers.clear();
    if (look.elite) {
        // The Elite export has one group per armor set, each a whole Elite in that harness.
        static const char* const kSetGroups[8] = { "-default", "-spec_ops", "-space", "-ultra", "-zealot", "-general", "-general", "-officer" };
        const char* g = kSetGroups[std::clamp(look.eliteSet, 0, 7)];
        referenceGroup = g;
        layers.push_back({ g, {}, {} });
        return;
    }

    referenceGroup = "-default";
    // Chest: "[R]" pieces without their own entry use the plain piece (the robot arm comes from the body below).
    const PvMulti* chest = ByName(PvChests(), look.chest);
    if (!chest) {
        std::string plain = look.chest;
        const size_t r = plain.find(" [R]");
        if (r != std::string::npos) plain.erase(r, 4);
        chest = ByName(PvChests(), plain);
    }
    const bool robotArm = look.prostheticArm || (chest && chest->hasProstheticArm);

    // 1. Body (its built-in helmet is left out so the chosen helmet shows).
    ObjPreviewLayer body;
    if (robotArm) {
        body.groupName = "-kat";
        body.excludedMaterials = { "spartan_helmet_kat_base", "spartan_helmet_kat_rubber", "spartan_helmet_kat_visor",
                                   "spartan_helmet_interior", "spartan_helmet_interior_visor" };
    } else if (look.female) {
        body.groupName = "-female";
        body.excludedMaterials = { "spartan_helmet_kat_base", "spartan_helmet_kat_rubber", "spartan_helmet_kat_visor",
                                   "spartan_helmet_interior", "spartan_helmet_interior_visor" };
    } else {
        body.groupName = "-default";
        body.excludedMaterials = { "spartan_helmet_default", "spartan_visor_default", "spartan_helmet_interior",
                                   "spartan_helmet_interior_visor" };
    }
    layers.push_back(std::move(body));

    // 2. Helmet + variant (a variant without preview data shows the base helmet).
    if (const PvHelmet* h = ByName(PvHelmets(), look.helmet)) {
        const PvHelmetVariant* v = ByName(h->variants, look.helmetVariant);
        if (!v || !v->replaceBaseHelmet) layers.push_back(h->baseLayer);
        if (v) for (const auto& l : v->extraLayers) layers.push_back(l);
    } else if (const PvHelmet* d = ByName(PvHelmets(), "MARK V [B]")) {
        layers.push_back(d->baseLayer);
    }

    // 3. Chest, shoulders, wrist, utility, knees.
    if (chest) for (const auto& l : chest->layers) layers.push_back(l);
    if (const PvShoulder* ls = ByName(PvShoulders(), look.leftShoulder)) {
        if (!ls->leftLayer.groupName.empty()) layers.push_back(ls->leftLayer);
    }
    if (const PvShoulder* rs = ByName(PvShoulders(), look.rightShoulder)) {
        if (!rs->rightLayer.groupName.empty()) layers.push_back(rs->rightLayer);
    }
    for (const auto* list : { &PvWrists(), &PvUtilities(), &PvKnees() }) {
        const std::string& name = list == &PvWrists() ? look.wrist : list == &PvUtilities() ? look.utility : look.knees;
        if (const PvMulti* m = ByName(*list, name)) for (const auto& l : m->layers) layers.push_back(l);
    }
}

} // namespace noble
