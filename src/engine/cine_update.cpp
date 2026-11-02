/*
 * The stride table is defined here (a header static) and the pad words the skip test reads (g_padMasks,
 * g_padCurButtons, g_padPrevButtons) are macros for the element / fields they are (see there). Opcodes
 * (CineRecord.opcode): 1 position key, 2 rotation key, 3 animation, 4/5 visibility flicker (two phases), 6 camera focal
 * scale, 7/8 attach/detach (two phases). A key is a u16 time word (low 14 bits: the key's time in 16 ms units; top 2
 * bits: 1 step / 2 interpolate) followed by its payload.
 */

#define SDW_MEMBERS_Cine Cine(); /* Cine_Construct */
#define SDW_MEMBERS_ScnObject \
    void AttachTo(ScnObject *parent, u8 joint, Vec3s *offset, Vec3s *rot, u32 arg, uptr arg2);

#include "sdw_types.h"
#include "sdw_enums.h"
class Instance;
struct Animator;
u32 Anim_Start(Instance *inst, Animator *animator, u16 id, u32 opts);
#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "../objects/instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
#include "sdw_global_views.h"

/* this object's copy of the cinematic header's opcode-stride table, indexed by Cine_Update and the key-cursor helpers */
static u8 g_cineOpStride2[9] = {0, 8, 8, 4, 2, 2, 4, 2, 2};
#include "../app/app_main.h"
#include "progress.h"
#include "draw2d.h"
#include "time.h"
#include "sfx_volume.h"
#include "cine.h"
#include "scn_tools.h"
#include "interface.h"
#include "input.h"
#include "fade.h"
#include "../objects/camera.h"
#include "prompt.h"
extern Wolf *g_pWolf;
/* The pad words below are not objects of their own: g_padMasks is &g_inputMap[4] and the two button words are fields of
 * g_pad, all defined by the Input object. Spelled as the element / fields they are, they compile to the same addresses
 * and leave no undefined symbol for the link. */
#define g_padMasks (g_inputMap + 4)           /* active-low button masks; [10] = action */
#define g_padCurButtons (g_pad.cur.buttons)
#define g_padPrevButtons (g_pad.prev.buttons)
extern void *g_dialogueCurText;

extern TextScroll g_textScroll;
#define g_scrollTextFlags \
    (g_textScroll.flags) /* ScrollText flags: 1 paging started, 2 finished, 0x10 page turned */
extern u32 g_gameTime;

void Dialogue_SetBoxActive(s32 active);
void Camera_StartScripted(ScnObject *owner, Camera *cam, u16 rotX, u16 rotY, u16 rotZ, Vec3s *pos, u16 focal, u32 mode,
                          s32 time);

/* one frame of the cinematic (called while it is active). Starts the voice stream once; in dialogue mode only keeps the
 * speaker's talk/idle animation and the animation opcodes going until the text is closed. Otherwise handles the skip
 * (action or Esc; with flags & 0x1000 a skip starts the level-exit fade instead of stopping), the start of dialogue mode,
 * then steps every record whose keys are not used up and applies it, drives the scripted camera, advances the clock and
 * stops when no record is live any more. The opcode bodies of cases 2, 3, 6 and 7/8 are written out here as in the
 * original, which has them inline while calling Cine_OpStartAnim, Cine_KeyAtLast etc. out of line elsewhere in the same
 * function; case 7/8 evaluates the attach/detach record like a camera focal-scale key (a copy of case 6). */
