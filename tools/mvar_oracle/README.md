# mvar format oracle

ReachVariantTool's reference map-variant parser (DavidJCobb/ReachVariantEditor, `html/src/raw/map_variant_mvar.js`,
unmodified) run headlessly against real MCC `.mvar` files, with a small stream shim that provides the API it expects:
MSB-first bit reads, byte-aligned little-endian integers, big-endian floats.

This is the ground truth for porting the format into `src/mvar`. Run it with Node:

    node tools/mvar_oracle/run_ref.js "<path to .mvar>"

What it has established so far:

* the chunk starts at +0x2F0 inside the BLF container (signature `mvar`, size 0x7028, version 0x1F, flags 0x1);
* the map id is a **32-bit bit field** read immediately after the three 3-bit fields (`activity`, `gameMode`,
  `engine`) with no byte alignment between - aligning first yields 24055 instead of 3006 for Forge World;
* the plain header fields (size, version, flags) are big-endian on disk, the rest little-endian;
* `VariantContentAuthor` is two 64-bit values, a variable-length ASCII name (max 16 bytes, null terminated) and a
  1-bit online flag - its variable length is why field offsets differ by a couple of bytes between files;
* with that, the forged-label table reports 7 labels for `atom.mvar` and the second copy of the map id lands at
  bit 2760.

Still to nail down: the exact author string read and the title/description wide-char alignment (the oracle currently
yields "tom" for "Atom", an 8-bit shift), then the object records.
