/*
 * The .WAR loader, first part: Load_WAR and the empty per-resource hook it calls, then Load_FreeWAR.
 *
 * Load_WAR reads the whole .WAR into one malloc'd blob (file bytes 4.. to blob+4, so blob offsets are file offsets; the
 * CRC slot blob[0..3] is never filled), checks the "V2.6" version (only a Debug_Printf on mismatch), and relocates every
 * resource in place: the table at blob+0x10 holds one u32 per resource, type << 24 | byte offset, with bit 0x40 of the
 * type marking a resource that is not counted. Mesh records (types 3, 0xb, 0x26) and type-4 records have their pointer
 * words turned from blob offsets into addresses. Returns the number of counted resources (types 3/4/5/0xb/0x26
 * without bit 0x40), or -1: file missing, read or CRC error, or a type-1/2/unknown resource. Every failure path closes
 * the file again and frees the WAR - including the ones taken after the file was already closed.
 * */
#include "sdw_enums.h"
#include "sdw_classes.h"

#include "../sdk/crt.h"

/* ---- the game's own helpers ---- */
#include "file.h"
#include "text.h"
#include "maths.h"
#include "game_state.h"
#include "id_list.h"
#include "../app/app_main.h"
#include "../objects/instance.h"
#include "asset_validation.h"
void Debug_Printf(const char *fmt, ...); /* a no-op stub */
void Load_FreeWAR(WarFile *war);

/* ---- globals ---- */

#define WAR_RES_TYPE(e) ((e) >> 24 & 0xff)
#define WAR_RES_OFFSET(e) ((e) & 0xffffff)

/* called with every relocated mesh-type record; compiled empty. */
void Load_WAR_ResourceHook_Stub(void *res) {}

/* loads and relocates the level's .WAR into war (= g_pDav->war). The counted-resource total, or -1. */
int Load_WAR(const char *path, WarFile *war)
{
    u32 *p;
    s32 result;
    u8 color;
    char version[8];
    u32 i;
    u16 k;
    u32 len;
    u16 counter;
    u32 maxSize;
    FileHandle file = {-1, 0, 0};

    result = 0;
    counter = 0;
    war->table = 0;
    war->blob = 0;
    Game_SetWarLevelHeader(0); /* the three globals are not adjacent (game_state.cpp) */
    g_animNameTable = 0;
    if (File_Open(path, &file) <= 0) {
        Debug_Printf("Load_WAR: WAR File not found\n");
        goto fail;
    }
    maxSize = 0xcaa30;
    if (file.size < 16 || file.size > 64 * 1024 * 1024)
        goto fail;
    war->blob = (u8 *)malloc(file.size);
    if (!war->blob)
        goto fail;
    if (File_ReadChecked(&file, war->blob + 4, file.size - 4) < 0) {
        Debug_Printf("Load_WAR: Read Error\n");
        goto fail;
    }
    File_Close(&file);
    if (!AssetValidation::War(war->blob, file.size))
        goto fail;
    war->header = (WarHeader *)war->blob;
    war->table = (u32 *)(war->blob + 0x10);
    /* The game leaves the records' offsets in place and resolves them from here (sdw_fileptr.h) */
    g_warFileBase = war->blob;
    color = war->header->clearR;
    war->header->clearR = 0; /* NUL-terminates the 4-byte version string at +4 for the compare */
    Text_Sprintf(version, "V%u.%u", 2, 6);
    if (!Str_IsPrefixOf(version, war->header->version))
        Debug_Printf(" Bad version: Viewer:%s, Dav&War:%s\n", version, war->header->version);
    war->header->clearR = color;
    for (i = 0; i < war->header->resourceCount; i++) {
        u32 *meshRec;
        u32 *mesh4;
        u8 *scnRec;

        switch (WAR_RES_TYPE(war->table[i]) & WAR_RES_TYPE_MASK) {
            case WAR_RES_IGNORED_1:
            case WAR_RES_IGNORED_2:
                goto fail;
            case WAR_RES_MESH:
            case WAR_RES_MESH_B:
            case WAR_RES_SKY:
                /* a mesh: words 0, 1 and (when set) 3 are blob offsets */
                meshRec = (u32 *)(WAR_RES_OFFSET(war->table[i]) + war->blob);
                Load_WAR_ResourceHook_Stub(meshRec);
                if (!(WAR_RES_TYPE(war->table[i]) & WAR_RES_UNCOUNTED))
                    counter++;
                break;
            case WAR_RES_MODEL:
                /* words 0, 1, 4, 5 and (when set) 3 are blob offsets; word 4 then points at two counted lists of offsets
             * {u32 n; u32 off[n]; u32 m; u32 off[m]}, 0 = none */
                mesh4 = (u32 *)(WAR_RES_OFFSET(war->table[i]) + war->blob);
                Load_WAR_ResourceHook_Stub(mesh4);
                if (!(WAR_RES_TYPE(war->table[i]) & WAR_RES_UNCOUNTED))
                    counter++;
                break;
            case WAR_RES_SCENARIC:
                /* a scenaric object record: relocated later, by its class's loader; counted even with bit 0x40 */
                scnRec = WAR_RES_OFFSET(war->table[i]) + war->blob;
                counter++;
                break;
            case WAR_RES_TYPE_8:
            case WAR_RES_TYPE_9:
            case WAR_RES_CINEMATIC:
            case WAR_RES_TYPE_27:
            case WAR_RES_COLL_GRID:
            case WAR_RES_COLL_TRIS:
            case WAR_RES_EXPORTS:
            case WAR_RES_HEADER3:
            case WAR_RES_OBJ_GRID:
            case WAR_RES_PAIRS:
            case WAR_RES_ANIM_NAMES:
                break;
            default:
                goto fail;
        }
    }
    return counter;
fail:
    File_Close(&file);
    Load_FreeWAR(war);
    return -1;
}

/* ----: Load_FreeWAR ---- */

void Load_FreeWAR(WarFile *war)
{
    s32 index;
    for (index = 0; index < g_worldObjCount; ++index)
        WorldObj_Free(g_worldObjs[index]);
    if (war->blob && war->blob == g_warFileBase) { /* the tables' pointer-sized copies go with the WAR (list.cpp) */
        if (g_warExportTable)
            free(g_warExportTable);
        if (g_warRelocTable)
            free(g_warRelocTable);
        g_warExportTable = 0;
        g_warExportCount = 0;
        g_warRelocTable = 0;
        g_warRelocCount = 0;
        g_warFileBase = 0;
    }
    if (war->blob) {
        free(war->blob);
        war->blob = 0;
    }
    g_animNameTable = 0;
}
