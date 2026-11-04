"""
Gate A - monta a cena de validacao do Live Beatdown.

Executado pelo editor via:
  UnrealEditor.exe LiveBeatdown.uproject -ExecutePythonScript="...\gate_a_setup.py"

O que faz:
  1. cria/abre o nivel /Game/Maps/L_GateA a partir do template padrao
     (chao, ceu e luz direcional ja vem nele)
  2. adiciona camera vertical 1080x1920 (9:16) enquadrando o personagem
  3. adiciona tres luzes de estudio (key, fill, rim)
  4. cria o MetaHuman original, monta o preview e coloca no nivel
  5. salva o asset do personagem e o nivel

Cada passo e isolado: se um falhar, os outros continuam e o erro vai para o log.
"""

import unreal
import traceback

LEVEL_PATH = "/Game/Maps/L_GateA"
TEMPLATE_PATH = "/Engine/Maps/Templates/Template_Default"

MH_PACKAGE = "/Game/Characters"
MH_NAME = "MH_Beatdown"
MH_PATH = MH_PACKAGE + "/" + MH_NAME

CAM_LOCATION = unreal.Vector(-350.0, 0.0, 100.0)
CAM_FOV = 30.0
ASPECT = 9.0 / 16.0


def log(msg):
    unreal.log("[GateA] " + str(msg))


def step(label):
    """Decorator de verdade: encapsula a funcao, executa no main() e engole erro."""
    def wrapper(fn):
        def guarded(*args, **kwargs):
            try:
                result = fn(*args, **kwargs)
                log("OK    " + label)
                return result
            except Exception:
                log("FALHA " + label)
                for line in traceback.format_exc().splitlines():
                    unreal.log_warning("[GateA]   " + line)
                return None
        return guarded
    return wrapper


def get_actors():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def spawn(actor_class, location, rotation=None):
    if rotation is None:
        rotation = unreal.Rotator(0.0, 0.0, 0.0)
    return get_actors().spawn_actor_from_class(actor_class, location, rotation)


@step("nivel")
def prepare_level():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        les.load_level(LEVEL_PATH)
        log("nivel existente carregado")
    else:
        les.new_level_from_template(LEVEL_PATH, TEMPLATE_PATH)
        log("nivel criado a partir do template")

    return les


@step("camera 9:16")
def add_camera():
    cam = spawn(unreal.CameraActor, CAM_LOCATION, unreal.Rotator(0.0, 0.0, 0.0))
    if cam is None:
        raise RuntimeError("spawn_actor_from_class retornou None")

    cam.set_actor_label("LB_Camera_Vertical")

    component = cam.get_editor_property("camera_component")
    component.set_editor_property("field_of_view", CAM_FOV)
    component.set_editor_property("constrain_aspect_ratio", True)
    component.set_editor_property("aspect_ratio", ASPECT)

    # Deixa o viewport do editor olhando pela camera.
    log("camera FOV %.1f, aspecto 9:16, a %.0f cm do alvo" % (CAM_FOV, -CAM_LOCATION.x))
    return cam


@step("viewport na camera")
def focus_viewport():
    info = unreal.EditorViewportCameraInfo(
        location=CAM_LOCATION, rotation=unreal.Rotator(0.0, 0.0, 0.0))
    # set_level_viewport_camera_info vive em UnrealEditorSubsystem, nao no LevelEditor.
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    editor.set_level_viewport_camera_info(info)


@step("luzes de estudio")
def add_studio_lights():
    key = spawn(unreal.RectLight, unreal.Vector(-260.0, -220.0, 260.0),
                unreal.Rotator(0.0, -35.0, 40.0))
    if key:
        key.set_actor_label("LB_Key_Light")
        comp = key.get_editor_property("rect_light_component")
        comp.set_editor_property("intensity", 6.0)
        comp.set_editor_property("source_width", 180.0)
        comp.set_editor_property("source_height", 180.0)

    fill = spawn(unreal.RectLight, unreal.Vector(-220.0, 240.0, 200.0),
                 unreal.Rotator(0.0, -25.0, -45.0))
    if fill:
        fill.set_actor_label("LB_Fill_Light")
        comp = fill.get_editor_property("rect_light_component")
        comp.set_editor_property("intensity", 2.5)
        comp.set_editor_property("source_width", 150.0)
        comp.set_editor_property("source_height", 150.0)

    rim = spawn(unreal.RectLight, unreal.Vector(240.0, 60.0, 230.0),
                unreal.Rotator(0.0, -30.0, 190.0))
    if rim:
        rim.set_actor_label("LB_Rim_Light")
        comp = rim.get_editor_property("rect_light_component")
        comp.set_editor_property("intensity", 4.0)
        comp.set_editor_property("source_width", 120.0)
        comp.set_editor_property("source_height", 200.0)

    log("key + fill + rim posicionadas")


@step("asset metahuman")
def create_metahuman_asset():
    asset = unreal.EditorAssetLibrary.load_asset(MH_PATH)

    if asset is None:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        asset = asset_tools.create_asset(
            asset_name=MH_NAME,
            package_path=MH_PACKAGE,
            asset_class=unreal.MetaHumanCharacter,
            factory=unreal.new_object(type=unreal.MetaHumanCharacterFactoryNew),
        )

    if asset is None:
        raise RuntimeError("nao foi possivel criar/carregar o MetaHuman")

    unreal.EditorAssetLibrary.save_asset(MH_PATH, only_if_is_dirty=False)
    log("asset pronto: " + MH_PATH)
    return asset


@step("preview do metahuman")
def spawn_metahuman_preview():
    asset = unreal.EditorAssetLibrary.load_asset(MH_PATH)
    mh = unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)

    if not mh.is_object_added_for_editing(asset):
        if not mh.try_add_object_to_edit(asset):
            raise RuntimeError("try_add_object_to_edit falhou")

    log("montando preview (corpo, rosto, cabelo, roupa)...")
    mh.assemble_for_preview(character=asset)

    actor = mh.spawn_meta_human_actor(character=asset)
    if actor is None:
        raise RuntimeError("spawn_meta_human_actor retornou None")

    actor.set_actor_location(unreal.Vector(0.0, 0.0, 0.0), False, False)
    actor.set_actor_rotation(unreal.Rotator(0.0, 0.0, 180.0), False)
    log("personagem no nivel: " + actor.get_actor_label())
    return actor


@step("salvar nivel")
def save_level():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    les.save_current_level()
    log("nivel salvo em " + LEVEL_PATH)


def main():
    log("=== inicio ===")
    prepare_level()
    add_camera()
    focus_viewport()
    add_studio_lights()
    create_metahuman_asset()
    spawn_metahuman_preview()
    save_level()
    log("=== fim ===")


try:
    main()
except Exception:
    # Nunca deixa excecao escapar: o ExecuterPythonScript encerra o editor com
    # codigo de erro se o script falhar.
    for line in traceback.format_exc().splitlines():
        unreal.log_error("[GateA] " + line)
