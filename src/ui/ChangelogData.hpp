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

    const std::vector<ChangeItem> v02037Items = {
        {"UI", "Pre-Game Lobby Redesign",
         "Reorganized pre-game lobby options into MATCH SETUP and LOBBY SETTINGS tabs for hosts. The Start button is placed directly at Row 0 for instant access."},
        {"UI", "Zero Overlap Layout",
         "Strictly bounded and clipped option viewports so settings can never collide or overlap with the Friends list or Hosting P2P invite code cards."},
        {"CONTROLLER", "Scroll & Navigation",
         "Added smooth mouse-wheel, trigger (LT/RT), and right-stick scrolling with a tactical scrollbar thumb. B/Esc returns cleanly from Lobby Settings back to Match Setup."}
    };

    notes.push_back({
        "v0.20.37",
        "Lobby UI Redesign & Layout Cleanup",
        "Clean tabbed pre-game lobby options, zero card overlap, and smooth scrolling",
        v02037Items
    });

    const std::vector<ChangeItem> v02036Items = {
        {"FORGE", "Forge Menu Names",
         "The Pelican, Spectre, Boathog and orbital beacons now have names in the Forge menu (for example Needler Spectre, Gauss Boathog, MAC Round and Spirit Drop: Wraith) instead of blank rows. Update Noble Forge World in MODS > RECOMMENDED MAPS to get them."},
        {"MODS", "Forge World Updates",
         "MODS > RECOMMENDED MAPS now checks whether your Noble Forge World is the newest one. It shows Up to date, or UPDATE AVAILABLE with an UPDATE button that downloads the new map. CHECK FOR UPDATES checks again."},
        {"FIXES", "Newest Forge World Pack",
         "DOWNLOAD in v0.20.35 could fetch an older Forge World pack without AI paths. It now always gets the newest pack, and an older Forge World no longer shows as Installed. If PLACE AI says the map has no AI paths, it tells you where to update."}
    };

    notes.push_back({
        "v0.20.36",
        "Forge World Updates",
        "Forge menu names for the new vehicles and beacons, and Forge World updates from MODS",
        v02036Items
    });

    const std::vector<ChangeItem> v02035Items = {
        {"FORGE", "AI Can Walk Forge World",
         "The Noble Forge World map now has AI paths across the whole map, so Elites, Grunts, Jackals, Brutes and Troopers walk around, chase players and fight. Download the new Forge World pack: the old one has no AI paths."},
        {"HOST TOOLS", "Map AI",
         "New host option in the custom games and Forge lobbies. Turn on MAP AI and AI squads join the game 15 seconds in and come back after they have all been killed. It only works on maps with AI paths (the new Forge World)."},
        {"FORGE", "Place AI in Forge",
         "In Forge, the host's match menu has PLACE AI and AI TYPE. Pick Elites, Grunts, Jackals, Brutes or Troopers and place a squad on the ground in front of you."},
        {"AI", "Who the AI Fight",
         "AI fight everyone: players, and Troopers against the Covenant. Everyone in the game sees and fights the host's AI. AI walk on the map's ground and on Forge pieces that sit on the ground; they don't climb onto raised Forge pieces yet."}
    };

    notes.push_back({
        "v0.20.35",
        "AI on Forge World",
        "Map AI, Place AI in Forge and a Forge World with AI paths",
        v02035Items
    });

    const std::vector<ChangeItem> v02034Items = {
        {"FIREFIGHT", "Extra Enemies",
         "New host option in the Firefight lobby: add 2, 4, 6 or 8 extra Covenant squads to every wave. A wave only ends when they are dead too. On Courtyard the first wave goes from 28 enemies to 36 with +4 squads and 48 with +8."},
        {"FIREFIGHT", "Wave Pacing",
         "Hosts can make the breaks between waves, rounds and sets shorter (half as long) or longer (one and a half or two times as long)."},
        {"FIREFIGHT", "Wave Squads, Dropships and Bonus Round in Game Options",
         "GAME OPTIONS for Firefight now has a Firefight Waves group: enemy squads in the opening, main and boss waves and in the bonus round, dropships on or off, and the bonus round length."},
        {"FIREFIGHT", "Allied Elites",
         "Optional host setting: up to 5 squads of Elites fight on your side, follow the players and come back after they have all died."},
        {"HOST TOOLS", "Votes",
         "Players can call a vote to kick a player, end the round or end the game. Hosts choose which votes are allowed and can turn voting off."},
        {"HOST TOOLS", "Next Map Vote",
         "After a game, players can vote on the next map from the lobby."},
        {"HOST TOOLS", "Team Controls",
         "Hosts can balance or shuffle the teams, and lock them so players can't switch."},
        {"HOST TOOLS", "Restart With Everyone",
         "Hosts can restart a multiplayer game and bring every player back with them."},
        {"HOST TOOLS", "Friendly Fire, Betrayal Booting and Team Changing",
         "These social settings are now in GAME OPTIONS for custom games."},
        {"LOBBY", "Join in Progress",
         "Hosts can stop players from joining a game that is already running."},
        {"LOBBY", "Lobby Rules",
         "Hosts can write a short set of rules that every player sees at the top of the lobby."},
        {"LOBBY", "Reorder Playlists",
         "Playlist entries can be moved up and down with LB / RB or Ctrl + Up / Down."},
        {"HOST TOOLS", "Disband From the Match",
         "The host's in-match menu now has DISBAND LOBBY."}
    };

    notes.push_back({
        "v0.20.34",
        "Bigger Firefight & More Host Tools",
        "Extra Enemies, Wave Pacing, Votes, Team Controls and Join in Progress",
        v02034Items
    });

    const std::vector<ChangeItem> v02033Items = {
        {"HOST TOOLS", "Campaign Host Menu",
         "Hosts get new rows in the pause menu: restart the mission, go to the next mission, revert to the last checkpoint, pause the game for everyone, and return to the lobby with your players or on your own."},
        {"HOST TOOLS", "Kick and Ban Players",
         "Hosts can kick a player (they can't rejoin for 2 minutes) or ban them for 30 minutes, 1 hour, 24 hours or for good. Bans follow the player's account, so a new name doesn't get around them. The lobby shows your ban list, and you can lift a ban."},
        {"HOST TOOLS", "Mute for Everyone and Co-hosts",
         "Hosts can mute a player's voice and text chat for the whole game, and make a player a co-host who can kick and mute too."},
        {"LOBBY", "Lock Your Lobby or Add a Password",
         "New JOINS option: open, invite only, password or locked. Players already in your lobby always stay, also after a match, and friends you invite can still get in. Password lobbies ask for the password in the server browser."},
        {"LOBBY", "Anti-AFK",
         "Hosts can remove players who stop playing after 3, 5 or 10 minutes. Players get a 30 second warning, and cutscenes and loading screens don't count."},
        {"CHAT", "Mute Someone's Chat for Yourself",
         "Every player can now hide one player's chat messages on their own screen, from the lobby or the scoreboard."},
        {"CHAT", "Word Filter",
         "Hosts can hide slurs in chat, plus their own list of words. It is off by default; the first-time setup asks if you want it."},
        {"SERVER BROWSER", "Clean Server Names",
         "Server names no longer show slurs or swear words, also when spelled with numbers, symbols or other spellings. Only whole words are hidden, so normal names like Assistance or Class stay the same."},
        {"CAREER", "No Credits for Very Short Games",
         "Leaving or restarting a game in under 5 minutes no longer pays credits, challenges or commendations. Games that end normally pay as before."},
        {"CAMPAIGN", "Restarts Wait for Everyone",
         "When the host restarts a campaign mission or Firefight game, the lobby now waits for every player to come back before starting again."},
        {"FIX", "Disbanding a LAN Lobby",
         "Players in LAN and direct games now leave properly when the host disbands the lobby."},
        {"DEDICATED SERVER", "Kick and Ban Commands",
         "The server console now has working players, kick, ban, bans and unban commands."}
    };

    notes.push_back({
        "v0.20.33",
        "Host Tools",
        "Kick, Ban, Mute, Lock Your Lobby and Campaign Host Options",
        v02033Items
    });

    const std::vector<ChangeItem> v02032Items = {
        {"CAMPAIGN", "Up to 8 Players in Campaign Co-op",
         "When hosting a campaign game you can now pick 4, 5, 6 or 8 players. The menu was still limited to 4 from before the co-op joining fix."}
    };

    notes.push_back({
        "v0.20.32",
        "Bigger Campaign Lobbies",
        "Host Campaign Co-op for Up to 8 Players",
        v02032Items
    });

    const std::vector<ChangeItem> v02031Items = {
        {"FIX", "Your Flame Colour Shows for Everyone",
         "Other players now see your armour flames in the colour you picked, in first and third person, instead of the stock colour. Fires in the level keep their normal look."},
        {"NETCODE", "Smoother Co-op on Shaky Connections",
         "In smaller games (up to 8 players) Noble now sends a backup copy of each game update, so one lost update no longer causes a stutter for everyone."},
        {"NETCODE", "Higher Upload Limits for Hosts",
         "The game's network limits from 2010 have been raised, so hosts with faster internet can send more updates to their players. Fast connections and dedicated servers are no longer held to a 10 Mbit/s limit."},
        {"SERVER BROWSER", "Closest Server Picked Automatically",
         "When you host, Noble now picks the server closest to you by itself. Campaign games with high ping also show a warning before you join."},
        {"UI", "Connection Info",
         "A new NETWORK STATS option in Settings shows your ping, packet loss and network use. The scoreboard marks players with a poor connection, and loading screens now show tips."},
        {"FIREFIGHT", "Up to 16 Players",
         "Firefight lobbies can now hold up to 16 players."},
        {"FIX", "Smoother Frame Times",
         "Fixed a small stutter caused by Noble checking your armour settings file far too often."},
        {"STABILITY", "Game Version Check",
         "Noble now checks that your Halo: Reach files are the version it was made for, so an MCC update can be spotted and handled safely."}
    };

    notes.push_back({
        "v0.20.31",
        "Flame Colours for Everyone & Smoother Online Play",
        "Your Flame Colour Shared, Smoother Co-op, Higher Upload Limits & 16-Player Firefight",
        v02031Items
    });

    const std::vector<ChangeItem> v02030Items = {
        {"FIX", "Co-op Joining Fixed",
         "Players could be dropped right after joining a campaign co-op game, leaving the host to play alone. This started in v0.20.26, when custom flame colours were added to the player data the game shares with everyone. That data is now left untouched, and every player gets in again."},
        {"NETCODE", "Flame Colours Shared Over Noble's Own Connection",
         "Your custom flame colour now reaches other Noble players through Noble's own network messages instead of the game's player data, so it can never interfere with joining a match in campaign, Firefight or multiplayer."}
    };

    notes.push_back({
        "v0.20.30",
        "Co-op Joining Fix",
        "Players No Longer Dropped on Join & Flame Colours Over Noble's Connection",
        v02030Items
    });

    const std::vector<ChangeItem> v02029Items = {
        {"PERFORMANCE", "Higher Frame Rates in Matches",
         "The menu process no longer keeps a full CPU core busy while a match is running, freeing that core for the game. On a Ryzen 5 5600G with an RTX 4070, the busiest spot on the map went from about 70 FPS to 110-120 FPS."},
        {"PERFORMANCE", "Lighter Per-Frame Work",
         "Flame colour tinting now skips its work when nothing needs tinting and checks each texture once per frame instead of on every draw. Frame pacing and the FPS counter read their settings a few times a second instead of every frame."},
        {"AUDIO", "No More Silent Launches",
         "Noble now clears a leftover Windows volume-mixer mute at launch. Closing the game while it was in the background could leave it muted in the Windows mixer on every later launch."}
    };

    notes.push_back({
        "v0.20.29",
        "Performance: Higher Match Frame Rates",
        "Idle Menu During Matches, Lighter Per-Frame Hooks & Mixer Mute Fix",
        v02029Items
    });

    const std::vector<ChangeItem> v02028Items = {
        {"NETCODE & MASTER SERVER", "Master Server 0.20.x Registration & Match Visibility",
         "Updated backend server protocol validation and client handshake to natively support 0.20.x releases, resolving the 403 client_outdated rejection that prevented hosted sessions from publishing to the Server Browser."},
        {"SERVER BROWSER", "Reliable Session Discovery & Version Synchronization",
         "Ensured all active community matches appear reliably in the live server browser across both release and dev clients with accurate telemetry and ping estimation."}
    };

    notes.push_back({
        "v0.20.28",
        "Server Browser Master Server Visibility & 0.20.x Registration",
        "Backend Master Server Compatibility, Match Discovery & Version Bump",
        v02028Items
    });

    const std::vector<ChangeItem> v02027Items = {
        {"CRASH FIXES", "Engine Crash Mitigation from Crash Dumps",
         "Patched null pointer dereferences at haloreach.dll+0x41E3EB, 0x92850 (tag resolve guard), and 0x1650A1 (unit iteration bounds). Modernized near-trampoline memory allocation via 64KB VirtualQuery scanner to guarantee 64-bit hook placement."},
        {"NETCODE & LOBBY", "Join-in-Progress & Peer De-Duplication",
         "Suppressed connection refusal reason 3 (session locked) across Campaign, Firefight, Custom Games, and Forge when network transport is active. Replaced packet-count heuristics with unique peer bitmasks, eliminating phantom bot entries and premature simulation starts."},
        {"CO-OP EXPANSION", "Expanded Campaign Co-Op & Firefight Loading",
         "Expanded Campaign co-op launch block allocation to support up to 16 players, ensuring smooth loading and roster stability across all cooperative and competitive modes."},
        {"SERVER BROWSER", "Client Version Indicator & Cross-Build Lobby Compatibility",
         "Added live client build version badge to the Server Browser header. Version checking enforces exact numeric releases while seamlessly allowing release and dev builds to connect when their version numbers match."}
    };

    notes.push_back({
        "v0.20.27",
        "Engine Crash Mitigation, Co-Op Expansion & Server Browser Version Sync",
        "Minidump Crash Fixes, Join-in-Progress & Accurate Lobby Rostering",
        v02027Items
    });

    const std::vector<ChangeItem> v02024Items = {
        {"UI & MOUSE", "Themed Reticle Cursor for Mouse & Keyboard",
         "The in-app mouse now displays the stylish controller circular reticle cursor with tactical tick marks and tactile click compression by default instead of the OS pointer, with pixel-perfect alignment and toggleable in Settings."},
        {"UI POLISH", "Cinematic Screen Transitions & Rock-Solid Focus",
         "Implemented smooth atmospheric horizon-scan screen transitions across all menus, eliminated cross-screen click-through, and added intelligent navigation debounce to keep mouse and keyboard button focus precise and glitch-free."},
    };

    notes.push_back({
        "v0.20.24",
        "Themed In-App Mouse Reticle & Seamless UI Transitions",
        "Themed Circular Reticle for Mouse Navigation + Glitch-Free UI Focus",
        v02024Items
    });

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