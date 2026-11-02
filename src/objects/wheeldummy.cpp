/*
 * WheelDummy (class 112, vtable, sizeof 0x54) - the stand-in for the big platform wheel of Level 10
 * (disc Lvl-11), used in the later phases of the level in place of the wheel of the previous phase. It has no designer
 * properties (PROPSIZE_WHEELDUMMY 0) and no geometry logic of its own: the live Wheel (class 111) copies one of its
 * platform model boxes, offset by the wheel's position, into this object's box every frame, and
 * the dummy exists only to answer the ground query for Ralph so he can stand on a platform that the wheel itself no
 * longer drives.
 *
 * The single WheelDummy field is the CollBox box at +0x40.
 * */
#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetUpdateMode(s32 mode);
#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
#include "sdw_classes.h"
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32

extern Wolf *g_pWolf;

/* vtable +0x00: the dummy is pure logic, so it is never updated (ScnUpdateMode 2 = never). */
void WheelDummy::PostLoadInit()
{
    SetUpdateMode(SCN_UPD_NEVER);
}

/* the answer to MSG_GROUND_QUERY: if Ralph is over the box in plan, the ground here is the box top plus 5 and the surface
 * is flat (normal straight up: 0xf000 = -0x1000 in 4.12, with the vertical axis pointing down). Note it tests g_pWolf
 * rather than the sender; the caller has already checked that the sender is the Wolf. Two arrangement devices here.
 * Comparing the whole && chain with 0 materialises it into such a temporary. */
s32 WheelDummy::AnswerGroundQuery(void *arg)
{
    Vec3s *pos;
    CollBox *b;
    GroundQuery *q;

    pos = &g_pWolf->pos;
    b = &box;
    if ((pos->x >= b->min.x && pos->x <= b->max.x && pos->z >= b->min.z && pos->z <= b->max.z) != 0) {
        q = (GroundQuery *)arg;
        q->pos.y = (s16)(box.min.y + 5);
        q->normal.y = (s16)0xf000;
        q->normal.x = 0;
        q->normal.z = 0;
        return 1;
    }
    return 0;
}

/* vtable +0x04: nothing to do. */
void WheelDummy::Update() {}

/* vtable +0x10: only the Wolf (class 0) is answered, and only MSG_GROUND_QUERY. */
sptr WheelDummy::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    if (sender && sender->GetClassId() == CLASSID_WOLF) {
        switch (msgId) {
            case MSG_GROUND_QUERY:
                return AnswerGroundQuery(arg);
        }
    }
    return 0;
}

/* the class factory for CLASSID 112 "WheelDummy": new WheelDummy (the base vtables in turn, then
 * WheelDummy's), then ScnLogic_Init(record) through vtable slot +0x20. */
ScnObject *WheelDummy_Create(void *record)
{
    WheelDummy *obj = new WheelDummy;
    obj = (WheelDummy *)obj->Init(record);
    return obj;
}
