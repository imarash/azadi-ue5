"""AZADI: Rise of the Dawn — one-shot editor bootstrap.

Builds everything the code-only repo cannot ship as text:

  1. /Game/Azadi/Materials : M_AzadiSolid, M_AzadiSign, MPC_Azadi
  2. /Game/Azadi/Maps/L_Bazaar : playable graybox (street, alleys, stalls,
     arches, courtyards, plaza, spawners, signs, cage, exit, lighting)
  3. Stub maps for the other four districts so campaign travel works.

Run inside the Unreal editor (Output Log → Cmd, switch to Python):

    py azadi_bootstrap.py            # build missing pieces
    py azadi_bootstrap.py --force    # rebuild maps from scratch

Everything spawned is plain engine types + Azadi C++ classes, so maps stay
valid when art packs (Fab/Leartes/Megascans) replace graybox volumes.
"""

import sys
import time

import unreal

FORCE = "--force" in sys.argv

DISTRICT_MAPS = ["L_Bazaar", "L_Rooftops", "L_Detention", "L_TVTower", "L_Plaza"]

MATERIAL_DIR = "/Game/Azadi/Materials"
MAP_DIR = "/Game/Azadi/Maps"
CUBE = "/Engine/BasicShapes/Cube.Cube"
PLANE = "/Engine/BasicShapes/Plane.Plane"

ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary


def log(msg):
    unreal.log("[azadi] {}".format(msg))


def get_subsystem(cls):
    return unreal.get_editor_subsystem(cls)


# ---------------------------------------------------------------- materials


def make_solid_material():
    path = "{}/M_AzadiSolid".format(MATERIAL_DIR)
    if EAL.does_asset_exist(path):
        return unreal.load_asset(path)
    mat = ASSET_TOOLS.create_asset("M_AzadiSolid", MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew())
    try:
        color = MEL.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -500, 0)
        color.set_editor_property("parameter_name", "Color")
        color.set_editor_property("default_value", unreal.LinearColor(0.5, 0.5, 0.55, 1.0))
        MEL.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)

        emissive = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -500, 300)
        emissive.set_editor_property("parameter_name", "Emissive")
        emissive.set_editor_property("default_value", 0.0)

        mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -250, 200)
        MEL.connect_material_expressions(color, "", mul, "A")
        MEL.connect_material_expressions(emissive, "", mul, "B")
        MEL.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

        rough = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -500, 500)
        rough.set_editor_property("r", 0.8)
        MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

        MEL.recompile_material(mat)
    except Exception as err:  # graph building is cosmetic; params may still exist
        log("M_AzadiSolid graph warning: {}".format(err))
    EAL.save_asset(path)
    log("created M_AzadiSolid")
    return mat


def make_sign_material():
    path = "{}/M_AzadiSign".format(MATERIAL_DIR)
    if EAL.does_asset_exist(path):
        return unreal.load_asset(path)
    mat = ASSET_TOOLS.create_asset("M_AzadiSign", MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew())
    try:
        mat.set_editor_property("two_sided", True)

        tex = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -600, 0)
        tex.set_editor_property("parameter_name", "SignTex")
        MEL.connect_material_property(tex, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)

        emissive = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 350)
        emissive.set_editor_property("parameter_name", "Emissive")
        emissive.set_editor_property("default_value", 0.35)

        mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 250)
        MEL.connect_material_expressions(tex, "RGB", mul, "A")
        MEL.connect_material_expressions(emissive, "", mul, "B")
        MEL.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

        rough = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -600, 550)
        rough.set_editor_property("r", 0.6)
        MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

        MEL.recompile_material(mat)
    except Exception as err:
        log("M_AzadiSign graph warning: {}".format(err))
    EAL.save_asset(path)
    log("created M_AzadiSign")
    return mat


def make_mpc():
    path = "{}/MPC_Azadi".format(MATERIAL_DIR)
    if EAL.does_asset_exist(path):
        return unreal.load_asset(path)
    mpc = ASSET_TOOLS.create_asset(
        "MPC_Azadi", MATERIAL_DIR, unreal.MaterialParameterCollection,
        unreal.MaterialParameterCollectionFactoryNew())
    try:
        liberation = unreal.CollectionScalarParameter()
        liberation.set_editor_property("parameter_name", "Liberation")
        liberation.set_editor_property("default_value", 0.0)
        mpc.set_editor_property("scalar_parameters", [liberation])
    except Exception as err:
        log("MPC param warning: {}".format(err))
    EAL.save_asset(path)
    log("created MPC_Azadi")
    return mpc


