#!/usr/bin/env python3
"""Guess a target folder for every type in refactor/class_map.csv (from src/include/sdw_classes.h).

Writes refactor/class_map_guess.csv: the map's columns plus
  category     the folder's theme (e.g. "enemies")
  target_path  folder + file stem under src/, without extension (e.g. game/entities/enemies/bull)
  confidence   high: the code says so (inventory flag, base class, owner); medium: name/role; low: a guess
  why          what the guess rests on
A helper (goes_with set) gets its owner's path, following owners up the chain: it belongs in the owner's header.
A helper named in GROUPS is placed there instead (the .DAV / .WAR records, which describe files, not their owner).

Usage: python3 refactor/guess_paths.py
"""
import csv
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
SRC = HERE.parent / "src"
TARGET_COL = "target_path (fill in: e.g. game/objects/npcs/bull)"

# folder -> (category, types). Order matters only for readability; a type must appear once.
GROUPS = {
    # ---- engine ----
    "engine/scene": ("scene object model", """ScnObject ScnBody ScnLogic ScnMobile ScnLogicShadowed Instance InstanceBase
        WorldObj ScnClassRegEntry ScnRecordSynth ObjGridHeader ListNode Heap Timer ActionHit InteractScan"""),
    "engine/math": ("math", "Vec2s Vec3f Vec3i Vec4i Vec4s Mat34s Mat44 Box6i"),
    "engine/collision": ("collision", """CollBox CollCell CollContact CollMapHeader CollRay CollRayHit CollTri ContactInfo
        GroundQuery ResolveScratch ZoneList"""),
    "engine/render": ("rendering", """Texture PolyBatcher RenderPoly BsPolyBlendFlat BsPolyBlendGouraud BsPolyFlat
        BsPolyGouraud BsPolyTexFlat BsPolyTexGouraud PolyTri Mesh MeshPart AnimMesh Model AltModel Shadow Frustrum
        Screen"""),
    "engine/animation": ("animation", "Animator AnimHeader AnimatedWorldObj AttachLink"),
    "engine/camera": ("camera", """Camera CamFlagBits CamModeParams CamPitchAdjBits CamProbeFrame CamRequestBits
        CamRestrict CamSamPush CamSetup CamShot"""),
    "engine/cinematics": ("cinematics & video", "Cine FmvList Video VideoPlayer"),
    "engine/audio": ("audio", "Sound StaticSound StreamSound SoundDevice StreamPlayer WaveFile SndBankEntry"),
    "engine/io": ("files & resources", """BsFile BsStream FileHandle MltHeader StringBank TextResBank PackJpeg
        WarLevelHeader Dav DavHeader DavBitmapRec Vdx7 Vdx7Record WarFile WarHeader"""),
    "engine/input": ("input", "InputDevice Joystick Keyboard Mouse InputMgr Pad"),
    "engine/fx": ("particles & effects", """Particle ParticleEmitter EmitterColumnParams EmitterDriftParams
        EmitterFadeParams EmitterRiseParams InlineEmitter1 InlineEmitter3 InlineEmitter4 InlineEmitter6
        InlineEmitter8 InlineEmitter10 InlineEmitter16 InlineEmitter32 TrailEmitter HoleFX Weather"""),
    "engine/navigation": ("movement & paths", """NavNode PathFollower Trajectory TrajFollower TrajPatrol MoveRecord
        MoveModifyArg LaunchArc WallAvoid"""),
    "app": ("application", "D3DApp"),
    # ---- ui ----
    "ui": ("menus & HUD", """Menu MenuBox MenuPage MenuState DialogBox ScrollText Sprite AnimSprite UiCursor UiFrame
        UiIcon UiQuad Map Font"""),
    # ---- game ----
    "game/state": ("game state & progress", "GameState Progress DialogueShownFlags BonusEntry TimeTravelArg"),
    "game/player": ("player", "ScnControllable Wolf Robot"),
    "game/entities/enemies": ("enemies", """Sam Sam_Pirate bull Shark Piranhas Bees Hive Bat CrocodileLevel09
        CrocodileLevel11 Dragon Elmer Gossamer_Boss Gossamer_Lev08 GossamerOnde Ghost DancingGhost PrayingGhost
        GhostHalo LazerRobot InstantMartian Marvin Rook Crowd"""),
    "game/entities/npcs": ("story characters", """DaffyElf DaffyLevel01 DaffyLevel02 DaffyLevel09 DaffyMilitary
        DaffyScene DaffyTrainingLevel DaffyWheel PorkyLevel01 bipbip BipbipLevel14 Sheep"""),
    "game/entities/critters": ("ambient animals", "Bird Butterfly Fish Firefly"),
    "game/entities/items": ("inventory items", """DefusableMine Dynamite elastic Fan FishingRod CompositeRod MagnetRod
        SaladRod Flute GhostCostume HairDryer HoneyPot Hoover InflatableSheep InstantHoover Key Magnet MineDetector
        Perfume RabbitCostume RemoteControl Rocket Salad Seed SheepCostume TimeMachineChrono Umbrella"""),
    "game/entities/collectibles": ("collectibles", "GoldenCoins Diamond Watch"),
    "game/entities/hazards": ("hazards & traps", """Mine GroundMine WaterMine WolfTrap FallingRock FireBall Lava Volcano
        Laser BlackHole Cactus TrafficJams Bullet CannonBall CannonBall2"""),
    "game/entities/props/doors": ("doors & gates", """AutomaticDoor DoorLevel DoorWorld DoorMechanism SecretDoor
        FallingGate FallingGate2 Jail"""),
    "game/entities/props/platforms": ("platforms & moving ground", """CrumblyPlat CrumblyGround IceGround SnowyGround
        WoodenLift WoodenPlatForm RollingCarpet RCarpetMobile Raft seesaw balance bridge Monolithe FrozenRiver
        IceCube SlidingIceCube Sail"""),
    "game/entities/props/vehicles": ("vehicles & launchers", """Train TrainCarBody TrainStation Pipe Pipe2 Crane Catapult
        CanonDummy CanonSheep CanonSimple Resizer GeyserIn GeyserOut WaterGeyser GeyserManger"""),
    "game/entities/props/switches": ("buttons & triggers", """SensibleButton SuperButton HitSwitch TriggedStone Goal
        Battery InstantSocket"""),
    "game/entities/props/signs": ("signs & info", "SignPost SignPostSimple SignPostAnimated SignTips SwirlSign Telescope Mailbox"),
    "game/entities/props/containers": ("boxes & containers", "box BoxCrate FloatingBox Case Anvil Bell"),
    "game/entities/props/nature": ("trees, rocks & plants", """Tree ElasticTree TreeSection Twig Seaweed HeapOfLeaf Leaf
        Bush Rocks Rock SmallRock HiddenRocks Snowball Torch"""),
    "game/entities/props/time_machine": ("time machine", "TimeMachineSphere TimeKeeper Chronometer"),
    "game/hub": ("hub & level select", "Wheel WheelDummy Scene_Wheel SceneSheepPanel MapLocation"),
    "game/logic": ("level managers", """AmbientSoundManager CameraManager CameraManager2 CameraRestriction
        CheckpointManager CinematicsManager CreditsManager DancingGhostManager FogManager MirrorManager ObjectManager
        SfxCineManager VisibilityManager LightSpot FacingCamera BonusManager MCardManager"""),
}

