"""Joins every Spartan Armory option into one table and checks it:
- the Armory's lists in src/ui/ArmorCustomizer.cpp (display name, permutation string id),
- the MCC item number ReachHost.cpp writes into the profile for it,
- the engine's armour list (pmcg selection names, from data/armor/rr_elite_pack.map.rrpmcg),
- Halopedia's name, picture, rank and cR (data/ui/reach/armory/pieces.csv) and docs/career/armory_overrides.json.
Writes docs/career/spartan_armory_catalog.json/.csv and prints problems. Standard library only.
  python tools/armory/build_spartan_catalog.py
"""
import csv, json, os, re, sys
REPO = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
if not os.path.isfile(os.path.join(REPO, "src", "ui", "ArmorCustomizer.cpp")):
    REPO = r"D:\Halo-Reach-Community-Edition"
P = lambda *a: os.path.join(REPO, *a)
REL = P("build", "bin", "Release", "data")

# Armory name -> Halopedia name, where they differ (same piece).
ALIAS = {
    "MJOLNIRMKVI": "MARKVI", "MJOLNIRMKV": "MARKV", "HYDRATION": "HYDRATIONCONTAINER",
    "FLAMES": "LEGENDARY", "BLUEFLAMES": "ETERNAL", "NONE": "DEFAULT",
    "AKISGRD": "AKIS", "CBRN@HAZOP": "CBRNHUL", "HUL@RECON": "HULI", "OPTREX": "GRD", "OPTREXCBRN": "GRDCBRN",
    "CORTANA": "CORTANAAI", "JOHN117": "JOHNS117", "CDRCARTER": "CARTERS259", "LCDRKAT": "KATS320",
    "WOJUN": "JUNS266", "WOEMILE": "EMILES239", "CWOJORGE": "JORGES052", "AUNTIEDOT": "AUNTIEDOTAI",
}

def norm(s):
    k = re.sub(r"[^A-Z0-9]", "", (s or "").upper())
    return ALIAS.get(k, k)

ac = open(P("src", "ui", "ArmorCustomizer.cpp"), encoding="utf-8").read()
def body(fn):
    i = ac.index(fn); j = ac.index("return", i); return ac[i:j]

# Armory lists
helmets = []
for m in re.finditer(r'\{\s*"([^"]+)",\s*"([^"]+)",\s*"[^"]*",\s*\{(.*?)\},\s*"', body("GetSpartanHelmets()"), re.S):
    variants = re.findall(r'\{"([^"]+)",\s*"([^"]+)"', m.group(3))
    helmets.append((m.group(1), m.group(2), variants))
shoulders = re.findall(r'\{"([^"]+)",\s*"([^"]+)",\s*"([^"]+)",', body("GetSpartanShoulders()"))
def simple(fn): return re.findall(r'\{"([^"]+)",\s*"([^"]+)",\s*"[^"]*",\s*(?:true|false)', body(fn))
chests, wrists, utils, knees = simple("GetSpartanChests()"), simple("GetSpartanWrists()"), simple("GetSpartanUtilities()"), simple("GetSpartanKneeGuards()")
def arr(name):
    i = ac.index(name); j = ac.index("}};", i); return re.findall(r'\{"([^"]+)",\s*"([^"]+)"', ac[i:j])
effects, voices = arr("kArmorEffects"), arr("kFirefightVoices")
visors = re.findall(r'\{"([^"]+)",\s*"([^"]+)",\s*[\d.]+f', ac[ac.index("kVisorColors"):ac.index("}};", ac.index("kVisorColors"))])

