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


## 2026-11-17 03:05 - concrete runtime findings (Ghidra MCP restored + RenderDoc)

### Tooling now working
- GhidraMCP headless bridge was started manually and is reachable at http://127.0.0.1:8081/mcp:
  `GHIDRA_INSTALL_DIR=C:\tools\ghidra_12.1.4_PUBLIC` and
  `python -m ghidra_mcp --headless --binary C:\Users\tyler\Documents\deepseek-harness\default-workspace\haloreach-copy.dll
   --project C:\Tools\GhidraMCP-headless --project-name haloreach-copy_ghidra_mcp --transport streamable-http
   --mcp-host 127.0.0.1 --mcp-port 8081`
  (pyghidra + ghidra_mcp installed into the DSH Python; extension jar already in the Ghidra install).
- The Harness MCP client stayed "server is disconnected" even after the bridge came up, so use the direct client
  `tools/ghidra_mcp_call.py` / `tools/ghidra_batch.py` (Python MCP client over streamable HTTP). Program:
  haloreach-copy.dll, image base 0x180000000, 27,709 functions.
- RenderDoc 1.46 is installed. Capture method that works: launch `renderdoccmd capture ... noble-dev.exe ...`,
  focus the **engine window** (`HaloReachCE_WindowClass`, not the launcher wrapper) and send F12; captures written to
  the `-c` template directory (**only when the engine window really has foreground focus**).
  Replay with `qrenderdoc --python script.py` (script runs before UI; write results to a file).
  Useful replay APIs: `pyrenderdoc.LoadCapture`, `pyrenderdoc.Replay().BlockInvoke(fn)`,
  `controller.GetRootActions()`, `controller.SetFrameEvent(eid, True)`, `controller.GetPipelineState()`,
  `pipe.GetShader(stage)`, `pipe.GetShaderReflection(stage)`, `controller.DisassembleShader(pipe.GetGraphicsPipelineObject(), refl, target)`.
  Note: child window captures can be black if F12 fires before the match frame; frame numbers from a capture with the
  third-person Spartan visible worked (frame 863 in `rdoc2`).

### Armor-flame draw identified (RenderDoc, capture frame 863)
- Five draws (events 10055, 10058, 10061, 10064, 10067), each 4 indices (billboard quad), same pipeline:
  - Pixel shader resource 8701; reflection: `UserParameterTexture_base_map`, `UserParameterTexture_base_map2`,
    `UserParameterTexture_palette`, `UserParameterTexture_alpha_map`, `depth_buffer`.
  - Vertex shader 8693; VS resources include `vs_particle_state_buffer`; VS constant blocks include
    `ParticleIndex`, `ParticleRenderVS`; so this is the particle renderer (not contrail).
  - Palette texture: **256x16 B8G8R8A8_SRGB**, RenderDoc ResourceId 34208; base_map 128x128 / 256x256 /
    alpha_map 512x512.
- PS disassembly (`ps_5_0`): samples alpha map, then base_map, then palette at UV `(base_map.r, v3.z)`;
  final colour `o0.xyz = palette * v1.xyz + v2.xyz` (per-vertex v1/v2 carried through the VS). There is **no
  per-draw colour constant block** in PS (only `ExposurePS`), so per-player colour cannot be selected from the PS CB;
  the palette is a shared texture and all five draws bind the same palette. Per-player implementation needs either:
  (a) per-particle/per-system palette index encoded in a vertex attribute / structured buffer, or
  (b) split/bind a different palette per owning player's draw, which needs the renderer to expose the owner.

### Runtime prototype written (partially working)
- `src/engine/ParticleTint.hpp/.cpp` + CMake source entry; dev flag `-armor-fx-runtime-tint r,g,b` (or rrggbb).
- It hooks `ID3D11DeviceContext::PSSetShaderResources` (vtable index **8** — ID3D11DeviceChild adds
  GetDevice/GetPrivateData/SetPrivateData/SetPrivateDataInterface before the context methods; index 4 is wrong),
  detects SRVs whose texture is 256x16 BGRA(SRGB), and substitutes a runtime-generated palette SRV (per-device cache).
- Hook install currently happens early in `main.cpp` (before engine Initialize). It successfully installs on the
  launcher's immediate context and detects palette candidates there, **but it never sees the game's particle
  draws**: the engine does not call the launcher's `D3D11CreateDevice*`/deferred-context paths we hooked. The game
  keeps rendering stock flames (verified capture). Next step: capture the engine's actual device/context (likely
  created in `SC_DLL.dll`/D3D11On12 or a context interface version whose method address differs), or hook the
  engine's particle submit path directly; then the existing palette substitution can be reused.
