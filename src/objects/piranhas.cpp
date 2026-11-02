#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
class Instance;
struct Animator;
u32 Anim_Start(Instance *inst, Animator *animator, u16 id, u32 opts);
#include "../engine/scn_tools.h"
#include "../engine/scenaric.h"
#define SDW_INLINE_FREE_INSTFLAGS_SET_U16_U16 1
#include "instance_inlines.h"
#undef SDW_INLINE_FREE_INSTFLAGS_SET_U16_U16
#define SDW_INLINE_FREE_INSTFLAGS_CLEAR_U16_U16 1
#include "instance_inlines.h"
#undef SDW_INLINE_FREE_INSTFLAGS_CLEAR_U16_U16

#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetFacing(s16 heading);         \
    void SetRotation(Vec3s *rotation);   \
    void SetUpdateMode(u8 mode);         \
    void SetTint(u32 color, s16 amount, s32 on);

#define SDW_MEMBERS_ZoneList Box *Find(Vec3s *point);
#define SDW_MEMBERS_TrajFollower \
    s16 GetPointIndex()          \
    {                            \
        return pointIndex;       \
    }
/* Expanded into StaticCtor_g_piranhaBubbleFx. */
#define SDW_MEMBERS_InlineEmitter10 InlineEmitter10();
/* Expanded into StaticCtor_g_piranhaRingFx. */
#define SDW_MEMBERS_InlineEmitter6 InlineEmitter6();
#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNOBJECT_SETFACING_S16 1
#define SDW_INLINE_SCNOBJECT_SETROTATION_VEC3S 1
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETFACING_S16
#undef SDW_INLINE_SCNOBJECT_SETROTATION_VEC3S
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8
#define SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
#define SDW_INLINE_SCNBODY_GETANIMID 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_GETANIMID
#define SDW_INLINE_ZONELIST_FIND_VEC3S 1
#include "../engine/zone_list_inlines.h"
#undef SDW_INLINE_ZONELIST_FIND_VEC3S
inline InlineEmitter10::InlineEmitter10()
{
    base.slotPool = slotBuf;
    base.particles = particleBuf;
    base.count = 10;
    base.Emitter_Reset();
}
inline InlineEmitter6::InlineEmitter6()
{
    base.slotPool = slotBuf;
    base.particles = particleBuf;
    base.count = 6;
    base.Emitter_Reset();
}

/* automatic root/constructor for storage. */
InlineEmitter10 g_piranhaBubblesShared;
s32 g_piranhaRingSpawnReq;
u8 g_numPiranhasAttacking;
s32 g_piranhaBubbleSpawnRequest;
Vec3s g_piranhaFxPos;
Piranhas *g_piranhaFxOwner;
/* automatic root/constructor for storage. */
InlineEmitter6 g_piranhaRingFx;
EmitterFadeParams g_piranhaRingFxParams = {0};   /* filled in by Piranhas_Init */
EmitterRiseParams g_piranhaBubbleFxParams = {0}; /* filled in by Piranhas_Init */
extern u32 g_waterColor;
extern Wolf *g_pWolf;
#define SDW_INLINE_FREE_ZONE_GETLIST_U8 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_FREE_ZONE_GETLIST_U8
extern s32 g_dtMs;
s32 Rand_Bounded(s32 maximum);
s32 Vec3s_DistSqXZ(Vec3s *a, Vec3s *b);
s32 Vec3s_Dist(Vec3s *a, Vec3s *b);
s32 Scenaric_FindByClass(u16 classId, ScnObject **out, s32 maximum);

#define SDW_INLINE_FREE_BOX_CONTAINSPOINT_BOX_VEC3S 1
#include "../engine/coll_box_inlines.h"
#undef SDW_INLINE_FREE_BOX_CONTAINSPOINT_BOX_VEC3S

void Piranhas::UpdateSharedFx()
{
    if (g_piranhaFxOwner == this) {
        g_piranhaBubblesShared.base.Emitter_UpdateRiseToCap(&g_piranhaBubbleFxParams, &g_piranhaFxPos,
                                                            g_piranhaBubbleSpawnRequest);
        g_piranhaRingFx.base.Emitter_UpdateFade(&g_piranhaRingFxParams, &g_piranhaFxPos, 0, g_piranhaRingSpawnReq);
        g_piranhaBubbleSpawnRequest = 0;
        g_piranhaRingSpawnReq = 0;
    }
}

void Piranhas::Render(Camera *view)
{
    SetTint(g_waterColor, submerged ? 0x800 : 0, 1);
    AdvanceAnim();
    ScnBody::Render(view);
    if (g_piranhaFxOwner == this) {
        if (g_piranhaBubblesShared.base.flags.active)
            g_piranhaBubblesShared.base.Emitter_Render(view, 0);
        if (g_piranhaRingFx.base.flags.active)
            g_piranhaRingFx.base.Emitter_RenderFlat_Fwd(view);
    }
}

