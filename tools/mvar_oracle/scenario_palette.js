// Parses the scenario tag exported by HREK's tool.exe ("export-tag-to-xml") into the map's build palette:
// flat entry index (= the variant's variant_quota_index), the entry name, and each variant's object tag path
// (= variant_index within the entry). This is the authoritative per-map palette; it needs no game session.
//
//   node tools/mvar_oracle/scenario_palette.js <scenario.xml> <mapId> <out.json>
const fs = require("fs");

const [xmlPath, mapId, outPath] = process.argv.slice(2);
if (!xmlPath || !mapId) {
  console.error("usage: scenario_palette.js <scenario.xml> <mapId> <out.json>");
  process.exit(2);
}
const lines = fs.readFileSync(xmlPath, "utf8").split("\n");
const stack = [];
const entries = [];
let inPalettes = false;
for (const raw of lines) {
  const line = raw.trim();
  if (line.includes('name="map variant palettes"')) { inPalettes = true; continue; }
  if (!inPalettes) continue;
  if (line.startsWith("</element>")) { stack.pop(); continue; }
  const elem = line.match(/^<element index="(\d+)" name="([^"]*)">/);
  if (elem) { stack.push({ index: +elem[1], name: elem[2] }); continue; }
  const obj = line.match(/^<field name="object" value="([^"]*)" type="tag reference"\/>/);
  if (obj && stack.length >= 3) {
    entries.push({ palette: stack[0].name, entry: stack[1].name, variant: stack[2].index, object: obj[1] });
  }
}
// Flat quota index = order of first appearance of an entry, matching the variant's quota table.
const byEntry = new Map();
for (const e of entries) {
  const key = e.palette + "/" + e.entry;
  if (!byEntry.has(key)) byEntry.set(key, { palette: e.palette, entry: e.entry, variants: [] });
  byEntry.get(key).variants.push(e.object);
}
const palette = [];
let flat = 0;
for (const value of byEntry.values()) {
  palette.push({ quota: flat++, palette: value.palette, entry: value.entry, variants: value.variants });
}
const result = { mapId: Number(mapId), entryCount: palette.length, entries: palette };
const json = JSON.stringify(result, null, 1);
if (outPath) fs.writeFileSync(outPath, json);
else console.log(json);
console.error("palette entries: " + palette.length);
