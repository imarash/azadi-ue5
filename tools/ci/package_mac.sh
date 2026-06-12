#!/usr/bin/env bash
#
# Cook, stage, and package AZADI for macOS, then wrap the .app into a .dmg.
#
# Designed to run on a self-hosted GitHub runner with UE 5.7 installed, but
# also works locally. Output:
#   dist/Azadi-macOS-<version>.dmg
#
# Requirements:
#   - Unreal Engine 5.7   -> set UE_ROOT (folder containing Engine/)
#   - hdiutil (ships with macOS); `create-dmg` is used if available for a nicer
#     drag-to-Applications layout, otherwise we fall back to plain hdiutil.
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PROJECT="$REPO_ROOT/azadi.uproject"
ARCHIVE_DIR="$REPO_ROOT/Build"
STAGE_DIR="$ARCHIVE_DIR/Mac"
OUTPUT="${OUTPUT:-$REPO_ROOT/dist}"
CONFIGURATION="${CONFIGURATION:-Shipping}"
APP_NAME="Azadi"

resolve_version() {
  if [[ -n "${AZADI_VERSION:-}" ]]; then
    echo "${AZADI_VERSION#v}"; return
  fi
  local ini="$REPO_ROOT/Config/DefaultGame.ini"
  if [[ -f "$ini" ]]; then
    local v
    v="$(grep -E '^ProjectVersion=' "$ini" | head -1 | cut -d= -f2 | tr -d '[:space:]')"
    [[ -n "$v" ]] && { echo "$v"; return; }
  fi
  echo "0.0.0"
}

UE_ROOT="${UE_ROOT:-}"
if [[ -z "$UE_ROOT" ]]; then
  echo "ERROR: UE_ROOT is not set. Point it at your UE 5.7 install (folder containing Engine/)." >&2
  exit 1
fi
RUNUAT="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
if [[ ! -x "$RUNUAT" ]]; then
  echo "ERROR: RunUAT.sh not found/executable at '$RUNUAT' (UE_ROOT='$UE_ROOT')." >&2
  exit 1
fi

VERSION="$(resolve_version)"
echo "==> Packaging AZADI $VERSION (Mac $CONFIGURATION)"
mkdir -p "$OUTPUT"

# --- Cook + stage + archive -------------------------------------------------
"$RUNUAT" BuildCookRun \
  -project="$PROJECT" \
  -noP4 -utf8output \
  -platform=Mac \
  -clientconfig="$CONFIGURATION" \
  -cook -build -stage -pak -package -archive \
  -archivedirectory="$ARCHIVE_DIR"

APP_PATH="$STAGE_DIR/$APP_NAME.app"
if [[ ! -d "$APP_PATH" ]]; then
  # Some UE versions name the staged app after the project; fall back to first .app.
  APP_PATH="$(/usr/bin/find "$STAGE_DIR" -maxdepth 1 -name '*.app' | head -1 || true)"
fi
if [[ -z "$APP_PATH" || ! -d "$APP_PATH" ]]; then
  echo "ERROR: No .app found in '$STAGE_DIR'." >&2
  exit 1
fi
echo "==> Staged app: $APP_PATH"

# --- Build the DMG ----------------------------------------------------------
DMG="$OUTPUT/Azadi-macOS-$VERSION.dmg"
rm -f "$DMG"

if command -v create-dmg >/dev/null 2>&1; then
  echo "==> Building DMG with create-dmg -> $DMG"
  create-dmg \
    --volname "AZADI $VERSION" \
    --window-size 540 380 \
    --icon-size 110 \
    --icon "$(basename "$APP_PATH")" 140 190 \
    --app-drop-link 400 190 \
    --no-internet-enable \
    "$DMG" "$APP_PATH" || {
      echo "create-dmg reported a non-zero exit; verifying output..."; }
fi

if [[ ! -f "$DMG" ]]; then
  echo "==> Building DMG with hdiutil -> $DMG"
  STAGING="$(mktemp -d)"
  cp -R "$APP_PATH" "$STAGING/"
  ln -s /Applications "$STAGING/Applications"
  hdiutil create -volname "AZADI $VERSION" \
    -srcfolder "$STAGING" -ov -format UDZO "$DMG"
  rm -rf "$STAGING"
fi

echo "==> Done. Artifacts in $OUTPUT"
ls -lh "$OUTPUT"