- No crash; flag/hook are inert when the flag is absent. Current build retains the dev code; it is not a user feature.


## 2026-11-17 04:03 - M1 PROOF: runtime palette override on a STOCK map

**Result:** stock Forge World (no map override, no mod), stock LEGENDARY flames, launched with
`-armor-fx-runtime-tint 255,0,255` -> the flames render **magenta**. Captured in
`docs/evidence/armor-fx-primary-2026-11-17/runtime-tint-proof.png`. No map/tag rebuild involved.

**What made it work (after four failed hook placements):**
- The engine renders with its own D3D11 context, not the launcher's immediate context. The reliable way to get
  the engine's real device/context is the **DXGI Present hook**: Noble's existing `HookSwapChainPresent` receives
  every swapchain; `swapChain->GetDevice(ID3D11Device)` + `device->GetImmediateContext()` returns the engine's
  immediate context. `ParticleTint::HookFromSwapChain()` is called at the top of the Present hook and installs the
  palette hook on that context.
- PSSetShaderResources vtable index correction: **8**, not 4 (ID3D11DeviceChild starts at index 3 with
  GetDevice/GetPrivateData/SetPrivateData/SetPrivateDataInterface; VSSetConstantBuffers is 7).
- Hooking `D3D11CreateDevice*` exports and `ID3D11Device::CreateDeferredContext` did not capture the engine context;
  the swapchain route did. Hook all context interface versions (base + ID3D11DeviceContext1..4) because the engine may
  call through a higher-version interface.
- Matched texture: **256x16, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB (fmt 91)**. Palette detection uses those dims/format and
  swaps in a runtime-generated 256x16 SRGB ramp (colour parsed from the flag). It creates one tint SRV per device.
- The palette is shared by the particle systems in the frame, so this M1 proof tints all flame particles in the
  process (single-player capture). M2 (per-player) still needs the owning player's variant/colour to choose a
  per-system or per-particle palette.

**Current dev flag:** `-armor-fx-runtime-tint r,g,b` (or 6-digit hex). Inert without the flag; no crash observed.
**Known side effect to check:** other particle systems that use the same 256x16 palette may also tint (the capture
showed a slight magenta cast in a cloud/haze behind the window); restrict detection to the particle PS / armour
effect path if it matters.

**Next (M2):** identify the owning player/system for each particle draw (likely via VS constant buffers
`ParticleRenderVS` / particle state buffer, or by splitting palette binds per system) and select one of 30 palettes;
then M3 peer replication.


## 2026-11-17 04:12 - M2 investigation: no per-player colour in particle CB

RenderDoc/qrenderdoc inspection of the five flame draws (frame 863, events 10055/58/61/64/67):
- VS shader **8693**, shader hash `f54376d4-5c51d3b9-77511f98-85bba44f`, `vs_5_0`.
  Constant blocks: ParametersVS, ViewVS, ExposureVS, LightingVS, FogVS, MeshVS, **ParticleIndex** (16 B),
  **ParticleRenderVS** (592 B), AtmosphereFogVS.
  `ParticleRenderVS` contains: `hidden_from_compiler`, `local_to_world` (3 rows), `local_to_world_pad`,
  `g_motion_blur_state`, `g_sprite` (`m_corner`), `g_render_state` (main/appearance/animation flags, first_person,
  curvature, uv_scroll_rate, game_time, billboard_type, vertex_count, fade, **custom_vertex_0/1**), and sprite frames.
  There is **no colour/team/player field**.
- VS reads `vs_particle_state_buffer` (structured, stride 80) at offsets 52/72 to produce the per-vertex values; PS
  then does `o0.xyz = palette * v1.xyz + v2.xyz`.
- Conclusion: per-player colour cannot be selected from the constant buffers. Options for M2:
  1. Find where the particle **state buffer** is filled and encode the owning player's palette index in an unused
     per-particle field, then patch the PS to use it (palette atlas UV offset). Requires a shader patch/DXBC replace
     plus buffer access.
  2. Split per-player particle systems and bind a different palette texture per draw; needs owner visibility in the
     draw path (not exposed in CB; would need engine particle-system struct/owner).
  3. Local-player-only tint first: use the local profile's selected effect/primary colour for the global palette (all
     flames in this client show that colour); a useful interim if full per-player is deferred.
