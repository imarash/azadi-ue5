"""AZADI editor startup hints (auto-run by the Python plugin)."""

import unreal

# Editor only: editor-script libraries crash in -game mode.
if unreal.is_editor():
    if not unreal.EditorAssetLibrary.does_asset_exist("/Game/Azadi/Maps/L_Bazaar"):
        unreal.log_warning(
            "[azadi] Graybox maps not built yet. Run:  py azadi_bootstrap.py  "
            "(Output Log -> Cmd dropdown -> Python). See ue5/azadi/docs/ARCHITECTURE.md."
        )