void Cine::Update()
{
    s32 any = 0;
    s32 live;
    s32 shown;
    CineRecord *rec;
    Vec3s rot;
    u16 scale;
    u16 scale2;

    if (!(runFlags & CINE_RUN_VOICE_STARTED) && *resource != 0)
        Voice_PlayStream(*resource, 0);
    runFlags |= CINE_RUN_VOICE_STARTED;
    switch (dialogueMode) {
        case 1:
            if (g_dialogueCurText != 0) {
                Dialogue_Show(g_dialogueCurText, 1);
                if ((g_dialogueShownFlags.all & DLGSHOWN_SHOWN) && (g_scrollTextFlags & SCROLLTEXT_F_CLOSED)) {
                    CaptureCamera();
                    Stop();
                    return;
                }
            }
            if (g_dialogueCurText == 0) {
                CaptureCamera();
                Stop();
                return;
            }
            {
                ScnBody *spk = (ScnBody *)dialogue.speaker;
                if (spk != 0) {
                    u16 anim;
                    if ((g_dialogueShownFlags.all & DLGSHOWN_SHOWN) && (g_scrollTextFlags & SCROLLTEXT_F_PAGETURNED))
                        talkStartTime = g_gameTime;
                    if (g_gameTime - talkStartTime < 0x2000)
                        anim = dialogue.talkAnim;
                    else
                        anim = dialogue.idleAnim;
                    if ((spk->anim.flags & ANIM_F_FINISHED) || spk->anim.animId != anim)
                        spk->PlayAnim(anim, 1, 1);
                }
            }
            ResetIterators();
            do {
                rec = scriptCursor;
                if (rec->count != 0) {
                    keyNext = keyCur = rec->keyCursor;
                    if (rec->opcode == CINE_OP_PLAY_ANIM && !(targetDesc->obj->flags & SCN_OF_CINE_UPDATE))
                        ((ScnBody *)targetDesc->obj)->AdvanceAnim();
                }
            } while (AdvanceScriptCursor());
            return;
    }
    if (dialogueText != 0)
        Dialogue_Show(dialogueText, 1);
    shown = g_dialogueShownFlags.all & DLGSHOWN_SHOWN;
    if (!(flags & CINE_NO_INPUT)) {
        if (dialogueText != 0 && !(g_pProgress->optionFlags & OPT_GATE) &&
            ((!(~g_padMasks[10] & g_padCurButtons) && (~g_padMasks[10] & g_padPrevButtons)) ||
             g_inputMgr.escPressed == 1)) {
            CaptureCamera();
            Stop();
            return;
        }
        if (dialogueText != 0 && shown && (g_scrollTextFlags & SCROLLTEXT_F_OPEN)) {
            if (dialogue.speaker == 0) {
                ResetIterators();
                do {
                    if (targetDesc->selector == CINE_TRACK_OBJECT_SPEAKER) {
                        u32 r = targetDesc->obj->HandleMessage(0, MSG_QUERY_TALK_ANIMS, 0);
                        if (r != 0) {
                            dialogue.speaker = targetDesc->obj;
                            dialogue.idleAnim = (u16)r;
                            dialogue.talkAnim = (u16)(r >> 16);
                            break;
                        }
                    }
                } while (SkipRecords());
            }
            RunOpcodes();
            if (dialogue.speaker != 0)
                ((ScnBody *)dialogue.speaker)->PlayAnim(dialogue.talkAnim, 1, 0);
            if (dialogue.camSetup != 0)
                Camera_StartScripted(0, &g_camera, dialogue.camSetup->rot[0], dialogue.camSetup->rot[1],
                                     dialogue.camSetup->rot[2], &dialogue.camSetup->eye, dialogue.camSetup->focal, 0,
                                     0x1000);
            dialogueMode = 1;
            talkStartTime = g_gameTime;
            return;
        }
        if (dialogueText != 0 && shown && (g_scrollTextFlags & SCROLLTEXT_F_CLOSED)) {
            CaptureCamera();
            Stop();
            return;
        }
        if (dialogueText == 0 && !(flags & CINE_NO_WOLF_FREEZE) &&
            ((!(~g_padMasks[10] & g_padCurButtons) && (~g_padMasks[10] & g_padPrevButtons)) ||
             g_inputMgr.escPressed == 1)) {
            if (flags & CINE_LEVEL_EXIT)
                Fade_StartLevelExit(0x1000);
            else {
                CaptureCamera();
                Stop();
                return;
            }
        }
    }
    ResetIterators();
    do {
        rec = scriptCursor;
        if (rec->count != 0) {
            if ((bool)(rec->opcode == CINE_OP_FLICKER_PHASE_A || rec->opcode == CINE_OP_FLICKER_PHASE_B)) {
                if (rec->keyCursor > (u16 *)((u8 *)rec + g_cineOpStride2[rec->opcode] * (rec->count - 1) + 8)) {
                    live = 0;
                } else {
                    LoadStepKeyPair();
                    if (time >= (*keyCur & 0x3fff) << 4) {
                        OpFlickerVisibility();
                        scriptCursor->keyCursor = keyNext;
                        if (!KeyPastLast())
                            LoadStepKeyPair();
                    }
                    live = 1;
                }
            } else if ((bool)(rec->opcode == CINE_OP_ATTACH_A || rec->opcode == CINE_OP_ATTACH_B)) {
                if (rec->keyCursor > (u16 *)((u8 *)rec + g_cineOpStride2[rec->opcode] * (rec->count - 1) + 8)) {
                    live = 0;
                } else {
                    LoadStepKeyPair();
                    if (time >= (*keyCur & 0x3fff) << 4) {
                        OpToggleActorEntry();
                        scriptCursor->keyCursor = keyNext;
                        if (!KeyPastLast())
                            LoadStepKeyPair();
                    }
                    live = 1;
                }
            } else {
                if (rec->keyCursor == (u16 *)((u8 *)rec + g_cineOpStride2[rec->opcode] * (rec->count - 1) + 8)) {
                    live = 0;
                } else {
                    LoadKeyPair();
                    if (time > (*keyNext & 0x3fff) << 4) {
                        scriptCursor->keyCursor = keyNext;
                        if (scriptCursor->opcode == CINE_OP_PLAY_ANIM)
                            OpStartAnim();
                        if (!KeyAtLast())
                            LoadKeyPair();
                    }
                    live = 1;
                }
            }
            any |= live;
            if (live) {
                switch (scriptCursor->opcode) {
                    case CINE_OP_KEY_POSITION:
                        OpKeyPosition();
                        break;
                    case CINE_OP_KEY_ROTATION:
                        KeyframeRot(&rot, keyCur + 1, keyNext + 1, (*keyCur & 0x3fff) << 4, (*keyNext & 0x3fff) << 4,
                                    *keyCur >> 14);
                        switch (targetDesc->selector) {
                            case CINE_TRACK_OBJECT:
                            case CINE_TRACK_OBJECT_SPEAKER:
                            case CINE_TRACK_STATIC:
                            case CINE_TRACK_STATIC_B:
                                if (!(targetDesc->obj->inst_flags & INST_F_ATTACHED))
                                    targetDesc->obj->rot = rot;
                                break;
                            case CINE_TRACK_CAMERA:
                                SetCamRot(&rot);
                                break;
                        }
                        break;
                    case CINE_OP_PLAY_ANIM:
                        if (time == 0)
                            ((ScnBody *)targetDesc->obj)->PlayAnim(scriptCursor->keyCursor[1], 1, 0);
                        if (!(targetDesc->obj->flags & SCN_OF_CINE_UPDATE))
                            ((ScnBody *)targetDesc->obj)->AdvanceAnim();
                        break;
                    case CINE_OP_KEY_CAMSCALAR:
                        KeyframeScalar(&scale, keyCur[1], keyNext[1], (*keyCur & 0x3fff) << 4, (*keyNext & 0x3fff) << 4,
                                       *keyCur >> 14);
                        if (targetDesc->selector == CINE_TRACK_CAMERA)
                            SetCamScalar(scale);
                        break;
                    case CINE_OP_ATTACH_A:
                    case CINE_OP_ATTACH_B:
                        KeyframeScalar(&scale2, keyCur[1], keyNext[1], (*keyCur & 0x3fff) << 4,
                                       (*keyNext & 0x3fff) << 4, *keyCur >> 14);
                        if (targetDesc->selector == CINE_TRACK_CAMERA)
                            SetCamScalar(scale2);
                        break;
                }
            } else if (scriptCursor->opcode == CINE_OP_PLAY_ANIM) {
                if (!(targetDesc->obj->flags & SCN_OF_CINE_UPDATE))
                    ((ScnBody *)targetDesc->obj)->AdvanceAnim();
            }
        }
    } while (AdvanceScriptCursor());
    if (hasCameraTrack && cameraHeld)
        Camera_StartScripted(0, &g_camera, camRot.x, camRot.y, camRot.z, &camPos, camFocalScale, (~flags >> 1) & 3,
                             0x1000);
    time = (g_rawTime * 1000 - startRawTime * 1000) >> 12;
    if ((flags & CINE_LEVEL_EXIT) && time >= endTriggerMs)
        Fade_StartLevelExit(0x1000);
    if (!any)
        Stop();
}

