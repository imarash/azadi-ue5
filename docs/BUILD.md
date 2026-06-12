# Build & Release Pipeline

> ⚠️ **Status: untested.** This GitHub release pipeline has not been run
> end-to-end yet. It still needs self-hosted runners provisioned (see below)
> and a first verification run before it can be considered reliable. Expect to
> debug it on the initial release.

This repo ships a GitHub Actions pipeline that packages **AZADI** into
distributable artifacts and publishes them on a GitHub Release:

| Platform | Artifact | Produced by |
|----------|----------|-------------|
| Windows  | `Azadi-Windows-<version>.zip` (portable, contains `Azadi.exe`) | `tools/ci/package_windows.ps1` |
| Windows  | `AzadiSetup-<version>.exe` (installer) | Inno Setup + `tools/installer/azadi.iss` |
| macOS    | `Azadi-macOS-<version>.dmg` | `tools/ci/package_mac.sh` |

The workflow lives at `.github/workflows/release.yml`.

## Why self-hosted runners?

Packaging an Unreal Engine game requires a full **UE 5.7** install (tens of GB)
plus the platform toolchain. GitHub-hosted runners do **not** have Unreal
Engine, so the Windows and macOS build jobs run on **self-hosted runners** that
you provision once. The lightweight `version` and `release` jobs run on
GitHub-hosted `ubuntu-latest`.

### One-time runner setup

For each machine (a Windows box and a Mac), install:

1. **Unreal Engine 5.7** (via Epic Games Launcher or source).
2. The matching platform toolchain:
   - Windows: Visual Studio 2022 with the *Game development with C++* workload.
   - macOS: Xcode + command line tools.
3. **Windows only — Inno Setup 6** (for the installer): <https://jrsoftware.org/isdl.php>.
4. **macOS only (optional) — `create-dmg`** for a nicer DMG layout:
   `brew install create-dmg`. Without it the script falls back to `hdiutil`.

Then register the machine as a repository runner
(Settings → Actions → Runners → New self-hosted runner) with these **labels**:

- Windows runner: `self-hosted`, `windows`, `unreal`
- macOS runner: `self-hosted`, `macos`, `unreal`

Finally, add a **repository variable** `UE_ROOT` pointing at the engine install
(Settings → Secrets and variables → Actions → Variables):

- Windows example: `C:\Program Files\Epic Games\UE_5.7`
- macOS example: `/Users/Shared/Epic Games/UE_5.7`

> If your two machines need different paths, set `UE_ROOT` as an environment
> variable on each runner instead of a repo variable — the scripts read
> `UE_ROOT` from the environment.

## Running it

### Tag a release (recommended)

```bash
git tag v0.1.0
git push origin v0.1.0
```

This triggers the pipeline, which builds all artifacts and publishes a release
named `AZADI v0.1.0` with the `.zip`, installer `.exe`, and `.dmg` attached.

### Manual run

Actions → **Build & Release** → *Run workflow*, optionally entering a version.
A manual run still publishes/updates the `v<version>` release and tag.

## Building locally

The same scripts work on a dev machine with UE 5.7 installed:

```powershell
# Windows (PowerShell)
$env:UE_ROOT = "C:\Program Files\Epic Games\UE_5.7"
./tools/ci/package_windows.ps1 -Version 0.1.0
```

```bash
# macOS
export UE_ROOT="/Users/Shared/Epic Games/UE_5.7"
AZADI_VERSION=0.1.0 ./tools/ci/package_mac.sh
```

Artifacts are written to `dist/`. Both `dist/` and `Build/` are intermediate
output (`Build/` is already gitignored).

## Versioning

Version is resolved in this order:

1. The tag / workflow input (`v1.2.3` → `1.2.3`), or `-Version` / `AZADI_VERSION`.
2. `ProjectVersion` from `Config/DefaultGame.ini`.
3. Fallback `0.0.0`.