# ---------------------------------------------------------------- helpers


class LevelBuilder(object):
    def __init__(self):
        self.actors = get_subsystem(unreal.EditorActorSubsystem)
        self.cube = unreal.load_object(None, CUBE)
        self.plane = unreal.load_object(None, PLANE)

    def box(self, label, cx, cy, sx, sy, sz, z=None, yaw=0.0):
        """Axis-aligned cube. z = center height; default sits on the ground."""
        cz = (sz * 0.5) if z is None else z
        actor = self.actors.spawn_actor_from_object(
            self.cube, unreal.Vector(cx, cy, cz), unreal.Rotator(0.0, 0.0, yaw))
        actor.set_actor_scale3d(unreal.Vector(sx / 100.0, sy / 100.0, sz / 100.0))
        actor.set_actor_label(label)
        return actor

    def spawn(self, cls, label, x, y, z, pitch=0.0, yaw=0.0):
        actor = self.actors.spawn_actor_from_class(
            cls, unreal.Vector(x, y, z), unreal.Rotator(pitch, yaw, 0.0))
        actor.set_actor_label(label)
        return actor

    def sign(self, slot, label, x, y, z, yaw, w, h):
        actor = self.actors.spawn_actor_from_object(
            self.plane, unreal.Vector(x, y, z), unreal.Rotator(90.0, yaw, 0.0))
        actor.set_actor_label(label)
        # Replace the plain plane actor with the sign actor class:
        sign = self.actors.spawn_actor_from_class(
            unreal.AzadiSignActor, unreal.Vector(x, y, z), unreal.Rotator(90.0, yaw, 0.0))
        sign.set_editor_property("slot_id", slot)
        sign.set_actor_scale3d(unreal.Vector(w / 100.0, h / 100.0, 1.0))
        sign.set_actor_label(label)
        self.actors.destroy_actor(actor)
        return sign

    def lighting(self, preset="dusk"):
        sun = self.spawn(unreal.DirectionalLight, "AzadiSun", 0, 0, 800, pitch=-12.0, yaw=55.0)
        sun.set_editor_property("tags", ["AzadiSun"])
        try:
            light = sun.get_component_by_class(unreal.DirectionalLightComponent)
            # Movable so district lighting presets can rotate the sun at runtime.
            light.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
            light.set_editor_property("intensity", 5.0)
            light.set_editor_property("light_color", unreal.Color(255, 178, 102, 255))
            light.set_editor_property("atmosphere_sun_light", True)
        except Exception as err:
            log("sun warning: {}".format(err))

        self.spawn(unreal.SkyAtmosphere, "SkyAtmosphere", 0, 0, 0)

        sky = self.spawn(unreal.SkyLight, "SkyLight", 0, 0, 500)
        try:
            sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
            sky_comp.set_editor_property("real_time_capture", True)
        except Exception as err:
            log("skylight warning: {}".format(err))

        fog = self.spawn(unreal.ExponentialHeightFog, "HeightFog", 0, 0, 100)
        try:
            fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
            fog_comp.set_editor_property("fog_density", 0.03)
        except Exception as err:
            log("fog warning: {}".format(err))

        try:
            ppv = self.spawn(unreal.PostProcessVolume, "ExposureClamp", 0, 0, 200)
            ppv.set_editor_property("unbound", True)
            ppv.set_editor_property("tags", ["AzadiGrade"])
            settings = ppv.get_editor_property("settings")
            settings.set_editor_property("override_auto_exposure_min_brightness", True)
            settings.set_editor_property("override_auto_exposure_max_brightness", True)
            settings.set_editor_property("auto_exposure_min_brightness", 0.8)
            settings.set_editor_property("auto_exposure_max_brightness", 1.2)
            ppv.set_editor_property("settings", settings)
        except Exception as err:
            log("post-process warning: {}".format(err))

    def navmesh(self, cx, cy, sx, sy, sz=1000.0):
        try:
            vol = self.spawn(unreal.NavMeshBoundsVolume, "NavBounds", cx, cy, sz * 0.5)
            vol.set_actor_scale3d(unreal.Vector(sx / 200.0, sy / 200.0, sz / 200.0))
        except Exception as err:
            log("navmesh volume warning (AI falls back to direct steering): {}".format(err))


