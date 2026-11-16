# Agent handoff prompt — Halo armor catalog

We are building a local armor catalog for the Halo project UI from Halopedia's Halo: Reach armor reference. Use the supplied `armor_catalog_template.csv` as the schema contract.

Required CSV columns, exact order:
`credits, attachments, category, name, ID, image file name, source URL, match status`

Rules:
- Exclude all Halo: The Master Chief Collection data, including seasons and tiers.
- `credits` must preserve the Halo: Reach rank and credit price together when present (example: `Recruit, 750 cR`). Preserve special unlock wording if no numeric price is listed.
- Do not invent IDs. Use the provided existing extracted asset/tag inventory to populate `ID`; if uncertain, leave it blank and set `match status` to `needs review` or `needs ID match`.
- Download/associate the correct image for each catalog row, save it locally, and put its relative path in `image file name`.
- Keep `source URL` for traceability.
- Handle left/right shoulders and helmet attachments/variants as distinct rows when the source and game assets distinguish them.
- Produce `armor_catalog.csv`, the `images/` directory, and a QA report listing missing images, duplicate/ambiguous names, unmatched game IDs, and catalog rows with no confirmed ID.
- The UI should load local images and use the catalog's ID field for the internal mapping. Do not fetch images from Halopedia at runtime.
- Start with the helmet category as a pilot. Verify the output against the source before processing all categories.
