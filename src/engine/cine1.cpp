/*
 * ResetIterators is last here (at its address), after ResolveTracks: ResolveTracks must be compiled without having seen
 * its body. The stride table is a header static: not const, internal linkage, one copy per including object. g_camPos
 * is g_camera.pos, not an object of its own: spelled as that field (a macro), same code.
 *
 * A script is a list of tracks, each an 8-byte CineTrack header followed by its records; a record is an 8-byte
 * CineRecord header followed by count * g_cineOpStride[opcode] bytes of keys, padded by 2 when a stride-2 opcode has an
 * odd count. The iterator (targetDesc, blocksRemaining, scriptCursor, scriptRemaining) walks records across all tracks.
 */
#define SDW_MEMBERS_Cine Cine(); /* Cine_Construct */
#define SDW_MEMBERS_ScnObject \
    void AttachTo(ScnObject *parent, u8 joint, Vec3s *offset, Vec3s *rot, u32 arg, uptr arg2);
#include "sdw_enums.h"
#include "scenaric_props.h"
#include "sdw_classes.h"

static u8 g_cineOpStride[9] = {0, 8, 8, 4, 2, 2, 4, 2, 2};
#include "../app/app_main.h"
#include "game_state.h"
#include "scenaric.h"
#include "id_list.h"
#include "scn_tools.h"
#include "../objects/camera.h"
#include "../objects/animation.h"
#define g_camPos (g_camera.pos) /* = g_camera + 0xd0: a field, not an object of its own */
extern u8 g_sharedScratch[];    /* shared scratch buffer */
extern Wolf *g_pWolf;

void Camera_StartScripted(ScnObject *owner, Camera *cam, u16 rotX, u16 rotY, u16 rotZ, Vec3s *pos, u16 focal, u32 mode,
                          s32 time);
void Dialogue_SetBoxActive(s32 active);

/* Cine_Construct, run on g_cinePlayer by the static initializer; the default flags pick the
 * level's CINSHEEPBBOX/CINBBOX exports when Cine_Start is given no boxes */
Cine::Cine()
{
    tracks = 0;
    trackCount = 0;
    time = 0;
    startRawTime = 0;
    active = 0;
    finished = 0;
    flags = CINE_AUTO_SHEEP_BBOX | CINE_AUTO_CIN_BBOX;
    cineId = 0;
    cineBox = 0;
    sheepBox = 0;
    attachCount = 0;
}

/* the record after the current one: header + count keys of the opcode's stride */
CineRecord *Cine::RecordSize()
{
    return (CineRecord *)((u8 *)scriptCursor + g_cineOpStride[scriptCursor->opcode] * scriptCursor->count + 8);
}

/* called from Scenaric_RenderAll: draws the selector-3/4 (cinematic-only) objects, with a freshly computed
 * camera distance, and steps over every other track. */
void Cine::RenderTaggedObjects()
{
    s32 done = 0;
    s32 i, n;
    ResetIterators();
    do {
        if (targetDesc->selector != CINE_TRACK_STATIC && targetDesc->selector != CINE_TRACK_STATIC_B) {
            n = targetDesc->recordCount;
            for (i = 0; i < n; i++)
                if (!AdvanceScriptCursor())
                    done = 1;
        } else {
            /* The record loop is written in both branches because the original has two copies of it. */
            s32 *d = (s32 *)g_sharedScratch;
            d[0] = targetDesc->obj->pos.x - g_camPos.x;
            d[1] = targetDesc->obj->pos.y - g_camPos.y;
            d[2] = targetDesc->obj->pos.z - g_camPos.z;
            d[0] *= d[0];
            d[1] *= d[1];
            d[2] *= d[2];
            targetDesc->obj->camDist2 = d[0] + d[1] + d[2];
            if (!(targetDesc->obj->flags & (SCN_OF_HIDDEN2 | SCN_OF_HIDDEN)))
                targetDesc->obj->Render(&g_camera);
            n = targetDesc->recordCount;
            for (i = 0; i < n; i++)
                if (!AdvanceScriptCursor())
                    done = 1;
        }
    } while (!done);
}

