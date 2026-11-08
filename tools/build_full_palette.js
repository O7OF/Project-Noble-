const fs = require('fs');
const path = require('path');

const existingPath = path.join(__dirname, '..', 'data', 'forge_palette.json');
let raw = fs.readFileSync(existingPath, 'utf8').replace(/^\uFEFF/, '');
let existing = JSON.parse(raw);

// Extract existing custom AI tabs
const aiTabs = existing.tabs.filter(t => 
    t.name === 'UNSC INFANTRY' || 
    t.name === 'COVENANT INFANTRY' || 
    t.name === 'SPECIAL & BOSSES' ||
    t.name === 'SPECIAL \u0026 BOSSES'
);

const fullPalette = {
    tabs: [
        {
            name: "SPAWNS",
            items: [
                { name: "Respawn Point", tag: "objects/multi/spawns/respawn_point", description: "Standard multiplayer respawn point.", max: 64, price: 0 },
                { name: "Initial Spawn Point", tag: "objects/multi/spawns/initial_spawn_point", description: "Match initial spawn point.", max: 32, price: 0 }
            ]
        },
        {
            name: "GOALS",
            items: [
                { name: "Capture The Flag - Flag Stand", tag: "objects/multi/ctf/flag_stand", description: "Base flag stand for CTF games.", max: 16, price: 0 },
                { name: "Capture The Flag - Capture Plate", tag: "objects/multi/ctf/capture_plate", description: "Capture return plate for CTF.", max: 16, price: 0 },
                { name: "King of the Hill - Hill Marker", tag: "objects/multi/koth/hill_marker", description: "Hill capture boundary marker.", max: 16, price: 0 },
                { name: "Oddball - Ball Spawn", tag: "objects/multi/oddball/ball_spawn", description: "Oddball skull spawn location.", max: 8, price: 0 },
                { name: "Territories - Territory Marker", tag: "objects/multi/territories/territory_marker", description: "Capturable territory zone marker.", max: 16, price: 0 },
                { name: "Race - Checkpoint", tag: "objects/multi/race/checkpoint", description: "Race track checkpoint gate.", max: 32, price: 0 },
                { name: "Infection - Safe Haven", tag: "objects/multi/infection/safe_haven", description: "Survivor safe haven zone.", max: 8, price: 0 },
                { name: "Assault - Bomb Spawn", tag: "objects/multi/assault/bomb_spawn", description: "Bomb spawn station for Assault.", max: 8, price: 0 },
                { name: "Assault - Bomb Arming Zone", tag: "objects/multi/assault/bomb_arming_zone", description: "Target arming detonation zone.", max: 8, price: 0 },
                { name: "Stockpile - Collection Area", tag: "objects/multi/stockpile/collection_area", description: "Stockpile flag collection zone.", max: 8, price: 0 }
            ]
        },
        {
            name: "WEAPONS",
            items: [
                { name: "Assault Rifle", tag: "objects/weapons/rifle/assault_rifle/assault_rifle", description: "UNSC MA37 Individual Combat Weapon System.", max: 32, price: 0 },
                { name: "DMR", tag: "objects/weapons/rifle/dmr/dmr", description: "UNSC Designated Marksman Rifle M392.", max: 32, price: 0 },
                { name: "Magnum", tag: "objects/weapons/pistol/magnum/magnum", description: "UNSC M6G Sidearm Pistol.", max: 32, price: 0 },
                { name: "Shotgun", tag: "objects/weapons/rifle/shotgun/shotgun", description: "UNSC M45 Tactical Shotgun.", max: 16, price: 0 },
                { name: "Sniper Rifle", tag: "objects/weapons/rifle/sniper_rifle/sniper_rifle", description: "UNSC SRS99 Anti-Materiel Sniper Rifle.", max: 8, price: 0 },
                { name: "Rocket Launcher", tag: "objects/weapons/support_high/rocket_launcher/rocket_launcher", description: "UNSC M41 Surface-to-Surface Rocket Launcher.", max: 8, price: 0 },
                { name: "Spartan Laser", tag: "objects/weapons/support_high/spartan_laser/spartan_laser", description: "UNSC Weapon/Anti-Vehicle Model 6 Grindell/Galvian.", max: 4, price: 0 },
                { name: "Grenade Launcher", tag: "objects/weapons/pistol/grenade_launcher/grenade_launcher", description: "UNSC M319 Individual Grenade Launcher.", max: 16, price: 0 },
                { name: "Plasma Pistol", tag: "objects/weapons/pistol/plasma_pistol/plasma_pistol", description: "Covenant Directed Energy Pistol.", max: 32, price: 0 },
                { name: "Plasma Repeater", tag: "objects/weapons/rifle/plasma_repeater/plasma_repeater", description: "Covenant Type-51 Directed Energy Rifle.", max: 32, price: 0 },
                { name: "Plasma Rifle", tag: "objects/weapons/rifle/plasma_rifle/plasma_rifle", description: "Covenant Type-25 Directed Energy Rifle.", max: 32, price: 0 },
                { name: "Needler", tag: "objects/weapons/pistol/needler/needler", description: "Covenant Type-33 Guided Munitions Weapon.", max: 32, price: 0 },
                { name: "Needle Rifle", tag: "objects/weapons/rifle/needle_rifle/needle_rifle", description: "Covenant Type-31 Guided Rifle.", max: 32, price: 0 },
                { name: "Concussion Rifle", tag: "objects/weapons/rifle/concussion_rifle/concussion_rifle", description: "Covenant Type-50 Concussion Weapon.", max: 16, price: 0 },
                { name: "Focus Rifle", tag: "objects/weapons/rifle/focus_rifle/focus_rifle", description: "Covenant Type-52 Special Applications Rifle.", max: 8, price: 0 },
                { name: "Fuel Rod Gun", tag: "objects/weapons/support_high/fuel_rod_gun/fuel_rod_gun", description: "Covenant Type-33 Light Anti-Armor Weapon.", max: 8, price: 0 },
                { name: "Energy Sword", tag: "objects/weapons/melee/energy_sword/energy_sword", description: "Covenant Type-1 Close Combat Weapon.", max: 8, price: 0 },
                { name: "Gravity Hammer", tag: "objects/weapons/melee/gravity_hammer/gravity_hammer", description: "Jiralhanae Type-2 Energy Weapon.", max: 8, price: 0 },
                { name: "Frag Grenade", tag: "objects/weapons/grenade/frag_grenade/frag_grenade", description: "UNSC M9 High-Explosive Fragmentation Grenade.", max: 64, price: 0 },
                { name: "Plasma Grenade", tag: "objects/weapons/grenade/plasma_grenade/plasma_grenade", description: "Covenant Type-1 Antipersonnel Grenade.", max: 64, price: 0 }
            ]
        },
        {
            name: "VEHICLES",
            items: [
                { name: "Mongoose", tag: "objects/vehicles/human/mongoose/mongoose", description: "UNSC M274 Ultra-Light All-Terrain Vehicle.", max: 16, price: 0 },
                { name: "Warthog", tag: "objects/vehicles/human/warthog/warthog", description: "UNSC M12 Force Application Vehicle with Vulcan chain gun.", max: 16, price: 0 },
                { name: "Warthog, Rocket", tag: "objects/vehicles/human/warthog/warthog_rocket", description: "UNSC M12A1 Light Anti-Armor Vehicle with rocket battery.", max: 8, price: 0 },
                { name: "Warthog, Gauss", tag: "objects/vehicles/human/warthog/warthog_gauss", description: "UNSC M12G1 Light Anti-Armor Vehicle with Gauss cannon.", max: 8, price: 0 },
                { name: "Scorpion", tag: "objects/vehicles/human/scorpion/scorpion", description: "UNSC M808B Main Battle Tank with 90mm cannon.", max: 4, price: 0 },
                { name: "Falcon", tag: "objects/vehicles/human/falcon/falcon", description: "UNSC UH-144 Falcon VTOL transport with dual side door guns.", max: 4, price: 0 },
                { name: "Ghost", tag: "objects/vehicles/covenant/ghost/ghost", description: "Covenant Type-32 Rapid Assault Vehicle.", max: 16, price: 0 },
                { name: "Banshee", tag: "objects/vehicles/covenant/banshee/banshee", description: "Covenant Type-26 Ground Support Aircraft.", max: 4, price: 0 },
                { name: "Revenant", tag: "objects/vehicles/covenant/revenant/revenant", description: "Covenant Type-48 Light Assault Gun Carriage.", max: 8, price: 0 },
                { name: "Wraith", tag: "objects/vehicles/covenant/wraith/wraith", description: "Covenant Type-26 Heavy Mortar Assault Tank.", max: 4, price: 0 },
                { name: "Pelican", tag: "objects/vehicles/human/pelican/pelican", description: "UNSC D77-TC Pelican Dropship. Flyable cockpit with wraith_d driver seat.", max: 4, price: 0 }
            ]
        },
        {
            name: "EQUIPMENT",
            items: [
                { name: "Armor Lock", tag: "objects/equipment/armor_lock/armor_lock", description: "Invulnerability shield module with EMP burst.", max: 16, price: 0 },
                { name: "Evade", tag: "objects/equipment/evade/evade", description: "Rapid directional dash and dodge maneuver.", max: 16, price: 0 },
                { name: "Jetpack", tag: "objects/equipment/jetpack/jetpack", description: "Vertical lift and sustained flight thruster.", max: 16, price: 0 },
                { name: "Hologram", tag: "objects/equipment/hologram/hologram", description: "Decoy holographic projection decoy.", max: 16, price: 0 },
                { name: "Active Camouflage", tag: "objects/equipment/active_camo/active_camo", description: "Bending optical cloaking field with radar jammer.", max: 16, price: 0 },
                { name: "Drop Shield", tag: "objects/equipment/drop_shield/drop_shield", description: "Deployable spherical regenerative shield bubble.", max: 16, price: 0 },
                { name: "Sprint", tag: "objects/equipment/sprint/sprint", description: "Burst velocity booster.", max: 16, price: 0 }
            ]
        },
        {
            name: "SCENERY",
            items: [
                { name: "Heavy Barrier", tag: "objects/levels/shared/barriers/heavy_barrier", description: "Reinforced steel anti-vehicle barricade.", max: 32, price: 0 },
                { name: "Jersey Barrier", tag: "objects/levels/shared/barriers/jersey_barrier", description: "Concrete highway traffic divider barrier.", max: 32, price: 0 },
                { name: "Street Cone", tag: "objects/levels/shared/barriers/street_cone", description: "Fluorescent orange roadside hazard cone.", max: 32, price: 0 },
                { name: "Small Barricade", tag: "objects/levels/shared/barriers/barricade_small", description: "Portable tactical infantry waist cover.", max: 32, price: 0 },
                { name: "Large Barricade", tag: "objects/levels/shared/barriers/barricade_large", description: "Full height protective defense wall barrier.", max: 32, price: 0 },
                { name: "Heavy Crate", tag: "objects/levels/shared/crates/crate_heavy", description: "Heavy metallic industrial cargo container.", max: 32, price: 0 },
                { name: "Ammo Crate", tag: "objects/levels/shared/crates/ammo_crate", description: "UNSC munitions supply footlocker.", max: 32, price: 0 },
                { name: "Radio", tag: "objects/levels/shared/gadgets/radio", description: "Military communication transceiver.", max: 16, price: 0 },
                { name: "Dice", tag: "objects/levels/shared/gadgets/dice", description: "Oversized physics rolling play cube.", max: 8, price: 0 },
                { name: "Target", tag: "objects/levels/shared/gadgets/target", description: "Shooting gallery accuracy range bullseye target.", max: 16, price: 0 }
            ]
        },
        {
            name: "BUILDING BLOCKS",
            items: [
                { name: "Block 1x1", tag: "objects/levels/shared/blocks/block_1x1", description: "Solid steel structural cube 1x1.", max: 64, price: 0 },
                { name: "Block 1x2", tag: "objects/levels/shared/blocks/block_1x2", description: "Solid structural block 1x2.", max: 64, price: 0 },
                { name: "Block 1x4", tag: "objects/levels/shared/blocks/block_1x4", description: "Long structural block 1x4.", max: 64, price: 0 },
                { name: "Block 2x2", tag: "objects/levels/shared/blocks/block_2x2", description: "Square structural block 2x2.", max: 64, price: 0 },
                { name: "Block 2x3", tag: "objects/levels/shared/blocks/block_2x3", description: "Heavy structural block 2x3.", max: 64, price: 0 },
                { name: "Block 2x4", tag: "objects/levels/shared/blocks/block_2x4", description: "Wide structural block 2x4.", max: 64, price: 0 },
                { name: "Block 3x3", tag: "objects/levels/shared/blocks/block_3x3", description: "Large square structural foundation 3x3.", max: 64, price: 0 },
                { name: "Block 3x4", tag: "objects/levels/shared/blocks/block_3x4", description: "Foundation slab 3x4.", max: 64, price: 0 },
                { name: "Block 4x4", tag: "objects/levels/shared/blocks/block_4x4", description: "Massive foundation pad 4x4.", max: 32, price: 0 },
                { name: "Block 5x5", tag: "objects/levels/shared/blocks/block_5x5", description: "Colossal foundation pad 5x5.", max: 32, price: 0 },
                { name: "Bridge Small", tag: "objects/levels/shared/bridges/bridge_small", description: "Walkway bridge small.", max: 32, price: 0 },
                { name: "Bridge Medium", tag: "objects/levels/shared/bridges/bridge_medium", description: "Vehicle bridge medium.", max: 32, price: 0 },
                { name: "Bridge Large", tag: "objects/levels/shared/bridges/bridge_large", description: "Highway bridge span large.", max: 16, price: 0 },
                { name: "Bridge Diagonal", tag: "objects/levels/shared/bridges/bridge_diagonal", description: "Diagonal connecting bridge.", max: 16, price: 0 },
                { name: "Bunker Small", tag: "objects/levels/shared/bunkers/bunker_small", description: "Fortified pillbox defense bunker.", max: 16, price: 0 },
                { name: "Bunker Large", tag: "objects/levels/shared/bunkers/bunker_large", description: "Fortified garrison command bunker.", max: 8, price: 0 },
                { name: "Room Small", tag: "objects/levels/shared/buildings/room_small", description: "Interior room small.", max: 16, price: 0 },
                { name: "Room Large", tag: "objects/levels/shared/buildings/room_large", description: "Interior room large.", max: 8, price: 0 },
                { name: "Tower", tag: "objects/levels/shared/buildings/tower", description: "Multi-level sniper lookout tower.", max: 8, price: 0 },
                { name: "Door", tag: "objects/levels/shared/doors/door", description: "Building doorway frame entrance.", max: 32, price: 0 },
                { name: "Window", tag: "objects/levels/shared/doors/window", description: "Observation window viewport frame.", max: 32, price: 0 },
                { name: "Incline 1x1", tag: "objects/levels/shared/inclines/incline_1x1", description: "Wedge incline ramp 1x1.", max: 32, price: 0 },
                { name: "Incline 1x2", tag: "objects/levels/shared/inclines/incline_1x2", description: "Wedge incline ramp 1x2.", max: 32, price: 0 },
                { name: "Incline 2x2", tag: "objects/levels/shared/inclines/incline_2x2", description: "Wide incline ramp 2x2.", max: 32, price: 0 },
                { name: "Column", tag: "objects/levels/shared/columns/column", description: "Square architectural support column.", max: 32, price: 0 },
                { name: "Cylinder", tag: "objects/levels/shared/columns/cylinder", description: "Round architectural support cylinder.", max: 32, price: 0 },
                { name: "Wall Corner", tag: "objects/levels/shared/covers/wall_corner", description: "Angle corner defensive wall.", max: 32, price: 0 },
                { name: "Wall Cover", tag: "objects/levels/shared/covers/wall_cover", description: "Chest-high combat defense wall.", max: 32, price: 0 },
                { name: "Shield Door", tag: "objects/levels/shared/covers/shield_door", description: "Energy barrier portal one-way shield.", max: 16, price: 0 },
                { name: "Landing Pad Small", tag: "objects/levels/shared/pads/landing_pad_small", description: "Helipad runway landing zone small.", max: 16, price: 0 },
                { name: "Landing Pad Large", tag: "objects/levels/shared/pads/landing_pad_large", description: "Heavy VTOL dropship landing platform.", max: 8, price: 0 },
                { name: "Grid", tag: "objects/levels/shared/platforms/grid", description: "Walkable perforated metal catwalk grid.", max: 32, price: 0 },
                { name: "Platform Small", tag: "objects/levels/shared/platforms/platform_small", description: "Elevated combat deck platform small.", max: 32, price: 0 },
                { name: "Platform Large", tag: "objects/levels/shared/platforms/platform_large", description: "Heavy staging deck platform large.", max: 16, price: 0 },
                { name: "Railing Small", tag: "objects/levels/shared/railings/railing_small", description: "Safety guard railing short.", max: 32, price: 0 },
                { name: "Railing Medium", tag: "objects/levels/shared/railings/railing_medium", description: "Safety guard railing long.", max: 32, price: 0 },
                { name: "Ramp Small", tag: "objects/levels/shared/ramps/ramp_small", description: "Access slope ramp small.", max: 32, price: 0 },
                { name: "Ramp Large", tag: "objects/levels/shared/ramps/ramp_large", description: "Access slope ramp large.", max: 16, price: 0 },
                { name: "Ramp Circular", tag: "objects/levels/shared/ramps/ramp_circular", description: "Curved spiral elevation ramp.", max: 16, price: 0 },
                { name: "Tube Small", tag: "objects/levels/shared/tubes/tube_small", description: "Enclosed crawlspace pipe tunnel small.", max: 32, price: 0 },
                { name: "Tube Medium", tag: "objects/levels/shared/tubes/tube_medium", description: "Enclosed access corridor tube medium.", max: 16, price: 0 },
                { name: "Tube Curved", tag: "objects/levels/shared/tubes/tube_curved", description: "Enclosed elbow turn tunnel curved.", max: 16, price: 0 },
                { name: "Wall 1x1", tag: "objects/levels/shared/walls/wall_1x1", description: "Vertical wall panel 1x1.", max: 64, price: 0 },
                { name: "Wall 1x2", tag: "objects/levels/shared/walls/wall_1x2", description: "Vertical wall panel 1x2.", max: 64, price: 0 },
                { name: "Wall 2x2", tag: "objects/levels/shared/walls/wall_2x2", description: "Vertical wall panel 2x2.", max: 64, price: 0 },
                { name: "Wall 2x3", tag: "objects/levels/shared/walls/wall_2x3", description: "Vertical wall panel 2x3.", max: 64, price: 0 },
                { name: "Wall 2x4", tag: "objects/levels/shared/walls/wall_2x4", description: "Vertical wall panel 2x4.", max: 64, price: 0 },
                { name: "Wall 4x4", tag: "objects/levels/shared/walls/wall_4x4", description: "Heavy defense wall slab 4x4.", max: 32, price: 0 },
                { name: "Wall 8x8", tag: "objects/levels/shared/walls/wall_8x8", description: "Enormous perimeter fortification wall 8x8.", max: 16, price: 0 }
            ]
        },
        {
            name: "GADGETS",
            items: [
                { name: "Fusion Coil", tag: "objects/levels/shared/explosives/fusion_coil", description: "Volatile high-yield plasma explosive barrel.", max: 32, price: 0 },
                { name: "Fuel Rod Canister", tag: "objects/levels/shared/explosives/fuel_rod_canister", description: "Pressurized radioactive explosive canister.", max: 32, price: 0 },
                { name: "Propane Tank", tag: "objects/levels/shared/explosives/propane_tank", description: "Flammable pressurized fuel gas canister.", max: 32, price: 0 },
                { name: "Gravity Lift", tag: "objects/levels/shared/lifts/grav_lift", description: "Vertical repulsor anti-gravity elevator beam.", max: 16, price: 0 },
                { name: "Teleporter, Two-Way", tag: "objects/levels/shared/teleporters/teleporter_two_way", description: "Bidirectional instant matter transmitter gate.", max: 16, price: 0 },
                { name: "Teleporter, Sender", tag: "objects/levels/shared/teleporters/teleporter_sender", description: "One-way outbound matter departure gate.", max: 16, price: 0 },
                { name: "Teleporter, Receiver", tag: "objects/levels/shared/teleporters/teleporter_receiver", description: "One-way inbound matter arrival terminus.", max: 16, price: 0 },
                { name: "Man Cannon", tag: "objects/levels/shared/lifts/man_cannon", description: "Pneumatic launch pad trajectory accelerator.", max: 16, price: 0 },
                { name: "Light Bridge", tag: "objects/levels/shared/lifts/light_bridge", description: "Solid energy light bridge walkway.", max: 8, price: 0 }
            ]
        },
        {
            name: "LIGHTS",
            items: [
                { name: "Red Light", tag: "objects/levels/shared/lights/light_red", description: "High-intensity omnidirectional red spotlight.", max: 32, price: 0 },
                { name: "Blue Light", tag: "objects/levels/shared/lights/light_blue", description: "High-intensity omnidirectional blue spotlight.", max: 32, price: 0 },
                { name: "Green Light", tag: "objects/levels/shared/lights/light_green", description: "High-intensity omnidirectional green spotlight.", max: 32, price: 0 },
                { name: "Yellow Light", tag: "objects/levels/shared/lights/light_yellow", description: "High-intensity omnidirectional yellow spotlight.", max: 32, price: 0 },
                { name: "White Light", tag: "objects/levels/shared/lights/light_white", description: "High-intensity omnidirectional white spotlight.", max: 32, price: 0 },
                { name: "Purple Light", tag: "objects/levels/shared/lights/light_purple", description: "High-intensity omnidirectional purple spotlight.", max: 32, price: 0 }
            ]
        },
        {
            name: "NATURAL",
            items: [
                { name: "Large Rock", tag: "objects/levels/shared/natural/rock_large", description: "Massive natural granite boulder.", max: 32, price: 0 },
                { name: "Flat Rock", tag: "objects/levels/shared/natural/rock_flat", description: "Flat natural limestone stepping stone slab.", max: 32, price: 0 },
                { name: "Spire Rock", tag: "objects/levels/shared/natural/rock_spire", description: "Vertical natural stone geological pillar.", max: 32, price: 0 },
                { name: "Cliff Rock", tag: "objects/levels/shared/natural/rock_cliff", description: "Rough cliff face natural rock wall.", max: 32, price: 0 },
                { name: "Tree", tag: "objects/levels/shared/natural/tree", description: "Temperate pine tree foliage.", max: 32, price: 0 },
                { name: "Bush", tag: "objects/levels/shared/natural/bush", description: "Dense natural concealing scrub brush.", max: 32, price: 0 }
            ]
        },
        ...aiTabs
    ]
};

fs.writeFileSync(existingPath, JSON.stringify(fullPalette, null, 4), 'utf8');
console.log('Successfully generated complete forge_palette.json with ' + fullPalette.tabs.length + ' tabs.');
