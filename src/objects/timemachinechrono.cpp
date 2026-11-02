
#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
class Instance;
struct Animator;
u32 Anim_Start(Instance *, Animator *, u16, u32);

#define SDW_MEMBERS_ScnObject       \
    static void *operator new(uptr); \
    void AttachTo(ScnObject *, u8, Vec3s *, Vec3s *, u32, uptr);

#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#define SDW_INLINE_SCNOBJECT_ISINWORLD 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_ISINWORLD
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
#define SDW_INLINE_SHADOW_SETVISIBLE_S32 1
#include "../engine/shadow_inlines.h"
#undef SDW_INLINE_SHADOW_SETVISIBLE_S32
s32 Scenaric_FindByClass(u16, ScnObject **, s32);
#include "timemachine.h"
void TimeMachineChrono::PostLoadInit()
{
    if (IsInWorld())
        SnapToGround(1);
    homePos = pos;
    if (Scenaric_FindByClass(CLASSID_TIMEMACHINESPHERE, (ScnObject **)&sphere, 1) != 1)
        sphere = 0;
    SetHeld(0);
}
void TimeMachineChrono::Update()
{
    AdvanceAnim();
}
sptr TimeMachineChrono::HandleMessage(ScnObject *sender, u32 msg, void *arg)
{
    ScnObject *holder;
    {
        u8 joint;
        {
            Vec3s *drop;
            switch (msg) {
                case MSG_CARRY_ANIM:
                    if (arg == (void *)9) {
                        PlayAnim(ACHRON01_ANIM_CHRONO1, 0, 1);
                        return 1;
                    }
                    return 0;
                case MSG_QUERY_ACTION:
                    if (sender->GetClassId() == CLASSID_WOLF)
                        return CTX_PICKUP;
                    return CTX_NONE;
                case MSG_QUERY_HELD_ACTION:
                    if (TimeMachine_IsInPresent(this))
                        return HELD_TIMEMACHINE;
                    return HELD_NONE;
                case MSG_HELD_STATE_BEGIN:
                    PlayAnim(ACHRON01_ANIM_CHRONO2, 0, 1);
                    if (held == 1 && TimeMachine_IsInPresent(this))
                        sphere->HandleMessage(this, MSG_TIMEMACHINE_START, 0);
                    else
                        return 0;
                    break;
                case MSG_TIMEMACHINE_SWAP:
                    return 1;
                case MSG_TIMEMACHINE_SWAP_OUT:
                    return 1;
                case MSG_PICKUP:
                    holder = sender;
                    joint = (u8)(uptr)arg;
                    AttachTo(holder, joint, 0, 0, 0, 0);
                    SetHeld(1);
                    shadow.SetVisible(0);
                    return 1;
                case MSG_DROP:
                    drop = (Vec3s *)arg;
                    Detach();
                    SetPosition(drop);
                    SetHeld(0);
                    shadow.SetVisible(1);
                    return 1;
                case MSG_CONTAINER_STATE:
                    switch ((s32)(sptr)arg) {
                        case CONTAINER_STORED:
                            SetHeld(0);
                            break;
                        case CONTAINER_RELEASED:
                            SetHeld(0);
                            homePos = pos;
                            break;
                    }
                    return 1;
                case MSG_CHECKPOINT_ROLLBACK:
                    Reset();
                    return 1;
            }
            return 0;
        }
    }
}
void TimeMachineChrono::SetHeld(u8 value)
{
    held = value;
    switch (value) {
        case 0:
            PlayAnim(ACHRON01_ANIM_OBJET, 0, 0);
            break;
        case 1:
            PlayAnim(ACHRON01_ANIM_LINK, 0, 0);
            break;
    }
}
void TimeMachineChrono::Reset()
{
    if (IsInWorld()) {
        PlayAnim(ACHRON01_ANIM_OBJET, 0, 0);
        SetPosition(&homePos);
        SetHeld(0);
    }
}
ScnObject *TimeMachineChrono_Create(void *record)
{
    TimeMachineChrono *object = new TimeMachineChrono;
    object = (TimeMachineChrono *)object->Init(record, 0);
    return object;
}
