# Prophets' Hand — cache probe for the ultimate Elite model (2026-10-31)

**Goal:** dump the `elite_ai_ultimate` tag/permutation table so Honor Guard / Council / Ossoona pieces can be isolated.

## What worked

HREK is installed at `D:\Steam\steamapps\common\HREK`. Its `tool.exe` can read the compiled mod cache header:

```text
tool.exe dump-cache-file-header-info <m10.map>
```

Result for `maps\m10.map`:

- header signature `1751474532`, version `13`, size `1,220,972,544`
- tag path `levels\solo\m10\m10`
- build `Jul 16 2023 16:12:13`, scenario `solo`, fmod `YES`
- sections:
  - header 40,960 bytes
  - debug / tag names 4,374,528 bytes
  - resource 1,050,271,744 bytes
  - tag data 162,922,496 bytes at `0x3ECDD000`
  - language pack 3,362,816 bytes

So the cache layout and the tag section are known.

## What did not work

- `tool.exe dump-tag-file-index <m10.map>` → `tag file index looked corrupted`. It expects a tag-file index, not a compiled cache.
- `tool.exe export-tag-to-xml` operates on the HREK **project tag datastore** (`project.xml`/`tagDb.sdf`); it cannot open a compiled scenario cache.
- Raw tags are not source tags: the tag section is compiled/relocated data, not a `tags\` directory.
- `dump-tag-file-index` cannot enumerate `bipd`/`hlmt`/`render_model` entries from this file.

## Consequence

The model/permutation names are known from the string table, but their **geometry and tag headers are still inside the compiled cache**. HREK cannot export them without a cache tag-index reader or a runtime tag query.

## Next options

1. **Armour-only pack from O7OF/the mod author** — still the fastest path; HREK can consume a normal tags/pack build.
2. **Write a cache tag-index reader** — parse the 40 KB header and tag index, then locate the `render_model`/`biped` tag data. This is a new tool, not a flag.
3. **Runtime tag query** — load the mod map and ask `haloreach.dll` for tag data by name; needs the engine's tag lookup address and a test harness.

No geometry was extracted in this probe. Do not claim the Honor Guard/Council pieces are available until one of the three paths succeeds.