void Piranhas::PickSwimTarget(Vec3s *out)
{
    do {
        out->x = waterBox->min[0] + Rand_Bounded(waterBox->max[0] - waterBox->min[0]);
        out->y = waterBox->min[1] + 200 + Rand_Bounded(200);
        out->z = waterBox->min[2] + Rand_Bounded(waterBox->max[2] - waterBox->min[2]);
    } while (Vec3s_DistSqXZ(&pos, out) < 500000);
}

void Piranhas::PostLoadInit()
{
    struct {
        Vec3s surface;
        u16 pad;
        u16 *props;
    } w;
    w.props = record;
    waterBox = Zone_GetList(ZONE_WATER)->Find(&pos);
    w.surface.x = pos.x;
    w.surface.y = pos.y;
    w.surface.z = pos.z;
    w.surface.y = waterBox->min[1];
    SetPosition(&w.surface);
    g_piranhaBubbleFxParams.riseSpeed = -120;
    g_piranhaBubbleFxParams.life = 0x2000;
    g_piranhaBubbleFxParams.spawnInterval = 0x400;
    g_piranhaBubbleFxParams.size = 10;
    g_piranhaBubbleFxParams.cap = 0;
    g_piranhaBubbleFxParams.sheetIndex = 2;
    g_piranhaRingFxParams.life = 0x2000;
    g_piranhaRingFxParams.fadeStart = 0;
    g_piranhaRingFxParams.spawnInterval = 0x400;
    g_piranhaRingFxParams.sizeStart = 50;
    g_piranhaRingFxParams.sizeEnd = 200;
    g_piranhaRingFxParams.sheetIndex = 1;
    SetUpdateMode(SCN_UPD_ALWAYS);
    SetState(PIRANHAS_ST_SWIM);
    inflatableSheep = 0;
    Scenaric_FindByClass(CLASSID_INFLATABLESHEEP, &inflatableSheep, 1);
    submerged = 1;
    path.count = 2;
    g_piranhaFxOwner = this;
    g_numPiranhasAttacking = 0;
    attackOrder = 0;
    g_piranhaBubbleSpawnRequest = 0;
    g_piranhaRingSpawnReq = 0;
}

void Piranhas::Reset()
{
    submerged = 1;
    g_numPiranhasAttacking = 0;
    g_piranhaBubblesShared.base.Emitter_Reset();
    g_piranhaRingFx.base.Emitter_Reset();
    SetState(PIRANHAS_ST_SWIM);
}

void Piranhas::Update()
{
    struct {
        s32 choice;
        Vec3s delta;
        u16 pad0;
        Vec3s velocity;
        u16 pad1;
        s32 arrived;
        u16 pad2;
        s16 heading;
    } w;
    switch (state) {
        case PIRANHAS_ST_LEAP:
            if (follower.GetPointIndex() == 2 && GetAnimId() != APIRANA1_ANIM_PIRANA3) {
                PlayAnim(APIRANA1_ANIM_PIRANA3, 0, 1);
                submerged = 0;
                g_piranhaRingSpawnReq = 1;
                g_piranhaFxPos.x = pos.x;
                g_piranhaFxPos.y = pos.y;
                g_piranhaFxPos.z = pos.z;
                g_piranhaFxPos.y = waterBox->min[1];
            }
            if (follower.GetPointIndex() == 3 && GetAnimId() != APIRANA1_ANIM_PIRANA2) {
                PlayAnim(APIRANA1_ANIM_PIRANA2, 1, 1);
                submerged = 1;
                g_piranhaRingSpawnReq = 1;
                g_piranhaFxPos.x = pos.x;
                g_piranhaFxPos.y = pos.y;
                g_piranhaFxPos.z = pos.z;
                g_piranhaFxPos.y = waterBox->min[1];
            }
            /* Original fallthrough: continue moving along the leap path. */
        case PIRANHAS_ST_SWIM:
            w.arrived = TrajFollower_Step(&follower, &w.velocity, &w.heading);
            if (w.arrived) {
                w.choice = Rand_Bounded(3);
                if (w.choice == 2)
                    SetState(PIRANHAS_ST_LEAP);
                else
                    SetState(PIRANHAS_ST_SWIM);
            }
            SetFacing(w.heading);
            Vec3s_ScaleByDt(&w.velocity, &w.delta);
            Translate(&w.delta);
            if (Rand_Bounded(2) == 1 && !g_piranhaRingSpawnReq) {
                g_piranhaBubbleSpawnRequest = 1;
                g_piranhaFxPos.x = pos.x;
                g_piranhaFxPos.y = pos.y;
                g_piranhaFxPos.z = pos.z;
            }
            break;
        case PIRANHAS_ST_ATTACK_WAIT:
            attackTimerMs -= g_dtMs;
            if (attackTimerMs <= 0)
                SetState(PIRANHAS_ST_BITE);
            break;
        case PIRANHAS_ST_BITE:
            SetPosition(&g_pWolf->pos);
            break;
    }
    if (state != PIRANHAS_ST_BITE && state != PIRANHAS_ST_ATTACK_WAIT) {
        if (Box_ContainsPoint(waterBox, &g_pWolf->pos) && !g_pWolf->HandleMessage(this, MSG_WOLF_IS_DEAD, 0) &&
            g_pWolf->HandleMessage(this, MSG_WOLF_IS_IN_WATER_STATE, 0) && g_numPiranhasAttacking < 1)
            SetState(PIRANHAS_ST_ATTACK_WAIT);
    }
    if (inflatableSheep) {
        if (Box_ContainsPoint(waterBox, &inflatableSheep->pos))
            inflatableSheep->HandleMessage(this, MSG_KILL, (void *)KILL_PIRANHAS);
    }
    UpdateSharedFx();
}