/* 1 if a track aims at obj */
s32 Cine::ScriptTargetsObject(ScnObject *obj)
{
    ResetIterators();
    do {
        if ((targetDesc->selector == CINE_TRACK_OBJECT || targetDesc->selector == CINE_TRACK_OBJECT_SPEAKER ||
             targetDesc->selector == CINE_TRACK_STATIC || targetDesc->selector == CINE_TRACK_STATIC_B) &&
            obj == targetDesc->obj)
            return 1;
    } while (SkipRecords());
    return 0;
}

/* the first position key of obj's track (record + 10: past the key's packed time word) */
Vec3s *Cine::FindFirstPosKeyFor(ScnObject *obj)
{
    SaveIterator();
    ResetIterators();
    do {
        if (targetDesc->obj == obj && scriptCursor->opcode == CINE_OP_KEY_POSITION)
            return (Vec3s *)((u8 *)scriptCursor + 10);
    } while (AdvanceScriptCursor());
    RestoreIterator();
    return 0;
}

/* the same for the current track's object */
Vec3s *Cine::FindFirstPosKey()
{
    ScnObject *obj = targetDesc->obj;
    SaveIterator();
    ResetIterators();
    do {
        if (targetDesc->obj == obj && scriptCursor->opcode == CINE_OP_KEY_POSITION)
            return (Vec3s *)((u8 *)scriptCursor + 10);
    } while (AdvanceScriptCursor());
    RestoreIterator();
    return 0;
}

/* the first rotation key of the current track's object */
Vec3s *Cine::FindFirstRotKey()
{
    ScnObject *obj = targetDesc->obj;
    SaveIterator();
    ResetIterators();
    do {
        if (targetDesc->obj == obj && scriptCursor->opcode == CINE_OP_KEY_ROTATION)
            return (Vec3s *)((u8 *)scriptCursor + 10);
    } while (AdvanceScriptCursor());
    RestoreIterator();
    return 0;
}

/* the level's first WAR_IDO_CINBBOX (8) export */
Box *Cine::FindCinBBox()
{
    u16 n;
    void **list = (void **)Scn_FindIdList(WAR_IDO_CINBBOX, &n);
    if (n == 0) {
        cineBox = 0;
        return 0;
    }
    return (Box *)*list;
}

/* the level's first WAR_IDO_CINSHEEPBBOX (9) export */
Box *Cine::FindCinSheepBBox()
{
    u16 n;
    void **list = (void **)Scn_FindIdList(WAR_IDO_CINSHEEPBBOX, &n);
    if (n == 0) {
        sheepBox = 0;
        return 0;
    }
    return (Box *)*list;
}

/* selector-0 target of opcode 1 */
void Cine::SetCamPos(Vec3s *p)
{
    camPos.x = p->x;
    camPos.y = p->y;
    camPos.z = p->z;
}

/* selector-0 target of opcode 2 */
void Cine::SetCamRot(Vec3s *r)
{
    camRot.x = r->x;
    camRot.y = r->y;
    camRot.z = r->z;
}

/* selector-0 target of opcode 6 */
void Cine::SetCamScalar(u16 v)
{
    camFocalScale = v;
}

/* hands the live camera to a scripted camera so the cinematic starts from where gameplay left it */
void Cine::CaptureCamera()
{
    if (cameraHeld)
        Camera_StartScripted(0, &g_camera, g_camera.rot.x, g_camera.rot.y, g_camera.rot.z, &g_camPos, g_camera.dist, 0,
                             0x1000);
    Camera_ReleaseAny();
    cameraCaptured = 1;
}

/* rewinds the iterator to the first record of the first track. Last in this object: ResolveTracks must not have seen its
 * body (see the header). */
void Cine::ResetIterators()
{
    targetDesc = tracks;
    blocksRemaining = trackCount;
    scriptCursor = (CineRecord *)(tracks + 1);
    scriptRemaining = tracks->recordCount;
}
