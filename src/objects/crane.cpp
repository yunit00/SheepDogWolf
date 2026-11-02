/* ScnObject::Text_GetClassString is declared returning char *, as it is defined, so the decorated names agree at link. */
/*
 * Crane (class 82 "Crane", vtable, sizeof 0xc0) - Daffy's crane on the frozen river: it lifts whatever the FrozenRiver
 * delivers to the end of its course (the Wolf, a frozen Sheep, or an ice floe) out of the water and drops it on the
 * bank.
 *
 * The function before PostLoadInit (Cine_ResolveText) is not in this file: is the 16-byte aligned
 * start of the Crane's file, so is the tail of the CinematicsManager's (which starts after padding at
 *).
 *
 * The sequence: FrozenRiver_MoveCargo sends MSG_RIVER_CARGO_END with the cargo when it reaches the last point of the
 * river's path. It is accepted only in CRANE_ST_IDLE; the crane then picks the drop spot and the four animations for
 * the cargo's class and runs CRANE_ST_CATCH (lower the hook) .. CRANE_ST_RETURN, each advancing when its animation has
 * finished. CRANE_ST_CATCH_HOLD attaches the cargo to the hook (MSG_PICKUP, joint 0xd) and tells the river it is busy
 * (MSG_RIVER_CRANE_BUSY, 1); CRANE_ST_RELEASE drops it at the drop spot (MSG_DROP); the end of CRANE_ST_RETURN releases
 * the camera and the river (MSG_RIVER_CRANE_BUSY, 0). CRANE_ST_WAIT_CINE_BOX and CRANE_ST_CINEMATIC play the optional
 * intro cinematic (CINE, 0 in the retail Lvl-08).
 * */

#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    /* inline, defined below */          \
    void SetUpdateMode(s32 mode); /* inline, defined below */

#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
class Instance;
struct Animator;
u32 Anim_Start(Instance *inst, Animator *animator, u16 id, u32 opts); /* (used by the PlayAnim inline) */
#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
#define SDW_INLINE_SCNBODY_ANIMFLAGS_U16 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_ANIMFLAGS_U16

extern Wolf *g_pWolf;
#include "../app/app_main.h"
#include "../engine/cine.h"
#include "../engine/scn_tools.h"
#include "../engine/sound_mgr.h"
#include "camera.h"

/* the cinematic header's static copy of the 9-byte opcode stride table (src/engine/cine.cpp, g_cineOpStride). */
static u8 s_cineOpStride[9] = {0, 8, 8, 4, 2, 2, 4, 2, 2};

s32 Scenaric_FindByClass(u16 classId, ScnObject **out, s32 max);
u16 Sound_Play(u16 soundId, void *owner, u16 volume, u8 flags, s32 rate);
void Camera_StartScripted(ScnObject *owner, Camera *cam, u16 rotX, u16 rotY, u16 rotZ, Vec3s *pos, u16 focal, u32 mode,
                          s32 time);

/* ---- inline helpers ---- */

/* A u32 designer property: the record's property block starts at +0x14. */
#define SDW_INLINE_FREE_SCN_GETPROPU32_VOID_U32 1
#include "../engine/scn_tools_inlines.h"
#undef SDW_INLINE_FREE_SCN_GETPROPU32_VOID_U32

#define SDW_INLINE_SCNOBJECT_STOPSOUND_U16 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_STOPSOUND_U16

/* A CAMERA property {u16 focal; s16 rot[3]; Vec3s eye} as a scripted camera owned by this object. */
#define SDW_INLINE_SCNOBJECT_STARTCAMERABLENDED_U16_U16_U16_VEC3S_U16 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_STARTCAMERABLENDED_U16_U16_U16_VEC3S_U16

/* ScnUpdateMode: SCN_UPD_NORMAL near the camera only, SCN_UPD_ALWAYS, SCN_UPD_NEVER, SCN_UPD_CINE also during cinematics.
 * The masks are 16-bit constants, hence (u16)~. */
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32

#define SDW_INLINE_FREE_STARTCINE_U32_U32_BOX_BOX_VOID 1
#include "../engine/cine_inlines.h"
#undef SDW_INLINE_FREE_STARTCINE_U32_U32_BOX_BOX_VOID

/* The cinematic player's finished flag, through a local of the inline. */
#define SDW_INLINE_FREE_CINE_ISFINISHED 1
#include "../engine/cine_inlines.h"
#undef SDW_INLINE_FREE_CINE_ISFINISHED

/* Whether p lies inside box on x and z (the vertical is not tested). */
#define SDW_INLINE_FREE_BOX_CONTAINSPOINTXZ_BOX_VEC3S 1
#include "../engine/coll_box_inlines.h"
#undef SDW_INLINE_FREE_BOX_CONTAINSPOINTXZ_BOX_VEC3S

/* ---- Crane ---- */

