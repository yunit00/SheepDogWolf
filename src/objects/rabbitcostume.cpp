
#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetUpdateMode(u8 mode);
#include "sdw_enums.h"
#include "scenaric_props.h"
#include "sdw_classes.h"
#define SDW_INLINE_SCNOBJECT_ISINWORLD 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_ISINWORLD
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8

sptr RabbitCostume::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    switch (msgId) {
        case MSG_QUERY_ACTION:
            if (sender->GetClassId() == CLASSID_WOLF)
                return CTX_PICKUP;
            break;
        case MSG_CONTAINER_STATE:
            switch ((u32)(uptr)arg) {
                case CONTAINER_RELEASED:
                    homePos = pos;
                    break;
            }
            return 1;
        case MSG_CHECKPOINT_ROLLBACK:
            SetPosition(&homePos);
            return 1;
    }
    return 0;
}

void RabbitCostume::PostLoadInit()
{
    if (IsInWorld())
        SnapToGround(1);
    homePos = pos;
    SetUpdateMode(SCN_UPD_NEVER);
}

ScnObject *RabbitCostume_Create(void *record)
{
    RabbitCostume *obj = new RabbitCostume;
    obj = (RabbitCostume *)obj->Init(record, 0);
    return obj;
}