sptr Piranhas::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    if (msgId == MSG_FREEZE)
        return 1;
    return 0;
}

void Piranhas::SetState(u8 newState)
{
    struct {
        s32 distance, skimDistance;
        s32 sx, sy, sz;
        s32 dx, dy, dz;
        Vec3s target;
        u16 pad;
    } w;
    switch (newState) {
        case PIRANHAS_ST_SWIM:
            submerged = 1;
            PlayAnim(APIRANA1_ANIM_PIRANA2, 1, 1);
            PickSwimTarget(&w.target);
            path.pts[0].x = pos.x;
            path.pts[0].y = pos.y;
            path.pts[0].z = pos.z;
            path.pts[1].x = w.target.x;
            path.pts[1].y = w.target.y;
            path.pts[1].z = w.target.z;
            path.count = 2;
            TrajFollower_Init(&follower, &path, 500, 0x800, 0, 1, 50);
            break;
        case PIRANHAS_ST_ATTACK_WAIT:
            g_numPiranhasAttacking++;
            attackOrder = g_numPiranhasAttacking;
            submerged = 0;
            g_pWolf->HandleMessage(this, MSG_FREEZE, 0);
            attackTimerMs = 1000;
            break;
        case PIRANHAS_ST_BITE:
            SetPosition(&g_pWolf->pos);
            SetRotation(&g_pWolf->rot);
            switch (attackOrder) {
                case 1:
                    PlayAnim(APIRANA1_ANIM_PIRANA1, 0, 0);
                    g_pWolf->HandleMessage(this, MSG_KILL, (void *)KILL_PIRANHAS);
                    break;
            }
            break;
        case PIRANHAS_ST_LEAP:
            PlayAnim(APIRANA1_ANIM_PIRANA2, 1, 1);
            PickSwimTarget(&w.target);
            path.pts[0].x = pos.x;
            path.pts[0].y = pos.y;
            path.pts[0].z = pos.z;
            path.pts[1].x = pos.x;
            path.pts[1].y = pos.y;
            path.pts[1].z = pos.z;
            path.pts[2].x = pos.x;
            path.pts[2].y = pos.y;
            path.pts[2].z = pos.z;
            path.pts[3].x = w.target.x;
            path.pts[3].y = w.target.y;
            path.pts[3].z = w.target.z;
            w.dx = w.target.x;
            w.dy = w.target.y;
            w.dz = w.target.z;
            w.dx -= pos.x;
            w.dy -= pos.y;
            w.dz -= pos.z;
            w.distance = Vec3s_Dist(&w.target, &pos);
            w.skimDistance = (w.distance - 500) / 2;
            w.sx = (s16)w.dx;
            w.sy = (s16)w.dy;
            w.sz = (s16)w.dz;
            w.sx *= w.skimDistance;
            w.sy *= w.skimDistance;
            w.sz *= w.skimDistance;
            w.sx /= w.distance;
            w.sy /= w.distance;
            w.sz /= w.distance;
            path.pts[1].x += (s16)w.sx;
            path.pts[1].y += (s16)w.sy;
            path.pts[1].z += (s16)w.sz;
            w.sx = (s16)w.dx;
            w.sy = (s16)w.dy;
            w.sz = (s16)w.dz;
            w.sx *= w.skimDistance + 500;
            w.sy *= w.skimDistance + 500;
            w.sz *= w.skimDistance + 500;
            w.sx /= w.distance;
            w.sy /= w.distance;
            w.sz /= w.distance;
            path.pts[2].x += (s16)w.sx;
            path.pts[2].y += (s16)w.sy;
            path.pts[2].z += (s16)w.sz;
            path.pts[1].y = waterBox->min[1] + 50;
            path.pts[2].y = waterBox->min[1] + 50;
            path.count = 4;
            TrajFollower_Init(&follower, &path, 500, 0x800, 0, 1, 50);
            break;
    }
    state = newState;
}

ScnObject *Piranhas_Create(void *record)
{
    Piranhas *obj = new Piranhas;
    obj = (Piranhas *)obj->Init(record, 0);
    return obj;
}
