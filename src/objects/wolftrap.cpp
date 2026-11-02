/*
 * WolfTrap (class 81, CLASSID 81 "WolfTrap", vtable, sizeof 0xa0) - the snap trap that catches Ralph (or the
 * Robot, class 0x65). Idle it answers MSG_QUERY_ACTION with 0x19; MSG_USE (1) from the victim snaps it shut: the
 * victim is frozen, snapped to the trap, and Sam and the Bell are both sent msg 0x33 (the alarm). Six PAD_CROSS
 * presses inside one 0x400 ms window open it again (state 2, 5.12 s), after which it re-arms at its home position.
 * */

#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetRotation(Vec3s *r);


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
#define SDW_INLINE_SCNOBJECT_SETROTATION_VEC3S 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_SETROTATION_VEC3S
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
#define SDW_INLINE_SHADOW_SETVISIBLE_S32 1
#include "../engine/shadow_inlines.h"
#undef SDW_INLINE_SHADOW_SETVISIBLE_S32

s32 Scenaric_FindByClass(u16 classId, ScnObject **out, s32 max);

#include "../engine/input.h"
#define g_padCurButtons (g_pad.cur.buttons)

#define g_padPrevButtons (g_pad.prev.buttons)

extern s32 g_dtMs;

/* vtable +0x00: find Sam and the Bell, remember the home position and rotation, arm. */
void WolfTrap::PostLoadInit()
{
    pSam = 0;
    Scenaric_FindByClass(CLASSID_SAM, &pSam, 1);
    pBell = 0;
    Scenaric_FindByClass(CLASSID_BELL, &pBell, 1);
    pVictim = 0;
    victimFrozen = 0;
    homePos = pos;
    homeRot = rot;
    shadow.radius = 0x14;
    SetState(WOLFTRAP_ST_ARMED);
}

/* vtable +0x14 */
void WolfTrap::Reset()
{
    SetState(WOLFTRAP_ST_ARMED);
    victimFrozen = 0;
}

/* vtable +0x04: state 1 counts PAD_CROSS press edges (each relayed to the victim as msg 0x40) and opens
 * at 6 within a 0x400 ms window; state 2 re-arms when its timer runs out; state 3 retries the freeze. */
void WolfTrap::Update()
{
    switch (state) {
        case WOLFTRAP_ST_CAUGHT:
            if (!(g_padCurButtons & ~g_inputMap[INPUT_SLOT_CROSS]) &&
                (g_padPrevButtons & ~g_inputMap[INPUT_SLOT_CROSS])) {
                pVictim->HandleMessage(this, MSG_STRUGGLE, 0);
                struggleCount++;
            }
            if (struggleCount >= 6)
                SetState(WOLFTRAP_ST_RELEASE);
            timerMs -= g_dtMs;
            if (timerMs <= 0) {
                struggleCount = 0;
                timerMs = 0x400;
            }
            break;
        case WOLFTRAP_ST_RELEASE:
            timerMs -= g_dtMs;
            if (timerMs <= 0)
                SetState(WOLFTRAP_ST_ARMED);
            break;
        case WOLFTRAP_ST_CATCHING:
            if (SetVictimFrozen(1))
                SetState(WOLFTRAP_ST_CAUGHT);
            break;
    }
    AdvanceAnim();
}

/* vtable +0x10: 2 (query action) answers 0x19 to Ralph or the Robot while armed; 1 (use) catches the
 * sender; 0xe (the victim lost its freeze) resets for Ralph, retries (state 3) for the Robot; 0x2c80 resets. */
sptr WolfTrap::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    if (msgId < MSG_WOLF_CAUGHT) {
        switch (msgId) {
            case MSG_QUERY_ACTION:
                if ((sender->GetClassId() == CLASSID_WOLF || sender->GetClassId() == CLASSID_ROBOT) &&
                    state == WOLFTRAP_ST_ARMED)
                    return CTX_WOLFTRAP;
                break;
            case MSG_USE:
                pVictim = sender;
                SetState(WOLFTRAP_ST_CAUGHT);
                return 1;
            case MSG_FREEZE:
                switch (sender->GetClassId()) {
                    case CLASSID_WOLF:
                        victimFrozen = 0;
                        Reset();
                        break;
                    case CLASSID_ROBOT:
                        victimFrozen = 0;
                        if (state == WOLFTRAP_ST_CAUGHT)
                            SetState(WOLFTRAP_ST_CATCHING);
                        break;
                }
                return 1;
        }
    } else {
        switch (msgId) {
            case MSG_WOLFTRAP_RESET:
                Reset();
                return 1;
        }
    }
    return 0;
}

/* ask the victim to freeze (msg 0xe) or unfreeze (0xf); the flag follows only an accepted answer. */
s32 WolfTrap::SetVictimFrozen(s32 frozen)
{
    if (frozen != victimFrozen && pVictim) {
        if (frozen) {
            if (pVictim->HandleMessage(this, MSG_FREEZE, (void *)1))
                victimFrozen = 1;
        } else {
            if (pVictim->HandleMessage(this, MSG_UNFREEZE, (void *)1))
                victimFrozen = 0;
        }
    }
    return victimFrozen;
}

/* enter a state: 0 armed at home (anim 4), 1 holding the victim (anim 1, alarm to victim, Sam and
 * Bell), 2 opening (anim 3, no shadow, 0x1400 ms), then store it. */
void WolfTrap::SetState(u8 newState)
{
    switch (newState) {
        case WOLFTRAP_ST_ARMED:
            SetPosition(&homePos);
            rot = homeRot;
            shadow.SetVisible(1);
            PlayAnim(APIEGE01_ANIM_TRAP3, 0, 0);
            pVictim = 0;
            break;
        case WOLFTRAP_ST_CAUGHT:
            PlayAnim(APIEGE01_ANIM_TRAP0, 0, 0);
            SetPosition(&pVictim->pos);
            SetRotation(&pVictim->rot);
            pVictim->HandleMessage(this, MSG_TRAP_STATE, (void *)1);
            SetVictimFrozen(1);
            if (pSam)
                pSam->HandleMessage(this, MSG_TRAP_STATE, (void *)1);
            if (pBell)
                pBell->HandleMessage(this, MSG_TRAP_STATE, (void *)1);
            struggleCount = 0;
            timerMs = 0x400;
            break;
        case WOLFTRAP_ST_RELEASE:
            PlayAnim(APIEGE01_ANIM_TRAP2, 0, 0);
            shadow.SetVisible(0);
            timerMs = 0x1400;
            pVictim->HandleMessage(this, MSG_TRAP_STATE, (void *)0);
            SetVictimFrozen(0);
            pVictim = 0;
            break;
    }
    state = newState;
}

/* the class factory for CLASSID 81 "WolfTrap": new WolfTrap (the base vtables in turn, then WolfTrap's),
 * then ScnMobile::Init(record, 0) through the vtable. */
ScnObject *WolfTrap_Create(void *record)
{
    ScnBody *obj = new WolfTrap;
    obj = obj->Init(record, 0);
    return obj;
}