def new_level(name):
    les = get_subsystem(unreal.LevelEditorSubsystem)
    path = "{}/{}".format(MAP_DIR, name)
    if EAL.does_asset_exist(path):
        log("{} exists — skipping (use --force to rebuild)".format(name))
        return None
    les.new_level(path)
    return LevelBuilder()


def force_clean():
    """Delete existing district maps. The currently-open world cannot be
    deleted safely, so switch to a scratch level first."""
    les = get_subsystem(unreal.LevelEditorSubsystem)
    scratch = "{}/Scratch_{}".format(MAP_DIR, int(time.time()))
    les.new_level(scratch)
    for name in DISTRICT_MAPS:
        path = "{}/{}".format(MAP_DIR, name)
        if EAL.does_asset_exist(path):
            EAL.delete_asset(path)
            log("deleted {}".format(name))
    return scratch


def save_level():
    get_subsystem(unreal.LevelEditorSubsystem).save_current_level()


# ---------------------------------------------------------------- L_Bazaar


def build_bazaar():
    b = new_level("L_Bazaar")
    if b is None:
        return

    # Ground strip: street + flanks + plaza
    b.box("Ground", 0, 5600, 7000, 12400, 20, z=-10)

    # West building row (street edge at x=-600, depth 800)
    west = [(0, 2300, 700), (2650, 4400, 550), (4400, 6200, 850), (6550, 9000, 600)]
    for i, (y0, y1, h) in enumerate(west):
        b.box("BldW_{}".format(i), -1000, (y0 + y1) * 0.5, 800, y1 - y0, h)

    # East building row
    east = [(0, 1700, 600), (2050, 5300, 800), (5650, 7600, 500), (7950, 9000, 750)]
    for i, (y0, y1, h) in enumerate(east):
        b.box("BldE_{}".format(i), 1000, (y0 + y1) * 0.5, 800, y1 - y0, h)

    # Rooftop parapet accents (skyline variation)
    for i, (cx, cy, w) in enumerate([(-1000, 1100, 500), (1000, 3600, 650), (-1000, 5300, 520), (1000, 8500, 600)]):
        b.box("Parapet_{}".format(i), cx, cy, w, 300, 140, z=west[0][2] + 70 if cx < 0 else east[1][2] + 70)

    # West courtyard (behind alley at y 6200-6550)
    b.box("CourtW_S", -2000, 5900, 1200, 100, 400)
    b.box("CourtW_N", -2000, 6900, 1200, 100, 400)
    b.box("CourtW_W", -2600, 6400, 100, 1100, 400)

    # East courtyard (behind alley at y 5300-5650)
    b.box("CourtE_S", 2000, 5000, 1200, 100, 400)
    b.box("CourtE_N", 2000, 6000, 1200, 100, 400)
    b.box("CourtE_E", 2600, 5500, 100, 1100, 400)

    # Plaza walls (y 9000..11200)
    b.box("PlazaN", 0, 11200, 3200, 100, 500)
    b.box("PlazaW", -1550, 10100, 100, 2200, 500)
    b.box("PlazaE", 1550, 10100, 100, 2200, 500)

    # Market stalls + canopies
    stalls = [(-380, 1200, 8), (340, 1900, -5), (-360, 3300, 0), (390, 3900, 12),
              (-340, 5300, -8), (370, 5800, 0), (-390, 7100, 5), (350, 7800, -10)]
    for i, (x, y, yaw) in enumerate(stalls):
        b.box("Stall_{}".format(i), x, y, 240, 170, 110, yaw=yaw)
        b.box("Canopy_{}".format(i), x, y, 300, 230, 12, z=240, yaw=yaw)

    # Street arches
    for i, y in enumerate([3000, 6200]):
        b.box("Arch_{}".format(i), 0, y, 1340, 250, 160, z=510)

    # Barricade chokepoint
    b.box("Barricade_0", -250, 4400, 350, 90, 110)
    b.box("Barricade_1", 150, 4650, 350, 90, 110)
    b.box("Barricade_2", -50, 4900, 350, 90, 110)

    # Street clutter
    crates = [(-520, 800, 120), (510, 2600, 90), (-500, 4200, 140), (530, 4800, 100),
              (-510, 6600, 110), (520, 7300, 130), (-480, 8400, 95), (300, 9600, 120)]
    for i, (x, y, s) in enumerate(crates):
        b.box("Crate_{}".format(i), x, y, s, s, s, yaw=(x * 7 + y) % 40)

    # Celebration festoons — hidden until AZADI 0.5 via responder
    festoons = []
    for i, y in enumerate([2000, 4800, 7600]):
        festoons.append(b.box("Festoon_{}".format(i), 0, y, 1200, 16, 8, z=400))
    responder = b.spawn(unreal.AzadiLiberationResponder, "CelebrationResponder", 0, 4800, 50)
    responder.set_editor_property("threshold", 0.5)
    responder.set_editor_property("show_when_liberated", festoons)

    # Signage (slots from the bazaar_fa signage set)
    b.sign("billboard_main", "Sign_Billboard", -590, 3600, 330, 0, 500, 250)
    b.sign("awning_grocer", "Sign_AwningW", -590, 1200, 260, 0, 260, 130)
    b.sign("awning_grocer", "Sign_AwningE", 590, 5800, 260, 180, 260, 130)
    b.sign("banner_street", "Sign_ArchBanner", 0, 3010, 360, 90, 120, 240)
    b.sign("mural_wall", "Sign_MuralE", 2540, 5500, 220, 180, 250, 250)
    b.sign("mural_wall", "Sign_MuralW", -2540, 6400, 220, 0, 250, 250)
    b.sign("billboard_main", "Sign_PlazaBoard", 0, 11140, 400, 270, 500, 250)

    # Gameplay actors
    b.spawn(unreal.PlayerStart, "PlayerStart", 0, 400, 120, yaw=90)
    b.spawn(unreal.AzadiEnemySpawner, "Spawner_AlleyW", -800, 2475, 50)
    b.spawn(unreal.AzadiEnemySpawner, "Spawner_CourtE", 800, 5475, 50)
    b.spawn(unreal.AzadiEnemySpawner, "Spawner_Stalls", -380, 7000, 50)
    plaza_spawner = b.spawn(unreal.AzadiEnemySpawner, "Spawner_Plaza", 0, 10000, 50)
    plaza_spawner.set_editor_property("activation_radius", 3000.0)

    b.spawn(unreal.AzadiCitizenCage, "CitizenCage", 2000, 5500, 10)

    exit_zone = b.spawn(unreal.AzadiExitZone, "ExitZone", 0, 10800, 200)
    exit_zone.set_actor_scale3d(unreal.Vector(2.0, 1.0, 1.0))

    b.lighting("dusk")
    b.navmesh(0, 5600, 7200, 12600)

    save_level()
    log("L_Bazaar graybox built")


