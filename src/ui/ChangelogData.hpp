#pragma once

#include "common/Version.hpp"

#include <string>
#include <vector>

namespace NobleChangelog {

struct ChangeItem {
    const char *category; // e.g. "UI", "NETCODE", "CONTROLLER", "FIX"
    const char *headline;
    const char *detail;
};

struct ReleaseNotes {
    std::string version;
    std::string title;
    std::string subtitle;
    std::vector<ChangeItem> items;
};

inline std::string CurrentChangelogVersionKey() {
    return "v" + std::to_string(noble::VERSION_MAJOR) + "." +
           std::to_string(noble::VERSION_MINOR) + "." +
           std::to_string(noble::VERSION_PATCH);
}

inline std::vector<ReleaseNotes> BuildReleaseNotes() {
    const std::string currentVer = CurrentChangelogVersionKey();
    std::vector<ReleaseNotes> notes;

    const std::vector<ChangeItem> v02023Items = {
        {"AUDIO & ENGINE", "Cutscene Audio Unmute & Failsafe Watchdog",
         "Ensured gameplay, gunfire, and footstep sound effects cleanly restore upon skipping campaign cutscenes across all scenarios with active listener reset and sound class un-ducking."},
        {"CONTROLLER", "Full Controller Keybind Schema Parity & Navigation",
         "Added full parity for all 66+ Keyboard & Mouse actions to controller mappings (Lower Weapon, 3rd Person, Vehicle 1st Person, Forge tools, Theater controls) with polished menu navigation."},
    };

    notes.push_back({
        "v0.20.23",
        "Cutscene Audio Synchronization & Controller Schema Parity",
        "Reliable Gameplay Audio Restoration & Extended Controller Mappings",
        v02023Items
    });

    const std::vector<ChangeItem> v02022Items = {
        {"AUDIO", "Campaign Cutscene Skip Audio Unmute & Listener Recovery",
         "Resolved the engine audio bug where skipping campaign cinematics left weapon firing, footsteps, and gameplay audio muted until dying. Automatically restores audio listeners from cinematic camera mode to first-person, un-ducks all 71 sound classes back to 0 dB, and unhides player units on skip and frame ticks."},
        {"CONTROLLER", "Full Keyboard & Mouse Parity for Controller Keybinds",
         "Expanded the controller binding schema to include all Keyboard & Mouse actions across categorized sections (Movement, Actions, Vehicle, Interface, Forge, Theater). Players can now map Lower Weapon, 3rd-Person Camera, Vehicle 1st Person, Night Vision, Falcon Ascend/Descend, Loadouts, Forge tools, and Theater controls to any controller button, paddle, or stick click."},
    };

    notes.push_back({
        "v0.20.22",
        "Cutscene Audio Recovery & Full Controller Keybind Parity",
        "Gameplay Audio Restored on Cutscene Skip + Extended Controller Schema",
        v02022Items
    });

    const std::vector<ChangeItem> v02021Items = {
        {"AUDIO", "Instant Match Start Audio Across All Game Modes",
         "Synchronized live audio mixer volumes and audio thread queues directly upon engine load and match start, ensuring game, music, and sound effects play immediately in Campaign, Firefight, Forge, and Multiplayer without requiring a respawn."},
        {"CAMPAIGN JIP", "Campaign Join-In-Progress Black Screen Resolution",
         "Eliminated stuck rasterizer black screen fades and loading locks for clients joining Campaign and Firefight sessions in progress, immediately transitioning joining players into the live gameplay simulation."},
        {"NETCODE", "Seamless Cutscene Auto-Skip for Mid-Match Joiners",
         "Connected joining guests directly to live world state, auto-skipping obsolete scenario opening scripts so late-joining players spawn with full camera and player control."},
    };

    const std::vector<ChangeItem> v02019Items = {
        {"MULTIPLAYER", "Match Appearance Isolation & Anti-Host-Swap",
         "Eliminated host customization pollution on remote players in multiplayer matches; appearance record generation and profile synchronization now strictly distinguish between local machine and remote peer datums."},
        {"CUSTOMIZATION", "Strict Per-Client Color & Permutation Parity",
         "Host and guest players in multiplayer sessions wear strictly their own Spartan/Elite colors, armor pieces, custom visor finishes, lights, and effects without cross-client overwrites."},
        {"NETCODE", "Direct Local User Mapping Verification",
         "Replaced fragile struct offset heuristics with the engine's canonical local user mapping table (player_mapping_globals), ensuring robust local player identification on both host and client machines."},
    };

    const std::vector<ChangeItem> v02018Items = {
        {"GAMEPLAY", "3rd-Person Dynamic Reticle Polish",
         "Refined 3rd-person crosshair tracking with multi-tier adaptive smoothing, eliminating camera drag when spinning or looking up and down while maintaining silky walk-cycle stabilization."},
        {"VISORS", "In-Match Custom Visor Color Replication",
         "Connected all 30 custom Spartan visor finishes to profile items and network appearance records, eliminating default visor fallbacks and replicating colors across host and non-host clients in matches."},
        {"MULTIPLAYER", "Armor Lights & Customization Network Replication",
         "Replicated Spartan and Elite armor lights color and brightness, piece picks, and armor effects across multiplayer peers using network appearance stamping and live shader constant synchronization."},
    };

    const std::vector<ChangeItem> v02017Items = {
        {"GAMEPLAY", "3rd-Person Dynamic Reticle Smoothing",
         "Added screen-space dynamic interpolation to the third-person reticle, eliminating character animation jitter and geometry snapping so the crosshair glides smoothly across the screen while moving."},
        {"ARMORY", "Intuitive Armory Controller Navigation",
         "Enhanced controller flow across customization menus: pressing Right or A seamlessly enters the options grid, Left on the edge returns to the slot list, and X provides an instant purchase shortcut with zero trap states."},
        {"CONTROLS", "Seamless Navigation Parity",
         "Refined D-pad and analog stick navigation boundaries across armor customization grids and slot categories with silky smooth cursor transitions."},
    };

    const std::vector<ChangeItem> v02016Items = {
        {"GAMEPLAY", "In-Match Multi-Team Tactical Selector",
         "Replaced the binary team toggle in the Pause Menu with a full tactical team selector modal supporting all 8 multiplayer teams with live color accents and squad badges."},
        {"ENGINE", "Native Team Synchronization & Instant Respawn",
         "Integrated instant engine team synchronization and clean biped respawn cycles when changing teams mid-match, updating spawn points, armor team colors, scoreboard rows, and voice chat channels."},
        {"FORGE", "Seamless Forge Team Switching",
         "Enabled instant team switching across all 8 teams in Forge without respawn interruption or simulation pause."},
        {"CONTROLS", "Tactical Team Menu Controller & Keyboard Parity",
         "Full D-Pad, thumbstick, face button, arrow key, and mouse navigation for selecting squads in-game."},
    };

    const std::vector<ChangeItem> v02015Items = {
        {"UI", "Party Widget & Lobby Player Card Overhaul",
         "Redesigned the top-right party card and lobby player rows into unified tactical cards, integrating rank badges, emblem frames, and status indicators with zero alignment jitter."},
        {"UI", "Full Nameplate Artwork Rendering",
         "Eliminated edge cropping on player nameplates, ensuring all custom banners, emblems, and insignia (such as Insider badges) render completely without cutoff."},
        {"ACCOUNT", "First-Launch Login Flow",
         "Streamlined startup onboarding so selecting Regular/Returning Player or New Player immediately opens the login menu to sign in or register."},
        {"ONLINE", "Server Browser & Online Play Gating",
         "Gated Server Browser and online multiplayer behind active logins, automatically defaulting sessions to local LAN play mode when signed out."},
    };

    const std::vector<ChangeItem> v02014Items = {
        {"MULTIPLAYER", "Extended 32-Player Matches",
         "Custom Games and Forge lobbies now support expanded matches up to 32 players with full roster and scoreboard tracking."},
        {"NETCODE", "High-Capacity Discovery & Sync",
         "Upgraded network beacon and server browser discovery to seamlessly discover, list, and sync high-capacity lobbies up to 32 players without truncation."},
        {"SERVER BROWSER", "Dynamic Capacity Filters",
         "Updated the Server Browser to accurately reflect live server capacity, allowing players to filter and join expanded community lobbies without capacity restriction warnings."},
        {"LOBBY", "Max Players Picker",
         "Host lobby settings now allow directly selecting up to 32 players for Custom Games and Forge sessions."},
        {"UI", "Loading Screen Multi-Column Roster",
         "Enhanced in-match loading screens with responsive multi-column roster formatting to cleanly present all players in expanded matches."},
    };

    const std::vector<ChangeItem> currentUiOverhaulItems = {
        {"UI", "Social Menu Overhaul",
         "Redesigned Friends, Requests, and Player Search into sleek tactical cards with prioritized Invite banners, live online/in-match presence, and instant Join/Invite/Remove actions."},
        {"UI", "Service Record & Ranks Overhaul",
         "Unified Service Record, Weekly Challenges, Commendations, and the 50-rank Progression Matrix into clean 2-column Reach tactical tiles with a pinned rank inspector."},
        {"ARMORY", "Armory Customization Overhaul",
         "Added two-pane Slot-to-Grid customization for Spartan, Elite, and Marine gear with 3D model rotation, two-step preview/purchase, and a controller-friendly RGB & hue wheel color picker."},
        {"CONTROLLER", "Full Gamepad & M&K Support",
         "D-pad, analog sticks, LB/RB tabs, LT/RT mode filters, and Y/X contextual actions now navigate every panel cleanly alongside full mouse and keyboard support."},
        {"UI", "Update Changelog Popup",
         "Added this What's New briefing on first launch after updating, plus a Main Menu button so you can review update notes anytime."},
    };

    const std::vector<ChangeItem> v01994Items = {
        {"UI", "Futuristic Rounded Halo Styling",
         "Smoothed corners across panels, tiles, buttons, scrollbars, sliders, and toggles for a cleaner sci-fi Reach interface."},
        {"UI", "Interactive Scrollbars & Smooth List Scrolling",
         "Upgraded pre-game map grid, variant drawers, options, lobby roster, scoreboard, and list scrollbars for full mouse drag/wheel and right-stick scrolling without focus collisions or snap-back."},
        {"CONTROLLER", "Virtual Cursor Mode & Anti-Hop Navigation",
         "Added toggleable analog Controller UI Cursor (Settings > Controller or L3 click) plus dominant-axis stick filtering and unified navigation so scrolling never jumps windows or accidentally opens submenus."},
        {"STABILITY", "Post-Match UI Stability",
         "Cleared lingering match inputs, cursor state, and render-thread overlap when returning from a game so menus stay calm and responsive."},
        {"NETCODE", "Join-in-Progress & Reconnect Fixes",
         "Fixed connection stability for joining players during active matches, improved host relay broadcasts, added reconnect grace windows, and ensured host map variants download cleanly on join."},
    };

    const std::vector<ChangeItem> v01993Items = {
        {"UI", "Tile-Based Map & Gamemode Pickers",
         "Overhauled Multiplayer, Custom Games, Campaign, Firefight, Forge, and Modded map pickers with cover-art tiles, inline map variant lists, and duplicate-launch guards."},
        {"SERVER BROWSER", "Server Browser Redesign",
         "Modernized the live Server Browser with filter pills, controller-ready server tiles, live player counts, ping display, and fast host details."},
        {"NETCODE", "Direct Host Lobby Chat & Low-Bandwidth Traffic",
         "Optimized lobby chat and match traffic to connect directly with match hosts for lower latency and minimal backend bandwidth overhead."},
    };

    const std::vector<ChangeItem> v019100Items = {
        {"CAMERA", "Smooth Local 3rd-Person & Zoom Camera",
         "Fixed third-person camera and zoom jitter across variable frame rates, ensuring smooth zoom transitions, responsive camera adjustments, and controller zoom support."},
        {"CAMERA", "Per-Client Vehicle 1st-Person Camera",
         "Kept third-person and vehicle first-person cameras strictly local per player with seat validation so cockpit views stay locked without snapping."},
        {"UI", "Post-Match Control & Focus Stability",
         "Fixed match-end Game Over focus highlights jumping wildly, reset post-game button/input/countdown states between matches, and resolved pointer fighting between mouse and controller."},
        {"UI", "Smooth Screen Transitions & Menu Return",
         "Settled focus transitions cleanly when returning to the Pre-Game Lobby or Main Menu and added smooth fade transitions while clearing leftover pickers across screens."},
    };

    const std::vector<ChangeItem> v019101Items = {
        {"UI", "New In-Match Loading & Waiting Screen",
         "Campaign, Custom Games, Forge and Firefight now get an improved Waiting For Players screen with the lobby roster, per-player status, progress and mode label, displaying full map artwork over the tactical background."},
        {"SERVER BROWSER", "Mod Awareness & Workshop Integration",
         "Server cards now clearly indicate required mods, call out version differences, and provide direct in-app download prompts for missing Steam Workshop content."},
        {"UI", "Dynamic Loading Roster",
         "The loading roster dynamically scales from small lobbies up to high-capacity sessions with a compact multi-column grid and player count tracking."},
        {"CAMERA", "Camera Polish & Post-Match Controls",
         "Includes jitter-free third-person and vehicle first-person cameras, smooth zoom blending, and stable post-match focus handling."},
    };

    const std::vector<ChangeItem> v019102Items = {
        {"UI", "Collapsible Mod Sections in Map Pickers",
         "The Modded tabs now group maps by mod package with collapsible sections and map counts for easy navigation, fully supported by search."},
        {"MULTIPLAYER", "Campaign Max Players & Navigation Fixes",
         "Campaign lobbies now allow selecting player limits up to 16 players, search boxes are properly bounded on-screen, and text rendering glitches have been resolved."},
        {"VISORS", "Custom Visor Color Mapping",
         "Resolved color rendering inconsistencies on custom visor finishes so every helmet finish displays accurately in-game and in menus."},
        {"SETTINGS", "Aligned Game Options Sliders",
         "All camera sliders in Settings > Game Options share unified alignment and clear value columns with clean text truncation for long labels."},
        {"CAMERA", "Independent FOV Configuration",
         "Adjusting match Field of View in Settings no longer affects Armory preview cameras, ensuring clean character inspection."},
    };

    const std::vector<ChangeItem> v019103Items = {
        {"LOBBY", "Lobby Share Codes & Quick Join",
         "Hosts of public and private lobbies can generate shareable lobby invite codes with a one-click copy button, allowing friends to join directly from any menu."},
        {"UI", "Variant Drawer Search & Category Filters",
         "The expanded base-map and variant drawer now includes real-time search and category filters with variant count tracking for fast selection."},
        {"SERVER BROWSER", "Server Browser Refresh & Pinned Actions",
         "Server list refresh indicator is restored, and server action buttons are permanently pinned in the details footer for instant access."},
        {"UI", "Menu Navigation & Click Area Fixes",
         "Settings rows are now fully clickable across the entire row width, and notification hints are responsive."},
        {"CONTROLLER", "Toggle Crouch & Toggle Zoom Bindings",
         "Controller toggle crouch and mouse/keyboard toggle zoom options are now available in Settings > Controls."},
    };

    const std::vector<ChangeItem> v01999Items = {
        {"NETCODE", "Multi-Mode Netcode & Connection Hardening",
         "Enhanced network routing validation, improved game variant verification, and resolved join-in-progress disconnects across Campaign, Firefight, Forge, and Custom Games."},
        {"MODS", "Seamless In-App Workshop Downloader",
         "Steam Workshop mods now download entirely in-app with live transfer rate and progress tracking, displaying creator metadata and full descriptions."},
        {"STABILITY", "Crash Prevention & Match Settlement Fixes",
         "Fixed intermittent crashes during match completion, resolved Campaign intro transition issues, and restored accurate player latency indicators on scoreboards."},
        {"LOBBY", "Lobby Options & Map Selection Polish",
         "Fixed Firefight and Custom Game option editing inside the lobby, cleared stale mod flags when switching to official maps, and improved host lobby controls."},
    };

    const std::vector<ChangeItem> v0200Items = {
        {"ARMORY", "Elite Armor Permutations & Customization",
         "All Elite armor sets (Honor Guard, Counselor, Ranger, Schism) display in the Armory, render properly in matches, and replicate across multiplayer."},
        {"ARMORY", "Armor Abilities on Elites",
         "Equipped Armor Abilities visibly mount and animate correctly on Elite player models during matches."},
        {"VISORS", "Custom Visor Network Replication",
         "All custom visor colors render accurately in-match on both Spartans and Elites and synchronize across all players."},
        {"FORGE", "Forge AI Network Synchronization",
         "Fixed AI companion and enemy spawning in Forge mode for non-host players, ensuring synced positions and health."},
        {"UI", "Loading Screen & Pause Menu Polish",
         "Enhanced loading screen emblems and artwork framing, and redesigned the in-game pause menu with tactical challenges and smooth scrolling."},
    };

    const std::vector<ChangeItem> v0202Items = {
        {"SETTINGS", "Reliable Settings Persistence",
         "Upgraded configuration saving pipeline to prevent settings like volume and custom keybinds from reverting to defaults, with instant feedback if a save fails."},
        {"CONTROLLER", "Gamepad Apply in Settings",
         "Controller A button activates Apply with clear focus highlighting, and menus automatically refresh to prevent overwriting settings."},
        {"UI", "Post-Match Transition Polish",
         "Fixed post-game focus locks and menu navigation conflicts, ensuring smooth return to the main menu with mouse, keyboard, or controller."},
        {"STABILITY", "Post-Game Summary Crash Fix",
         "Resolved game-over crashes and scoreboard sync failures for guests upon mission completion, ensuring accurate challenge progress and rewards."},
        {"GAMEPLAY", "Match Completion Watchdog",
         "Ensured Campaign mission cutscenes transition cleanly into the post-match summary without skipping progression screens."},
    };

    const std::vector<ChangeItem> v0203Items = {
        {"PERFORMANCE", "Binary Footprint & Startup Speed",
         "Optimized application binary size and cleaned distribution packaging for faster startup and lower disk usage."},
        {"DIAGNOSTICS", "Streamlined Diagnostics",
         "Added lightweight diagnostic reporting and system info in Settings."},
    };

    const std::vector<ChangeItem> v0204Items = {
        {"PERFORMANCE", "Memory Footprint Optimization",
         "Reduced runtime memory overhead and improved asset caching across long multiplayer sessions."},
        {"DIAGNOSTICS", "Automated Crash Diagnostics",
         "Integrated lightweight automated crash diagnostics for faster issue identification."},
        {"STABILITY", "System Stability & Settings Polish",
         "Refined runtime error handling and diagnostic reporting across all game modes."},
    };

    const std::vector<ChangeItem> v0207Items = {
        {"NETCODE", "Firefight Join-in-Progress Stability",
         "Resolved early match disconnects and handshaking issues so late-joining players can join active Firefight and Custom Game sessions smoothly."},
        {"SETTINGS", "Settings Reset Protection",
         "Fixed an issue where user preferences could be reset on launch, ensuring all custom controls and audio settings remain intact."},
        {"MULTIPLAYER", "Lobby Roster & Scoreboard Visibility",
         "Loading screen roster now displays all joined players, service tags correctly display per player, and your local player is distinctly highlighted on scoreboards."},
        {"PERFORMANCE", "Thread Scheduling & CPU Priority",
         "Optimized background thread scheduling and CPU priority for smoother, stutter-free frame delivery."},
    };

    const std::vector<ChangeItem> v0208Items = {
        {"NETCODE", "Modded Custom Game Join-in-Progress",
         "Joining players automatically resolve host mod content on official maps, preventing content mismatch errors and disconnects."},
        {"NETCODE", "Session Slot Protection",
         "Ensured sessions accurately respect public slot availability and lobby locks, preventing connection errors when lobbies fill up."},
    };

    const std::vector<ChangeItem> v0209Items = {
        {"CAMERA", "3rd-Person Perspective & Dynamic Reticle",
         "Added third-person perspective with an accurate dynamic crosshair, smooth aiming adjustments, vehicle first-person mode, and field-of-view controls."},
        {"UI", "All-Mode Scoreboard Synchronization",
         "Scoreboards in Campaign, Multiplayer, Firefight, and Forge now display accurate player names, service tags, and mode-specific team standings."},
    };

    const std::vector<ChangeItem> v02010Items = {
        {"UI", "Post-Match Mouse & Keyboard Navigation",
         "Eliminated mouse cursor drift and unintended item selection on the post-match results screen, ensuring keyboard and mouse input remain stable and responsive."},
        {"UI", "Input Transition Guards",
         "Added click release guards when transitioning between matches and menus, preventing accidental button activations from in-game fire inputs."},
        {"MULTIPLAYER", "High-Capacity Match Stability",
         "Optimized session memory allocation and roster handling for high-capacity multiplayer and custom game sessions."},
    };

    const std::vector<ChangeItem> v02011Items = {
        {"NETCODE", "High-Capacity Session Architecture",
         "Upgraded internal network synchronization and session state handling to support expanded lobby capacities up to 32 players."},
        {"NETCODE", "Lobby Slot Assignment & Joining Reliability",
         "Improved player slot assignment and join request queuing to ensure reliable matchmaking in high-capacity games."},
        {"STABILITY", "Lobby Capacity Verification Suite",
         "Added automated verification checks for high-capacity sessions to guarantee network packet integrity and connection stability."},
    };

    const std::vector<ChangeItem> v02012Items = {
        {"CONTROLLER", "Full Directional Stick Navigation & Thresholds",
         "Unblocked Left Stick analog navigation across all menus, calibrated cardinal deadzones, and added keyboard arrow key parity so menus navigate cleanly with D-pad, sticks, and keys."},
        {"CONTROLLER", "Smooth Right-Stick Window Scrolling",
         "Enabled responsive proportional Right-Stick scrolling in Service Record, Ranks, Challenges, Settings, and Social menus alongside Trigger page scrolling."},
        {"UI", "Service Record & Menu Hover Snap-Back Fix",
         "Fixed controller tile selection snapping back to the mouse cursor; mouse hover now requires active physical pointer movement before updating menu focus."},
        {"UI", "Service Record Layout & Tactical Visual Polish",
         "Redesigned the tab bar with native bumper prompts ([LB] / [RB]), flattened Ranks into a clean single-container scroll view, eliminated double scrollbars, and improved grid spacing."},
        {"UI", "Universal Controller Button Activation",
         "Wired Controller 'A' button to select, activate, and inspect tiles, cards, and dropdown items across Service Record, Challenges, Commendations, and Social menus."},
    };

    if (currentVer != "v0.19.93" && currentVer != "v0.19.94") {
        if (currentVer != "v0.19.99") {
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 21) {
                notes.push_back(ReleaseNotes{
                    currentVer,
                    "MATCH-START AUDIO SYNC & CAMPAIGN JOIN-IN-PROGRESS BLACK SCREEN FIX",
                    "Immediate Game Audio on Launch, JIP Black Screen Elimination & Seamless Mid-Match Spawning",
                    v02021Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 19) {
                notes.push_back(ReleaseNotes{
                    "v0.20.19",
                    "MULTIPLAYER CUSTOMIZATION ISOLATION & PARITY",
                    "Zero Host-to-Client Color Pollution, Dedicated Peer Appearance Records & Canonical Local Mapping",
                    v02019Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 18) {
                notes.push_back(ReleaseNotes{
                    "v0.20.18",
                    "IN-MATCH VISOR & LIGHTS REPLICATION, 3P RETICLE POLISH",
                    "30-Finish Visor Replication, Armor Lights Sync, Elite Customization Netcode & Snappy 3P Aiming",
                    v02018Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 17) {
                notes.push_back(ReleaseNotes{
                    "v0.20.17",
                    "3RD-PERSON RETICLE SMOOTHING & INTUITIVE ARMORY CONTROLS",
                    "Smooth Reticle Tracking, Zero Walk-Cycle Jitter, Armory Grid Flow & Controller Purchase Shortcuts",
                    v02017Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 16) {
                notes.push_back(ReleaseNotes{
                    "v0.20.16",
                    "IN-MATCH MULTI-TEAM SELECTION & SEAMLESS TEAM SWITCHING",
                    "8-Team Tactical Selector Modal, Native Team Datum Sync, Instant Respawn & Forge Parity",
                    v02016Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 15) {
                notes.push_back(ReleaseNotes{
                    "v0.20.15",
                    "EXTENDED PARTY CARDS, NAMEPLATE FIXES & ONLINE LOGIN FLOW",
                    "Unified Lobby Player Cards, Zero Nameplate Cutoff, First-Start Login & Auto Local Play Mode",
                    v02015Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 14) {
                notes.push_back(ReleaseNotes{
                    "v0.20.14",
                    "EXTENDED 32-PLAYER LOBBIES, HIGH-CAPACITY NETCODE & BROWSER FILTERS",
                    "32-Player Custom Games & Forge, Widened Network Discovery, Dynamic Browser Filters & Multi-Column Rosters",
                    v02014Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 13) {
                notes.push_back(ReleaseNotes{
                    "v0.20.13",
                    "CONTROLLER NAVIGATION, SMOOTH SCROLLING & SERVICE RECORD POLISH",
                    "Directional Stick Nav, Right-Stick Scrolling, Zero Mouse Snap-Back & UI Visual Polish",
                    v02012Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 12) {
                notes.push_back(ReleaseNotes{
                    "v0.20.12",
                    "CONTROLLER NAVIGATION, SMOOTH SCROLLING & SERVICE RECORD POLISH",
                    "Directional Stick Nav, Right-Stick Scrolling, Zero Mouse Snap-Back & UI Visual Polish",
                    v02012Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 11) {
                notes.push_back(ReleaseNotes{
                    "v0.20.11",
                    "HIGH-CAPACITY SESSION NETCODE & LOBBY EXPANSION",
                    "High-Capacity Network Sync, Reliable Slot Assignment & Lobby Stability",
                    v02011Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 10) {
                notes.push_back(ReleaseNotes{
                    "v0.20.10",
                    "POST-MATCH INPUT STABILIZATION & HIGH-CAPACITY LOBBIES",
                    "Keyboard & Mouse Focus Fixes, Input Leak Suppression & 32-Player Session Support",
                    v02010Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 9) {
                notes.push_back(ReleaseNotes{
                    "v0.20.9",
                    "3RD-PERSON CAMERA, DYNAMIC RETICLE & SCOREBOARD SYNC",
                    "3rd-Person Camera, Dynamic Reticle, Seated Perspectives & Mode-Correct Scoreboards",
                    v0209Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 8) {
                notes.push_back(ReleaseNotes{
                    "v0.20.8",
                    "MODDED CUSTOM GAMES JOIN-IN-PROGRESS & SESSION LOCKS",
                    "Stock-Map Content Mod Resolution, Session Lock Gating & Join-in-Progress Polish",
                    v0208Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 7) {
                notes.push_back(ReleaseNotes{
                    "v0.20.7",
                    "JOIN-IN-PROGRESS STABILITY, SETTINGS MIGRATION FIX & PERFORMANCE",
                    "Smooth Match Handshakes, Settings Reset Prevention, Loading Roster & CPU Optimizations",
                    v0207Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 4) {
                notes.push_back(ReleaseNotes{
                    "v0.20.4",
                    "CLIENT PERFORMANCE, MEMORY EFFICIENCY & CRASH DIAGNOSTICS",
                    "Advanced Binary Code Optimization, Memory Overhead Reduction & Streamlined Diagnostics",
                    v0204Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 3) {
                notes.push_back(ReleaseNotes{
                    "v0.20.3",
                    "BINARY OPTIMIZATION & DIAGNOSTICS POLISH",
                    "Executable Footprint Optimization, Package Cleanup & Direct Settings Diagnostics",
                    v0203Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 2) {
                notes.push_back(ReleaseNotes{
                    "v0.20.2",
                    "SETTINGS SAVE FIXES, END-MATCH UI & POST-GAME CRASH FIXES",
                    "Settings File Versioning, Controller Apply, Post-Match Report Stability & Non-Host Crash Fixes",
                    v0202Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR >= 20) {
                notes.push_back(ReleaseNotes{
                    "v0.20.0",
                    "ELITE ARMOR, CUSTOM VISORS, FORGE AI & UI OVERHAUL",
                    "Elite Sets, Armor Abilities, Custom Visor Colors, Forge AI Sync, Pause Menu & Loading Polish",
                    v0200Items,
                });
            }
            if (noble::VERSION_MINOR == 19 && noble::VERSION_PATCH >= 103) {
                notes.push_back(ReleaseNotes{
                    "v0.19.103",
                    "LOBBY SHARE CODES, CLICK FIXES & INPUT TOGGLES",
                    "Shareable Lobby Codes, Variant Drawer Filters, Pinned Server Actions, Toggle Crouch/Zoom",
                    v019103Items,
                });
            }
            notes.push_back(ReleaseNotes{
                "v0.19.102",
                "MAP PICKERS, VISOR & GAME OPTIONS FIXES",
                "Collapsed Mod Sections, Campaign Max Players, Visor Colors, Aligned Options Sliders",
                v019102Items,
            });
        }
        notes.push_back(ReleaseNotes{
            "v0.19.99",
            "ALL-MODE NETCODE SYNC, STABILITY & IN-APP WORKSHOP",
            "Reliable Network Routing, Variant Verification & Smooth Workshop Downloads",
            v01999Items,
        });
    }

    if (currentVer != "v0.19.93") {
        notes.push_back(ReleaseNotes{
            "v0.19.94",
            "FUTURISTIC UI POLISH, SCROLLBARS & VIRTUAL CURSOR",
            "Rounded Hi-Tech Panels, Fixed Scrolling & Controller Cursor Mode",
            v01994Items,
        });
        notes.push_back(ReleaseNotes{
            "v0.19.93",
            "UI OVERHAUL, MAP PICKERS & NETCODE",
            "Social, Service Record, Armory, Map Pickers & Direct Lobby Netcode",
            [&]() {
                std::vector<ChangeItem> merged = currentUiOverhaulItems;
                merged.insert(merged.end(), v01993Items.begin(), v01993Items.end());
                return merged;
            }(),
        });
    } else {
        std::vector<ChangeItem> merged = currentUiOverhaulItems;
        merged.insert(merged.end(), v01993Items.begin(), v01993Items.end());
        notes.push_back(ReleaseNotes{
            "v0.19.93",
            "UI OVERHAUL, MAP PICKERS & NETCODE",
            "Social, Service Record, Armory, Map Pickers & Direct Lobby Netcode",
            std::move(merged),
        });
    }

    notes.push_back(ReleaseNotes{
        "v0.19.92",
        "UNIFIED CONTROLLER & MENU OVERHAUL",
        "Main Menu, Pre-Game Lobby, Settings, Pause Menu & Scoreboard",
        {
            {"CONTROLLER", "Unified D-Pad & Stick Navigation",
             "Standardized D-pad navigation, left/right stick scrolling, [A] Select, and [B] Back across Main Menu, Lobby, Settings, Pause, Match-End, and Scoreboard."},
            {"UI", "Zero Overlap & Stale Layer Cleanup",
             "Eliminated leftover modal inputs, double-activation button hits, and overlapping widgets for responsive low-latency menus."},
        },
    });

    notes.push_back(ReleaseNotes{
        "v0.19.91",
        "THEATER MODE & PLAYBACK CONTROLS",
        "Full Keyboard, Mouse & Controller Pause Access",
        {
            {"FIX", "Theater Pause Menu for M&K and Gamepad",
             "Fixed Theater Mode pause menu access so Esc/Start and keyboard/mouse controls open pause and playback controls reliably alongside controllers."},
        },
    });

    return notes;
}

} // namespace NobleChangelog