# where the guess is weaker than "the name or role says so"
LOW = {"Crowd", "Rook", "GossamerOnde", "GhostHalo", "Marvin", "InstantMartian", "Sail", "Resizer", "Anvil", "Bell",
       "Case", "Torch", "Watch", "Diamond", "LightSpot", "FacingCamera", "Goal", "Monolithe", "TrafficJams", "Cactus",
       "Mailbox", "Telescope", "Battery", "InstantSocket", "ActionHit", "InteractScan", "LaunchArc", "WallAvoid",
       "BoxCrate", "Font", "Map"}


def snake(name):
    s = re.sub(r"(?<=[a-z0-9])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])", "_", name)
    return re.sub(r"_+", "_", s).lower()


def main():
    rows = list(csv.DictReader(open(HERE / "class_map.csv", newline="")))
    where = {}
    for folder, (category, names) in GROUPS.items():
        for n in names.split():
            if n in where:
                raise SystemExit("%s is in %s and %s" % (n, where[n][0], folder))
            where[n] = (folder, category)
    inventory = set(GROUPS["game/entities/items"][1].split())
    owners = {r["type"].strip().lstrip("└").strip(): r["goes_with (helper of)"].strip() for r in rows}

    def placed_owner(owner):
        """the first owner up the chain that GROUPS places, or None"""
        while owner and owner not in where:
            owner = owners.get(owner, "")
        return owner or None

    out = []
    for r in rows:
        name = r["type"].strip().lstrip("└").strip()
        owner = r["goes_with (helper of)"].strip()
        top = placed_owner(owner) if owner and name not in where else None
        r = dict(r)
        if top:
            folder, category = where[top]
            r.update(category=category, target_path="%s/%s" % (folder, snake(top)), confidence="high",
                     why="helper of %s: goes in %s's header" % (owner, top))
        elif name in where:
            folder, category = where[name]
            if name in inventory:
                conf, why = "high", "registered as an inventory item (scn_register.cpp)"
            elif name in LOW:
                conf, why = "low", "guess from the name; check the class"
            elif r["base"] in ("ScnBody", "ScnMobile", "ScnLogic") or folder.startswith("engine"):
                conf, why = "medium", "role from the name, base %s" % (r["base"] or "none")
            else:
                conf, why = "medium", "role from the name"
            r.update(category=category, target_path="%s/%s" % (folder, snake(name)), confidence=conf, why=why)
        else:
            r.update(category="uncategorized", target_path="", confidence="",
                     why=("helper of %s, which is not placed" % owner) if owner else "not placed yet")
        r[TARGET_COL] = r["target_path"]
        out.append(r)
    cols = list(rows[0].keys()) + ["category", "target_path", "confidence", "why"]
    cols.remove(TARGET_COL)
    with open(HERE / "class_map_guess.csv", "w", newline="") as f:
        w = csv.DictWriter(f, cols, extrasaction="ignore")
        w.writeheader()
        w.writerows(out)
    missing = [r["type"].strip() for r in out if r["category"] == "uncategorized"]
    print("%d types, %d placed, %d not placed" % (len(out), len(out) - len(missing), len(missing)))
    if missing:
        print("not placed:", " ".join(missing))


if __name__ == "__main__":
    main()
