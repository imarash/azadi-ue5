# AZADI — UE5 Version 1 (Modular Liberation FPS)

Copy everything below the line into Mythos.

---

# AZADI — UE5 Version 1 (Modular Liberation FPS)

## Role

Senior UE5 architect + SMEs. Build **AZADI: Rise of the Dawn** as a **modular, data-driven UE5 FPS** from scratch. This is v1 — no browser port, no retro raycast legacy. Big rewrites welcome.

**North star:** Medal of Honor / Call of Duty 4 urban combat in a fictional contemporary Kurdish-Persian city. Same liberation story: district by district, hope rises, world changes.

**Fictional only.** Supernatural regime monsters — never real people, orgs, or targets.

---

## Core Principles

1. **Modular** — districts, enemies, weapons, UI, audio, and environment packs are swappable plugins/modules
2. **Customizable** — designers tune via DataAssets/JSON, not C++ recompiles; modders can add districts and asset packs without forking core
3. **Data-driven** — campaign, spawn tables, liberation curves, signage sets, and HUD skins live in config
4. **Ship slices** — one playable alley before perfect architecture

---

## Project Layout

```
ue5/azadi/
  azadi.uproject
  Content/Azadi/
    Core/           GameMode, Player, Subsystems
    Modules/        plug-in gameplay features
      Combat/
      Liberation/   meter, murals/decals, citizen AI
      Districts/    per-map logic
    Data/           DataAssets + JSON mirrors
      Campaign/
      Weapons/
      Enemies/
      StylePacks/
    Maps/           L_Rooftops, L_Bazaar, L_Detention, L_TVTower, L_Plaza
    Environment/    kitbash imports + signage materials
    Characters/
    Weapons/
    UI/             HUD, menus, FA RTL localization
    Audio/
  Plugins/
    AzadiModKit/    optional: load external district + asset packs
  Config/
  docs/
    ARCHITECTURE.md
    MODDING.md
```

---

## Module System

Everything registers through **`UAzadiModule`** (or equivalent subsystem):

| Module         | Responsibility                                                             |
| -------------- | -------------------------------------------------------------------------- |
| **Campaign**   | district order, objectives, unlock rules                                   |
| **Liberation** | AZADI meter → world state (material swaps, decals, crowd, music intensity) |
| **Combat**     | weapons, damage, ADS, projectiles                                          |
| **District**   | per-level overrides (enemy budget, signage set, lighting preset)           |
| **StylePack**  | HUD skin, color grade, font, audio stem mapping                            |
| **AssetPack**  | environment kit refs (Fab/Leartes/Megascans) + slot bindings               |

**Campaign JSON example:**

```json
{
  "districts": [
    {
      "id": "bazaar",
      "map": "L_Bazaar",
      "objective": "reach_exit",
      "stylePack": "urban_grit",
      "assetPack": "leartes_bazaar"
    }
  ]
}
```

Adding a district = new map + District DataAsset + campaign entry. No core edits.

---

## Campaign (5 beats)

1. **Rooftops at night** — traversal, escape
2. **Bazaar** — first weapons, alley firefights
3. **Detention** — free prisoners, checkpoint interiors
4. **TV tower** — brutalist vertical, antenna yard
5. **Palace plaza** — boss arena

**Enemies:** Wraith, Baton Husk, Censor Drone, Propaganda Caster, Inquisitor, Supreme Shadow — reskin in UE5; behavior via Enemy DataAssets.

**Weapons:** whip, pistol, shotgun, drum AR, molotov launcher, megaphone chant — tune per StylePack.

---

## Environment & Realism

Kitbash stack (import natively, Lumen):

- Modular Village (Fab) — suburbs
- Town Gigapack (Leartes) — bazaars, alleys, rooftops
- Desert City (Fab) — compounds, domes, plaza
- Megascans + riot/military props — checkpoints, barriers, cars, substations
- **Persian signage / billboards / murals** — highest realism priority; separate StylePack slot

Realism = streetscape clutter, not hero architecture.

---

## Customization / Modding

**AzadiModKit plugin** loads packs from `Content/Mods/<id>/`:

- `manifest.json` — id, version, dependencies
- optional maps, enemy skins, signage textures, music stems, District + StylePack DataAssets

Document in `docs/MODDING.md`. Goal: community district packs without source access.

---

## Sprint Order

1. **Spike** — First Person template, L_Bazaar graybox, move/shoot/ADS, 1 enemy, liberation meter stub
2. **Modules** — Campaign + Liberation + Combat subsystems; DataAsset schema
3. **Vertical slice** — full bazaar district, kitbash pass, 3 signage mats, 2 enemies, citizen rescue stub
4. **ModKit** — manifest loader + one example mod pack
5. **Expand** — remaining districts; FA localization; polish pass

---

## Success

- [ ] Bazaar playable 5+ minutes — feels like urban military FPS
- [ ] New district addable via Data + map only
- [ ] StylePack swap changes HUD + grade without code
- [ ] Example mod pack loads alternate signage or district
- [ ] `docs/ARCHITECTURE.md` + `docs/MODDING.md` exist

---

## First Action

1. Create `ue5/azadi/` from UE5 First Person template
2. Define Campaign + District + StylePack DataAsset headers
3. Blockout `L_Bazaar`, import one kitbash pack
4. Playable: move, ADS, shoot Wraith, liberation meter ticks on kill

**Session title:** AZADI UE5 v1 — modular data-driven urban liberation FPS; big changes OK; moddable districts and style packs.
