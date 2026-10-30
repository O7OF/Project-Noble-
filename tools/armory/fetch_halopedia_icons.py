"""Downloads the Halo: Reach Armory icons from Halopedia's "Armor customization (Halo: Reach)" page.

Run from the repo root:
    python tools\\armory\\fetch_halopedia_icons.py

Saves every piece icon into build\\bin\\Release\\data\\ui\\reach\\armory\\<category>\\<Halopedia file name>
(helmets, shoulders_left, shoulders_right, chests, wrists, utility, visors, knees, effects, voices, elite_sets)
and writes manifest.csv there (category, file, url). Files already downloaded are skipped, so it can be re-run.
Names and rank / cR for each piece are already in docs\\career\\armor.json; this only fetches the pictures.
Uses only the Python standard library.
"""
import csv
import json
import os
import sys
import time
import urllib.parse
import urllib.request

API = "https://www.halopedia.org/api.php"
PAGE = "Armor_customization_(Halo:_Reach)"
OUT = os.path.join("build", "bin", "Release", "data", "ui", "reach", "armory")
HEADERS = {"User-Agent": "ProjectNoble-icon-fetch/1.0 (personal, one-off download of Reach Armory icons)"}

# File-name endings -> folder. Order matters (first match wins).
CATEGORIES = [
    ("_Helmet_Icon.png", "helmets"),
    ("_LShoulder_Icon.png", "shoulders_left"),
    ("_RShoulder_Icon.png", "shoulders_right"),
    ("_Chest_Icon.png", "chests"),
    ("_Wrist_Icon.png", "wrists"),
    ("_Utility_Icon.png", "utility"),
    ("_Visor_Icon.png", "visors"),
    ("_Knee_Icon.png", "knees"),
    ("_Effect_Icon.png", "effects"),
    ("_Armor_Effect.png", "effects"),
    ("_Firefight_Icon.png", "voices"),
]


def category_of(name):
    if name.startswith("HR_Rank_"):
        return None  # rank icons are already in data\ui\reach\ranks
    if name.startswith("HR_Elite_") and name.endswith("_Icon.png"):
        return "elite_sets"
    for ending, folder in CATEGORIES:
        if name.endswith(ending):
            return folder
    return None


def api(params):
    params = dict(params, format="json")
    url = API + "?" + urllib.parse.urlencode(params)
    req = urllib.request.Request(url, headers=HEADERS)
    with urllib.request.urlopen(req, timeout=30) as r:
        return json.load(r)


def main():
    images = api({"action": "parse", "page": PAGE, "prop": "images"})["parse"]["images"]
    wanted = [(n, category_of(n)) for n in images]
    wanted = [(n, c) for n, c in wanted if c]
    print(f"{len(images)} images on the page, {len(wanted)} armour icons to fetch")

    # Image URLs, 50 titles per request.
    urls = {}
    names = [n for n, _ in wanted]
    for i in range(0, len(names), 50):
        batch = names[i:i + 50]
        res = api({"action": "query", "titles": "|".join("File:" + n for n in batch),
                   "prop": "imageinfo", "iiprop": "url"})
        for page in res.get("query", {}).get("pages", {}).values():
            info = page.get("imageinfo")
            if info:
                urls[page["title"].split(":", 1)[1].replace(" ", "_")] = info[0]["url"]
        time.sleep(0.5)

    rows, got, skipped, missing = [], 0, 0, []
    for name, cat in wanted:
        url = urls.get(name)
        if not url:
            missing.append(name)
            continue
        folder = os.path.join(OUT, cat)
        os.makedirs(folder, exist_ok=True)
        path = os.path.join(folder, name)
        rows.append((cat, name, url))
        if os.path.exists(path) and os.path.getsize(path) > 0:
            skipped += 1
            continue
        req = urllib.request.Request(url, headers=HEADERS)
        with urllib.request.urlopen(req, timeout=60) as r, open(path, "wb") as f:
            f.write(r.read())
        got += 1
        print(f"  {cat}/{name}")
        time.sleep(0.3)  # be polite to the wiki

    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, "manifest.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["category", "file", "url"])
        w.writerows(rows)
    print(f"Done: {got} downloaded, {skipped} already there, {len(missing)} without a URL")
    for n in missing:
        print("  no URL:", n)
    print("Pictures are in", os.path.abspath(OUT))
    return 0 if not missing else 1


if __name__ == "__main__":
    sys.exit(main())
