
#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
class Instance;
struct Animator;
u32 Anim_Start(Instance *, Animator *, u16, u32);

#define SDW_MEMBERS_ScnObject                           \
    static void *operator new(uptr);                     \
    void SetInstancePriority(u32 priority)              \
    {                                                   \
        partHeight = (u8)(priority >> 4);               \
    }                                                   \
    void StartCamera(u16, u16, u16, Vec3s *, u16, u32); \
    void SetUpdateMode(s32 mode);

#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#define SDW_INLINE_SCNOBJECT_SETCONTACTENABLED_S32 1
#define SDW_INLINE_SCNOBJECT_SETVISIBLE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_SETCONTACTENABLED_S32
#undef SDW_INLINE_SCNOBJECT_SETVISIBLE_S32
#define SDW_INLINE_SCNOBJECT_SETFLAG40_S32 1
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETFLAG40_S32
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32
#define SDW_INLINE_SCNBODY_ANIMFLAGS_U16 1
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_ANIMFLAGS_U16
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
extern Wolf *g_pWolf;
#include "../app/app_main.h"
#include "../engine/scn_tools.h"
#include "camera.h"
u16 g_fallingGateModelIds[5] = {WAR_IDO_AGRILL01, WAR_IDO_AGRILL02, WAR_IDO_AGRILL03, WAR_IDO_AGRILL04,
                                WAR_IDO_AGRILL05};
