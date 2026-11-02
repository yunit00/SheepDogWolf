#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetUpdateMode(s32 mode);
#include "sdw_classes.h"
#include "../app/app_main.h"
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#define SDW_INLINE_SCNOBJECT_SETVISIBLE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_SETVISIBLE_S32
#define SDW_INLINE_SCNOBJECT_SETMOVEMENTENABLED_S32 1
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETMOVEMENTENABLED_S32
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32
ScnObject *g_pGhostHalo;

void GhostHalo::PostLoadInit()
{
    SetMovementEnabled(0);
    SetVisible(0);
    SetUpdateMode(SCN_UPD_NEVER);
}

sptr GhostHalo::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    Vec3s scale;
    Vec3s *point;
    /* The original accesses sender before its null check; preserve that order. */
    if ((sender->GetClassId() == CLASSID_DANCINGGHOST || sender->GetClassId() == CLASSID_PRAYINGGHOST) && sender) {
        switch (msgId) {
            case MSG_HALO_DRAW_AT:
                point = (Vec3s *)arg;
                SetPosition(point);
                scale.x = 800;
                scale.y = 800;
                scale.z = 800;
                RenderFacingCamera(&g_camera, 0, &scale, 0);
                AdvanceAnim();
                break;
        }
    }
    return 0;
}

ScnObject *GhostHalo_Create(void *record)
{
    ScnBody *obj = new GhostHalo;
    obj = obj->Init(record, 0);
    g_pGhostHalo = obj;
    return obj;
}
