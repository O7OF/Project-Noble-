"""Downloads the Halo: Reach Armory pictures from Halopedia's "Armor customization (Halo: Reach)" page.

For every armour table on the page (Helmets, Shoulders, Chests, Wrists, Utility, Visor colors, Knee guards,
Armor effects, Firefight voices, and the Elite tables) it saves each piece's picture with the piece's name,
plus the Halo: Reach rank and cR (the Master Chief Collection columns are ignored).

Output (default: build/bin/Release/data/ui/reach/armory/ in the repo):
  <slot>/<piece>.png                 e.g. helmets/cqc.png
  <slot>/<helmet>__<attachment>.png  helmet attachments, e.g. helmets/cqc__cbrn.png
  pieces.json / pieces.csv           slot, name, attachment-of, rank, cR, file, source picture
  pieces_report.txt                  what was found per table, and pieces not in docs/career/armor.json

Uses only Python's standard library, so it also runs with Blender's Python:
  python tools/armory/fetch_halopedia_armory.py
  blender --background --python tools/armory/fetch_halopedia_armory.py
Options (after "--" when run through Blender): --out <folder>, --dry-run (no picture downloads).
"""

import csv
import html
import json
import os
import re
import sys
import time
import urllib.parse
import urllib.request
from html.parser import HTMLParser

SITE = "https://www.halopedia.org"
PAGE = "Armor_customization_(Halo:_Reach)"
USER_AGENT = "ProjectNobleArmoryFetcher/1.0 (personal use; one page, polite delay)"
DELAY = 0.4  # seconds between picture downloads

