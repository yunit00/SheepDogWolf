# The input system

How a key press or stick push becomes Ralph moving, and where to change it. Everything here is the game's own
code as decompiled; the file references are to this repository's sources.

The short version: the PC game fakes a **PlayStation 1 pad**. DirectInput devices are read, squeezed into a PS1
button word plus one analog stick, and every piece of game code only ever looks at that fake pad. So the
feel of the controls is mostly decided in four places: the **binding tables** (which key/button is which PS1 button),
the **remap table** (which PS1 button is which action), **`ScnControllable::ReadPad`** (pad → movement intent)
and the **Wolf's movement profiles** (intent → speed and turning).

## 1. The pipeline

```
DirectInput devices           InputMgr (g_inputMgr)            Pad (g_pad)                 game code
─────────────────────         ──────────────────────           ─────────────               ─────────────────────────────
Keyboard  256 keys   ─┐       Poll():                          Pad_ReadRaw → raw           ScnControllable::ReadPad
Joystick  X,Y,Z +    ─┼──►     axes → axisX/axisY (0..255)  ──► Latch(): prev = cur,     ──► (stick → dir + magnitude,
          12 buttons  │        12 bindings → padBits           cur = raw, auto-repeat      buttons → WolfActionBits)
Mouse     4 buttons  ─┘         (PS1 active-low word)          AnalogToDpadBits (menus)    Wolf state machine, camera,
                               Esc / Enter / P edges,                                      menus (Pad_MenuHeld/Pressed),
                               arrow-key menu word                                         objects (catapult, cannon...)
```

Once per frame `Main_Loop` (src/app/app_main.cpp) calls `Input_Poll()` (src/engine/input.cpp), which
runs everything left of "game code".

| Layer | Files | What it does |
|---|---|---|
| Devices | src/app/input_device.cpp, src/app/keyboard.cpp, src/app/joystick.cpp, src/engine/mouse.cpp | DirectInput 8 devices: button arrays and three axes normalised to -1..1 |
| Manager | src/engine/input_mgr.cpp (`InputMgr`, one instance `g_inputMgr`) | binding tables, builds the PS1 word, keyboard edges, loads/saves bindings |
| Pad | src/engine/input.cpp (`Pad`, `g_pad`, `g_inputMap`) | PS1 libpad imitation: frames, latch, auto-repeat, dead zones, remap, menu tests |
| Game | src/game/scn_controllable.cpp, src/game/wolf*.cpp, src/objects/camera.cpp, many objects | reads `g_pad` through the remap table |

## 2. Devices