# MCC item ids (ReachHost.cpp)
rh = open(P("src", "engine", "ReachHost.cpp"), encoding="utf-8").read()
hid = {k: int(v) for k, v in re.findall(r'\{"([a-z0-9_]+)",\s*(\d+)\}', rh[rh.index("Helmets() {"):rh.index("Chests() {")])}
cid = {k: int(v) for k, v in re.findall(r'\{"([a-z0-9_]+)",\s*(\d+)\}', rh[rh.index("Chests() {"):rh.index("VoiceItem")])}
vid = {k: int(v) for k, v in re.findall(r'\{"([a-z0-9_]+)",\s*(\d+)\}', rh[rh.index("VoiceItem"):rh.index("} // namespace armory_ids")])}
base = {k: int(v) for k, v in re.findall(r'constexpr uint32_t k(\w+) = (\d+);', rh)}

# engine armour list (pmcg) selection names per list
pm = {}
cur = None
for line in open(os.path.join(REL, "armor", "rr_elite_pack.map.rrpmcg"), encoding="utf-8"):
    t = line.split()
    if not t: continue
    if t[0] == "L": cur = t[1]; pm[cur] = []
    elif t[0] == "S" and cur: pm[cur].append(t[1] if len(t) > 1 else "")
LIST = {"helmet": "20", "lshoulder": "2c", "rshoulder": "38", "chest": "44", "wrist": "50", "utility": "5c", "knees": "68", "effect": "80"}

# Halopedia + overrides
hp = list(csv.DictReader(open(os.path.join(REL, "ui", "reach", "armory", "pieces.csv"), encoding="utf-8")))
ov = json.load(open(P("docs", "career", "armory_overrides.json"), encoding="utf-8"))["spartan"]
def find_hp(slot, name, parent=None):
    for r in hp:
        if r["slot"] != slot: continue
        if parent is None and r["attachment_of"]: continue
        if parent is not None and norm(r["attachment_of"]) != norm(parent): continue
        key = re.sub(r"[^A-Z0-9]", "", name.upper()) + "@" + re.sub(r"[^A-Z0-9]", "", (parent or "").upper())
        if norm(r["name"]) == (ALIAS.get(key) or norm(name)): return r
    return None
def override(kind, name, parent=None):
    for o in ov.get(kind, []):
        if norm(o["name"]) == norm(name) and norm(o.get("attachment_of")) == norm(parent): return o
    return None
removed = [norm(x) for x in ov.get("removed", {}).get("helmets", [])]

rows, problems = [], []
def add(slot, idx, name, sid, item, pmlist, hprow, parent=None, ovr=None, note=""):
    pmidx = pm.get(LIST.get(pmlist, ""), []).index(sid) if pmlist and sid in pm.get(LIST.get(pmlist, ""), []) else None
    rank = (ovr or {}).get("rank") or (hprow or {}).get("rank") or ""
    rank = {"Lieutenant Colonel": "Lt. Colonel", "Field Marshal": "Field Marshall"}.get(rank, rank)
    cr = (ovr or {}).get("credits") if ovr else ((hprow or {}).get("credits") or "")
    rows.append({"slot": slot, "armory_index": idx, "name": name, "attachment_of": parent or "", "string_id": sid,
                 "mcc_item": item, "engine_index": pmidx, "halopedia_name": (hprow or {}).get("name", ""),
                 "picture": (hprow or {}).get("file", ""), "rank": rank, "credits": cr, "note": note})
    if item is None: problems.append(f"{slot} {name!r} ({sid}): no MCC item number in ReachHost.cpp")
    if pmlist and pmidx is None: problems.append(f"{slot} {name!r}: string id {sid!r} not in the engine's armour list")
    if hprow is None and not ovr: problems.append(f"{slot} {name!r}{' (on '+parent+')' if parent else ''}: no Halopedia match (no picture / rank / cR)")

for i, (name, sid, variants) in enumerate(helmets):
    if norm(name) in removed or any(norm(name) == r for r in removed):
        rows.append({"slot": "helmet", "armory_index": i, "name": name, "string_id": sid, "note": "REMOVED (overrides)"}); continue
    h = find_hp("helmets", name)
    add("helmet", i, name, sid, hid.get(sid), "helmet", h, ovr=override("helmets", name))
    for v, vsid in variants:
        if v.upper() == "BASE": continue
        add("helmet attachment", i, v, vsid, hid.get(vsid), "helmet", find_hp("helmets", v, name), parent=name, ovr=override("helmets", v, name))
