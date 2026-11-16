# PART N - NETWORKING (Halo 3, ODST, Reach, and MCC)
Written 2026-11-12 from web research. This part SUPERSEDES section B9 ("almost nothing"), which only covers what the Discord corpus said. Tags: [C] primary source (Bungie talk/slides, 343 posts), [S] secondary summary or fan guide, [U] my inference, not stated by any source.
Primary source: David Aldridge (Bungie lead networking engineer), "I Shot You First: Gameplay Networking in Halo: Reach", GDC 2011. Read via slide transcriptions (slidetodoc, studylib) and summaries (Wolfire, Edgegap, UCL). Original slide file not read directly.

## N1. Architecture (Halo 3, ODST multiplayer, Reach)
- No dedicated servers in the originals. One console is the host ("physics host"); the others are clients. Host is authoritative over damage and object creation/destruction. [C]
- Design lineage: "The Tribes Engine Networking Model" (Frohnmayer and Gift, GDC 1998/99): host/client, cheat-resilient, semi-reliable delivery, persistent state plus transient events, bandwidth that scales to what is available. [C]
- Rejected alternatives (per the deck): lockstep/deterministic input passing, reliable transport (TCP or homegrown), sending all state as one atomic blob (Quake 3 model). [C]
- Bandwidth need grows as O(N squared) with player count; Halo targets 16 players, vehicles, hundreds of replicated objects (Edgegap: designed for 2,000+ synced entities). [C]
- Stack, bottom to top: transport (sockets) -> channel manager / flow and congestion control -> three replication protocols -> prioritization -> game interface (extract/apply). [C]
- Halo 3 differs from Halo 2 by allowing "limited client trust": if latency stays under a threshold, a client's action completes, even outside normal auto-aim range. A fan guide (Bungie.net forum v1.7, May 2007, written during the beta) says so; treat as secondhand. [S]
- ODST/Halo 3 campaign: Firefight and campaign co-op use a synchronous LOCKSTEP model (host collects every player's input, bundles it, everyone advances one frame). ODST multiplayer and Forge use the distributed host/client model. Bungie said ODST's co-op reuses the Halo 3 campaign model. (Bungie Weekly Update, 2009.) [C]
  - Lockstep consequences: any input needs a round trip to the host before the game reacts; packet loss stalls everyone (inputs are ordered, so the whole game waits for retransmission). Bungie's advice: do not share the connection, stop background downloads, wired or interference-free wireless, play with nearby players.
- Reach has two models: PvP host/client, and a separate one for Campaign/Firefight (GDC 2015 Destiny talk). Reach Firefight was notorious for lag; for MCC, 343 said Firefight matchmaking moved to dedicated servers with a new asynchronous model. [C/S]

## N2. The three replication protocols (Reach) [C]
| Protocol | Direction | Guarantee | Examples |
|---|---|---|---|
| State data | host to client only | latest value eventually arrives; intermediate values may be skipped | position, health, capture timer, ~150 more properties |
| Events | both directions | none (unreliable notifications) | fire request, weapon fired, projectile detonated, ~50 more; they explain why, state carries the truth |
| Control data | both ways | best effort, high frequency | host to client: all players' analog stick values; client to host: own biped position; ~15 more; ~20 bits per player |
- Guarantees are mixed per message type: guaranteed (game-over notice, grenade-throw request), ordered only (positions), none (sparks, bullet decals). (Wolfire)
- Replication is never fully reliable; every packet is filled in priority order because there is always more data than fits.

## N3. Prioritization [C]
- Priority is computed per object, per client: distance and direction first, then size and speed; shooting and damage add boosts; special cases (thrown grenades rank above what threat alone would give; dead bodies get near-max relevance for several seconds; idle grenades update ~3 Hz).
- Output per object: final priority, relevance, desired update period in ms. Low priority means slower updates, never a stall.
- Flow control ensures low-priority traffic never starves high-priority traffic.

## N4. Hiding lag: how Bungie designed mechanics [C]
Four rules: (1) decide what needs a single authority, (2) always ask where the lag is hidden, (3) change game mechanics if it helps networking, (4) reserve time to iterate.
- Grenade throw: client predicts the animation but not the release; the host creates the real grenade at the release frame and the predicted one is deleted. Lag lands on grenade appearance.
- Armor Lock: shipped version shows animation/FX immediately and the host starts invulnerability after a delay (3 frames plus round trip).
- Assassination: strict obey-the-host for all peers, including participants; the predict-locally version worked in playtests but failed in the public beta.
- Melee: needed post-ship fixes in Halo 2 and 3; rare host-ignored client strikes in the Reach beta.
- Shots are sent target-relative ("at this player's head"), so the host judges against the target's position at that time. (Halo 3 Bungie post: client sends who it aimed at and how far ahead; the same hit happens on the host two ticks later.)
- Prediction shown instantly: muzzle flash, sound, animation, aim, movement. The host confirms damage, spawns, grenades.

## N5. Bandwidth numbers [C]
- Halo 3 breakdown: 50% positions/velocities/orientations, 20% player control data, 20% weapons/bullets/damage, 10% other. Host to client control replication alone was 22% of host upstream.
- Reach cut bandwidth by over 80% (to about 20% of Halo 3). Four of five major wins were gameplay changes, not encoding:
  - Control replication: removed duplicate/unneeded data, 60% less (14% overall).
  - Equipment priority bug: dropped grenades inherited an equipment boost via a shared parent class; limiting the boost to ACTIVE equipment freed about 20%.
  - Ragdolls: no longer networked (only initial state); bullets and grenades pass through them; about 10-12%.
  - Item friction: constant artificial friction replaced by fake friction.
  - High-rate-of-fire weapons: some damage effects client-predicted; statistics traffic capped at 10% of a packet or less.
  - Encoding tricks (fixed-point positions, limited ranges) were a false start; a bucketing scheme saved 1% and cost 30+ hours of debugging.
- Reach figures from the deck: 250 kbit/s minimum total upstream for the host of a solid 16-player game; 675 kbit/s maximum from any single peer; 45 kbit/s maximum host to one client; ~1 kbit/s to replicate one combat biped to one client; 10 Hz minimum packet rate for solid play; latency limits 100 ms (tournament) / 200 ms (casual) close quarters and 133 / 300 ms ranged. (Edgegap adds: 384 kbps design goal; naive send-everything would be ~20 Mbps; playtesters noticed nothing up to ~150 ms.)
- Encoding rules: raw bits for rare values, limited range for common values, floats to fixed point, exploit temporal coherence; default to a high packet rate and lower it when the simulation gets rich.

## N6. Host selection, migration, tooling [C]
- Connection quality is recorded per player and matchmaking picks good hosts. Host migration moves the game to a better host if the host drops; the deck says it is hard to add late. (A fan source says a late-joining player with a better connection does not take over unless the host lags out. [S])
- Tools Bungie built: bandwidth/priority profilers; profile data spliced into "films" (deterministic replays) so network use can be analysed after the fact; monthly network playtests under traffic shaping; a controller button for "report lag"; full-time network testers.
- "Measure first, big picture over encoding, work with designers."

## N7. Master Chief Collection [C unless noted]
- MCC uses dedicated servers for matchmade games and the custom game browser, running the Unified Dedicated Server (UDS), a trusted authority that cannot be tampered with like PC clients (Halo Waypoint, 2022 incident post). Configuration failure falls back to a peer host, the way custom games work. Custom games and campaign co-op historically peer-hosted (2014 statement).
- Tick rate: Halo 3 clients sent 15 packets/s and the server 30/s; the 2019 update made both 60/s, including peer-hosted games. Reports list Halo 2, Halo 2 Anniversary, Halo 3 (one lists Halo 4 as well); none name Reach, so Reach's MCC packet rate is UNVERIFIED.
- Side effect of a higher tick rate: each tick covers less time, so per-tick projectile reach changed and weapon feel shifted (SteamDB patch notes about Halo 2: tick went from 1/30 s to 1/60 s).
- Reach in MCC: matchmaking relays can be disabled in Network Settings if matchmaking misbehaves (Windows Central, Steam threads). Campaign co-op still shows input lag; Halo 3 campaign co-op is reported worse unless the frame rate is locked to 60 (community reports [S]); 343 said fixing co-op latency "would require rewriting netcode" (screenshot quoted by players [S]).
- Halo Infinite favours the shooter with server-side rewind (latency compensation) per 343, a design choice to compare against Halo 3's host authority.

## N8. Practical connection checklist for players (stable connection, fewer drops) [mixed]
1. Wired Ethernet; wireless mesh will cost packet loss no matter what (Steam MCC troubleshooting). Packet loss is the biggest single killer in Bungie's own advice.
2. Nothing else using the line: downloads, streaming, VPNs (VPN raised packet loss in Infinite fixes lists).
3. NAT type open/moderate. Ports frequently quoted for MCC/Xbox services: UDP 88, 500, 3074, 3075, 3544, 4500 (and TCP/UDP 3074). Steam-listed ranges: TCP 3074, 27015-27030, 27036-27037; UDP 3074, 3544, 4380, 4500, 27000-27031, 27036. [S, community]; Halo Support's "no ping to datacenters" article says to open UDP 3075 plus Xbox network ports.
4. Allow MCC through the firewall; IPv6 on; no drawing tablets plugged in (Microsoft Answers).
5. If matchmaking/finding games fails: switch off relay in MCC network settings.
6. Prefer a good host for custom games: pick the player with the best upstream (Bungie recorded connection quality; MCC custom games are peer-hosted).
7. Cap frame rate at 60 for Halo 3 co-op (community-reported).

## N9. What this means for Noble and mod content [U unless stated]
1. The host is authoritative; clients simulate predicted copies. A client that lacks your modified tags cannot render or simulate your content. Every player in a session likely needs identical tag/map data. (Cf. Project Reclaimer: "every player in a game needs the same version".) No source documents how stock MCC clients handle custom tags; test with two PCs, do not assume.
2. Armour LOOK (render model, shader, change colours) is local cosmetic state; state that others must see must be replicated by the player's biped/armour data. Keep new armour cosmetic-only; avoid adding new always-on replicated properties.
3. Spend like Bungie: new networked objects, effects, vehicles and particles cost bandwidth. The 65,535-particle patch (C5a) raises client limits but nothing in this research suggests it is network-safe; test.
4. Avoid: always-on per-player data, many small physics objects, ragdoll-like simulated props, high rate-of-fire projectiles with per-bullet damage. Bungie's biggest savings came from removing exactly these.
5. Add a version/hash check: refuse to join when pack versions differ (Project Reclaimer does exactly this).
6. Measure: use traffic shaping to simulate 25-100 ms and 1-3% loss; use packet capture or the engine's own network debug (the engine has network statistics; names unverified) to count bytes per player with and without the mod; watch the 45 kbit/s per-client and 250 kbit/s host budgets as sanity checks for 16 players (Reach era; MCC at 60 Hz will use more).

## N10. Ideas to improve networking after 12 years
Sourced ideas (existing community engine work and 343 statements):
- Project Reclaimer Engine (Halo 3 recompiled to Rust, community project): rebuilt network layer; encrypted authenticated UDP plus a reliable stream for chat/votes/downloads; separate send thread; "enhanced networking" prioritises objects by how wrong they would look on each client's screen (in view update more, zoomed scope more, behind less); remote vehicles blend toward updates; small position jumps smoothed over ~100 ms instead of snapping; own-vehicle prediction window grows with speed and ping (host-limited); numbered events so duplicate packets cannot apply twice; host migration; STUN plus masters for NAT punch-through (up to four masters, no port forwarding in most cases, symmetric-NAT pairs excepted). Its own benchmark (64 clients, 25 ms, 1% loss): worst 1% aircraft position error 6.4 to 0.74 mrad; data downloaded by a watcher 13.9 to 6.6 MB; kinks 28 per minute to 0.5; Classic (Halo 3 model) remains selectable. It states bad connections stay bad: it "still predicts the way Halo 3 does".
- 343 for MCC: dedicated servers for matchmaking, 60 Hz tick, asynchronous model for Reach Firefight.
- Bungie's own ideas: connection-quality-aware host selection and migration; target-relative shot data.

My ideas, labelled inference (none claimed by sources; each needs testing):
1. Priority tuning: bias update rates by what each viewer can see (the same idea Reclaimer uses). For Reach, priority lives in game code/tags; nothing documented is exposed to tag editing, so this means engine work, not a mod.
2. Smooth, don't snap, remote players and vehicles: client-side smoothing over ~100 ms (as Reclaimer does) would hide corrections on high-jitter links.
3. Relay/hosting: a relay or small VPS acting as a dedicated host (the UDS model) removes the weak-host problem and NAT trouble, but MCC does not let mods run a UDS, and anti-cheat blocks tampering; feasible only for an offline/anti-cheat-off community mode.
4. Lobby-side quality checks: measure RTT/loss between all players before the game and pick the lowest-RTT host (what Bungie did in matchmaking). A custom lobby could do this at the community level (Discord bot, mod menu) even if not in the engine.
5. Version pinning and hash checks to prevent desyncs from mismatched content.
6. Players: prefer wired, 60 fps lock for co-op, low-latency DNS/router QoS (SQM/fq_codel) to prevent bufferbloat under load [general networking practice, not Halo-specific].
7. Test harness: a repeatable emulated-network profile (25 ms/1% loss as a baseline, 100 ms/3% as a bad day) and a "report lag" key, as Bungie did.

## N11. Gaps (not found)
- Reach MCC packet rate, tick rate (30 Hz originally per community; MCC rate unverified).
- Whether stock MCC clients accept modified tags, and what Easy Anti-Cheat does in modded sessions (mod launch already requires anti-cheat off per the Discord corpus, B9).
- Direct reading of the GDC slides (only transcriptions/summaries read).
- The Bungie.net network guides by "One One Seven" (v2.1.1, 2009) exist at halo.bungie.org/misc/networkguide.html; only a snippet was read (a "latency window" of about 300 ms in Halo 3 for host acceptance of client actions, stated secondhand).

## N12. Sources
Wolfire GDC summary (wolfire.com/blog/2011/03/GDC-Session-Summary-Halo-networking/); Edgegap deep dive; UCL summary; slidetodoc and studylib slide transcriptions; Bungie Weekly Update 6/20/08 and the ODST update (halo.bungie.org/bwu item 182 and 249); fan guides (Bungie.net forum post 344898; halo.bungie.org/misc/networkguide.html); Halo Waypoint "MCC Server Incident Summary"; GameSpot and Gamepur on the 60 Hz update; Windows Central/PC Gamer on Reach Firefight dedicated servers; GDC 2015 Destiny talk (Justin Truman); Project Reclaimer engine page (projectreclaimer.dev/engine); 343 "Closer Look at the Halo Infinite Online Experience"; Steam MCC troubleshooting threads; Halo Support articles.
