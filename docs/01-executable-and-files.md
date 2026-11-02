# The original program and the game's files

Everything here is verified against the files unless marked *(inferred)*.

## The original program

`SheepD3D.exe`, the PAL PC release (SHA-1 `ca39374d53ae0030c5bd8c90dda45465e446dfe3`), built on 26 July 2001 with Microsoft Visual C++ 6.0. It used DirectDraw with Direct3D 7 immediate mode, DirectInput 8, DirectSound, WINMM's mmio functions (for WAV files) and DirectShow (for the movies). Its clock was RDTSC calibrated against `QueryPerformanceCounter` ([the frame loop](03-time-and-frame-loop.md)). This port replaces all of those with SDL3; the Windows build still uses DirectShow for the movies.

The code is C++ built without RTTI, so class names come from strings, the disc headers and behaviour, not from the binary's type information.

## Naming evidence on the disc and in the program

1. **Developer headers left on the disc** in `Levels/Lvl-03/`:
   - `Scenaric_Classes.h` (2,382 lines): every scenaric object class with its `CLASSID_*`, per-property byte offsets (`PROPERTY_SAM_GREENZONEBOX 44`, ...) and `PROPSIZE_*`. It is the schema of the object instances in the level files and the source of the canonical class names (Wolf, Sam, Sheep, Dynamite, Rocket, Mailbox...). `tools/scenaric_to_c.py` turns it into `src/include/scenaric_props.h`.
   - `GameRes.h`: the resource ids exported from the DAV and WAR files (`WAR_IDO_*` objects, `DAV_IDI_*` images).
   - `Cin_Lvl-03.h`: cinematic ids. `Lvl-03.SoundIndex` / `.TextIndex`: sound and text ids; the index files also keep the original asset paths.
2. **Strings in the program** (about 930): state names (`SAM_STT_FOLLOWNODES`, `EnterReachGoalState`, `EnterFollowTrajectoryState`, `EnterSearchNearestNodeState`), function names in diagnostics (`Flock_GetObjNearestAvailableSheep_Vision`, `Install_ScenaricResource`, `Load_DAVnWAR`, `GetResourceType`), the loader's progress messages, which give the initialisation order (collisions, shadows, camera, clusters, menus, map), camera and AI debug messages, and Hungarian-style variable names.
3. **PlayStation heritage**: button tokens (`$B_CROSS$`, `$B_L1$`), memory-card message tables with placeholder entries for the PC version, and the PlayStation memory-card save format ([the object system](04-object-system.md)). The PC build is a port of the PlayStation code base.

## Data formats

All uncompressed, little-endian and magic-tagged. [uhwot/sdw_re_stuff](https://github.com/uhwot/sdw_re_stuff) (MIT) documents the asset formats of this build with a Go exporter, ImHex patterns and a WAR CRC32 tool.

| Extension | Magic | Contents |
|---|---|---|
| `.WAR` | CRC32 + `V2.6` | A level's world: objects, scenaric instances and their property blocks, collision, trajectories and nodes. The resource table is at 0x10 ([00-conventions.md, "WAR level files"](00-conventions.md#war-level-files)) |
| `.DAV` | `VDX7` + `CHEK`×4 | Graphics: named sections (`_SRA`, `_SDA`, "----Section material----"), textures, models |
| `.SND` | hash + `vdx7` | A bundle of RIFF/WAVE blobs |
| `.MLT` | hash + `v1.2` | Localised strings (NUL-separated, per language) |
| `.BSC` | `BlackSheep Controller ConfigV2.0` | Key and pad mapping (`REMAP`, `REMAP_DIR`) |
| `.BSV` | `BSV_FILEV2.5` | The music and voice bank index |
| `.BSM` | `GREETINGV1.0` | Launcher text |
| `.BVS` | `RIFF…AVI ` | Plain AVI (Indeo 5) |
| `.sdw` | - | The bonus gallery: a JPEG archive with a name/offset/size table |

The original kept saves in the Windows registry, in the layout of a PlayStation memory card ([the object system](04-object-system.md)); the port keeps the same values as files in the user's data folder ([BUILDING.md](../BUILDING.md#4-running)).

Levels: `Lvl-00` ... `Lvl-17`, plus `Scene`, `Wheel`, `Intro`, `Fend`, `Ending` and `Demos`. [00-conventions.md](00-conventions.md#levels-two-numberings) maps the `Lvl-NN` folders to the level numbers players use.
