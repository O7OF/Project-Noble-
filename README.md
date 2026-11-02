<h1 align="center">Project Noble</h1>

<p align="center">
  <b>Halo: Reach Community Edition: a standalone Halo: Reach client for Halo: The Master Chief Collection on Steam.</b><br>
  Your own copy of Reach, with new menus, a live 3D Armory, the original career, and online play with friends.
</p>

<p align="center">
  <a href="../../releases/latest"><img src="https://img.shields.io/github/v/release/O7OF/Project-Noble-?label=latest%20version" alt="Latest version"></a>
  <img src="https://img.shields.io/badge/platform-Windows%2010%20%7C%2011-blue" alt="Windows 10 and 11">
  <img src="https://img.shields.io/badge/price-free-brightgreen" alt="Free">
</p>

<p align="center">
  <img src="docs/screenshots/main-menu.png" width="900" alt="Project Noble's main menu">
</p>

## Download

<p align="center">
  <a href="../../releases/latest"><img src="https://img.shields.io/badge/DOWNLOAD%20FOR%20WINDOWS-latest%20version-2ea44f?style=for-the-badge&logo=windows&logoColor=white" height="56" alt="Download Project Noble for Windows"></a>
</p>

**[Download the latest version](../../releases/latest)**: on the page that opens, scroll to **Assets** and click `Project-Noble-vX.Y.Z-win64.zip` (the `.zip` file, not the `.sha256` or "Source code" ones).

Or see every version on the [Releases page](../../releases).
Only download Project Noble from this repository. Copies shared anywhere else could be tampered with.

---

## How to play (3 steps)

**1. Download the zip from the latest release** (link above).
Your browser saves a file called `Project-Noble-vX.Y.Z-win64.zip`, usually in your **Downloads** folder.

**2. Extract the whole folder somewhere you can write to.**
Right-click the zip, choose **Extract All...**, and pick a folder like `C:\Games\Project Noble`.
Keep everything that's in the zip together. Noble needs its `data` folder next to it.

**3. Open the folder and double-click `Noble.exe`.**
If a blue box says **"Windows protected your PC"**, click **More info**, then **Run anyway**.
This appears because Project Noble is a free community project and isn't signed by a paid certificate. It's normal and only happens the first time.

<p align="center">
  <img src="docs/windows-warning.svg" width="640" alt="Click More info, then Run anyway">
</p>

**That's it.** Make sure Steam is running. Noble checks that you own Halo: Reach, then opens its main menu.

---

## What you need

| | |
|---|---|
| **Windows** | Windows 10 or 11, 64-bit |
| **Halo: Reach** | Halo: The Master Chief Collection **on Steam**, with the **Halo: Reach** DLC installed |
| **Graphics** | Any Direct3D 11 graphics card |
| **Disk space** | About 2 GB for the client and its menu map |

- The **Microsoft Store / Game Pass** version of MCC doesn't work. You need the **Steam** version.
- You can play with a **mouse and keyboard** or an **Xbox controller**.
- Noble never changes your MCC files and doesn't connect to official MCC online services or anti-cheat.

---

## What's in it

- **Standalone Reach client:** no MCC launcher. Noble loads Reach from your own MCC install after checking Steam ownership.
- **New front end:** main menu, Campaign, Firefight, Custom Games, Forge, server browser, settings, Service Record and social screens.
- **3D Armory:** the real in-engine Spartan and Elite, with close-up cameras for every slot: helmets, shoulders, chest, wrists, utility, knees, visors, colours and armour effects.
- **Career:** all 50 Reach ranks, the original credit system, commendations, daily and weekly challenges, post-game payouts and a Service Record.
- **Play together:** host or join custom games, Firefight and campaign co-op, with LAN discovery, an online server list and a party lobby.
- **Mods:** a mod catalog, Steam Workshop subscriptions, map previews and per-lobby mod downloads.
- **Looks right on any screen:** native-resolution rendering, live windowed/borderless switching, FOV, FPS limit, VSync, FXAA and brightness.
- **Updates itself:** a panel in the main menu tells you when a new version is out.

| Armory: Spartan | Armory: Elite |
|---|---|
| ![Spartan](docs/screenshots/armory-spartan.png) | ![Elite](docs/screenshots/armory-elite.png) |

| Settings | Windowed mode |
|---|---|
| ![Settings](docs/screenshots/settings.png) | ![Windowed](docs/screenshots/windowed.png) |

