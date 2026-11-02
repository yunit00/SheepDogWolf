/*
 * The level has two halves; the one whose OTHERPART property names the other is the master. Each half counts Ralph
 * (class 0) and sheep (class 0xb) in its FALLINGZONE: with both there it creaks (anim 5, one count per loop). At loop 6
 * the master freezes Ralph (msg 0xE) and, when Ralph and the sheep are more than 300 apart, frames them with a scripted
 * camera; at loop 12 the half collapses (anim 2, then 4 fallen) and the master sends msg 0 arg 1 to everything in the
 * zone, releases the camera and from then on pushes the objects in IDOBJECTSFALLZONE down. If the count drops below 2
 * before loop 12 the bridge settles back (anim 0) and unfreezes Ralph. Designer properties (bridgeProps): FALLINGZONE
 * +0, IDOBJECTSFALLZONE +4, OTHERPART +8.
 * */

#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetUpdateMode(u8 mode);

#define SDW_MEMBERS_bridge                                                                      \
    void StartSnapCam(u16 rotX, u16 rotY, u16 rotZ) \
    {                                                                                           \
        Camera_StartScripted(this, &g_camera, rotX, rotY, rotZ, &snapCamPos, 0x280, 0, 0x1000); \
    }
#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
class Camera;
class ScnObject;
class Instance;
struct Animator;
u32 Anim_Start(Instance *inst, Animator *animator, u16 id, u32 opts); /* (used by the PlayAnim inline) */
void Camera_StartScripted(ScnObject *owner, Camera *cam, u16 rotX, u16 rotY, u16 rotZ, Vec3s *pos, u16 focal, u32 mode,
                          s32 time);
#include "../app/app_main.h"
#include "../engine/scn_tools.h"
#include "../engine/id_list.h"
#include "../engine/scenaric.h"
#include "camera.h"
#include "../engine/fixed_math.h"
#include "../engine/input.h"
#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#define SDW_INLINE_SCNOBJECT_SETPARTHEIGHT_U32 1
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_SETPARTHEIGHT_U32
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
#define SDW_INLINE_SCNBODY_ANIMFLAGS_U16 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_ANIMFLAGS_U16
#include "../engine/grid_queries.h"
#include "../engine/property_math.h"

#include "../sdk/crt.h"
extern Wolf *g_pWolf;

/* A designer property: the dword at record + 0x14 + off (bridgeProps). */
#define SDW_INLINE_FREE_PROPU32_VOID_U32 2
#include "../engine/scn_tools_inlines.h"
#undef SDW_INLINE_FREE_PROPU32_VOID_U32

/* vtable +0x00: read the three properties. No record or no FALLINGZONE leaves the bridge dead (state 3);
 * IDOBJECTSFALLZONE falls back to FALLINGZONE. The idle bridge is camera-culled (update mode 0). */
void bridge::PostLoadInit()
{
    Box **zoneList;
    u16 n;
    u32 zoneId;
    u16 *rec;
    SetPartHeight(0);
    state = BRIDGE_ST_IDLE;
    rec = record;
    if (rec == 0)
        state = BRIDGE_ST_BROKEN;
    fallZone = Scn_GetPropBox(rec, 0);
    if (fallZone == 0)
        state = BRIDGE_ST_BROKEN;
    zoneId = PropU32(rec, 4);
    zoneList = (Box **)Scn_FindIdList((u16)zoneId, &n);
    if (zoneList)
        pushZone = *zoneList;
    else if (fallZone)
        pushZone = fallZone;
    else {
        pushZone = 0;
        state = BRIDGE_ST_BROKEN;
    }
    wolfFrozen = 0;
    otherPart = Scn_GetPropObject(rec, 8);
    pushDown = 0;
    SetUpdateMode(SCN_UPD_NORMAL);
}

/* vtable +0x04: the creak / collapse state machine (see the file comment), then the push-down and the
 * animation step. */
void bridge::Update()
{
    switch (state) {
        case BRIDGE_ST_IDLE:
            creakLoops = 0;
            if (CountWolfAndSheep() >= 2) {
                PlayAnim(APONTS01_ANIM_POSE2, 0, 0);
                state = BRIDGE_ST_CREAK;
                SetUpdateMode(SCN_UPD_ALWAYS);
                break;
            }
            break;
        case BRIDGE_ST_CREAK:
            if (AnimFlags(ANIM_F_FINISHED)) {
                if (creakLoops == 6) {
                    if (otherPart) {
                        if (pairDist > 300)
                            StartSnapCamera();
                        wolfFrozen = g_pWolf->HandleMessage(this, MSG_FREEZE, 0);
                    }
                    PlayAnim(APONTS01_ANIM_POSE2, 0, 0);
                    creakLoops++;
                    break;
                }
                if (creakLoops == 12) {
                    SetPartHeight(0x100);
                    PlayAnim(APONTS01_ANIM_BRAKE2, 0, 0);
                    if (otherPart) {
                        Collapse();
                        Camera_ReleaseAny();
                    }
                    state = BRIDGE_ST_BREAK;
                    break;
                }
                PlayAnim(APONTS01_ANIM_POSE2, 0, 0);
                creakLoops++;
            }
            if (CountWolfAndSheep() < 2) {
                PlayAnim(APONTS01_ANIM_POSE1, 0, 0);
                state = BRIDGE_ST_IDLE;
                SetUpdateMode(SCN_UPD_NORMAL);
                if (wolfFrozen) {
                    g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
                    wolfFrozen = 0;
                }
                break;
            }
            g_pad.Rumble_stub(50, g_rumbleSeqImpact, 0x1000);
            break;
        case BRIDGE_ST_BREAK:
            if (AnimFlags(ANIM_F_FINISHED)) {
                PlayAnim(APONTS01_ANIM_FBRAKE2, 0, 0);
                state = BRIDGE_ST_BROKEN;
            }
            break;
    }
    if (pushDown)
        PushObjectsDown();
    AdvanceAnim();
}