for i, (name, l, r) in enumerate(shoulders):
    h = find_hp("shoulder", name)
    add("left shoulder", i, name, l, base["LeftShoulder"] + i, "lshoulder", h)
    add("right shoulder", i, name, r, base["RightShoulder"] + i, "rshoulder", h)
for i, (name, sid) in enumerate(chests):
    add("chest", i, name, sid, cid.get(sid), "chest", find_hp("chest_armor", name), ovr=override("chests", name))
for lst, slot, hs, key, pml in [(wrists, "wrist", "wrist_attachment", "Wrist", "wrist"), (utils, "utility", "utility", "Utility", "utility"),
                                 (knees, "knees", "knee_guards", "Knees", "knees"), (visors, "visor", "visor_color", "Visor", None)]:
    for i, (name, sid) in enumerate(lst):
        add(slot, i, name, sid, base[key] + i, pml, find_hp(hs, name), ovr=override(slot, name) if slot == "utility" else None)
for i, (name, sid) in enumerate(effects):
    add("armor effect", i, name, sid, base["Effect"] + i, None, find_hp("armor_effects", name), ovr=override("armor_effects", name))
for i, (name, sid) in enumerate(voices):
    add("firefight voice", i, name, sid, vid.get(sid), None, find_hp("firefight_voices", name))

# Halopedia pieces the Armory doesn't offer
used = {r["picture"] for r in rows if r.get("picture")}
for r in hp:
    if r["section"] == "Spartan" and r["file"] not in used and not any(norm(r["attachment_of"] or r["name"]) == x for x in removed):
        problems.append(f"Halopedia {r['slot']} {r['name']!r}{' (on '+r['attachment_of']+')' if r['attachment_of'] else ''}: not in the Armory")

os.makedirs(P("docs", "career"), exist_ok=True)
json.dump(rows, open(P("docs", "career", "spartan_armory_catalog.json"), "w", encoding="utf-8"), indent=1, ensure_ascii=False)
with open(P("docs", "career", "spartan_armory_catalog.csv"), "w", encoding="utf-8", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(rows[0].keys())); w.writeheader()
    for r in rows: w.writerow({k: r.get(k, "") for k in rows[0].keys()})
# C++ table for the Armory (src/ui/SpartanArmoryCatalog.inc): picture, rank and cR per option.
def cstr(v): return '"' + str(v or "").replace("\\", "/").replace('"', '\\"') + '"'
with open(P("src", "ui", "SpartanArmoryCatalog.inc"), "w", encoding="utf-8", newline="\n") as f:
    f.write("// Generated by tools/armory/build_spartan_catalog.py (docs/career/spartan_armory_catalog.csv). Re-run it after\n"
            "// changing the Armory lists, armory_overrides.json or the Halopedia download; don't edit by hand.\n"
            "// { slot, permutation string id, picture folder (under data/ui/reach), picture name, rank, cR }\n")
    for r in rows:
        if r.get("note", "").startswith("REMOVED") or not r.get("string_id"): continue
        pic = r.get("picture") or ""
        folder, stem = (("armory/" + os.path.dirname(pic)), os.path.splitext(os.path.basename(pic))[0]) if pic else ("", "")
        try: cr = int(r.get("credits") or 0)
        except (TypeError, ValueError): cr = 0
        f.write("{ %s, %s, %s, %s, %s, %d },\n" % (cstr(r["slot"]), cstr(r["string_id"]), cstr(folder), cstr(stem), cstr(r.get("rank")), cr))

print(f"{len(rows)} Armory options; {len(problems)} problems")
print("\n".join(problems))
