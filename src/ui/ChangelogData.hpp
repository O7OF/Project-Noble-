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

    const std::vector<ChangeItem> currentUiOverhaulItems = {
        {"UI", "Social Menu Overhaul",
         "Redesigned Friends, Requests, and Player Search into sleek tactical cards with prioritized Invite banners, live online/in-match presence, and instant Join/Invite/Remove actions."},
        {"UI", "Service Record & Ranks Overhaul",
         "Unified Service Record, Weekly Challenges, Commendations, and the 50-rank Progression Matrix into clean 2-column Reach tactical tiles with a pinned rank inspector."},
        {"UI", "Armory Overhaul",
         "Added two-pane Slot-to-Grid customization for Spartan, Elite, and Marine gear with 3D model rotation, two-step preview/purchase, and a controller-friendly RGB & hue wheel color picker."},
        {"CONTROLLER", "Full Gamepad & M&K Support",
         "D-pad, analog sticks, LB/RB tabs, LT/RT mode filters, and Y/X contextual actions now navigate every panel cleanly alongside full mouse and keyboard support."},
        {"FEATURE", "Update Changelog Popup",
         "Added this What's New briefing on first launch after updating, plus a Main Menu button so you can review update notes anytime."},
    };

    const std::vector<ChangeItem> v01994Items = {
        {"UI", "Futuristic Rounded Halo Styling",
         "Smoothed corners across panels, tiles, buttons, scrollbars, sliders, and toggles for a cleaner sci-fi Reach interface."},
        {"FIX", "Interactive Scrollbars & Smooth List Scrolling",
         "Upgraded pre-game map grid, variant drawers, options, lobby roster, scoreboard, and list scrollbars for full mouse drag/wheel and right-stick scrolling without focus collisions or snap-back."},
        {"CONTROLLER", "Virtual Cursor Mode & Anti-Hop Navigation",
         "Added toggleable analog Controller UI Cursor (Settings > Controller or L3 click) plus dominant-axis stick filtering and unified UiPad navigation so scrolling never hops windows or accidentally opens submenus."},
        {"FIX", "Post-Match UI Stability",
         "Cleared lingering match inputs, cursor state, and render-thread overlap when returning from a game so menus stay calm and responsive."},
        {"NETCODE", "Join-in-Progress & Anti-Boot Fixes",
         "Prevented engine peer boots on active transport sessions, fixed host relay broadcasts/deduplication for joining clients, added reconnect grace windows, and materialized host map variants cleanly on join."},
    };

    const std::vector<ChangeItem> v01993Items = {
        {"UI", "Tile-Based Map & Gamemode Pickers",
         "Overhauled Multiplayer, Custom Games, Campaign, Firefight, Forge, and Modded map pickers with cover-art tiles, inline map variant lists, and duplicate-launch guards."},
        {"UI", "Server Browser Redesign",
         "Modernized the live Server Browser with filter pills, controller-ready server tiles, live player counts, ping display, and fast host details."},
        {"NETCODE", "Host-Driven Lobby Chat & Low-Bandwidth Netcode",
         "Moved lobby chat and match traffic to the host's direct connection so backend servers handle matchmaking and accounts with minimal bandwidth and disk overhead."},
    };

    const std::vector<ChangeItem> v019100Items = {
        {"FIX", "Smooth Local 3rd-Person & Zoom Camera",
         "Fixed third-person camera and zoom jitter across variable/sub-60 FPS frames by removing Present-frame skips, switching zoom interpolation to smooth QPC timing, caching zoom settings, and forwarding controller zoom state."},
        {"FIX", "Per-Client Local Vehicle 1st-Person Camera",
         "Kept third-person and vehicle first-person cameras strictly local per client with unit/seat validation and finite coordinate guards so cockpit cameras stay locked without snapping."},
        {"UI", "Post-Match Control & Focus Stability",
         "Fixed match-end Game Over focus highlights jumping wildly, reset post-game button/input/countdown states between matches, and ordered Virtual Cursor before physical mouse input to stop pointer fighting across M&K and controller."},
        {"UI", "Smooth Screen Transitions & Menu Return",
         "Settled focus transitions cleanly when returning to the Pre-Game Lobby or Main Menu and added smooth fade transitions while clearing leftover pickers across screens."},
    };

    const std::vector<ChangeItem> v019101Items = {
        {"UI", "New In-Match Loading & Waiting Screen",
         "Campaign, Custom Games, Forge and Firefight now get a WAITING FOR PLAYERS screen with the lobby roster, per-player status, progress and mode label; the map picture is shown complete (no more zoomed stretch) over Reach's moving diamond grid with the Noble and UNSC emblems. Hosts waiting for remote players keep this screen instead of a black frame."},
        {"UI", "Server Browser Blue-Glass Layout & Mod Awareness",
         "Larger blue-glass server cards are now the default. Modded sessions show MOD / MOD - NOT INSTALLED, the details pane names the required Workshop item, version differences are called out, and DOWNLOAD MOD & JOIN opens the Workshop download prompt. Warnings wrap inside the pane."},
        {"UI", "Lobby-Sized Loading Roster",
         "The loading roster scales from small lobbies to 16 / 32 / 64 / 128-player sessions with a compact multi-column grid and a joined/expected count."},
        {"FIX", "Smooth Local Cameras & Match-End UI",
         "Includes the v0.19.100 work: jitter-free third-person and vehicle first-person cameras, smooth QPC zoom blending, controller zoom, and stable post-match focus/cursor handling."},
    };

    const std::vector<ChangeItem> v019102Items = {
        {"UI", "Collapsed Mod Sections & Fold in Map Pickers",
         "The MODDED tabs (Campaign, Custom Game, Firefight, lobby map select) list one compact section per mod with its map count; click to expand, click the mod heading (FOLD) to collapse. Search still expands and filters everything."},
        {"UI", "Campaign MAX PLAYERS & Search Fix",
         "Campaign now has a MAX PLAYERS row (4/8/12/16) wired into HOST CO-OP instead of a hard-coded 16, the campaign search box is clamped on-screen, and the expanded-map header no longer shows a '?' placeholder for the missing em-dash glyph."},
        {"FIX", "Extended Visor Finishes No Longer White",
         "haloreach.dll only has five real visor slots; the extra Armory finishes were stored as out-of-range slots and drew white. Every finish now maps to a valid stock slot (nearest colour) - the Armory still previews the exact finish."},
        {"FIX", "Game Options Sliders Aligned",
         "All camera sliders in SETTINGS > GAME OPTIONS share one start, length and value column (they used to shrink by their value text width), and long labels are ellipsised instead of running under the track."},
        {"FIX", "Front-End FOV No Longer Zooms the Armory",
         "Saving a FOV change from the front-end no longer writes the match FOV into the Armory's engine camera; it is applied when a match starts, and in-match changes still apply live."},
    };

    const std::vector<ChangeItem> v019103Items = {
        {"FEATURE", "Lobby Share Codes & Join with Code",
         "Hosts of public and private lobbies get a 23-character NB1-XXXX-XXXX-XXXX-XXXX code with COPY CODE in the HOSTING panel; join from Main Menu, the Server Browser, or the Social screen. Private lobbies stay unlisted but the code still joins them."},
        {"UI", "Variant Drawer Search & Category Filters",
         "The expanded base-map/variant drawer now has a search box and a category cycler with an N OF M VARIANTS count, so 353-variant Forge World AI Battlegrounds collapses to the matching entries instead of a long scroll."},
        {"FIX", "Server Browser Refresh & Pinned Actions",
         "Refresh shows its working status again, and FAVOURITE / COPY INVITE are pinned in a footer under the details pane so they stay visible and clickable."},
        {"FIX", "Click Fixes in Menus & Options",
         "Game Options rows are clickable anywhere on the row again, and the main-menu What's New hint is clickable."},
        {"CONTROLLER", "Toggle Crouch & Toggle Zoom Bindings",
         "Controller toggle crouch and mouse/keyboard toggle zoom are now available in Settings > Controls."},
    };

    const std::vector<ChangeItem> v01999Items = {
        {"NETCODE", "All-Mode Netcode & Sync Hardening",
         "Verified bidirectional UDP routes before skipping relay, added Blam! membership handshake waits before host launch, SHA-256 verified preset JSON/binary game variants and .mvar map variants, and fixed join-in-progress kicks across Campaign, Firefight, Forge, and Custom Games."},
        {"FEATURE", "Smooth In-App Workshop Downloader",
         "Downloading Steam Workshop mods now subscribes and downloads in-app with a live progress bar (downloaded / total, MB/s, ETA, stage), full mod metadata, creator name, tags, and scrollable description without popping open Steam."},
        {"FIX", "Engine Hook Register Safety & Stability",
         "Preserved volatile CPU/SSE registers across GameFinish, IsWhitespace, and VariantSource detours, fixed UI null-slot guard jumps, fixed Campaign intro black-screens, and restored real per-player ping IDs on the scoreboard."},
        {"FIX", "Mode Options, Map Picker & Lobby Integrity",
         "Fixed Firefight & Custom Games option editing inside the lobby, cleared stale Workshop mod flags when switching back to stock maps, and hardened host-only WebSocket controls and leave cleanup on the master server."},
    };

    const std::vector<ChangeItem> v0200Items = {
        {"ARMORY", "Elite Armor Pieces & Full Customization",
         "All Elite armor sets and pieces (Honor Guard, Counselor, H2A Ranger, H2A armor, Schism) now display in the Armory, render properly in-match for whoever equips them, and sync cleanly across multiplayer."},
        {"ARMORY", "Armor Abilities on Elites",
         "Equipped Armor Abilities (Jet Pack, Evade, Drop Shield, Armor Lock, Active Camo, Hologram, Sprint) now visibly display and mount correctly on Elites during matches."},
        {"VISORS", "Custom Visor Colors In-Match & Over Network",
         "All custom and preset visor colors render accurately in-match on both Spartans and Elites and replicate over the network across multiplayer clients."},
        {"FORGE", "Forge AI Spawning Fix & Net Sync",
         "Fixed non-host clients spawning duplicate or static invincible AI clones in Forge; AI spawning is now authoritatively handled with clean network synchronization."},
        {"UI", "Loading Screen & Pause Menu Polish",
         "Restored NOBLE and UNSC emblems and diamond grid on loading screens with no overlap, overhauled the in-game pause menu with sleek tactical framing, smooth scrollwheel navigation, and Service Record-matched Challenges."},
    };

    const std::vector<ChangeItem> v02010Items = {
        {"UI", "Post-Match Keyboard & Mouse Focus Stabilization",
         "Eliminated mouse hover jitter and unintended selection triggers on the post-match results panel and main menu by adding intentional mouse motion deadzone filtering and preventing sub-pixel sensor noise from fighting keyboard navigation."},
        {"UI", "Post-Match Input Settling & Click Leak Prevention",
         "Introduced transition settle periods and physical mouse button release guards when returning from a game, preventing gameplay firing inputs or reveal skip clicks from accidentally activating post-game menu buttons."},
        {"ENGINE", "High-Capacity Session Slot Management",
         "Optimized 32-player peer seat structure math and memory alignment safeguards for high-capacity multiplayer and custom game sessions."},
    };

    const std::vector<ChangeItem> v0209Items = {
        {"CAMERA", "3rd-Person Camera & Dynamic Reticle",
         "Implemented engine-level 3rd-person camera hooks with dynamic reticle screen projection, aiming raycasts, vehicle first-person support, and smooth field-of-view interpolation."},
        {"UI", "All-Mode Roster & Scoreboard Sync",
         "Multiplayer, Campaign, Firefight, and Forge scoreboards now seed real player names and tags from the launch roster, correctly identifying local players and displaying mode-accurate team columns."},
    };

    const std::vector<ChangeItem> v0208Items = {
        {"NETCODE", "Modded Custom Games Join-in-Progress",
         "Guests now resolve host mod content overrides on stock maps and launch matches with their matching local mod installed, preventing content mismatch aborts and desyncs."},
        {"NETCODE", "Join-Lock Gating & Slot Parity",
         "Refusal overrides are now cleanly gated on public slot availability (respecting the engine's launch join-locks) preventing guests from being forced into locked sessions."},
    };

    const std::vector<ChangeItem> v0207Items = {
        {"NETCODE", "Firefight & Match Join-in-Progress",
         "Suppressed premature match aborts (reason 6) and dropped join-in-progress refusal gates during active host sessions so late-joining players smoothly complete handshakes without kicking guests back to the main menu."},
        {"SETTINGS", "Settings Reset & Startup Migration Fix",
         "Prevented settings resets caused by startup mirror sync overwriting saved settings.json with stale defaults; migration now only executes on fresh installs when no existing configuration is present."},
        {"MULTIPLAYER", "Lobby Roster & Scoreboard Visibility",
         "Guests now see the full player roster on loading screens, only the local client is tagged 'YOU' on the scoreboard, service tags are accurately mapped per player, and invite connections display the host's actual profile name."},
        {"PERFORMANCE", "CPU Priority & Thread Scheduling",
         "Disabled OS power throttling, elevated render thread priority, and optimized background power state handling for smoother frame delivery on multi-core processors."},
    };

    const std::vector<ChangeItem> v0204Items = {
        {"PERFORMANCE", "Binary Optimization & Load Times",
         "Applied advanced link-time code folding and footprint optimization for reduced memory overhead and faster client startup."},
        {"DIAGNOSTICS", "Streamlined Crash Reporting",
         "Enhanced standalone minidump generation (noble-crash.dmp) for fast, lightweight crash capture without disk logging overhead."},
        {"STABILITY", "Settings & Runtime Polish",
         "Refined settings diagnostics and status reporting for streamlined troubleshooting across all game modes."},
    };

    const std::vector<ChangeItem> v0203Items = {
        {"OPTIMIZATION", "Binary Footprint & Code Efficiency",
         "Optimized executable code generation and removed auxiliary build files from distribution packages."},
        {"DIAGNOSTICS", "Streamlined Crash & Diagnostics",
         "Lightweight minidump handling and direct Settings diagnostics reporting."},
    };

    const std::vector<ChangeItem> v0202Items = {
        {"SETTINGS", "Settings Menu Save Fix & Diagnostics",
         "Added settingsVersion 2 gating so music volume (50%) and custom paddle keys never revert to defaults on load; hardened save pipeline with per-process temp files (settings.json.<pid>.tmp), MoveFileExW replace, in-place fallbacks, on-screen failure warnings, and full save diagnostics."},
        {"CONTROLLER", "Controller Apply in Settings Panel",
         "Gamepad 'A' button now directly activates APPLY when focused on the bottom action box with full focus-ring highlighting, and the panel automatically reloads from disk on re-open to prevent overwriting settings with stale cached edits."},
        {"UI", "End-Match UI & Transition Polish",
         "Fixed end-of-match UI focus losing control, eliminated erratic option highlights and pointer fighting upon game over, ensured smooth transitions back to the main menu with full keyboard, mouse, and controller control."},
        {"CRASH FIX", "Non-Host After-Action Report Crash Fix",
         "Unified post-game match settlement for hosts and guests, eliminated double-payouts and scoreboard misidentifications, fixed guest challenge/commendation calculations, and eliminated non-host post-match report crashes and black screens upon mission completion."},
        {"ENGINE", "Match-Over Lifecycle & Watchdog",
         "Hardened engine cutscene transitions so campaign post-mission cinematics no longer swallow game-over reports, and added watchdog detection for unreported match ends to reliably surface the post-match summary."},
    };

    if (currentVer != "v0.19.93" && currentVer != "v0.19.94") {
        if (currentVer != "v0.19.99") {
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 10) {
                notes.push_back(ReleaseNotes{
                    currentVer,
                    "POST-MATCH INPUT STABILIZATION & HIGH-SLOT SESSION EXPANSION",
                    "Keyboard & Mouse Focus Fixes, Input Leak Suppression & 32-Player Peer Seat Math",
                    v02010Items,
                });
            }
            if (noble::VERSION_MAJOR == 0 && noble::VERSION_MINOR == 20 && noble::VERSION_PATCH >= 9) {
                notes.push_back(ReleaseNotes{
                    "v0.20.9",
                    "3RD-PERSON CAMERA, DYNAMIC RETICLE & SCOREBOARD SYNC",
                    "3rd-Person Engine Hooks, Raycasted Dynamic Reticle, Seated Cameras & Mode-Correct Scoreboards",
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
                    "Advanced Binary Code Folding, Memory Overhead Reduction & Streamlined Diagnostics",
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
                "Collapsed Mod Sections, Campaign MAX PLAYERS, Visor Slots, Aligned Options Sliders",
                v019102Items,
            });
        }
        notes.push_back(ReleaseNotes{
            "v0.19.99",
            "ALL-MODE NETCODE SYNC, STABILITY & IN-APP WORKSHOP",
            "Bidirectional UDP Guards, Variant/Map Verification & Smooth Workshop Downloads",
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