/* opcode 1: the position key, plus worldOffset with flags & 0x10; to the camera or through SetPosition */
void Cine::OpKeyPosition()
{
    Vec3s pos;
    KeyframePos(&pos, (Vec3s *)(keyCur + 1), (Vec3s *)(keyNext + 1), (*keyCur & 0x3fff) << 4, (*keyNext & 0x3fff) << 4,
                *keyCur >> 14);
    switch (targetDesc->selector) {
        case CINE_TRACK_OBJECT:
        case CINE_TRACK_OBJECT_SPEAKER:
        case CINE_TRACK_STATIC:
        case CINE_TRACK_STATIC_B:
            if (!(targetDesc->obj->inst_flags & INST_F_ATTACHED)) {
                if (flags & CINE_RELATIVE_TO_WOLF) {
                    pos.x += worldOffset.x;
                    pos.y += worldOffset.y;
                    pos.z += worldOffset.z;
                }
                targetDesc->obj->SetPosition(&pos);
            }
            break;
        case CINE_TRACK_CAMERA:
            if (flags & CINE_RELATIVE_TO_WOLF) {
                pos.x += worldOffset.x;
                pos.y += worldOffset.y;
                pos.z += worldOffset.z;
            }
            SetCamPos(&pos);
            break;
    }
}