---

## Controls

In the menus:

| Controller | Keyboard and mouse | What it does |
|---|---|---|
| D-pad / left stick | ↑ ↓ ← → or the mouse | Move around the menu |
| **A** | **Enter** or click | Select |
| **B** | **Esc** | Back |
| **LB / RB** | **Q / E** | Switch tabs |

In a match, Noble uses MCC's default PC controls. You can rebind every key in **SETTINGS > KEYBOARD & MOUSE**. **Esc** (or **Start** on a controller) opens the pause menu.

---

## Questions

**How do I update?**
When a new version is out, an **UPDATE AVAILABLE** panel shows up in the main menu. Click **DOWNLOAD**, then **RESTART & INSTALL**. Noble swaps its files and restarts. Your settings, Armory, career and mods (everything in `data\userdata`) stay as they are.

**Is it safe?**
Noble doesn't change any MCC files, and it never asks for passwords or logins. Each release has a `.sha256` checksum next to the zip so you can check your download, and the in-menu updater only installs builds from this repository's Releases page.

**My antivirus flagged it.**
Some antivirus programs are wary of new, unsigned programs. Compare your zip against its `.sha256` file on the [Releases page](../../releases/latest), or check it on [VirusTotal](https://www.virustotal.com/). If it still worries you, [open an issue](../../issues) and tell us which antivirus said what.

**It opens and closes straight away, or says it can't find Halo: Reach.**
Make sure Steam is running and Halo: Reach is installed in MCC (on Steam, right-click MCC > Properties > DLC). Then check the log: `noble-client.log` in Noble's folder.

**It closes right away and nothing is saved.**
This is usually Windows Security's **Controlled folder access** (ransomware protection), which stops unknown programs saving to folders like Documents and Desktop. Either move Noble to a folder like `C:\Games\Project Noble`, or allow it:
1. Open **Windows Security** (search for it in the Start menu).
2. Click **Virus & threat protection**, then **Manage ransomware protection**.
3. Click **Allow an app through Controlled folder access**, then **Add an allowed app**.
4. Pick `Noble.exe` from **Recently blocked apps**, or browse to it in Noble's folder.

**An update failed.**
Your previous version is still on disk. The updater writes a log to `%TEMP%\ProjectNobleUpdate\apply_update.log`. Attach it when you [report the problem](../../issues).

**Can I use my own menu backgrounds?**
Yes. Put your images in `data\ui\custom\backgrounds` and pick one in **SETTINGS**.

**Where do mods go?**
Subscribe in the in-game **MODS** screen, or put your own mods in `data\mods`. Release zips don't include any mods.

**How do I remove it?**
Delete Noble's folder. That also removes your settings, Armory and career, which live in its `data\userdata` folder.

**Something's broken or I have an idea.**
[Tell us here](../../issues).

---

## Credits

- **Project Noble:** [O7OF](https://github.com/O7OF) and [WeabooTyler117](https://github.com/WeabooTyler117)
- Thanks to the Halo modding community, whose research made a standalone Reach client possible, and to everyone testing Noble.

---

<details>
<summary><b>For modders and developers</b> (click to open)</summary>

### What's in this repository

This is Project Noble's **release channel**: downloads, changelogs and the files the in-menu updater reads.

| Path | What it is |
|---|---|
| `README.md` | This page |
| `release-notes/` | The full notes for each release, as posted on the Releases page |
| `docs/screenshots/` | The screenshots on this page |

### How releases work

Each release has one asset, `Project-Noble-vX.Y.Z-win64.zip`, with a matching `.sha256` checksum. When the main menu opens, the client reads the GitHub Releases API for this repository and compares the newest tag with its built-in version. It only downloads the zip when the player clicks **DOWNLOAD**. The update helper waits for the client to close, copies the new files over the old ones (leaving `data\userdata` alone) and restarts it.

The menu map is compiled into `Noble.exe` and unpacked to `data\menumaps` on first launch. The zip contains no loose map, mod or model files.

### Source code

The client's source is developed in a separate repository. Bug reports and ideas go to the [Issues page here](../../issues).

</details>

---

<sub>
Halo: Reach © Microsoft Corporation. Project Noble was created under Microsoft's "<a href="https://www.xbox.com/en-US/developers/rules">Game Content Usage Rules</a>" using assets from Halo: Reach, and it is not endorsed by or affiliated with Microsoft.
Project Noble is free and will always be free.
</sub>
