# HUD text and brightness: Noble vs Reach Renaissance (2026-10-27)

Findings from comparing Noble's `TextAtlas` / `GammaPass` (in `src/engine/ReachHost.cpp`) with
RR's `host-probe.exe` (`host_probe::text::*`, `host_probe::gamma::*`, read with `llvm-objdump -d -C`).
No Noble code was changed for this; it is a list of differences to fix.

Symptoms on Sword Base / Firefight: most HUD and scoreboard text shows as boxes or garbage, a few
strings render ("RECON", compass letters); HUD icons, compass and motion tracker look dim / dark blue.

## 1. Glyph record (slot 62, `text::glyph`): two layout bugs  ← most likely cause of the boxes

Signature matches Noble: `glyph(host, u16 ch, u8* out, u32 size, f32 scale, const char* face)`,
returns 1 and writes **26 bytes** to `out` (same size as Noble's `HostGlyphData`).
RR builds the record in `text::Atlas::glyph` (0x1402862a0, at 0x1402869c3..0x140286a0f) from the
`GLYPHMETRICS` returned by `GetGlyphOutlineW(hdc, ch, GGO_GRAY8_BITMAP, &gm, ...)`:

| Offset | RR writes | Noble today |
|---|---|---|
| +0x00 u16 | character | same ✓ |
| +0x02..+0x07 | **0** (it stores the u64 `x << 48`) | **atlasX** at +0x02 ✗ |
| +0x08 u16 | **atlas X** | 0 (padX) ✗ |
| +0x0A u16 | atlas Y | same ✓ |
| +0x0C u16 | gmBlackBoxX (0 if the outline is empty) | same ✓ |
| +0x0E u16 | gmBlackBoxY | same ✓ |
| +0x10 i16 | **gmptGlyphOrigin.y** | origin.x ✗ |
| +0x12 i16 | **gmptGlyphOrigin.x** | origin.y ✗ |
| +0x14 i16 | gmCellIncX (advance) | same ✓ |
| +0x16 i16 | -ascent (font field) | -tmAscent ✓ |
| +0x18 u8 | atlas page index | 0 ✓ (one page) |
| +0x19 u8 | 0 | 0 ✓ |

Suggested struct:

```cpp
#pragma pack(push, 1)
struct HostGlyphData {
    uint16_t ch;        // +0x00
    uint16_t pad[3];    // +0x02 .. +0x07  (zero)
    uint16_t atlasX;    // +0x08
    uint16_t atlasY;    // +0x0A
    uint16_t width;     // +0x0C gmBlackBoxX
    uint16_t height;    // +0x0E gmBlackBoxY
    int16_t  originY;   // +0x10 gmptGlyphOrigin.y
    int16_t  originX;   // +0x12 gmptGlyphOrigin.x
    int16_t  advanceX;  // +0x14 gmCellIncX
    int16_t  ascent;    // +0x16 -ascent
    uint8_t  page;      // +0x18
    uint8_t  reserved;  // +0x19
};
#pragma pack(pop)
static_assert(sizeof(HostGlyphData) == 26);
```

(Keep the field *names* used by `RasterizeGlyphLocked` / `RegisterIcon`; only the order changes.
`RegisterIcon` sets originX = 0, originY = h, which stays correct with the new order.)

With X in the wrong place the engine reads X = 0 for every glyph and samples the wrong part of the
atlas, which fits "mostly boxes, a few strings partly right".

Pixel format matches RR: RGB = 0xFF, A = min((gray * 255 + 32) >> 6, 255) from the 0..64 GRAY8
value; GRAY8 rows padded to 4 bytes; pages are 1024 px wide (0x1000 bytes per row).

## 2. Atlas texture (slot 63, `text::atlas`): matches

RR `text::d3d::create_texture`: square, MipLevels 1, ArraySize 1, `DXGI_FORMAT_R8G8B8A8_UNORM` (28),
SampleDesc {1,0}, Usage DEFAULT, BindFlags SHADER_RESOURCE, no CPU access. `text::atlas(page)`
returns that page's SRV (RR keeps up to 16 pages). Noble's single 1024x1024 page is equivalent as
long as it doesn't fill up (Noble wraps to (1,1) and overwrites old glyphs when full; RR adds a page).

## 3. Other text slots: to verify

* **Slot 66 `register_icon`**: RR reads `dx` = u16 char, `r8d` = u32 size, `xmm3` = f32 scale, then
  the 5th and 6th arguments as **32-bit integers** (stack +0x278 / +0x280 in its frame), not a
  `const char* face`. Noble's handler treats arg 5 as `face` and arg 6+ as width/height/... Worth
  re-deriving the argument list from RR's `text::register_icon` (0x14028cad0) before trusting the
  icon path (weapon/grenade icons in the HUD come through here).
* **Slot 65 `kerning(host, u16 a, u16 b, u32 size)`**: RR returns a real kerning value; Noble returns
  0. Cosmetic only.
* **Slot 64 `has_icon`**: argument use (r8d size, xmm3 scale, face on the stack) is consistent with
  Noble.

## 4. Brightness (`GammaPass`): not the HUD fix

RR `gamma::frame` runs the same shader as Noble (`pow(saturate(c.rgb), gamma.x)`), and RR's default
`gamma::EXPONENT` is **1.0** (no change; it is only the user "ui.gamma" setting). Noble's default is
also 1.0 now. So in RR the HUD is bright without any gamma correction; the dim HUD has another
cause. Noble's comment saying the engine "outputs linear HDR" and needs gamma is not supported by
RR's code.

Swap chain (RR `win::create_swap_chain`): R8G8B8A8_UNORM, BufferUsage 0x30, **BufferCount 1**,
SwapEffect DISCARD, **Flags 0**. Noble: BufferCount 2, Flags ALLOW_MODE_SWITCH. Probably harmless,
listed for completeness.

### Likely cause of the dim HUD: profile settings (to test)

Keys, volumes and zoom look were all blank-profile problems, fixed by filling the profile (slot 34).
Noble's built-in profile still leaves most of the header (+0x000..+0x1AB, +0x1E0..+0x42B) and the
tail (+0xA68..+0xBFF) zero. A HUD/display brightness or HUD-opacity setting there would explain the
dim HUD. Quick test, no code change:

```
noble-client.exe -map 20_sword_slayer -mode multiplayer -profile D:\Reach\data\profile_1.bin
```

If the HUD is bright with RR's captured profile, bisect the remaining profile regions the same way
the zoom-look block (+0x1B4..+0x1DF) was found, and add the default values to Noble's profile.