- `tsh`: RenderDoc API note for future work: `GetCBufferVariableContents(pipeline, shader, stage, entryPoint,
  cbufslot, buffer, offset, length)`; `ShaderVariable.value` fields did not print through the Python wrapper in this
  version (values need reading raw via `GetBufferData` or a newer wrapper).
- Known side effect: matching any 256x16 BGRA(SRGB) palette also catches other particle systems using the same
  palette; the magenta proof also tinted a little haze/cloud in the window. Restrict to the particle PS/shader hash
  (or a dedicated palette pointer) before shipping.


## 2026-11-17 04:47 - Steps 1-3 implemented (runtime colours now usable)

1. **Restricted matching to the particle palette bind:** PSSetShaderResources slot **2** only, 256x16
   DXGI_FORMAT_B8G8R8A8_UNORM_SRGB (fmt 91). This stops the hook tinting other particle systems that happen to use a
   same-sized palette; the earlier cloud/haze cast is gone in the slot-restricted captures.
2. **Runtime palette selection from the 30 primary colours:**
   - `ParticleTint` now accepts `-armor-fx-runtime-variant N` / `-armor-fx-runtime-primary N` (0..29) in addition to
     the manual RGB flag.
   - The real path is profile-driven: when the selected `armorEffect` is a primary-flame variant, ReachHost writes the
     **stock LEGENDARY effect item** to the profile (so the flame particle exists on every stock map) and calls
     `ParticleTint::SetVariantIndex(variant)`. `PRIMARY MATCH` uses the primary colour index. Non-variant effects turn
     the override off.
   - Armory list expanded back to **38 entries** (7 stock + auto + 30 variants); the six previously-hidden colours
     (rust/coral/peach/gold/yellow/pale) are selectable again and are rendered by the runtime palette (their Armory
     preview currently falls back to the stock flame effect - see caveat).
   - Catalog rows added for the six; a crash from expanding the array without the six entries (null `stringId`) was
     fixed by inserting them with CRLF-correct lines.
3. **Verified automatic path on stock Forge World (no runtime CLI flags):** `-armor-effect-test 35` = GOLD FLAME
   produced gold flames; 32 = rust, 37 = pale, 30 = brick also captured. Contact sheet:
   `docs/evidence/armor-fx-primary-2026-11-17/runtime-variants-contact.png`. Armory screenshot with 30 entries:
   `runtime-armory-gold.png`.

**Caveats / next:**
- Armory **preview** for the six restored variants shows the stock flame colour (the menu preview path does not yet run
  through the runtime palette; tiles are labelled correctly but thumbnails are Tyler's photo pass).
- Per-player selection is still M2 step 4: no owner/colour in `ParticleRenderVS`; VS uses
  `vs_particle_state_buffer` (stride 80) and PS does `palette*v1+v2`. Current runtime colour is local/global.
- All 30 colours should be tested by Tyler in match/Forge; the palette generation is one shared code path, and
  gold/rust/pale/brick plus the original magenta proof are confirmed.


## 2026-11-17 05:10 - Last six colours FIXED (rust / coral / peach / gold / yellow / pale)

**Symptom:** the other 24 flame variants worked; the six restored entries showed the stock orange flame and their Armory
tiles looked stock. Two separate causes:

1. **Match/profile:** custom effect selections for these six have no extra effect tag slot, so the old path wrote a pmcg
   item with no attachment. ReachHost now writes the **stock LEGENDARY effect item** for every runtime variant and lets
   `ParticleTint::SetVariantIndex` supply the palette. Verified on stock Forge World: rust (32), coral (33), peach (34),
   gold (35), yellow (36), pale (37) all produce the correct flame colour. Proof:
   `docs/evidence/armor-fx-primary-2026-11-17/runtime-six-colours.png`.
2. **Armory equipped preview:** the six were previewed through `legendary_flame`, whose menu-map palette is not the
   256x16 particle palette the runtime hook matches. They are now previewed through the **steel custom effect** (an
   existing custom effect with the 256x16 palette) and the runtime palette recolours it to the selected colour. The
   Armory context is hooked through the existing Armory depth-clear hook (`ParticleTint::HookFromContext`), so the
   equipped character preview shows the correct colour (gold screenshot: `runtime-armory-gold.png`).
   - The six **grid thumbnails** are still generic stock pictures until Tyler's photo pass; the equipped 3D preview and
     in-match flames are correct.
