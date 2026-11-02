# Project Noble

**Halo: Reach Community Edition** — a standalone Halo: Reach client for *Halo: The Master Chief Collection* on Steam. It loads MCC's own `haloreach.dll`, renders the game, and replaces the front-end with Noble's own menus and customization.

This repository is the **release channel**: every download, changelog and auto-update asset lives here. The source code lives at [O7OF/test-build-01](https://github.com/O7OF/test-build-01).

![Main menu](docs/screenshots/main-menu.png)

## Download

Get the latest build from **[Releases](https://github.com/O7OF/Project-Noble-/releases/latest)**.

1. Download `Project-Noble-vX.Y.Z-win64.zip`.
2. Extract the **whole folder** to a writable location, e.g. `Documents\Project Noble`.
3. Run `Noble.exe` (or `play.bat`).

The client checks this repository when the main menu opens. If a newer release exists, an **UPDATE AVAILABLE** panel appears in the menu: download it there, then press **RESTART & INSTALL**. Settings, appearance and mods in `data\userdata` are preserved.

## Features

- **Standalone Reach client** — no MCC launcher; loads the retail `haloreach.dll` through Steam ownership verification, without modifying the MCC install.
- **Custom front-end** — main menu, campaign, Firefight, Custom Games, Forge, server browser, settings, service record and social screens.
- **3D Armory** — live in-engine Spartan and Elite preview with full-body framing, per-slot close-up cameras, armor sets, helmets, shoulders, chest, wrists, utility, knees, visors, colors and effects.
- **Career & progression** — 50 Reach ranks, the original credit system, commendations, challenges, post-game payouts and a service record.
- **Multiplayer** — host or join custom games, Firefight and campaign co-op; LAN discovery, online lobby advertisement, party/lobby roster and NAT/UPnP diagnostics.
- **Mods & Workshop** — mod catalog, Workshop subscriptions, map previews and per-lobby mod downloads.
- **Auto-updates** — checks this release repository and installs new builds in place.
- **Native-resolution rendering** — the engine renders at your display's native resolution and scales into the window, so menu text, HUD, subtitles and gameplay stay crisp at 720p windowed and 1080p borderless.
- **Live display switching** — windowed and borderless full screen apply immediately from SETTINGS > GRAPHICS, with no restart.
- **Custom menu backgrounds** — your own images in `data\ui\custom\backgrounds` (or Noble/Reach art) are drawn behind the entire front-end instead of the engine map's camera cuts.
- **Quality-of-life** — controller support, key rebinding, FOV and camera settings, FPS limit, VSync, FXAA, brightness, menu music/sounds and subtitles.

## Screenshots

| Main menu (borderless) | Settings (windowed) |
|---|---|
| ![Main menu](docs/screenshots/main-menu.png) | ![Settings](docs/screenshots/settings.png) |

| Armory — Spartan | Armory — Elite |
|---|---|
| ![Spartan](docs/screenshots/armory-spartan.png) | ![Elite](docs/screenshots/armory-elite.png) |

## Requirements

- Windows 10 or Windows 11, 64-bit
- Steam with **Halo: The Master Chief Collection** and the **Halo: Reach** DLC installed
- A Direct3D 11 capable GPU
- ~2 GB free disk space for the client and menu map

The client verifies Steam ownership at launch. It does not bypass anti-cheat or connect to official MCC services.

## Updates

The release asset is named `Project-Noble-vX.Y.Z-win64.zip` and ships with a `.sha256` checksum. The client reads the GitHub Releases API for this repository, compares the tag with its built-in version, and downloads the zip only when you press **DOWNLOAD**. The update helper waits for the client to close, copies the new files over the old ones (leaving `data\userdata` alone), and restarts it.

If an update ever fails, the previous build is still on disk and the helper writes a log to `%TEMP%\ProjectNobleUpdate\apply_update.log`.

## Notes

- Release zips intentionally do **not** include the development `data\mods` folder. Put your own mods there.
- The menu map (`data\menumaps\mainmenu.map`) is included so a fresh install works without copying files from another installation.
- Halo and Halo: Reach are trademarks of Microsoft. This is a community project; no MCC files are redistributed except content the client is allowed to use locally.

## Links

- **Source / issues:** https://github.com/O7OF/test-build-01
- **Releases:** https://github.com/O7OF/Project-Noble-/releases