u8 g_fallingGateAnims[5][4] = {
    {AGRILL01_ANIM_CLOSED, AGRILL01_ANIM_OPEN, AGRILL01_ANIM_MV_CLOSE, AGRILL01_ANIM_MV_OPEN},
    {AGRILL02_ANIM_CLOSED, AGRILL02_ANIM_OPEN, AGRILL02_ANIM_MV_CLOSE, AGRILL02_ANIM_MV_OPEN},
    {AGRILL03_ANIM_CLOSED, AGRILL03_ANIM_OPEN, AGRILL03_ANIM_MV_CLOSE, AGRILL03_ANIM_MV_OPEN},
    {AGRILL04_ANIM_CLOSED, AGRILL04_ANIM_OPEN, AGRILL04_ANIM_MV_CLOSE, AGRILL04_ANIM_MV_OPEN},
    {AGRILL05_ANIM_CLOSED, AGRILL05_ANIM_OPEN, AGRILL05_ANIM_MV_CLOSE, AGRILL05_ANIM_MV_OPEN},
};
s32 Scenaric_FindByClass(u16, ScnObject **, s32);
s32 ObjGrid_QueryBoxOverlap(CollBox *, ScnObject **);
void Camera_StartScripted(ScnObject *, Camera *, u16, u16, u16, Vec3s *, u16, u32, s32);
#define SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_U32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_U32
inline u32 GateProperty(void *record, u32 offset)
{
    return *(u32 *)((u8 *)record + offset + 0x14);
}
void FallingGate::PostLoadInit()
{
    ScnObject *train;
    u16 *props = record;
    closedAtStart = 1;
    if (Scenaric_FindByClass(CLASSID_TRAIN, &train, 1))
        closedAtStart = GateProperty(props, 0);
    onlyOneActivation = GateProperty(props, 12);
    cutCamera = GateProperty(props, 4);
    camera = Scn_GetPropCamera(props, 8);
    closed = closedAtStart;
    SetVisible(closed);
    modelIndex = GateProperty(props, 16);
    if (modelIndex >= 10) {
        keepCamera = 1;
        modelIndex -= 10;
    } else
        keepCamera = 0;
    if (modelIndex < 1)
        modelIndex = 0;
    else if (modelIndex > 5)
        modelIndex = 4;
    else
        --modelIndex;
    SwapModel(&gateModels[modelIndex]);
    if (closed) {
        SetContactEnabled(1);
        PlayAnim(g_fallingGateAnims[modelIndex][0], 0, 0);
    } else {
        SetContactEnabled(0);
        PlayAnim(g_fallingGateAnims[modelIndex][1], 0, 0);
    }
    SetInstancePriority(0);
    SetFlag40(1);
    busy = 0;
    wolfFrozen = 0;
    cameraReleased = 1;
    cameraActive = 0;
    solidBox = GetFirstSolidBox();
    retriggered = 0;
    SetUpdateMode(SCN_UPD_ALWAYS);
    unk100 = 0;
    state = FGATE_ST_IDLE;
}
void FallingGate::Update()
{
    switch (state) {
        case FGATE_ST_MOVING:
            FinishMove();
            break;
        case FGATE_ST_SINGLE_SHOT:
            if (AnimFlags(ANIM_F_FINISHED)) {
                if (closed)
                    SetVisible(1);
                else
                    SetVisible(0);
                if (!keepCamera)
                    Camera_ReleaseScripted(this);
                state = FGATE_ST_IDLE;
                if (wolfFrozen)
                    wolfFrozen = !g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
            } else if (camera)
                StartCamera(camera->rot[0], camera->rot[1], camera->rot[2], &camera->eye, camera->focal, 0);
            break;
    }
    AdvanceAnim();
}
void FallingGate::FinishMove()
{
    if (AnimFlags(ANIM_F_FINISHED)) {
        busy = 0;
        state = FGATE_ST_IDLE;
        if (closed) {
            s32 count;
            {
                ScnObject *objects[10];
                {
                    CollBox world;
                    world.Box_Translate(solidBox, &pos);
                    count = ObjGrid_QueryBoxOverlap(&world, objects);
                    for (s32 i = 0; i < count; ++i) {
                        if (objects[i]->GetFirstSolidBox() && objects[i] != this &&
                            objects[i]->GetClassId() != CLASSID_SLIDINGICECUBE)
                            objects[i]->HandleMessage(this, MSG_KILL, (void *)KILL_CRUSH);
                    }
                    PlayAnim(g_fallingGateAnims[modelIndex][0], 0, 0);
                }
            }
        } else {
            SetContactEnabled(0);
            SetVisible(0);
            PlayAnim(g_fallingGateAnims[modelIndex][1], 0, 0);
        }
        if (wolfFrozen)
            wolfFrozen = !g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
        if (camera && !keepCamera)
            Camera_ReleaseScripted(this);
    } else
        SetVisible(1);
}
sptr FallingGate::HandleMessage(ScnObject *sender, u32 msg, void *arg)
{
    s32 showCamera;
    switch (msg) {
        case MSG_SWITCH_ON:
            if (!busy && closedAtStart == closed) {
                if (closedAtStart) {
                    SetContactEnabled(0);
                    closed = 0;
                } else {
                    closed = 1;
                    SetContactEnabled(1);
                }
                if (camera) {
                    u8 i;
                    {
                        ScnObject **occupants;
                        {
                            s32 freezeWolf = 1;
                            occupants = (ScnObject **)arg;
                            for (i = 0; i < 4; ++i)
                                if (occupants[i] && (occupants[i]->GetClassId() == CLASSID_WOLF ||
                                                     occupants[i]->GetClassId() == CLASSID_ROBOT))
                                    freezeWolf = 0;
                            if (freezeWolf && g_pWolf->HandleMessage(this, MSG_QUERY_CONTROLLED, 0))
                                wolfFrozen = g_pWolf->HandleMessage(this, MSG_FREEZE, 0);
                            showCamera = 1;
                            if (showCamera) {
                                if (state != FGATE_ST_MOVING && !retriggered) {
                                    if (!cameraActive) {
                                        cameraActive = 1;
                                        cameraReleased = 0;
                                        if (cutCamera)
                                            StartCamera(camera->rot[0], camera->rot[1], camera->rot[2], &camera->eye,
                                                        camera->focal, CAMSCR_BLEND_OUT);
                                        else
                                            StartCamera(camera->rot[0], camera->rot[1], camera->rot[2], &camera->eye,
                                                        camera->focal, CAMSCR_BLEND_IN | CAMSCR_BLEND_OUT);
                                    }
                                } else
                                    retriggered = 1;
                            } else if (!cameraReleased && !keepCamera) {
                                cameraReleased = 1;
                                cameraActive = 0;
                                Camera_ReleaseScripted(this);
                            }
                        }
                    }
                }
                state = FGATE_ST_MOVING;
                busy = 1;
                if (closedAtStart) {
                    closed = 0;
                    PlayAnim(g_fallingGateAnims[modelIndex][3], 0, 0);
                } else {
                    closed = 1;
                    PlayAnim(g_fallingGateAnims[modelIndex][2], 0, 0);
                }
            } else
                retriggered = 1;
            if (onlyOneActivation) {
                SetContactEnabled(0);
                if (camera)
                    wolfFrozen = g_pWolf->HandleMessage(this, MSG_FREEZE, 0);
                state = FGATE_ST_SINGLE_SHOT;
                sender->HandleMessage(this, MSG_BUTTON_LOCK, (void *)1);
            }
            break;
        case MSG_SWITCH_OFF:
            if (!busy && closedAtStart != closed) {
                busy = 1;
                state = FGATE_ST_MOVING;
                if (closedAtStart) {
                    closed = 1;
                    SetContactEnabled(1);
                    PlayAnim(g_fallingGateAnims[modelIndex][2], 0, 0);
                } else {
                    closed = 0;
                    SetContactEnabled(0);
                    PlayAnim(g_fallingGateAnims[modelIndex][3], 0, 0);
                }
                retriggered = 0;
                if (cameraActive) {
                    Camera_ReleaseScripted(this);
                    cameraActive = 0;
                }
            }
            break;
        case MSG_GATE_QUERY:
            return closed;
        case MSG_FREEZE:
            wolfFrozen = 0;
            return 1;
    }
    return 0;
}
ScnObject *FallingGate_Create(void *record)
{
    FallingGate *object = new FallingGate;
    object = (FallingGate *)object->InitWithAltModels(record, &object->mainModel, 5, g_fallingGateModelIds,
                                                      object->gateModels);
    return object;
}
