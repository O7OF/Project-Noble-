# Noble master server

A small list of online Noble games. A player who hosts from the SERVERS tab registers here, sends a
heartbeat every 10 seconds, and is dropped 30 seconds after the last one. Players browsing the
SERVERS tab download the list. Based on the Outbreak-Z session registry written by O7OF.

Needs Node.js 18 or newer, nothing else.

```
node master_server.js
```

Settings (environment variables): `PORT` (default 8080), `TRUST_PROXY=1` when it runs behind a
reverse proxy that sets `X-Forwarded-For`.

In Noble: SERVERS tab, MASTER SERVER..., enter `http://<address>:8080`, SAVE (or SETTINGS, PLAYER).

Protections: each listing gets a secret token that heartbeats and removal must send
(`X-Session-Token`); the listed address is always the address the registration came from; limits on
body size (4 KB), games per IP (4), requests per IP (120 a minute), total games (2000) and text length.

Tests: `node test_master_server.js`.

Note: the list only shows games. Joining another player's game isn't possible yet; Noble doesn't
link two engines together.
