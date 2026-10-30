# Halo Reach Community Edition

Halo Reach Community mod edition - the client front-end and engine host (this repo) plus the dedicated backend.

## Repositories

| what | repo | notes |
|---|---|---|
| Client / engine / menus | `https://github.com/O7OF/test-build-01` | this checkout; C++20, build with CMake |
| **Backend, netcode and server scripts** | **`https://github.com/O7OF/ServerTestDeploy`** | Node/Express + WebSocket relay; deployed at `https://outbreak-z-test-server.onrender.com` |

**Rule: any netcode or server-script change goes to `O7OF/ServerTestDeploy`, never to this repo.** The client repo no
longer carries `server/` (commit `19207fb` removed it). Work in a clone of the backend repo:

```powershell
.\tools\server-repo.ps1            # clone next to this checkout (or pull it) and show branch/head/behind
```

Then `node --check server.js`, commit and push from that clone. Diagnostics after a failed join: `GET /api/health`
(version/commit/relay counters) and `GET /api/relay/stats` (live relay rooms with `hasHost` / `guests`).
