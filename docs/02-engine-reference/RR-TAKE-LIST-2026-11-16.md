# Reach Renaissance: what we can take, what we cannot

> **Scope update 2026-11-16 (owner):** Noble's own **multiplayer work is stopped** - O7 is taking the multiplayer
> workstream. **Visors and lighting are removed from our task list.** This document remains as clean-room reference
> material only; it is not an active work queue. The take-list companion marks the affected items as out of scope.


> Companion to RR-REFERENCE-2026-11-16.md. Explicit list of reference material and ideas Noble can use from the RR
> study, and the items that are off-limits or not worth taking. Clean-room rule stays absolute: no RR code, binaries,
> assets, strings, tag packs, UI text, names or ports are copied into Noble. Taking an idea means writing our own
> implementation.

## 1. The hard boundary (never take)

| Item | Why not | Instead |
|---|---|---|
| RR executables, DLLs, decompiled C/disassembly, symbol dumps, patch bytes | proprietary/freeware binaries with no code licence; copying violates licence and clean-room | read/analyse for understanding; write our own code from the design |
| RR tag packs (.map), .rrreloc/.rrneeds/.rrpmcg/.rremotes contents, caches | contain RR/derivative game content and their encoding | design our own pack metadata if useful; never redistribute theirs |
| RR menus/themes (themes/classic/*.js/css/html), UI text, emblems, icons, fonts, videos, music, voice lines | creative assets/strings owned by RR | write our own UI/content; use our own art/audio |
| RR protocol payloads, master/relay/voice wire formats, service endpoints | their implementation/IP; no interop requirement | define our own if we ever add services |
| RR names/branding, player/profile names, map names, release text | trademarks/content | plain descriptive references only |
| Their exact engine patch bytes / replacement streams (e.g. the byte arrays behind install_stream128) | RR reverse-engineering output | note the RVA locations as candidates and derive our own patches |
| RR user data, logs, screenshots, monetisation/account data | privacy/ownership | none |

Also off-limits in practice: translating decompiled RR C into our C++ line-by-line, or using RR helper code/structs
copied from the binary. Function/module names and RVA addresses as facts are fine to document; their bodies/bytes are
not.

## 2. What we can take: reference material and ideas (prioritised)

### 2.1 High value - directly informs our next capacity work

| # | Idea / reference | What RR shows | How Noble can use it |
|---|---|---|---|
| 1 | Capacity is a profile | default/M64/M81/M128/FF32 selected per session; engine patch groups gated per profile; stock path untouched | grow -slot-players into a profile concept: stock 16 default, dev 32, later 64/128; gate each profile's patch set and keep 16/32 byte-identical |
| 2 | Machine vs player split | M128 = 128 players on up to 64 machines (2 per machine); M81 = 81 machines, 1 player each | our capacity model assumes peers == players; adding a machine/player split lets 64 players run on 32 machines, keeping machine masks smaller - evaluate with our mask model |
| 3 | Explicit mask/set configuration | profile fields machine_index_bits, machine_set_bytes; u64 full/delta membership masks; resync/checksum; recipient sets | formalise words/bits per machine and player set in the capacity profile and plan full/delta mask updates |
| 4 | Patch-table installers | install_stream128 builds expected/replacement entries over engine RVAs; p64::Batch; m64_join code sites/bit-branch generation | our piecewise-cave synthesis + self-test already matches this shape; use RR's installer structure as a checklist (expected bytes, per-profile gates, all-or-nothing) |
| 5 | Capacity storage planning | capacity::machine_storage_plan, moved player block, player-storage sizes, machine admission counts | formalise our SlotShadowLayout plan into named machine/player storage numbers per profile; cross-check our chunked layout against their counts |
| 6 | Big-lobby roster/scoreboard | scoreboard rows/session users/peer records/unique name index, built for the profile count | design our UI/roster around the profile's count instead of clamping after the fact; use the module split as a requirements list |
| 7 | Membership validation | machine/player/appearance validators; wire fields; drop invalid entries | add equivalent validation to our membership-message handling so a bad peer/player record is rejected visibly, not accepted into the session |
| 8 | Host-option facts + version/profile advertising | server-browser facts advertise host profile and client version | extend Noble's server browser/lobby facts to advertise effective seats/profile and refuse incompatible builds clearly |
| 9 | Profiles/UI clamping | Max Players options come from the chosen profile; party size follows it | wire our picker/lobby/server-browser seats to one capacity profile source (effective seats started this; finish for 64/128) |
| 10 | Host migration behaviour | documented: big lobbies do not migrate; game ends and guests return to menus | acceptance criterion for our profiles; make our behaviour/message explicit too |

### 2.2 Medium value - product/feature ideas to evaluate

| # | Idea / reference | What RR shows | How Noble can use it |
|---|---|---|---|
| 11 | ~~Armour-light glow + brightness UX~~ **OUT OF SCOPE (visors/lighting removed 2026-11-16)** | exact-RGB look.glow + level table 1,2,3,5,8,12,16,24,32,50,64; draw-hook replaces the light colour; sRGB conversion; menu preview glow | spec our own armour-light colour/brightness customiser if desired; implementation approach (hook + colour conversion) is a valid design reference, not code |
| 12 | ~~Per-piece exact-RGB armour colours / colour wheel~~ **OUT OF SCOPE (visors/lighting removed 2026-11-16; reference only)** | per-piece primary/secondary, exact RGB beyond palette, Elite piece colours | UI/feature requirements for our armory if we extend beyond palette colours |
| 13 | ~~Visor/lighting-related appearance items in 11-12~~ **OUT OF SCOPE**; tag-pack metadata model | .rrneeds deps, .rrreloc relocation/patch records, .rrpmcg model customisation, .rremotes animation remotes | design our own pack manifest with dependency/relocation sections for portable mod packs |
| 14 | Dedicated-server requirements list | config options (big_lobby, playlists, admin dashboard, bans, restart/next-game), player-hosted servers | keep as a future Noble dedicated-server requirement list if scope expands; no need to copy the service |
| 15 | Relay/NAT fallback pattern | direct-first connections, optional relay for NAT failures, UI warns about IP visibility | if we ever add relay, use direct-first + explicit privacy warning; do not build it now |
| 16 | Network-mode UX | Offline / Host Online (LAN) / Host on Server Browser as explicit host choices | simplify Noble hosting UX around the same three intents (we already have LAN/host options) |
| 17 | Forge/Firefight authoring features | spawn/firing/dropship waypoints, wave traits, AI limits 96/128/192, Save-and-Test | input to our Forge/AI workstream requirements; implement our own equivalents |
| 18 | Map/gametype sharing flow | File Share packs rotations + maps; import/export; Maps folder management | reference UX for our mod/pack sharing path |
| 19 | Bug-report/diagnostics UX | run logs with masked IPs, per-run folders, bug-report bundles | improve Noble log bundling/privacy masking if not already done |
| 20 | Menu/content velocity ideas | WebView2 themes/animations/guides/changelog/MOTD | reference for features we might add to Noble's in-engine UI (animations, guides, MOTD); not a reason to rewrite the UI stack |

### 2.3 Low value / fact-only material already extracted

| # | Material | Status |
|---|---|---|
| 21 | RR shipped player-limit facts (64 / 81 / 128 experimental; 2-per-machine; big_lobby=81) | documented in the reference doc; useful for comparisons and player-facing expectations |
| 22 | Engine RVA list seen in install_stream128 (0x421340..0x421bd3) | recorded as candidate capacity-related patch locations; useful cross-check when we widen masks/sessions - verify independently against our engine, do not reuse their bytes |
| 23 | Module naming/layout (capacity, lobby_profile, m64_transport, memberx, m64_member, m64_update, p64_actions, scoreboard) | useful as a code-organisation map for our own work |
| 24 | Changelog/README statements about features and limitations | reference/requirements material only |
| 25 | The decompile workspace and method (Ghidra + Rust demangler + retained symbols) | our own tooling; method reusable |

## 3. What we cannot/should not take (beyond the hard boundary)

| Item | Why |
|---|---|
| RR's whole own-network stack (master, relay, voice, dedicated services, launcher) | permanent second product/ops burden and version lock; Noble stays MCC-integrated (see trade-off section in the reference doc). Only revisit if the product goal becomes hosting 64/128 without MCC networking. |
| RR's launch-block/version-lock mechanism | breaks on every MCC update; not a model for Noble's patch gating. |
| RR's WebView2 UI architecture as a port target | it is a whole second UI project; Noble's in-engine UI + our own work is the product. Take UX ideas only. |
| RR's protocol compatibility (joining RR servers) | not a goal; requires their formats and service; avoid IP/licence exposure. |
| RR's experimental surfaces as-is (M128 2-per-machine, 81-machine dedicated, no host migration) | documented experimental/unvalidated; treat as requirements caution, not target behaviour. |
| RR's progression/accounts/chat/voice/moderation implementations | Noble/MCC already has these (owner-confirmed); duplicating is wasted surface area. |
| RR's content (armour packs, Forge maps, emblems, audio, menu art) | licence/content ownership; we must author/curate our own. |

## 4. Rule of thumb for other agents

Take behaviour, structure, requirements and verified facts. Never take code, bytes, assets, strings or data. When in
doubt, write down the RR idea in the reference docs and implement it independently; if an RR-derived fact is needed
(e.g. an RVA or a limit), record it as a candidate to verify against our engine, not as a patch to copy.

