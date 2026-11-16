# Halopedia armor catalog starter kit

## Purpose
Create a Halo: Reach armor reference catalog for the project UI. This kit intentionally excludes all Halo: The Master Chief Collection season/tier data.

## CSV columns (exact order)
1. `credits` — keep the Halo: Reach rank and price together, e.g. `Recruit, 750 cR`. This preserves both rank and credit requirement in one field.
2. `attachments` — attachment/variant name where applicable; blank when not applicable.
3. `category` — Helmet, Shoulder, Chest, Wrist, Utility, Visor color, Knee guard, Armor effect, Firefight voice, etc.
4. `name` — display name of the armor piece.
5. `ID` — your project's actual internal asset/tag ID. Leave blank until matched against the extracted game files.
6. `image file name` — relative path inside `images/`, e.g. `helmets/mark_v_b.png`.
7. `source URL` — Halopedia page used as the reference.
8. `match status` — e.g. `needs ID match`, `matched`, `needs review`, `image missing`.

## Important
- The CSV template contains one example row only; it is not the finished catalog.
- Do not invent IDs. The ID field is intentionally blank until your existing extracted asset/tag list is joined in.
- Halo: Reach unlock info only. No MCC season or tier fields.
- Before bulk downloading, review Halopedia's current terms, robots guidance, and image licensing/attribution requirements. Keep source URLs and attribution in the project.
- Image filenames should be stable and filesystem-safe. If two items share a name, append a category/variant suffix.
- Verify that the downloaded image is the actual armor item and not an attachment thumbnail or unrelated icon.

## Windows setup
1. Install Python 3.11 or newer from https://www.python.org/downloads/.
2. During installation, enable **Add python.exe to PATH**.
3. Extract this ZIP to a working folder, for example:
   `D:\\CHaloBattlefrontProjectTools\\armor_catalog`
4. Open that folder in File Explorer, click the address bar, type `cmd`, and press Enter.
5. Create a virtual environment:
   `py -m venv .venv`
6. Activate it:
   `.venv\\Scripts\\activate`
7. Install packages:
   `python -m pip install --upgrade pip`
   `pip install requests beautifulsoup4 pandas lxml`
8. Keep the CSV template as the schema/contract for the agent. The included scraper is a starter for page inspection, not a guaranteed one-click full extraction.

## How to pass this to the project agent
Send the agent:
- `armor_catalog_template.csv`
- this README
- the actual extracted asset/tag inventory file containing IDs (CSV/JSON/text, if available)
- the image folder after collection

Ask the agent to:
1. Preserve the eight CSV columns exactly; do not add MCC columns.
2. Populate Reach rank + cR in `credits`.
3. Populate `attachments` only when applicable.
4. Match `name`/category to the existing game asset/tag IDs using the supplied inventory, not guessed names.
5. Set `match status` to `matched`, `needs review`, `needs ID match`, or `image missing`.
6. Generate a report of unmatched IDs, unmatched armor rows, duplicate names, and missing images.
7. Make the game UI read `image file name` and `ID` from the catalog, with images stored locally rather than fetched from Halopedia at runtime.

## Suggested project layout
armor_catalog/
  armor_catalog_template.csv
  armor_catalog.csv
  images/
    helmets/
    shoulders/
    chest/
    wrists/
    utility/
    visor_colors/
    knee_guards/
    armor_effects/
    firefight_voices/
  source_notes/
  reports/
