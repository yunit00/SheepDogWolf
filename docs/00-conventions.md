# Conventions and key facts

The facts every other note assumes.

## The game

The port targets the PAL PC release of *Sheep, Dog 'n' Wolf*: its data (levels, sounds, music, texts) is read from your copy of the disc at run time. The original program was built with Microsoft Visual C++ 6.0 for 32-bit Windows and DirectX 7/8; its code is C++, a port of the PlayStation code base.

## Names

- Canonical names come from the game disc's headers (`Levels/Lvl-03/Scenaric_Classes.h`, `GameRes.h`) and from strings in the original program; they are preferred over invented ones. Functions are named `Module_Function` in the style of the original (`Flock_…`, `Load_…`, `SAM_STT_…`); C++ methods drop the class prefix (`ScnControllable::SteerRun`).
- Claims in the notes are marked *verified* (read in the machine code or the data bytes) or *inferred*.

## Levels: two numberings

Players number the levels 0-14 plus two bonus levels, B1 and B2. On the disc, B1 sits after level 4 and B2 after level 8, which shifts everything after them:

| Player numbering | Disc folder |
|---|---|
| Level 0-4 | `Lvl-00` .. `Lvl-04` |
| **B1** | **`Lvl-05`** |
| Level 5-8 | `Lvl-06` .. `Lvl-09` |
| **B2** | **`Lvl-10`** |
| Level 9-14 | `Lvl-11` .. `Lvl-16` |

`Lvl-17` is Planet X, which has no player-facing number. The notes say which numbering a level number is in. `Levels/` also holds `Scene`, `Wheel`, `Intro`, `Fend`, `Ending` and `Demos` ([docs/01-executable-and-files.md](01-executable-and-files.md)).

**Axes.** Positions are three signed 16-bit values. The notes call them **x / y / z**: the vertical axis points **down** (more negative is higher).

**Units.** Time is in 1/4096 s ([the frame loop](03-time-and-frame-loop.md)); angles are in 4096ths of a turn; many fractions are 4.12 fixed point (0x1000 = 1.0).

## WAR level files

The resource table (`g_pDav->war.table`) is `u32[]` where the **top byte is the type and the low 24 bits a byte offset** into the level's blob (`g_pDav->war.blob`); the `.MLT` string bank follows. `GetResourceType` classes them: 0 = types 3/4/0xB/0x26, 1 = type 5 (scenaric record), 2 = type 0xA (cinematic), 3 = types 8, 9, 0x27, 0x80-0x86 or any type with bit 0x40, 4 = ignored (types 1/2). `Install_WarResource` publishes the type-3 ones: **0x80** collision grid, **0x81** collision triangles, **0x82** export table (`{u32 count; entries[]}`, relocated), **0x83** a three-u32 header, **0x84** the object/cluster grid (`ObjGrid_Init`), **0x85** `{u32 count; pair[2][]}` with the second word of each pair relocated against the blob base, **0x86** the model animation table.

Types 3, 4, 0xB and 0x26 are geometry records (`tools/war_meshes.py` reads them). With bit 0x40 set, 0x43 and 0x44 are the objects' static and animated models; animated models are stored at 8 times the level's units. Still undecoded as content: types 8, 9, 0xA and 0x27, and what 0x83's three words and 0x85's pairs mean. The level tools (`tools/war_*.py`) read these files directly.

The records hold 32-bit file offsets. The port loads a file whole and leaves the offsets in place, resolving them on use (`src/include/sdw_fileptr.h`), because a 64-bit address does not fit where the original wrote its 32-bit ones.

## Where to go next

[docs/01-executable-and-files.md](01-executable-and-files.md) (the game's files), [docs/03-time-and-frame-loop.md](03-time-and-frame-loop.md), [docs/04-object-system.md](04-object-system.md), and [src/README.md](../src/README.md) for the source.
