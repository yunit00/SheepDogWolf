
#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetUpdateMode(s32 mode);
#include "sdw_enums.h"
#include "sdw_classes.h"
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32
s32 Box_GroundQueryFlatTop(GroundQuery *, CollBox *, Vec3s *, s32);
ScnObject *Case_Create(void *record)
{
    ScnBody *object = new Case;
    object = object->Init(record, 0);
    return object;
}
void Case::PostLoadInit()
{
    SetUpdateMode(SCN_UPD_NEVER);
    SnapToGround(1);
}
sptr Case::HandleMessage(ScnObject *sender, u32 message, void *arg)
{
    switch (message) {
        case MSG_GROUND_QUERY:
            return Box_GroundQueryFlatTop((GroundQuery *)arg, GetFirstSolidBox(), &pos, 0);
    }
    return 0;
}
