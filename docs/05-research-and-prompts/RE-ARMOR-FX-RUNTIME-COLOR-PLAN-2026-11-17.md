# RE plan - runtime particle colour override for armor flames (2026-11-17)

Status: **chosen approach**, not started. Goal: make the 30 primary-colour flames work on every stock map for Noble
players without rebuilding/shipping maps, and retire the extra-effect-attachment design (which is capped at 4 extras =
24 colours by engine gating).

Related: `HANDOFF-ARMOR-FX-COLOURS-2026-11-17.md`, `RE-ARMOR-FX-COLORS-2026-11-15.md`,
`RE-VISOR-COLOR-2026-11-15.md`, `../../docs/02-engine-reference/TOPIC-armor-appearance-colors-armory.md`.

## Why runtime
- Map rebuild route measured at ~2.8x stock size: all 44 Reach maps ~25-30 GB built; MP-only ~11-12 GB; map mods are
  custom-games-only and every peer needs them.
- Extra effect-tag route: only 5 armor-effect attachments total gate correctly (stock + 4 extras) -> 24 of 30 colours.
- Runtime recolour of the stock flame particle reaches all maps, all players running Noble, with no map download.

## What we know
- Stock flame effect: `objects\characters\spartans\fx\armor_fx\armor_fx.effect` event `legendary_flame` ->
  `particles\head_fire.particle`; the particle is palette-driven. It references `fire.bitmap` (and a covenant-fire
  palette) plus blue-plasma palettes for secondary bits.
- Proven colour path so far: patch a *tag* palette reference / palette pixels and rebuild the map (blue-flame proof).
  Native "change color"/particle colour-field tinting was tried previously and left flames orange (see ledger 1311).
- Armory/material tint path works only in the menu map (rmsh scan finds 0 tags in match maps); not usable for matches.
- Engine particle pools: `c_particle_emitter_gpu`, `c_particle_gpu`, `c_particle_emitter_gpu::s_row` (pool resizer
  already knows them). Effect/particle init RVA area 0xAAE8460/0xAAE8976/0xAA49D8 in EngineLimits notes.
- The profile effect item id is already written to profile fields +0x68/+0x90 and maps to custom ids 0xF0..0x10D for
  Noble; a remote Noble client receives the appearance record. Whether the custom id survives to all peers needs a
  network test (fallback: auto-match from replicated primary colour).
- Effect attachments/marker gating: 5 total effect attachments is the empirical limit; irrelevant for runtime approach.

## Approach candidates (ranked)
1. **Per-particle-system palette pointer override (preferred).**
   Find where a spawned particle system stores its palette bitmap/GPU palette texture handle. Hook spawn or first update,
   resolve the owning object -> player appearance -> selected variant/primary colour, and point that system at a
   runtime-generated palette texture (or one of 30 preloaded/generated palettes). Per-player correct, no draw-call owner
   lookup needed if done at spawn.
2. **D3D palette-SRV swap at particle draw.**
   Hook `PSSetShaderResources`/particle draw, identify the flame palette texture (size/content) and swap to the palette for
   the currently drawn system's owner. Requires the draw path to expose owner/system identity.
3. **Shader-constant tint of the particle shader.**
   If the flame particle shader has a tint/colour constant (like visor `visor_tint`), upload a per-draw tint. Native tag
   tinting failed; but a render-state upload hook could still work. Must verify shader parameter exists and is used by
   the palette path.
4. **Local-only palette substitution** (replace the fire palette globally per client with the local player's colour).
   Cheap proof, not correct for other players' colours; only as a stepping stone/fallback.

## Milestones
- **M0:** get Ghidra MCP callable from the agent session (currently `mcp__ghidra__*` tools are not exposed in run_code;
  check the MCP plugin wiring/direct tool exposure). Without it, runtime RE is blocked on static analysis.
- **M1:** prove a visible runtime tint on stock maps without rebuilding: e.g. force the local player's legendary flame
  magenta (or pure blue) on `20_sword_slayer` using only client memory patching. Capture front/third person.
- **M2:** per-effect colour: map all 30 palette colours; local player only; verify no map tag edits and no mod maps.
- **M3:** per-player: remote Noble clients see each other's selected colour. Test host+guest, check appearance record
  replication and fallback when the peer has no custom id.
- **M4:** retire the extra-attachment build path: Armory list back to all 30 + auto; remove ext attachments and the
  dropped stock attachments; keep code/assets for reference or remove dead tags.
- **M5:** docs + tests + no-map-download verification on several stock maps (forge, boneyard, sword slayer, campaign).

## Technical starting points (Ghidra / static RE)
- Find refs to palette bitmap tag paths/strings and to `head_fire.particle`; but runtime tag pointers are cached, not
  looked up by name (previous tag_get trace found no calls).
- Find the particle renderer: look for functions using particle GPU pools / palette SRVs, and the effect spawn path
  (effect tag events -> particle systems). Pool names and init RVAs above.
- Candidate: particle-system render/update uses a palette texture per system; inspect struct fields for a bitmap/
  texture pointer, palette animation state and owner/object handle.
- Use existing hooks framework (`src/hooks/EnginePatches.cpp`, MinHook, signature-checked; log RVAs, patch profiles
  `data\limits`). For a first test, a manual byte patch/trampoline + log is fine, no repo commit until proven.
- Previous negative results to avoid repeating: patching shader tags (match maps don't use rmsh scan), patching particle
  color/self_illum tag fields (flames stayed orange), patching visor colour setter slot 6 (no visual change).

## Test protocol
- Kill `noble-dev`/`MCC-Win64-Shipping` before each run; a stale process shows the previous build.
- Use `-third-person -third-person-front` for front captures; `-capture-at`, `-quit-after`, `-NoFocus`.
- No map rebuilds/overrides for M1+: load stock `20_sword_slayer` (or another stock map) with normal launch.
- Log: current effect id, primary colour, particle system/owner pointer, chosen palette; capture front/third person.
- Verify with at least two colours and two maps; check other players' flames in a 2-client lobby.

## Risks
- GPU particle batching may not expose per-system palette at draw; spawn-time pointer may be copied into GPU buffers
  (then need buffer/copy hook).
- Network replication of custom appearance effect ids to peers is unverified; fallback auto-match/primary colour is
  replicated and may be the safe default.
- Anti-cheat/matchmaking policy for client-side cosmetic runtime patches (custom games/Noble peers only).
- Ghidra MCP availability; time to locate particle render path.