**The port's devices read SDL3, not DirectInput** (src/platform/platform.h, `Platform_Read*`, from keyboard.cpp,
joystick.cpp and mouse.cpp). The keyboard is still presented to the game indexed by DirectInput scan code (`s_keyMap` in
src/platform/platform_sdl.cpp translates SDL's scan codes), so the bindings, the saved configurations and the `.BSC`
files are unchanged; key names come from SDL in the current layout. Nothing reads input without the window's focus (as
DirectInput's foreground mode). What follows describes the devices as the game sees them.

- **Keyboard** (src/app/keyboard.cpp): 256 keys by DirectInput scan code (`DIK_*`). It also has six "axis keys";
  the four menu direction bindings set X-/X+/Y-/Y+ (default: the arrow keys), and the keyboard's `axisX/axisY` are
  -1 / 0 / +1 from them. The keyboard is a **digital** device: its axes become d-pad bits, never an analog stick.
- **Joystick** (src/app/joystick.cpp): in the port, an **SDL gamepad** (SDL's standard layout, so every controller SDL
  knows reads the same). It delivers the **left stick** (range ±65535, as DirectInput's was) and **12 buttons already
  in PlayStation order** (Triangle, Circle, Cross, Square, L1, R1, L2, R2, Select, Start, L3, R3), from the fixed table
  `s_padMapping` in src/platform/platform_sdl.cpp (§3), and the **right stick** (`rawRX/rawRY` → `axisRX/axisRY`, no
  calibration). Not read yet: **the d-pad**. Axis values
  are rounded down to a multiple of 8; there is **no dead zone at this level** (it comes later, §5).
- **Mouse** (src/engine/mouse.cpp): created but never selected in practice.

`InputDevice::Update` (src/app/input_device.cpp) makes `buttonPressed[]` from `buttonDown[]`: with `allowHeld`
set (the normal case) a held button stays "pressed" every frame, so edges are computed later from the pad frames.

Which device drives the pad is chosen at start-up: `g_inputMgr.SelectDevice(g_launcherInputMode)`
(src/app/app_main.cpp, `App_InitGameSystems`). It is the controller option (`--controller=keyboard|N`, or the
launcher's choice: src/platform/options.h): the keyboard, or SDL's N-th controller as the joystick.
If the selected pad stops answering, `CheckDevices`
(src/engine/input_mgr.cpp) falls back to the keyboard table and takes the pad back when it answers again.

## 3. Bindings: device buttons → PS1 buttons

`InputMgr` holds two tables of 16 `{device, code}` entries (src/engine/input_mgr.cpp): `keyTable` (keyboard mode)
and `padTable` (joystick mode). Entries 0..11 are PS1 buttons (`INPUT_BIND_*`), 12..15 the menu directions.
`Poll()` (src/engine/input_mgr.cpp) clears one bit of the active-low word `padBits` per bound button that is down.

**The controller's mapping is fixed** (no rebinding, no saved controller config). The platform layer reads SDL's standard
gamepad and hands the game the twelve PlayStation buttons in slot order, so `InputMgr::ApplyFixedPadMapping`
(src/engine/input_mgr.cpp) binds PlayStation button *n* to the controller's button *n*, after the saved / default
bindings are loaded (an old `UserConf`, or the disc's `DefPConf.BSC`, made for 2001 DirectInput button numbers, cannot
scramble it), and `InputMgr::BindAction` refuses to rebind the controller from the pause menu. **To change the mapping,
edit `s_padMapping` in src/platform/platform_sdl.cpp**: one line per PlayStation button, a gamepad button or a trigger.
SDL names the face buttons by position, so the same table fits Xbox, PlayStation and Switch pads:

| Slot | PS1 button | Keyboard default (DefKConf) | Controller (fixed) |
|---|---|---|---|
| 0 | Triangle | Left Shift | north (Y / Triangle) |
| 1 | Circle | Z | east (B / Circle) |
| 2 | Cross | X | south (A / Cross) |
| 3 | Square | Space | west (X / Square) |
| 4 | L1 | Left Alt | left shoulder |
| 5 | R1 | C (the disc says Left Ctrl, see below) | right shoulder |
| 6 | L2 | A | left trigger (pulled past `PAD_TRIGGER_THRESHOLD`) |
| 7 | R2 | S | right trigger |
| 8 | Select | F12 | back / view / share |
| 9 | Start | Enter | start / menu / options |
| 10 | L3 | F1 | left stick click (L3/R3 are forced released anyway, §4) |
| 11 | R3 | F2 | right stick click |
| 12-15 | menu up/right/down/left | arrow keys | – (menus also follow the left stick) |

The port changes one disc default: sneak (R1) is C instead of Left Ctrl, because macOS takes Ctrl + arrow keys for
Mission Control and switching Spaces before the game sees them. `InputMgr::ApplyDefaultKeyChanges` makes the change
only when the disc's defaults are loaded, so a layout saved from the pause menu is kept; it applies on every OS so the
layout is the same everywhere.

Keyboard bindings: stored bindings win over the defaults. `LoadConfig` (src/engine/input_mgr.cpp) reads the saved `KeybConf` /
`UserConf` values (Config key, see src/platform/save_store.h for where the port keeps them) and only falls
back to the `.BSC` files. The blob format ("BlackSheep Controller Config" "V2.0" [MASTER dev scale×3 offset×3]
"REMAP" count {u8 dev, u8 pad, u16 code}×count "REMAP_DIR" u16×4) is documented at `LoadBindingTable`
(src/engine/input_mgr.cpp). Device index: 1 keyboard, 2 joystick, 4 mouse. Pause, P, Enter and Esc can't be
bound on the keyboard from the in-game menu (`DIK_IS_RESERVED`), though the default file binds Start to Enter.

Fixed keyboard keys, outside the tables (`Poll`, src/engine/input_mgr.cpp):
- **Esc**: opens the pause (options) menu (src/engine/scenaric_loop.cpp); skips cutscenes; "back" in menus.
- **P / Pause**: the plain "paused" screen (src/engine/scenaric_loop.cpp).
- **Enter**: "confirm" in menus.
- **Arrow keys**: the menu direction word, with its own auto-repeat (500 ms, then every 125 ms).

## 4. The pad frame and the remap table

`Pad_ReadRaw` (src/engine/input.cpp) copies `axisX/axisY` (0..255, centre 0x80) and `padBits` into `g_pad.raw`.
It also copies the right stick (`InputMgr::rightX/rightY`, 0..255, centre 0x80; centred on the keyboard). The original
PC build set it to centre every frame, since no DirectInput axis fed it; the port fills it, so the camera code reads it
as on the PlayStation (§7).

`Pad::Latch` (src/engine/input.cpp) then: forces L3 and R3 released, shifts `cur` to `prev` and `raw` to `cur`,
and runs two auto-repeaters (500 ms first, then 125 ms; one global delay shared by everything).
`AnalogToDpadBits` (src/engine/input.cpp) folds the left stick into d-pad bits for menus (`menuCur`).

**Actions are not tied to buttons directly.** Game code tests `g_padMasks[i]`, which is `g_inputMap[4 + i]`
(src/engine/input.cpp), a table of PS1 bit masks the options screen can permute:

| `g_inputMap` slot | `g_padMasks[i]` | Default button | Keyboard default | Used for |
|---|---|---|---|---|
| 0 | – | Select | F12 | the map / select screen (src/engine/scenaric_loop.cpp) |
| 3 | – | Start | Enter | no gameplay use found |
| 4-7 | 0-3 | Up, Right, Down, Left | arrows | digital movement, aiming (cannon, telescope) |
| 8 | 4 | L2 | A | camera turn left (hold) / snap left |
| 9 | 5 | R2 | S | camera turn right; L2+R2 recentres |
| 10 | 6 | L1 | Left Alt | inventory wheel (hold, left/right to pick) |
| 11 | 7 | R1 | C | sneak (hold) |
| 12 | 8 | Triangle | Left Shift | look mode (hold); robot / cannon uses |
| 13 | 9 | Circle | Z | run (the tap rhythm, §6) |
| 14 | 10 | Cross | X | action: talk, pick up, push, use; skip dialogue |
| 15 | 11 | Square | Space | jump; again in the air = double jump |

The options screen's presets (`Input_ApplyControlConfig`, src/engine/input.cpp; saved in `Progress.controls`)
only permute slots 10-15 (L1, R1 and the four face buttons). Menus use `Pad_MenuHeld / Pressed / Repeat`
(src/engine/input.cpp), which test fixed PS1 bits (Cross = confirm, Triangle = back) plus the keyboard
fallbacks (Enter, Esc, arrows).

## 5. From pad to movement intent: `ScnControllable::ReadPad`

src/game/scn_controllable.cpp turns the pad into three numbers and a bit set on the player object:

- **Analog pad** (pad type 7, i.e. a joystick device is selected): `Pad_AnalogToStick` (src/engine/input.cpp):
  a **round dead zone of radius 56 out of 128 (44 %)**, full strength at 120; strength 0..256 and a direction.
- **Keyboard / digital**: the d-pad bits give x, y = ±256; a diagonal is (181, 181). **Always full strength**:
  the keyboard can't walk slowly.
- Buttons become `WolfActionBits` (`WOLF_ACT_*`, sdw_enums.h): jump edge / held (+ double jump), run edge / held,
  action edge / held, sneak held.

`GetStickHeading` (src/game/scn_controllable.cpp) turns the stick into a world heading **relative to the camera's
current yaw** (`g_camYaw`), recomputed every frame. With the camera still swinging behind Ralph, "up" keeps
changing direction under you, which is a big part of the drifting feel.

## 6. Ralph's movement

`Wolf::StateMachine` (src/game/wolf.cpp onward) picks a state; each state has a descriptor
(`g_wolfStateDescriptors0`, anim / **profile** / camera mode) and moves through `Mobile_Steer`
(src/game/scn_controllable.cpp) with that profile's `MoveRecord`:

```
target speed = maxSpeed × stick strength / 256 × cos(angle between stick and current motion)
speed        approaches the target at `acceleration` / `deceleration`
facing, motion heading   turn toward the stick at most at maxFacingTurnRate / maxTravelTurnRate,
                         accelerating the turn at facingTurnAcceleration / travelTurnAcceleration
```

The profiles are the table `g_wolfMoveProfilesNormal[surface][profile]` (src/game/wolf.cpp; row 1 is ice) and
`g_wolfMoveProfilesCarry` (carrying something). The ones that matter most:

| Profile | Used by | maxSpeed | accel / decel | travel turn rate / accel |
|---|---|---|---|---|
| 0 | walk, jumps, landing | 600 | 4000 / 3000 | 81920 / 245760 |
| 1 | sneak | 250 | 1000 / 1000 | 20480 / 61440 |
| 2 | **run** | 1200 | 1200 / 1200 | **3413** / 40960 |
| 3 | push | 150 | 800 / 1000 | 1024 / 2048 |
| 6 | climb | 100 | 1000 / 1000 | 20480 / 61440 |
| 7 | climb-box walk | 300 | 2000 / 1000 | 20480 / 61440 |
| 8 | swim | 300 | 1000 / 1000 | 4096 / 12288 |

(Speeds in world units per second; the turn values are relative, the higher the faster. On ice, row 1: Ralph still
faces the stick quickly, but his direction of motion turns far slower (walk: 2048 instead of 81920), so he slides.)

**Running is a rhythm game.** Pressing run starts `WOLF_ST_RUN_START`; to keep the sprint, `Wolf::RunTapCheck`
(src/game/wolf_misc.cpp) wants the next run press **between 0.1 s and 0.75 s** after the previous one
(`bank->params[5] = 0x199`, `params[6] = 0xc00`, in 1/4096 s). Tapping too fast cancels it, too slow drops it. While
running Ralph turns at 3413 instead of 81920: about **24 times slower than walking**, which is why a sprint feels
like a runaway train.

Jump numbers are in `Wolf::InitMoveConfig` (src/game/wolf.cpp): `params[1] = 200` (jump height),
`params[2] = 0x4fd` (ascent time, 0.31 s); a second jump press counts after a quarter of that.

## 7. The camera

`Camera_UpdateFollow` and its siblings (src/objects/camera.cpp, from about line 1850): the follow camera turns with
L2 / R2 (hold; both = recentre), or snaps a step per press when the level's camera restriction says `snapYaw`.
**It already reads a right stick** (`Pad_StickToDeadzonedAxes(pad->cur.rightX, ...)`, src/objects/camera.cpp;
square dead zone of 56), and the port fills it from the controller's right stick (§4): left/right turns the follow camera (the vertical value is
computed but the follow camera does not use it), and it also drives the look, Sam-chase and directed cameras and the
telescope. It is ignored where a CameraRestriction sets `forbidPad` or a trajectory (the level's scripted cameras). Per-mode speeds are
`g_camModeParams[mode]`; there is also an automatic yaw (`g_camAutoYawRate`) pulling the camera behind Ralph.

## 8. Timing

`Time_Update` (src/engine/time.cpp): `g_dt` is the frame time in 1/4096 s, **capped at 0xAA (41.5 ms)**, so under
about 24 fps the game runs in slow motion rather than skipping. Input is read once per frame, before the game frame.

## 9. Where to tweak: the easy wins

In rough order of payoff:

1. **A modern pad layout**: done (fixed, §3). Making it configurable: §11.
2. **Hold to run** instead of the tap rhythm: in `Wolf::RunTapCheck` (src/game/wolf_misc.cpp) keep
   `WOLF_FB_RUN` set while `padBits & WOLF_ACT_RUN_HELD`, and clear it on release.
3. **Steerable running**: raise profile 2's `maxTravelTurnRate` / `maxFacingTurnRate` (src/game/wolf.cpp;
   3413 now, walking has 81920).
4. **Smaller dead zones**: 56/128 in `Pad_AnalogToStick` (src/engine/input.cpp, `0xc40` = 56², and `0x38`),
   `Pad_StickToDeadzonedAxes` and `Pad::AnalogToDpadBits`. Around 16-24 suits modern sticks.
5. **Right-stick camera**: done (§4, §7). Mouse camera: not yet.
6. **D-pad on a gamepad**: read `rgdwPOV[0]` in `Joystick::Update` and clear the `PAD_UP/RIGHT/DOWN/LEFT` bits in
   `InputMgr::Poll`.
7. **Walking on the keyboard**: e.g. a "walk" key that scales the ±256 of the digital path in `ReadPad`.
8. **Less drift**: `GetStickHeading` could use the camera yaw sampled when the stick left the dead zone rather than
   every frame (classic "camera-relative lock").

## 10. The SDL3 input layer

Done at the device level (§2): SDL fills the same `InputDevice` fields DirectInput did, and the mouse is read through
SDL too (relative motion and wheel), ready for a mouse camera. A deeper replacement would sit one level up, because
everything above `InputMgr` only reads what `InputMgr::Poll` produces:

- `axisX`, `axisY` (0..255, centre 0x80) and the right-stick pair once added;
- `padBits` (the PS1 active-low word, `PAD_*` bits);
- `escPressed`, `enterPressed`, `pausePressed` (edges) and `menuNav` (arrow-key word with repeat);
- `IsDeviceDigital(0)`, which decides between the analog and the digital movement paths.

An SDL3 gamepad / keyboard backend only needs to fill these (plus `GetAnyPressed` for the controls menu's
rebinding and the `Load/SaveBindingTable` blobs if the in-game remapping should keep working). The controller already
goes through `SDL_Gamepad` with the fixed PlayStation mapping of §3, right stick included; the d-pad and using the
keyboard and a controller at the same time are the next steps.

## 11. Making the controller mapping configurable (not done)

A sensible way to add it later, without touching the game's own input code:

- **Keep the platform layer as the only place that knows SDL's buttons.** Turn `s_padMapping` into a runtime table
  (`Platform_SetPadMapping(const PadMappingEntry[12])`), still delivering the twelve PlayStation buttons in slot order;
  the game side (`ApplyFixedPadMapping`, the binding tables, `Poll`) stays as it is.
- **Store it beside the other options**, as text the user can also edit: one save-store string per PlayStation button
  in the app key, e.g. `PadCross=south`, `PadL2=lefttrigger` (`SDL_GetGamepadButtonFromString` /
  `SDL_GetGamepadAxisFromString` parse SDL's own names), read by `Options_LoadSaved` (src/platform/options.c) and
  defaulting to the current table. Not the game's `UserConf` blob: its format is per device index and button number,
  made for 2001 joysticks.
- **Edit it in the launcher**: a "Controller" page with one row per PlayStation button and a "press a button" capture
  (the launcher already opens SDL's gamepad subsystem), plus "Reset to defaults". Optionally `--pad-map=...` on the
  command line, for testing.
- **Leave the in-game controls menu keyboard-only** (`BindAction` keeps refusing the controller), or teach it to call
  the platform setter; the launcher route avoids touching the decompiled menu code.
- **Button prompts**: the game draws PlayStation glyphs (`$B_CROSS$`...); with a configurable mapping they still name
  the PlayStation button, which stays correct as long as the mapping is described in those terms.
