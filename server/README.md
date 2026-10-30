# server/ - working copy of the Noble backend

**Canonical repo: https://github.com/O7OF/ServerTestDeploy** (that is what runs on the host).

This folder is a working copy so the client and the server can be changed together. Any change to netcode or server
scripts has to reach BOTH repos:

```powershell
.\tools\push-server.ps1 -Message "lobby: keep relay room across re-register"
```

That syncs `server.js`, `src/`, `package*.json`, `grant_item.js`, `reset_db.js`, `set_dev.js`, `update_auth.js` into a
clone of ServerTestDeploy, commits and pushes. It deliberately skips `node_modules/` (installed on the host) and
`database.db` (live player data - pass `-IncludeDb` only when you mean to overwrite it).

Latest upstream commits at the time of writing (2026-10-30):

```
7f00da8  add update_auth.js (admin auth update script) from the client repo
24075d8  feat(auth): wipe db, enforce single-session concurrency block and case-insensitive uniqueness
83602ac  Netcode, lobby sync, database reset, and dev script updates
37bcf64  fix(master): reuse session id when host re-registers (lobby -> match) so relay room and guests stay in one session
e1301b3  Add pre-game lobby signaling endpoints, real-time roster sync, and WebSocket control frames
510b797  feat(relay): WebSocket netcode packet relay, NAT punch signaling, upgraded set_dev.js
```
