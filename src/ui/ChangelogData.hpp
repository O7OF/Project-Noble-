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

    if (currentVer != "v0.19.93") {
        if (currentVer == "v0.19.94") {
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
            notes.push_back(ReleaseNotes{
                currentVer,
                "FUTURISTIC UI POLISH, SCROLLBARS & VIRTUAL CURSOR",
                "Rounded Hi-Tech Panels, Fixed Scrolling & Controller Cursor Mode",
                [&]() {
                    std::vector<ChangeItem> merged = v01994Items;
                    merged.insert(merged.end(), currentUiOverhaulItems.begin(), currentUiOverhaulItems.end());
                    return merged;
                }(),
            });
            notes.push_back(ReleaseNotes{
                "v0.19.93",
                "MAP PICKERS, SERVER BROWSER & NETCODE",
                "Tile-Based Pickers & Direct Host Lobby Networking",
                v01993Items,
            });
        }
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
