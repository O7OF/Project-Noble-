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

    if (currentVer != "v0.19.93" && currentVer != "v0.19.94") {
        if (currentVer != "v0.19.99") {
            notes.push_back(ReleaseNotes{
                currentVer,
                "SMOOTH LOCAL CAMERAS, POST-MATCH UI CONTROL & TRANSITIONS",
                "Jitter-Free 3rd-Person/Vehicle Cameras & Clean Match-End UI Focus",
                v019100Items,
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
