# AZADI UE5 — Modding

Community packs add districts, restyle the HUD and color grade, and replace street signage **without engine, source, or editor access**. A pack is a folder of JSON + PNG.

The shipped example: [`Mods/example_neon_dawn/`](../Mods/example_neon_dawn/) — restyles the bazaar with neon liberation murals and a matching style pack.

---

## Pack layout

```
<Project>/Mods/<your_mod_id>/
  manifest.json        required
  data/*.json          def files (same format as core data)
  textures/*.png       referenced via "file:" refs
```

Packs are discovered in `<Project>/Mods/` and `<Project>/Content/Mods/` (use the latter if your pack also carries cooked `.uasset` content). Load order: **core JSON → core DataAssets → mods** (dependency-sorted). Later registrations override earlier ones by id.

## manifest.json

```json
{
  "id": "example_neon_dawn",
  "name": "Neon Dawn",
  "version": "1.0.0",
  "author": "you",
  "description": "what it does",
  "dependencies": [],
  "data": ["data/style_neon.json", "data/signage_neon.json"],
  "campaignAppend": ["my_district"],
  "campaignInsertAfter": "bazaar"
}
```

| Field | Meaning |
|-------|---------|
| `id` | unique pack id (folder name by convention) |
| `enabled` | optional, default `true`; ship dormant packs with `false` (the example does) |
| `dependencies` | pack ids that must load first; unmet → pack skipped with a warning |
| `data` | def files in load order; omit to auto-load every `data/*.json` |
| `campaignAppend` | district ids spliced into the campaign order |
| `campaignInsertAfter` | anchor district; empty = append at the end |

## Data files

Self-describing, identical to core data under `Content/Azadi/Data/`:

```json
{ "type": "district", "items": [ { "id": "bazaar", ... } ] }
```

`type` ∈ `campaign` · `district` · `weapon` · `enemy` · `stylepack` · `signageset` · `assetpack`. Field reference: [`Source/Azadi/Data/AzadiTypes.h`](../Source/Azadi/Data/AzadiTypes.h) — every UPROPERTY is a JSON key (case-insensitive; booleans keep their `b` prefix, e.g. `bAutomatic`).

**Override by id.** Registering `"id": "bazaar"` replaces the core bazaar district. Registering a new id adds content (reference it from a district/campaign to surface it).

## Texture refs

| Form | Meaning |
|------|---------|
| `file:textures/mural.png` | PNG on disk, relative to the JSON file; imported at runtime |
| `/Game/Path/To.Texture` | content asset (requires the pack to ship cooked content) |

Signage slots take an `occupiedTexture` (regime propaganda) and a `liberatedTexture` (mural revealed when the district's AZADI meter passes its flip threshold). Power-of-two sizes recommended; the placeholders are 256–512px.

## What each def type lets you mod

| Goal | Type | Notes |
|------|------|-------|
| Reskin street signage | `signageset` | override `bazaar_fa` or add a set + point a district at it |
| New HUD skin + color grade + music | `stylepack` | point a district's `stylePackId` at it |
| Rebalance or reskin enemies | `enemy` | tint/scale/stats; `meshPath` accepts cooked mesh assets |
| New weapons | `weapon` | add id, then include it in a district `loadout` |
| New district | `district` + `campaignAppend` | needs a map: reuse a core map (`"map": "L_Bazaar"`) for remix mods, or ship a cooked map |
| Tune the campaign | `campaign` | full reorder (replaces core order) |

## Shipping maps / uasset content (advanced)

Loose JSON/PNG packs need no tooling. Packs that add **maps, meshes, or audio** must cook content with a matching engine version:

1. Build your content in a UE 5.7 project (or this one) under `Content/Mods/<id>/`.
2. Package/cook for the target platform; distribute your pack's `.uasset`/`.umap` under `Content/Mods/<id>/` (editor + packaged builds both mount project content).
3. Reference assets by object path (`/Game/Mods/<id>/Maps/L_MyDistrict` → district `mapRoot` + `map`).

A pak-based DLC flow is the eventual target; v1 documents the loose-content route only.

## Testing your pack

1. Drop the folder into `ue5/azadi/Mods/`.
2. Launch (editor Play or `make ue5-game`).
3. Check the log for `ModKit: loading '<id>' ...` and `ModKit: '<id>' registered N defs`.

To try the shipped example, set `"enabled": true` in `Mods/example_neon_dawn/manifest.json` and play the bazaar — HUD, grade, and liberated signage all switch to the neon look.

Invalid manifests, unknown def types, bad texture paths, and missing dependencies are logged and skipped — a broken mod never blocks boot.

---

*Keep it fictional. Packs that reference real people, organizations, or targets are not welcome — the monsters are monsters.*
