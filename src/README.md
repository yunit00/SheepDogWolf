# src/ - the source

| Directory | What it holds |
|---|---|
| `app/` | The application shell: `Game_Main` and the main loop (`app_main.cpp`), the device wrapper the game draws through (`d3dapp.cpp`), and the keyboard / controller input devices |
| `engine/` | The engine: level loading (`list.cpp`, `load_war.cpp`, `load_dav.cpp`), the scenaric object system (`scenaric*.cpp`), meshes and the polygon batcher, collision (`collide.cpp`, `coll_clip.cpp`), the camera maths, text, menus, sound, the frame clock (`time.cpp`, `timer.cpp`), saves (`registry.cpp`, `card.cpp`) |
| `game/` | The player characters: Ralph the wolf (`wolf*.cpp`) and the shared controllable-object code |
| `objects/` | The level objects, one file per scenaric class (sheep, doors, cannons, bosses, ...), and the camera |
| `fx/` | The hole transition effect |
| `render/` | The renderer interface the game draws through (`render_device.h`), the choice of backend (`render_select.cpp`) and the SDL3 GPU backend with its shaders (`sdl3/`) |
| `platform/` | SDL3: `main`, the window and input (`platform_sdl.cpp`), the start-up options (`options.c`: saved settings and command-line flags), the audio mixer (`platform_audio_sdl.cpp`) and the DirectSound interfaces over it (`dsound_sdl.cpp`), the settings and save files (`save_store.c`), and on macOS and Linux the Windows, C runtime and multimedia-I/O names the game code calls (`*_compat.cpp`), with the case-insensitive file lookup |
| `launcher/` | `SheepLauncher`: the Dear ImGui window that picks the options and starts the game with them as flags |
| `sdk/` | Declarations of the Windows, DirectX and C runtime names the game code uses, spelled as their SDKs spell them and holding only what `src/` uses |
| `include/` | Shared headers: the basic types (`sdw_types.h`), the enums (`sdw_enums.h`), every class (`sdw_classes.h`), file-offset pointers for level data (`sdw_fileptr.h`), the renderer hooks the Direct3D call sites use (`sdw_render.h`), and `scenaric_props.h`, generated from your copy of the game |
| `compat/` | The Windows build's C runtime shim and the pure-virtual slots |
| `jpeg/` | IJG libjpeg 6, unmodified |

## How the code is organised

- **Classes.** `sdw_classes.h` declares every class with its base, its virtual methods and its fields. Before including it, a file defines `SDW_MEMBERS_<Class>` for members only it needs (constructors, inline helpers); a shared member header (`game/wolf.h`) defines the macro once, and a file adds to it with `SDW_EXTRA_<Class>`.
- **Shared inline helpers** have one body, in the owner's `<file>_inlines.h`: a file defines the `SDW_INLINE_<HELPER>` selectors it needs, includes the header and undefines them again.
- **Level data.** The `.WAR` and `.DAV` level files are loaded into memory whole; their records hold 32-bit offsets from the start of the file. Fields that hold such an offset are `SDW_WARPTR(T)` / `SDW_DAVPTR(T)`, which resolve it on use, and a slot the game fills with an object's address at run time is `SDW_OBJREF(T)` (`sdw_fileptr.h`).
- **Rendering.** The game's Direct3D 7 call sites are kept; `SDW_RD(dev)` and the other hooks in `sdw_render.h` route them to `g_renderDevice`, the SDL3 backend.
- **Units.** Time is in 1/4096 s, angles in 4096ths of a turn, positions in game units with the vertical axis pointing down ([docs/00-conventions.md](../docs/00-conventions.md)).