/* opcode 2: the rotation key, written straight into the object (not when attached) or the camera */
void Cine::OpKeyRotation()
{
    Vec3s rot;
    KeyframeRot(&rot, keyCur + 1, keyNext + 1, (*keyCur & 0x3fff) << 4, (*keyNext & 0x3fff) << 4, *keyCur >> 14);
    switch (targetDesc->selector) {
        case CINE_TRACK_OBJECT:
        case CINE_TRACK_OBJECT_SPEAKER:
        case CINE_TRACK_STATIC:
        case CINE_TRACK_STATIC_B:
            if (!(targetDesc->obj->inst_flags & INST_F_ATTACHED))
                targetDesc->obj->rot = rot;
            break;
        case CINE_TRACK_CAMERA:
            SetCamRot(&rot);
            break;
    }
}

/* opcode 3, on reaching a key: starts the key's animation id (once) */
void Cine::OpStartAnim()
{
    ((ScnBody *)targetDesc->obj)->PlayAnim(scriptCursor->keyCursor[1], 1, 0);
}

/* opcode 3, every frame: steps the animation unless the object is frozen (flags & 0x20) */
void Cine::OpAdvanceAnim()
{
    if (!(targetDesc->obj->flags & SCN_OF_CINE_UPDATE))
        ((ScnBody *)targetDesc->obj)->AdvanceAnim();
}

/* opcodes 4/5: visibility from the parity of the key index (inverted for 5), so successive keys alternate */
void Cine::OpFlickerVisibility()
{
    s32 on = ((u8 *)scriptCursor->keyCursor - (u8 *)scriptCursor) >> 1 & 1;
    ScnObject *obj;
    if (scriptCursor->opcode == CINE_OP_FLICKER_PHASE_B)
        on = on == 0;
    obj = targetDesc->obj;
    if (on)
        obj->flags &= ~SCN_OF_HIDDEN;
    else
        obj->flags |= SCN_OF_HIDDEN;
}

/* opcode 6: the camera's focal scale key (camera tracks only) */
void Cine::OpKeyCamScalar()
{
    u16 v;
    KeyframeScalar(&v, keyCur[1], keyNext[1], (*keyCur & 0x3fff) << 4, (*keyNext & 0x3fff) << 4, *keyCur >> 14);
    if (targetDesc->selector == CINE_TRACK_CAMERA)
        SetCamScalar(v);
}

/* opcodes 7/8: attach (or detach, by the key parity as for 4/5) every attachTable entry whose parent is
 * the track's object. attachCount is never made positive in this build, so this does nothing. */
void Cine::OpToggleActorEntry()
{
    s32 on = ((u8 *)scriptCursor->keyCursor - (u8 *)scriptCursor) >> 1 & 1;
    s32 i;
    if (scriptCursor->opcode == CINE_OP_ATTACH_B)
        on = on == 0;
    if (on) {
        for (i = 0; i < attachCount; i++)
            if (targetDesc->obj == attachTable[i].parent)
                AttachEntry(i);
    } else {
        for (i = 0; i < attachCount; i++)
            if (targetDesc->obj == attachTable[i].parent)
                DetachEntry(i);
    }
}

/* 1 if the record's key cursor is on its last key */
s32 Cine::KeyAtLast()
{
    CineRecord *r = scriptCursor;
    return r->keyCursor == (u16 *)((u8 *)r + g_cineOpStride2[r->opcode] * (r->count - 1) + 8);
}

/* 1 if the key cursor has moved past the last key */
s32 Cine::KeyPastLast()
{
    CineRecord *r = scriptCursor;
    return (u16 *)((u8 *)r + g_cineOpStride2[r->opcode] * (r->count - 1) + 8) < r->keyCursor;
}

/* keyCur = the cursor's key, keyNext = the one after (itself at the last key) */
void Cine::LoadKeyPair()
{
    CineRecord *r = scriptCursor;
    keyCur = r->keyCursor;
    if (r->keyCursor == (u16 *)((u8 *)r + g_cineOpStride2[r->opcode] * (r->count - 1) + 8))
        keyNext = keyCur;
    else
        keyNext = (u16 *)((u8 *)r->keyCursor + g_cineOpStride2[r->opcode]);
}

/* the same for the stride-2 step opcodes, except that at the last key keyNext is one key past the end,
 * so the cursor can leave the record and KeyPastLast retires it */
void Cine::LoadStepKeyPair()
{
    CineRecord *r = scriptCursor;
    keyCur = r->keyCursor;
    if (r->keyCursor == (u16 *)((u8 *)r + g_cineOpStride2[r->opcode] * (r->count - 1) + 8))
        keyNext = keyCur + 1;
    else
        keyNext = (u16 *)((u8 *)r->keyCursor + g_cineOpStride2[r->opcode]);
}