/* the snap camera of the master half: from the midpoint of Ralph and the sheep, back along the bridge's
 * facing + 90 degrees by the pair distance (at least 600) and 100 up, looking back at them. */
void bridge::StartSnapCamera()
{
    Mat34s m;
    Vec3s facing;
    Vec4i off = {0, 0, 0};
    Vec3s ang;
    snapCamPos = midpoint;
    snapCamRot.x = 0;
    snapCamRot.y = 0;
    snapCamRot.z = 0;
    if (pairDist < 600)
        pairDist = 600;
    off.z = -(s16)pairDist;
    facing = rot;
    ang.x = 0;
    ang.y = facing.y + 0x400;
    ang.z = 0;
    Mat34s_FromEulerYXZ(&ang, &m);
    Mat34s_TransformTransposedVec3i(&m, (Vec3i *)&off, &off);
    snapCamRot.x = 0;
    snapCamRot.y = facing.y + 0x400;
    snapCamRot.z = 0;
    snapCamPos.x = off.x + midpoint.x;
    snapCamPos.y = off.y + midpoint.y - 100;
    snapCamPos.z = off.z + midpoint.z;
    StartSnapCam(snapCamRot.x, snapCamRot.y, snapCamRot.z);
}

/* how many of Ralph and the sheep have their origin in fallZone (Ralph and every sheep each count once;
 * the positions kept are the last of each kind). With two or more, the midpoint of the last Ralph and the last sheep
 * found and their XZ distance are stored for the snap camera. */
u8 bridge::CountWolfAndSheep()
{
    ScnObject *obj;
    Vec3s ralph;
    s32 i;
    ScnObject *grid[64];
    Vec3s sheepAt;
    u8 count;
    s32 nb;
    count = 0;
    nb = ObjGrid_QueryBoxPoints((const CollBox *)fallZone, grid);
    for (i = 0; i < nb; i++) {
        obj = grid[i];
        switch (obj->GetClassId()) {
            case CLASSID_SHEEP:
                count++;
                sheepAt = obj->pos;
                break;
            case CLASSID_WOLF:
                count++;
                ralph = obj->pos;
                break;
        }
    }
    if (count >= 2) {
        midpoint.x = (ralph.x + sheepAt.x) / 2;
        midpoint.y = (ralph.y + sheepAt.y) / 2;
        midpoint.z = (ralph.z + sheepAt.z) / 2;
        pairDist = (s32)sqrt((double)(ralph.x - sheepAt.x) * (ralph.x - sheepAt.x) +
                             (ralph.z - sheepAt.z) * (ralph.z - sheepAt.z));
    }
    return count;
}

/* the master's collapse: unfreeze Ralph, msg 0 with arg 1 to every object in fallZone that is not a
 * bridge, and start the push-down. */
void bridge::Collapse()
{
    ScnObject *obj;
    s32 i;
    ScnObject *grid[64];
    s32 nb;
    if (wolfFrozen) {
        g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
        wolfFrozen = 0;
    }
    nb = ObjGrid_QueryBoxPoints((const CollBox *)fallZone, grid);
    for (i = 0; i < nb; i++) {
        obj = grid[i];
        if (obj->GetClassId() != CLASSID_BRIDGE)
            obj->HandleMessage(this, MSG_KILL, (void *)KILL_FALL_PIT);
    }
    pushDown = 1;
}

/* move every object whose origin is inside pushZone's XZ rectangle, other than a bridge, Ralph or a sheep,
 * 10 units down. Per update, not scaled by the frame time. */
void bridge::PushObjectsDown()
{
    Vec3s step;
    ScnObject *obj;
    s32 i;
    ScnObject *grid[64];
    s32 nb;
    if (pushZone) {
        nb = ObjGrid_QueryPointsInRectXZ(pushZone->min[0], pushZone->min[2], pushZone->max[0], pushZone->max[2], grid);
        for (i = 0; i < nb; i++) {
            obj = grid[i];
            if (obj->GetClassId() != CLASSID_BRIDGE && obj->GetClassId() != CLASSID_WOLF &&
                obj->GetClassId() != CLASSID_SHEEP) {
                step.x = step.z = 0;
                step.y = 10;
                obj->Translate(&step);
            }
        }
    }
}

/* vtable +0x14: back to the idle pose (anim 0) and state (dead without a FALLINGZONE); no freeze, no
 * push-down. */
void bridge::Reset()
{
    SetPartHeight(0);
    PlayAnim(APONTS01_ANIM_POSE1, 0, 0);
    if (fallZone == 0)
        state = BRIDGE_ST_BROKEN;
    else
        state = BRIDGE_ST_IDLE;
    wolfFrozen = 0;
    pushDown = 0;
}

/* vtable +0x10: msg 0xE (Ralph releasing the freeze on his side) forgets it; returns 1 for it, else 0. */
sptr bridge::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    if (msgId == MSG_FREEZE) {
        wolfFrozen = 0;
        return 1;
    }
    return 0;
}

/* the class factory for CLASSID 6 "bridge": new bridge, then ScnBody::Init(record, 0) through the vtable. */
ScnObject *bridge_Create(void *record)
{
    ScnBody *obj = new bridge;
    obj = obj->Init(record, 0);
    return obj;
}
