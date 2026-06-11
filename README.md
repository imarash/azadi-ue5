# AZADI: Rise of the Dawn — UE5

Modular, data-driven **Unreal Engine 5.7** urban liberation FPS. Same story as the [browser game](https://github.com/imarash/azadi-rise-of-the-dawn) — district by district, hope rises, the world changes — built from scratch in UE5 (not a port).

**Fictional only.** Supernatural regime monsters — never real people, orgs, or targets.

## Requirements

- Unreal Engine **5.7** (Epic Games Launcher)
- macOS, Windows, or Linux with a standard UE5 toolchain

## Quickstart

```bash
# Compile (override UE_ROOT if your engine lives elsewhere)
export UE_ROOT="/Users/Shared/Epic Games/UE_5.7"
"$UE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh" AzadiEditor Mac Development \
  -Project="$(pwd)/azadi.uproject" -WaitMutex

# Build graybox maps + materials (one-time; add --force to rebuild)
"$UE_ROOT/Engine/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/azadi.uproject" \
  -ExecutePythonScript="$(pwd)/Content/Python/azadi_bootstrap.py" \
  -RenderOffScreen -unattended -nopause -nosplash

# Open and play (L_Bazaar is the startup map)
open azadi.uproject
```

In-editor: Output Log → Cmd → Python → `py azadi_bootstrap.py`

## Controls

| Action | Input |
|--------|-------|
| Move | WASD |
| Look | Mouse |
| Fire | LMB |
| ADS | RMB |
| Reload | R |
| Interact (free citizens) | E |
| Weapons | 1–3, mouse wheel, Q |

## What ships in v1

- **Data-driven modules:** Campaign, Liberation (AZADI meter), District, StylePack, Combat — all tuned via JSON under `Content/Azadi/Data/`
- **Five districts** in campaign order: Rooftops → Bazaar → Detention → TV Tower → Plaza
- **Six enemies**, **six weapons**, signage sets with runtime PNG import
- **AzadiModKit** plugin — community packs from `Mods/<id>/manifest.json`
- **Example mod:** `Mods/example_neon_dawn/` (ships disabled; set `"enabled": true` to try)

## Docs

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — system map, module table, data flow
- [docs/MODDING.md](docs/MODDING.md) — community pack authoring

## Related

- Browser prototype: [imarash/azadi-rise-of-the-dawn](https://github.com/imarash/azadi-rise-of-the-dawn)

## License

MIT — see [LICENSE](LICENSE).
