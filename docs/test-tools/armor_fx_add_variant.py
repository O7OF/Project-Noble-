#!/usr/bin/env python3
"""Add the armour-effect colour variant 'legendary_flame_blue' to HREK tags (blue flames, pmcg index 7).

Run through docs/test-tools/runmb.ps1 (ManagedBlam in Blender). It edits HREK tags in place after writing .bak
backups, then rebuild Forge World with tool.exe and load it in Noble with -map-override.

What it changes (idempotent):
  armor_fx.effect            + event 'legendary_flame_blue' (copy of legendary_flame) -> head_fire_blue.particle,
                               location 4; location 4's marker renamed armor_fx_03_shoulders -> armor_fx_06
                               (ManagedBlam's Save drops newly added location elements, so the unused shoulders
                               location is reused instead)
  spartans.render_model      + region armor_fx permutation 'armor_fx_06'
                             + marker group 'armor_fx_06' (marker bound to permutation index 6)
  player_model_customization + selection 'legendary_flame_blue' -> armor_fx / armor_fx_06 in BOTH the Spartan and
                               Elite armour-effect lists (list count 7 -> 8)

Engine side: dev build flag -armor-fx-probe maps the custom item id 0xF0 (240) to internal pmcg index 7, and
-armor-effect-item 240 puts that item id in the profile. See docs/project-docs/RE-ARMOR-FX-VARIANTS-2026-11-16.md.
"""
import os, shutil, traceback
from mbstart import B, mb_stop

HT = r"D:\Steam\steamapps\common\HREK\tags"
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []
def L(s): out.append(s); print(s)

def p(rel): return os.path.join(HT, rel)
def backup(rel):
    src = p(rel); dst = src + ".bak"
    if not os.path.exists(dst):
        shutil.copy2(src, dst); L("backup " + os.path.relpath(dst, HT))
def load(rel):
    base, ext = os.path.splitext(rel)
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def field(fs, name):
    for f in fs:
        if str(f.FieldName) == name: return f
    return None

def add_effect():
    rel = r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"
    backup(rel); tf = load(rel)
    locs = field(tf.Fields, "locations")
    marker = field(locs.Elements[4].Fields, "marker name")
    if str(marker.Data) != "armor_fx_06":
        marker.SetStringData("armor_fx_06"); L("effect: location 4 marker -> armor_fx_06")
    ev = field(tf.Fields, "events")
    names = [str(field(ev.Elements[i].Fields, "event name").Data) for i in range(ev.Elements.Count)]
    if "legendary_flame_blue" not in names:
        el = ev.DuplicateElement(0); fs = el.Fields
        field(fs, "event name").SetStringData("legendary_flame_blue")
        ps = field(fs, "particle systems"); p0 = ps.Elements[0].Fields
        ref = field(p0, "particle"); base, ext = os.path.splitext(
            r"objects\characters\spartans\fx\armor_fx\particles\head_fire_blue.particle")
        ref.Path = B.Tags.TagPath.FromPathAndExtension(base, ext[1:])
        field(p0, "location").Value = 4
        L("effect: added event legendary_flame_blue -> head_fire_blue.particle loc 4")
    tf.Save(); tf.Dispose()

def add_model():
    rel = r"objects\characters\spartans\spartans.render_model"
    backup(rel); tf = load(rel)
    regions = field(tf.Fields, "regions"); region = None
    for i in range(regions.Elements.Count):
        fs = regions.Elements[i].Fields
        if str(field(fs, "name").Data) == "armor_fx": region = regions.Elements[i]; break
    perms = field(region.Fields, "permutations")
    pnames = [str(field(perms.Elements[i].Fields, "name").Data) for i in range(perms.Elements.Count)]
    if "armor_fx_06" not in pnames:
        el = perms.DuplicateElement(0); field(el.Fields, "name").SetStringData("armor_fx_06")
        L("render_model: + permutation armor_fx_06")
    groups = field(tf.Fields, "marker groups")
    gnames = [str(field(groups.Elements[i].Fields, "name").Data) for i in range(groups.Elements.Count)]
    if "armor_fx_06" not in gnames:
        el = groups.DuplicateElement(0); field(el.Fields, "name").SetStringData("armor_fx_06")
        m = field(el.Fields, "markers").Elements[0].Fields
        field(m, "permutation index").Data = 6
        L("render_model: + marker group armor_fx_06 (permutation index 6)")
    tf.Save(); tf.Dispose()

def add_pmcg():
    rel = r"globals\player_model_customization.player_model_customization_globals"
    backup(rel); tf = load(rel)
    for listname in ("spartan armor effect selections", "elite armor effect selections"):
        block = field(tf.Fields, listname)
        names = [str(field(block.Elements[i].Fields, "selection name").Data) for i in range(block.Elements.Count)]
        if "legendary_flame_blue" in names:
            L("pmcg %s: already present" % listname); continue
        el = block.DuplicateElement(1); fs = el.Fields
        field(fs, "selection name").SetStringData("legendary_flame_blue")
        bits = field(fs, "customized bits").Elements[0].Fields
        field(bits, "region name").SetStringData("armor_fx")
        field(bits, "permutation name").SetStringData("armor_fx_06")
        L("pmcg %s: + legendary_flame_blue -> armor_fx/armor_fx_06 (count %d)" % (listname, block.Elements.Count))
    tf.Save(); tf.Dispose()

def verify():
    tf = load(r"objects\characters\spartans\fx\armor_fx\armor_fx.effect")
    ev = field(tf.Fields, "events")
    L("verify events:")
    for i in range(ev.Elements.Count):
        fs = ev.Elements[i].Fields; ps = field(fs, "particle systems").Elements[0].Fields
        L("  %d %s loc=%s -> %s" % (i, field(fs, "event name").Data,
                                        field(ps, "location").Value,
                                        field(ps, "particle").Path.RelativePathWithExtension))
    tf.Dispose()
    tf = load(r"globals\player_model_customization.player_model_customization_globals")
    for listname in ("spartan armor effect selections", "elite armor effect selections"):
        block = field(tf.Fields, listname)
        L("%s [%d] last=%s" % (listname, block.Elements.Count,
                                field(block.Elements[block.Elements.Count - 1].Fields, "selection name").Data))
    tf.Dispose()

try:
    add_effect(); add_model(); add_pmcg(); verify(); mb_stop()
except Exception:
    L("ERROR\n" + traceback.format_exc())
open(os.path.join(OUT, os.environ.get("OUT", "armor_fx_variant.txt")), "w", encoding="utf-8").write("\n".join(out))
