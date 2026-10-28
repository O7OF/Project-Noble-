# FOV in Halo: Reach (`haloreach.dll`) — static findings

Target: `...\Halo The Master Chief Collection\haloreach\haloreach.dll`
SHA-256 `738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894`, PE x86-64,
image base `0x180000000`, size `0x4EDA000`. Analysed with REA 6.8.0 + Ghidra 12.1 (read-only).

## Answer

**The game's field-of-view value is 78 degrees.**

| | |
|---|---|
| VA | `0x180B42C10` (RVA `0xB42C10`, file offset `0xB41210`) |
| Type | `float`, degrees |
| Bytes in the shipped DLL | `00 00 9C 42` = `0x429C0000` = **78.0f** |
| Uniqueness | only `78.0f` in every non-`.text` section of the DLL |

Supporting code:

* `FUN_1800CAD04` (camera matrix build) reads it, converts degrees→radians with
  `0.017453292` (π/180), and **falls back to `1.3613569` rad — exactly 78° — when the value
  is not valid**.
* `FUN_1800CAFF4` is the FOV validator: true only for finite
  `0.017453292 ≤ fov ≤ 2.6179938` rad, i.e. **1° … 150°**.

## How the engine consumes it

```
FUN_1800CAD04(camera):
  fov  = *(float *)(camera + 0x40)                  ; radians
  vfov = *(float *)(camera + 0x6c) = 2*atan(tan(fov/2) / aspect)
  ref  = DAT_180B42C10 * 0.017453292                ; 78° → 1.361357 rad
         (if the validator rejects it → 1.3613569)
  *(float *)(camera + 0x70) = fov / ref             ; FOV ratio (78° reference)
```

`FUN_1801E2D2C` keeps `DAT_180B42C10 … DAT_180B42CBC` (0x2C floats = 176 bytes) in sync with the
per-user settings block (`pfVar1`); **FOV is element `[0]` of that block**. The refresh only
overwrites a global whose current value still equals the previously copied value, so a value
written from outside survives the refresh cycle.

## Script-side setter: `camera_set_field_of_view`

* name string RVA `0x9FDCC0` (VA `0x1809FDCC0`)
* HS definition record RVA `0xA18B78` (0x48-byte entries); implementation wrapper VA `0x1801AD2EC`
* wrapper: the argument is **degrees** → `× 0.017453292` → radians → validated by
  `FUN_1800CAFF4` → stored in the per-thread camera override (`+0x10`), with duration at `+0x8`
  and timestamp at `+0xC`; rejected input clears the override.

So: degrees in, **1–150** accepted, radians internally.

## Related FOV globals

| Global | Kind | Variable RVA |
|---|---|---|
| `render_first_person_fov_scale` | float | `0x4E389F4` (BSS, runtime-initialised) |
| `reduce_widescreen_fov_during_cinematics` | bool | `0x4E38F52` (BSS) |
| `camera_fov` | globals-table record RVA `0xB42948` | value pointer not bound in the static image |

## Implications for Noble's FOV slider

* Write `base + 0xB42C10` as a float in **degrees**, clamped to `1…150`.
  The current slider range 60–120 sits inside the engine's accepted range.
* Or call `camera_set_field_of_view(degrees)` (script path) for a timed override.
* 78° is the compiled-in default; the live value is replaced from the per-user settings block at
  run time, so a runtime capture is still needed to confirm the value the engine actually holds
  during play.

## How Noble implements it (0.8.2)

* `src/engine/CameraFov.{hpp,cpp}` — `SetEngineModule()` (called from `main.cpp` right after
  `LoadReachDll`), `Apply(degrees)` (VirtualProtect + write of the float), `Read()`, and `Tick()`.
* `ReachHost` slot 1 calls `CameraFov::Tick()` every engine frame: it re-reads
  `Settings::fieldOfView` at most every 250 ms, writes the global when the value changed, and
  re-asserts the same value every 2 s (the engine's own settings refresh re-applies the block
  value, and its "only if unchanged" guard lets an outside write survive).
* `SETTINGS > VIDEO > FIELD OF VIEW`: APPLY calls `CameraFov::Apply()` immediately. Default and
  RESET are **78**, Reach's own value, instead of the earlier placeholder 90.
* The pause menu (Esc) has a **SETTINGS** entry that renders the same `SettingsPanel` over the
  game, so the FOV — and every other setting — can be changed mid-match. `< BACK` or Esc returns
  to the pause menu; Esc while rebinding a key still only cancels the rebind.
* Unverified in game: the camera may cache its FOV when it is built, in which case the value
  takes effect on the next map load/respawn rather than on APPLY. The log line
  `CameraFov: game field of view set to N degrees` confirms the write.


## Evidence (REA)

| Evidence | Operation |
|---|---|
| `ev_f6a3a6e36f763c5e43823aa8f7642dce0b0adee28eac18b3a9cf52de7910b381` | `search_strings` "camera_set_field_of_view" |
| `ev_e9244e7ec58b809c32ea5e0b79e3bf962e398da4a4ca804b89ee781ebf610849` | `procedure_pseudo_code` `0x1801AD2EC` |
| `ev_fa90de81ec5b52ff5a8bf188f51dc3e8922adc7e05def170da90712a73eba96b` | `procedure_pseudo_code` `0x1800CAFF4` |
| `ev_2e7b6b9ccb600507b9821aeb66fb28598e1cc3e54a9294614781527c2eb421d0` | `procedure_pseudo_code` `0x1800CAD04` |
| `ev_f3620473621c41cda3550a6923a697296443ce0660085a5c5e9706fa8af0e534` | `procedure_pseudo_code` `0x1800C8558` |
| `ev_2fd531fa6b34fab4af5d19235844b5f25c55b268490ba48a1cf0d249ae6405de` | `procedure_pseudo_code` `0x1801E2D2C` |
| `ev_08a52480953936753fa11673ada224687c7fcd2288e2bf69fc4131c84a6f2237` | `xrefs` `0x180B42C10` |

Limitations: static analysis only — no execution was observed; no game process was running.
Decompiler text is Ghidra pseudocode, not original source. The DLL references
`halo3_dll_cache_release_UUI_x64.pdb` but no symbols were recovered.