/* vtable +0x00: reads the properties, finds the FrozenRiver, remembers its rotation, always updates, and
 * starts idle (CRANE_ST_IDLE) or, with an intro cinematic, waiting for the Wolf (CRANE_ST_WAIT_CINE_BOX). */
void Crane::PostLoadInit()
{
    u16 *props = record;
    cine = Scn_GetPropU32(props, 8);          /* PROPERTY_CRANE_CINE */
    activationBox = Scn_GetPropBox(props, 0); /* PROPERTY_CRANE_ACTIVATIONBOX */
    cineBox = Scn_GetPropBox(props, 12);      /* PROPERTY_CRANE_CINEBOX */
    cineSheepBox = Scn_GetPropBox(props, 20); /* PROPERTY_CRANE_CINESHEEPBOX */
    cineFlag = Scn_GetPropU32(props, 16);     /* PROPERTY_CRANE_CINEFLAG */
    cineText = Scn_GetPropU32(props, 24);     /* PROPERTY_CRANE_CINETEXT */
    cineTextStr = Text_GetClassString((u8)cineText);
    Scenaric_FindByClass(CLASSID_FROZENRIVER, &frozenRiver, 1);
    homeRot.x = rot.x;
    homeRot.y = rot.y;
    homeRot.z = rot.z;
    camera = Scn_GetPropCamera(props, 4); /* PROPERTY_CRANE_CAMERA */
    SetUpdateMode(SCN_UPD_ALWAYS);
    if (cine) {
        ready = 0;
        SetState(CRANE_ST_WAIT_CINE_BOX);
    } else {
        SetState(CRANE_ST_IDLE);
        ready = 1;
    }
    cargoAttached = 0;
    idleSound = 0;
}

/* hang the cargo on the hook (MSG_PICKUP, joint 0xd; the cargo's reply is kept) or drop it at the drop spot
 * (MSG_DROP). Returns 1 if that changed anything; nothing happens without a cargo or when it is already in that state. */
s32 Crane::SetCargoAttached(s32 attach)
{
    if (cargo && attach != cargoAttached) {
        if (attach)
            cargoAttached =
                cargo->HandleMessage(this, MSG_PICKUP, (void *)0xd);
        else {
            cargo->HandleMessage(this, MSG_DROP, &drop);
            cargoAttached = 0;
        }
        return 1;
    }
    return 0;
}

/* freeze (MSG_FREEZE) or release (MSG_UNFREEZE) the Wolf, keeping his reply. Nothing calls it. */
s32 Crane::SetWolfLock(s32 lock)
{
    if (lock != wolfLocked) {
        if (lock)
            wolfLocked = g_pWolf->HandleMessage(this, MSG_FREEZE, 0);
        else {
            g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
            wolfLocked = 0;
        }
        return 1;
    }
    return 0;
}

/* enter a state: its animation, and on entry to CRANE_ST_IDLE the rest pose and the idle sound, on
 * CRANE_ST_CATCH_HOLD the hook takes the cargo, on CRANE_ST_RELEASE it lets go. CRANE_ST_CINEMATIC keeps the animation
 * it has. */
void Crane::SetState(u8 newState)
{
    switch (newState) {
        case CRANE_ST_IDLE:
            PlayAnim(ADGRUE01_ANIM_STAND, 1, 1);
            cargo = 0;
            rot = homeRot;
            StopSound(idleSound);
            idleSound = Sound_Play(SND_CRANE_IDLE, this, 0xff, SNDF_LOOP | SNDF_POSITIONAL, 0x1000);
            break;
        case CRANE_ST_CATCH:
            PlayAnim(ADGRUE01_ANIM_CATCH, 0, 1);
            break;
        case CRANE_ST_CATCH_HOLD:
            PlayAnim(ADGRUE01_ANIM_CATCH1, 0, 1);
            frozenRiver->HandleMessage(this, MSG_RIVER_CRANE_BUSY,
                                       (void *)1);
            SetCargoAttached(1);
            break;
        case CRANE_ST_SWING1:
            PlayAnim(swingAnim1, 0, 1);
            break;
        case CRANE_ST_SWING2:
            PlayAnim(swingAnim2, 0, 1);
            break;
        case CRANE_ST_RELEASE:
            PlayAnim(releaseAnim, 0, 1);
            SetCargoAttached(0);
            break;
        case CRANE_ST_RETURN:
            PlayAnim(returnAnim, 0, 1);
            break;
        case CRANE_ST_WAIT_CINE_BOX:
            PlayAnim(ADGRUE01_ANIM_STAND, 1, 1);
            break;
    }
    state = newState;
}

/* vtable +0x04: CRANE_ST_CATCH .. CRANE_ST_RETURN advance when their animation has finished; the end of
 * CRANE_ST_RETURN frees the camera and the river. CRANE_ST_WAIT_CINE_BOX starts the intro cinematic when the Wolf
 * stands in cineBox (x and z only) - and goes to CRANE_ST_CINEMATIC whether he does or not; CRANE_ST_CINEMATIC waits for
 * the cinematic player's finished flag. */
