# mvar format oracle

ReachVariantTool's reference map-variant parser (DavidJCobb/ReachVariantEditor, `html/src/raw/map_variant_mvar.js`)
run headlessly against real MCC `.mvar` files, plus an independent, byte-exact reference implementation
(`@blamnetwork/blf`) used to settle the fields the 360-era parser gets wrong.

    node tools/mvar_oracle/run_ref.js <file.mvar> [more...]
    node tools/mvar_oracle/run_ref.js --scan <directory>

The runner is self-contained: `mvar_ref.js` is unmodified except for a trailing `module.exports`, and
`variant_content_author.js` supplies the one class the reference bundle expects from another file. The stream shim is
a true bit reader - reading whole bytes from `buf[pos >> 3]` silently discards the bit offset and corrupts everything
after the first unaligned field.

## What the format actually is

A map variant is a BLF container:

    _blf   header 0x30
    chdr   chunk  0x2C0 (byte-aligned copy of the metadata)
    mvar   chunk  0x7028
    _eof   chunk  0x11
    _fsm   chunk  0x1D0 (thumbnail; not part of the 0x7329-byte variant proper)

The `mvar` chunk is the one this module reads:

    0x00  "mvar", u32 chunk_length (big-endian), u16 major 31, u16 minor 1   <- BLF chunk header, 0x0C bytes
    0x0C  20-byte hash
    0x20  u32 packed_length (big-endian)
    0x24  packed bitstream (0x7000 bytes of storage)
    0x7024 4 zero bytes
    ---------------------------------------------------------------- 0x7028

Everything inside the packed bitstream is MSB-first bits; the "byte" fields are bit reads, not aligned reads.

## Region map (chunk-relative bits, atom.mvar)

    variant header        0      .. 0xEA4   (3,748)
    651 object slots      0xEA4  .. 0x131D7 (78,295)
    quota table           0x131D7.. 0x149D7 (84,439)   N x 24 bits
    zero padding          .. 0x149D8

`packed_length` counts bytes from bit 288 (chunk offset 0x24), so the padding check is
`288 + packed_length * 8 - quotas_end` in 0..7 bits. All 278 shipped Reach variants pass it.

## Object record (MCC layout)

ReachVariantTool documents the Xbox 360 record; MCC uses a different one, which the blf library
(`s_variant_object_datum`) reproduces byte for byte. Widths:

    presence                      1
    flags                         2
    quota index                   {1 + 8}      read_index: a presence bit, value only when clear
    variant index                 {1 + 5}
    position                      in-bounds 1, then one quantized field per axis
    axes                          up-is-global 1 {+20}, forward angle 14
    spawn relative to             10
    boundary shape                2 {+11 per field: 1 -> 1, 2 -> 3, 3 -> 4}
    user data                     8
    spawn time                    8
    cached type                   5
    label index                   {1 + 8}
    placement flags               8
    team                          4  (stored +1)
    primary change colour         {1 + 3}
    type tail                     cached 1 -> 8 (spare clips)
                                  cached 12/13/14 -> 5 + 5 (teleporter channel/passability)
                                  cached 19 -> {1 + 8} (location name)

Per-axis position bits, for a 21-bit target: `min(ceil(log2(min(ceil(range / (2 * maxError)), 0x800000))), 26)`
with `maxError = (1/120) / 2^(21-16)`. Note `ceil`, not floor: atom.mvar needs 20/21/19 bits, not 19/20/18.

## Quota table

The 9-bit field read just before the second copy of the map id is the placeable-object quota count N. After the 651
object slots come `min(256, N)` quotas of 24 bits each: minimum 8, maximum 8, placed-on-map 8. Atom has N = 256,
so the table is 6,144 bits.

## Hash

`sha1(GEN3_SALT || packed_length (big-endian) || packed_bytes[0..packed_length])` with

    GEN3_SALT = EDD43009666D5C4A5C3657FAB40E022F535AC6C9EE471F01F1A44756B7714F1C36EC

Shipped MCC files do **not** carry that hash: decoding and re-encoding 278 variants with the blf library reproduces
every payload byte but never the stored 20 bytes, so the file's hash is a legacy value the engine does not enforce.
The writer therefore preserves the stored hash when the payload is unchanged (which makes an unedited variant
byte-identical) and writes the canonical hash once an object is actually edited.

## Verification

* `mvar-tool` (headless, `tools/mvar_tool.cpp`) parses, repacks and byte-compares: 278/278 Reach variants identical.
* The same 278 files decode -> encode to identical bytes through `@blamnetwork/blf`, an independent implementation.
* `mvar-tool -nudge <in> <out> <slot> <axis>` moves one object by a single position quantum; the hash is recomputed
  and a re-parse sees the new value, so the writer really is splicing the object region, not copying the file.

## Field-order facts that are easy to get wrong

* The map id is a 32-bit *bit* field straight after three 3-bit fields, with no alignment; aligning yields 24055
  instead of 3006 for Forge World.
* `VariantContentAuthor` is 64 + 64 bits, a bit-packed ASCII name (max 16 bytes, null terminated) and a 1-bit online
  flag; its variable length is why later offsets move by a byte or two between files.
* Title and description are stored only up to the terminator, not as fixed 128-character fields.
* The header, authors, title, bounds and label table land at 3,748 for atom.mvar; the 651 object slots run from there
  to 78,295 and the quotas to 84,439.

## The engine's own serializer (for saving edited maps)

The hosted engine can build the container itself, and this is how an edited Forge map becomes bytes:

* `RVA 0x17AC38` - `void serialize(void* outChunk, void* variantStruct)`. Fills a `0x7028`-byte `mvar` chunk:
  `mvar`, size `0x7028`, version `0x1F`, flags `1`, packed length, packed data (via `0x6D04C`), and the SHA-1
  with `BCrypt`. This is the function to hook or call: it takes the live variant struct directly.
* `RVA 0x156974` - the save routine. Builds the whole `_blf` file (header, `chdr`, the chunk above, `_eof`) from the
  live session variant and hands it to the save device. Its two callers are `0x692CC` and itself via a data slot at
  `0x18E5FB2C`.

Noble hooks `0x17AC38` and keeps the chunk it produces, so a save works even when the save device writes nothing
(which is what happens in the hosted build: the routine runs, no file appears). The chunk is wrapped in an existing
container - the session's `.mvar`, or any shipped variant - by replacing bytes `[chunkOffset, +0x7028)` and patching
the `+0xC0` title; the engine reads the map and objects from the replaced chunk.