def _find_repo():
    # Run as a file (python / blender --python): two folders up. Run from Blender's Text Editor, __file__ may be
    # just the text's name, so also try the current folder and the usual checkout.
    cands = []
    try:
        cands.append(os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")))
    except NameError:
        pass
    cands += [os.getcwd(), r"D:\Halo-Reach-Community-Edition"]
    for c in cands:
        if os.path.isfile(os.path.join(c, "docs", "career", "armor.json")):
            return c
    return cands[0] if cands else os.getcwd()


REPO = _find_repo()


# ---- tiny HTML tree --------------------------------------------------------------------------------------------

class Node:
    __slots__ = ("tag", "attrs", "children", "parent", "text")

    def __init__(self, tag, attrs=None, parent=None):
        self.tag = tag
        self.attrs = dict(attrs or [])
        self.children = []
        self.parent = parent
        self.text = ""

    def iter(self, tag=None):
        for c in self.children:
            if isinstance(c, Node):
                if tag is None or c.tag == tag:
                    yield c
                yield from c.iter(tag)

    def all_text(self):
        out = []
        for c in self.children:
            out.append(c if isinstance(c, str) else c.all_text())
        return "".join(out)

    def cls(self):
        return self.attrs.get("class", "") or ""


VOID = {"img", "br", "hr", "meta", "link", "input", "source", "wbr", "col", "area", "base", "embed", "param", "track"}


class TreeBuilder(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.root = Node("root")
        self.cur = self.root

    def handle_starttag(self, tag, attrs):
        n = Node(tag, attrs, self.cur)
        self.cur.children.append(n)
        if tag not in VOID:
            self.cur = n

    def handle_startendtag(self, tag, attrs):
        self.cur.children.append(Node(tag, attrs, self.cur))

    def handle_endtag(self, tag):
        n = self.cur
        while n is not None and n.tag != tag:
            n = n.parent
        if n is not None and n.parent is not None:
            self.cur = n.parent

    def handle_data(self, data):
        self.cur.children.append(data)


# ---- helpers -------------------------------------------------------------------------------------------------

def get(url, binary=False):
    req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    with urllib.request.urlopen(req, timeout=60) as r:
        data = r.read()
    return data if binary else data.decode("utf-8", "replace")


def clean(s):
    s = html.unescape(s or "")
    s = s.replace("\xa0", " ")
    return re.sub(r"\s+", " ", s).strip()


def slug(s):
    s = clean(s).lower()
    s = s.replace("/", "_").replace("[", "").replace("]", "")
    s = re.sub(r"[^a-z0-9_]+", "_", s)
    return re.sub(r"_+", "_", s).strip("_") or "unnamed"


def original_url(img):
    """Full-size picture URL from a thumbnail <img> (src, data-src or srcset)."""
    src = img.attrs.get("data-src") or img.attrs.get("src") or ""
    if src.startswith("data:"):
        src = img.attrs.get("data-src") or ""
    if not src:
        return ""
    if src.startswith("//"):
        src = "https:" + src
    elif src.startswith("/"):
        src = SITE + src
    # /images/thumb/a/ab/File.png/120px-File.png -> /images/a/ab/File.png
    m = re.match(r"(.*/images/)thumb/(.+?/[^/]+\.(?:png|jpg|jpeg|gif|webp))/[^/]+$", src, re.I)
    if m:
        src = m.group(1) + m.group(2)
    return src


def grid(table):
    """Rows of the table as lists of (column index, cell node), honouring rowspan/colspan."""
    rows = []
    pending = {}  # column -> (cell, rows left)
    for tr in table.iter("tr"):
        # skip rows of nested tables
        p = tr.parent
        while p is not None and p.tag != "table":
            p = p.parent
        if p is not table:
            continue
        cells = [c for c in tr.children if isinstance(c, Node) and c.tag in ("td", "th")]
        row = []
        col = 0
        it = iter(cells)
        nxt = next(it, None)
        while nxt is not None or any(k >= col for k in pending):
            if col in pending:
                cell, left = pending[col]
                row.append((col, cell, True))
                if left <= 1:
                    del pending[col]
                else:
                    pending[col] = (cell, left - 1)
                col += int(cell.attrs.get("colspan", "1") or 1)
                continue
            if nxt is None:
                col += 1
                if col > 64:
                    break
                continue
            span = int(re.sub(r"\D", "", nxt.attrs.get("colspan", "1")) or 1)
            rspan = int(re.sub(r"\D", "", nxt.attrs.get("rowspan", "1")) or 1)
            row.append((col, nxt, False))
            if rspan > 1:
                pending[col] = (nxt, rspan - 1)
            col += span
            nxt = next(it, None)
        rows.append(row)
    return rows


RANK_RE = re.compile(r"\b(Recruit|Private|Corporal|Sergeant|Warrant Officer|Captain|Major|Lt\.? Colonel|Lieutenant Colonel|"
                     r"Commander|Colonel|Brigadier|General|Field Marshal+|Hero|Legend|Mythic|Noble|Eclipse|Nova|"
                     r"Forerunner|Reclaimer|Inheritor)(?: Grade \d)?", re.I)


def unlock_of(cell):
    t = clean(cell.all_text())
    cr = None
    m = re.search(r"([\d,]+)\s*cR", t)
    if m:
        cr = int(m.group(1).replace(",", ""))
    r = RANK_RE.search(t)
    return {"rank": r.group(0) if r else None, "credits": cr, "text": t}


def piece_name(cell):
    """The piece's name: the cell's link texts (not the picture's), else its plain text."""
    names = []
    for a in cell.iter("a"):
        if any(isinstance(c, Node) and c.tag == "img" for c in a.children):
            continue
        t = clean(a.all_text())
        if t:
            names.append(t)
    text = " ".join(names) if names else clean(cell.all_text())
    # "UA/HUL" is written as two links, "UA" + "/" + "HUL": use the cell text when it is longer
    full = clean(cell.all_text())
    if full and len(full) <= 60 and len(full) >= len(text):
        text = full
    return text


# ---- main ----------------------------------------------------------------------------------------------------

def main(argv):
    out = os.path.join(REPO, "build", "bin", "Release", "data", "ui", "reach", "armory")
    dry = False
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = argv[1:]
    i = 0
    while i < len(argv):
        if argv[i] == "--out" and i + 1 < len(argv):
            out = os.path.abspath(argv[i + 1])
            i += 2
            continue
        if argv[i] == "--dry-run":
            dry = True
        i += 1
    os.makedirs(out, exist_ok=True)

    api = (SITE + "/api.php?action=parse&format=json&formatversion=2&prop=text&page=" + urllib.parse.quote(PAGE))
    print("Reading", SITE + "/" + PAGE)
    page = json.loads(get(api))
    if "error" in page:
        print("Halopedia said:", page["error"])
        return 1
    tb = TreeBuilder()
    tb.feed(page["parse"]["text"])
    with open(os.path.join(out, "halopedia_page.html"), "w", encoding="utf-8") as f:
        f.write(page["parse"]["text"])

    # Section ("Spartan" / "Elite") of each table: the last h2/h3 heading before it.
    section = "Spartan"
    order = []
    for n in tb.root.iter():
        if n.tag in ("h2", "h3", "h4"):
            t = clean(n.all_text()).replace("[edit]", "").strip()
            if t:
                section = t
        elif n.tag == "table":
            order.append((section, n))

    items = []
    report = []
    for sec, table in order:
        rows = grid(table)
        if not rows:
            continue
        # Header row: the one with "Unlock" headings. Title: the row above it (e.g. "Helmets").
        head_i = next((k for k, r in enumerate(rows) if any("unlock" in clean(c.all_text()).lower() for _, c, _ in r)), None)
        if head_i is None:
            continue
        title = clean(" ".join(clean(c.all_text()) for _, c, _ in rows[head_i - 1])) if head_i > 0 else ""
        headers = {col: clean(c.all_text()) for col, c, _ in rows[head_i]}
        if not title:
            title = headers.get(0, "table")
        slot = slug(title)
        if sec.lower().startswith("elite") and not slot.startswith("elite"):
            slot = "elite_" + slot
        cols = sorted(headers)
        # Picture columns: not "Unlock ..." headers. Each is followed by its Reach unlock column; MCC columns are skipped.
        pic_cols = [c for c in cols if not headers[c].lower().startswith("unlock")]
        reach_col = {}
        for pc in pic_cols:
            later = [c for c in cols if c > pc]
            for c in later:
                h = headers[c].lower()
                if h.startswith("unlock") and "master chief" not in h:
                    reach_col[pc] = c
                    break
                if not h.startswith("unlock"):
                    break
        found = 0
        parent = None
        for r in rows[head_i + 1:]:
            bycol = {col: (cell, spanned) for col, cell, spanned in r}
            for k, pc in enumerate(pic_cols):
                if pc not in bycol:
                    continue
                cell, spanned = bycol[pc]
                imgs = [im for im in cell.iter("img") if original_url(im)]
                if not imgs:
                    continue
                name = piece_name(cell)
                if k == 0:
                    parent = name
                if spanned:   # a rowspan cell repeats on the next rows: count it once
                    continue
                uc = reach_col.get(pc)
                unlock = unlock_of(bycol[uc][0]) if uc in bycol else {"rank": None, "credits": None, "text": ""}
                is_attachment = k > 0
                fname = (slug(parent) + "__" + slug(name)) if is_attachment else slug(name)
                url = original_url(imgs[0])
                ext = os.path.splitext(urllib.parse.urlparse(url).path)[1].lower() or ".png"
                rel = os.path.join(slot, fname + ext)
                items.append({
                    "section": sec, "slot": slot, "name": name,
                    "attachment_of": parent if is_attachment else None,
                    "rank": unlock["rank"], "credits": unlock["credits"], "unlock_text": unlock["text"],
                    "file": rel.replace("\\", "/"), "source": url,
                })
                found += 1
        report.append(f"[{sec}] {title}: {found} pictures (columns: {', '.join(headers[c] for c in cols)})")

    # Download.
    done = failed = 0
    for it in items:
        path = os.path.join(out, it["file"])
        os.makedirs(os.path.dirname(path), exist_ok=True)
        if dry or os.path.exists(path):
            continue
        try:
            data = get(it["source"], binary=True)
            with open(path, "wb") as f:
                f.write(data)
            done += 1
            print(f"  {it['file']}")
            time.sleep(DELAY)
        except Exception as e:  # keep going; listed in the report
            failed += 1
            it["error"] = str(e)
            print(f"  FAILED {it['file']}: {e}")

    with open(os.path.join(out, "pieces.json"), "w", encoding="utf-8") as f:
        json.dump(items, f, indent=2, ensure_ascii=False)
    with open(os.path.join(out, "pieces.csv"), "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["section", "slot", "name", "attachment_of", "rank", "credits", "file", "source"])
        for it in items:
            w.writerow([it["section"], it["slot"], it["name"], it["attachment_of"] or "", it["rank"] or "",
                        "" if it["credits"] is None else it["credits"], it["file"], it["source"]])

    # Compare with docs/career/armor.json (names already collected from the same page).
    known = set()
    try:
        with open(os.path.join(REPO, "docs", "career", "armor.json"), encoding="utf-8") as f:
            armor = json.load(f)
        for slot, lst in armor.items():
            if not isinstance(lst, list):
                continue
            for e in lst:
                known.add(slug(e.get("name", "")))
                for a in e.get("attachments", []) or []:
                    known.add(slug(e.get("name", "")) + "__" + slug(a.get("name", "")))
    except Exception as e:
        report.append(f"(could not read docs/career/armor.json: {e})")
    unknown = [it for it in items if known and os.path.splitext(os.path.basename(it["file"]))[0] not in known]
    report.append("")
    report.append(f"{len(items)} pieces found, {done} pictures downloaded, {failed} failed"
                  + (" (dry run, nothing downloaded)" if dry else ""))
    if unknown:
        report.append(f"{len(unknown)} names not in docs/career/armor.json (check these by hand):")
        report += [f"  {it['file']}  ({it['name']})" for it in unknown]
    with open(os.path.join(out, "pieces_report.txt"), "w", encoding="utf-8") as f:
        f.write("\n".join(report) + "\n")
    print("\n".join(report))
    print("\nSaved to", out)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
