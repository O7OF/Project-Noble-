const fs = require('fs');
const path = 'docs/HANDOFF.md';
let content = fs.readFileSync(path, 'utf8');
const headerMarker = '## 0. Pick up here (2026-10-29, 00:20 ET)';
const newSection = `## 0. Pick up here (2026-10-29, 00:35 ET) - Pre-Game Lobby, Guest Black Screen Fix, WebSocket Signaling & Build Verification

Compiled cleanly via \`build.bat\` with **0 errors and 0 warnings**:
- **Fresh Binaries Built:** \`noble-client.exe\` (2,562,048 bytes) and \`noble-dedicated.exe\` (538,624 bytes).
- **Released to Test Directory:** Copied directly to \`C:\\Users\\Chey1\\Desktop\\Release For Test\\\`.

### 1. Fixed Guest Stuck on Load & Black Screen on Join (\`src/engine/BlamEngine.cpp\`)
- **Root Cause Identified from Logs & Reverse Engineering:**
  In \`BlamEngine::LaunchMap\`, a background thread was unconditionally posting \`Command 10\` (Start) and \`Command 11\` (Advance State) after a 300 ms delay on both Host and Guest. On a remote guest connecting to an existing match or lobby, forcing these commands hijacked the engine state machine out of \`state 9\` (\`join_synchronize\`) into \`state 11\` (simulation) before host session replication packets had arrived. Because the guest's local Blam engine had not yet received the game variant, player slot assignments, or initial world state from the host, the engine faulted into an unrendered black screen / void.
- **Fix:**
  Wrapped the manual \`PostCommand(10)\` and \`PostCommand(11)\` calls in \`if (!isGuest)\`. For guest clients, \`BlamEngine\` leaves the state machine alone, allowing \`haloreach.dll\`'s native Blam! session manager to complete handshake, membership replication, and scenario transition automatically upon packet receipt.

### 2. Pre-Game Lobby Implementation (\`src/ui/FrontEndMenu.cpp\`, \`src/ui/ServerBrowser.cpp\`, \`src/network/MasterServerClient.cpp/.hpp\`, \`src/network/NetworkTransport.cpp/.hpp\`)
- **Screen State:** Added \`Screen::PreGameLobby\` to \`enum class Screen\`.
- **Hosting Flow:**
  In \`CustomGame\`, \`Firefight\`, \`Forge\`, and \`Campaign\`, clicking **"HOST LOBBY"** or **"HOST CO-OP"** now calls \`SetupHostLobby(...)\`:
  - Registers the session on the master server with \`in_progress = false\` and initializes the host player in the lobby roster with their Peer ID (\`P:ID\`), Machine ID, username, service tag, and \`[DEV]\` badge.
  - Binds UPnP port \`11774\` and LAN discovery on port \`11775\`.
  - Transitions to \`Screen::PreGameLobby\` without launching the match immediately.
- **Pre-Game Lobby UI Layout (\`DrawPreGameLobby\`):**
  - **Left Column:**
    - High-resolution map preview banner art (\`UiArt::HeroBanner\` / \`UiArt::MapPreview\`).
    - Map cycler / selector and Gametype cycler.
    - Difficulty & Skulls selectors (for Campaign & Firefight).
    - Host Network Status Card with External IP, Port, LAN IP, and one-click "COPY INVITE" button.
  - **Right Column ("PLAYERS (X/16)"):**
    - Live player roster listing all connected peers in the lobby.
    - Host badge, glowing gold \`[DEV]\` badge for authenticated developers, \`P:ID\` (Peer ID), service tag, and ping status.
  - **Bottom Action Bar:**
    - **Host View:** "START MATCH" (dispatches \`LaunchLobbySync\` HTTP call and WebSocket \`lobby_launch\` JSON control frame, then triggers game launch for all players) and "CANCEL" (stops hosting, tears down relay/session, and returns to menu).
    - **Guest View:** "WAITING FOR HOST" (listens on WebSocket and polls HTTP for the launch signal, then automatically executes synchronized match launch) and "LEAVE" (notifies backend, disconnects relay, and returns to Server Browser).
- **Server Browser Integration:**
  - Games still in the pre-game lobby state are flagged with \`[LOBBY]\`.
  - Clicking "JOIN" on a \`[LOBBY]\` server joins the pre-game lobby room (\`Screen::PreGameLobby\`) as a guest instead of forcing an immediate Join-In-Progress relaunch.

### 3. Backend Master Server & WebSocket Relay Updates (\`ServerTestDeploy\`)
- All backend changes were deployed to \`https://github.com/O7OF/ServerTestDeploy.git\` (\`main\` branch, commit \`e1301b3\`) and live-reloaded on Render (\`https://outbreak-z-test-server.onrender.com\`):
  - In-memory lobby rosters tracked per session room: \`GET /api/sessions/:id/lobby\`, \`POST /api/sessions/:id/lobby/join\`, \`POST /api/sessions/:id/lobby/leave\`, and \`POST /api/sessions/:id/lobby/launch\`.
  - Multi-client WebSocket relay control message broadcasting (\`broadcastLobbyEvent\`): propagates \`lobby_launch\` and \`lobby_roster\` JSON control frames in real-time across peers in the room alongside binary UDP game packets.

---

`;

if (content.includes(headerMarker)) {
    content = content.replace(headerMarker, newSection + headerMarker);
    fs.writeFileSync(path, content, 'utf8');
    console.log('HANDOFF.md updated successfully');
} else {
    console.error('Header marker not found');
}