void Crane::Update()
{
    switch (state) {
        case CRANE_ST_CATCH:
            if (AnimFlags(ANIM_F_FINISHED))
                SetState(CRANE_ST_CATCH_HOLD);
            break;
        case CRANE_ST_CATCH_HOLD:
            if (AnimFlags(ANIM_F_FINISHED))
                SetState(CRANE_ST_SWING1);
            break;
        case CRANE_ST_SWING1:
            if (AnimFlags(ANIM_F_FINISHED))
                SetState(CRANE_ST_SWING2);
            break;
        case CRANE_ST_SWING2:
            if (AnimFlags(ANIM_F_FINISHED))
                SetState(CRANE_ST_RELEASE);
            break;
        case CRANE_ST_RELEASE:
            if (AnimFlags(ANIM_F_FINISHED))
                SetState(CRANE_ST_RETURN);
            break;
        case CRANE_ST_RETURN:
            if (AnimFlags(ANIM_F_FINISHED)) {
                Camera_ReleaseAny();
                frozenRiver->HandleMessage(this, MSG_RIVER_CRANE_BUSY, 0);
                SetState(CRANE_ST_IDLE);
            }
            break;
        case CRANE_ST_WAIT_CINE_BOX:
            if (Box_ContainsPointXZ(cineBox, &g_pWolf->pos))
                StartCine(cine, cineFlag, cineBox, cineSheepBox, cineTextStr);
            SetState(CRANE_ST_CINEMATIC);
            break;
        case CRANE_ST_CINEMATIC:
            if (Cine_IsFinished()) {
                ready = 1;
                SetState(CRANE_ST_IDLE);
            }
            break;
    }
    AdvanceAnim();
}

/* vtable +0x10. MSG_RIVER_CARGO_END (arg = the cargo), only in CRANE_ST_IDLE: the drop spot is the crane's
 * position plus an offset, and the animations of CRANE_ST_SWING1 .. CRANE_ST_RETURN are chosen, per cargo class (the
 * Wolf also gets the CAMERA shot); then CRANE_ST_CATCH. MSG_FREEZE clears wolfLocked. */
sptr Crane::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    switch (msgId) {
        case MSG_RIVER_CARGO_END:
            if (state == CRANE_ST_IDLE) {
                cargo = (ScnObject *)arg;
                drop.pos = pos;
                switch (cargo->GetClassId()) {
                    case CLASSID_WOLF: /* the Wolf */
                        if (camera)
                            StartCameraBlended(camera->rot[0], camera->rot[1], camera->rot[2], &camera->eye,
                                               camera->focal);
                        swingAnim1 = ADGRUE01_ANIM_MOVE3;
                        swingAnim2 = ADGRUE01_ANIM_PUT3;
                        releaseAnim = ADGRUE01_ANIM_PUT3A;
                        returnAnim = ADGRUE01_ANIM_BACK3;
                        drop.pos.x += 0x8c;
                        drop.pos.z += 0x13c;
                        break;
                    case CLASSID_SHEEP: /* a Sheep */
                        swingAnim1 = ADGRUE01_ANIM_MOVE2;
                        swingAnim2 = ADGRUE01_ANIM_PUT2;
                        releaseAnim = ADGRUE01_ANIM_PUT2A;
                        returnAnim = ADGRUE01_ANIM_BACK2;
                        drop.pos.x += 0x13a;
                        drop.pos.z += 0x91;
                        break;
                    case CLASSID_SLIDINGICECUBE: /* a SlidingIceCube (floe) */
                        swingAnim1 = ADGRUE01_ANIM_MOVE;
                        swingAnim2 = ADGRUE01_ANIM_PUT;
                        releaseAnim = ADGRUE01_ANIM_PUT1;
                        returnAnim = ADGRUE01_ANIM_BACK;
                        drop.pos.x -= 0x117;
                        drop.pos.z += 0xce;
                        break;
                }
                drop.placed = 1;
                drop.flag1 = 0;
                SetState(CRANE_ST_CATCH);
                return 1;
            }
            return 0;
    }
    if (msgId == MSG_FREEZE)
        wolfLocked = 0;
    return 0;
}

/* vtable +0x14 (level restart): a cargo still on the hook is dropped at the drop spot, then idle, or back
 * to waiting for the intro cinematic if it has not played. */
void Crane::Reset()
{
    if (cargoAttached) {
        cargo->HandleMessage(this, MSG_DROP, &drop);
        cargoAttached = 0;
    }
    if (ready)
        SetState(CRANE_ST_IDLE);
    else
        SetState(CRANE_ST_WAIT_CINE_BOX);
}

/* the class factory for CLASSID 82 "Crane": new Crane (the base constructors inlined: the ScnObject,
 * ScnBody and Crane vtables in turn), then ScnBody_Init(record, 0) through the vtable. */
ScnObject *Crane_Create(void *record)
{
    ScnBody *obj = new Crane;
    obj = obj->Init(record, 0);
    return obj;
}
