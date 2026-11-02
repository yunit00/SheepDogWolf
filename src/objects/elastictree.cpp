
#include "sdw_types.h"
#include "sdw_enums.h"
#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetUpdateMode(s32 mode);
#include "sdw_classes.h"
#include "../engine/scn_tools.h"
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32

void ElasticTree::PostLoadInit()
{
    void *properties = record;
    Box *box = Scn_GetPropBox(properties, 4);
    camRestrict = Scn_GetPropObject(properties, 0);
    pullUpPoint = (Vec3s *)&box->min;
    unk48 = 0;
}
void ElasticTree::Update()
{
    if (camRestrict)
        camRestrict->HandleMessage(this, MSG_CAMRESTRICT_ENABLE, 0);
    SetUpdateMode(SCN_UPD_NEVER);
}
sptr ElasticTree::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    void **out;
    switch (msgId) {
        case MSG_ELASTICTREE_GET_INFO:
            out = (void **)arg;
            out[0] = pullUpPoint;
            out[1] = camRestrict;
            return 1;
    }
    return 0;
}
void ElasticTree::Reset() {}
ScnObject *ElasticTree_Create(void *record)
{
    ElasticTree *object = new ElasticTree;
    object = (ElasticTree *)object->Init(record);
    return object;
}