# ---------------------------------------------------------------- stubs


def build_stub(name, cages=0, platforms=False):
    b = new_level(name)
    if b is None:
        return

    b.box("Ground", 0, 0, 4000, 4000, 20, z=-10)
    b.box("WallN", 0, 2000, 4100, 100, 350)
    b.box("WallS", 0, -2000, 4100, 100, 350)
    b.box("WallW", -2000, 0, 100, 4100, 350)
    b.box("WallE", 2000, 0, 100, 4100, 350)

    if platforms:
        for i, (x, y, h) in enumerate([(-900, -500, 200), (600, 200, 320), (-300, 900, 240), (1100, 1100, 400)]):
            b.box("Roof_{}".format(i), x, y, 700, 700, h)

    for i in range(cages):
        b.spawn(unreal.AzadiCitizenCage, "CitizenCage_{}".format(i), -1200 + i * 1200, 800, 10)

    b.spawn(unreal.PlayerStart, "PlayerStart", 0, -1700, 120, yaw=90)
    b.spawn(unreal.AzadiEnemySpawner, "Spawner_A", -800, 300, 50)
    b.spawn(unreal.AzadiEnemySpawner, "Spawner_B", 800, -300, 50)
    b.spawn(unreal.AzadiExitZone, "ExitZone", 0, 1800, 200)

    b.lighting()
    b.navmesh(0, 0, 4200, 4200)

    save_level()
    log("{} stub built".format(name))


def main():
    log("bootstrap starting (force={})".format(FORCE))
    make_solid_material()
    make_sign_material()
    make_mpc()

    scratch = force_clean() if FORCE else None

    build_bazaar()
    build_stub("L_Rooftops", platforms=True)
    build_stub("L_Detention", cages=3)
    build_stub("L_TVTower", platforms=True)
    build_stub("L_Plaza")

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    if scratch and EAL.does_asset_exist(scratch):
        EAL.delete_asset(scratch)
    log("bootstrap done — Play In Editor from L_Bazaar")


if __name__ == "__main__":
    main()
