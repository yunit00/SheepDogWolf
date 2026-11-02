/*
 * The class LAYOUTS come from the generated src/include/sdw_classes.h; this header only adds members through its hooks.
 *
 * Include this INSTEAD of sdw_classes.h. A Wolf source file that needs members not declared here adds them with
 * SDW_EXTRA_<Class> (e.g. SDW_EXTRA_Wolf) defined before the include. Where the Wolf files' declarations differ, the
 * defining file's is used.
 */
#ifndef SDW_WOLF_H
#define SDW_WOLF_H


#define SDW_MEMBERS_ScnObject                                                                                    \
    static void *operator new(uptr size); /* Scenaric_Alloc: every class factory's `new` (level heap) */ \
    void AttachTo(ScnObject *parent, u8 joint, Vec3s *offset, Vec3s *rot, u32 arg, uptr arg2);     \
    CollBox *GetFirstModelBox();                                                                                 \
    void SetFacing(s16 f);                       \
    AttachLink *GetAttachLink()                                                                                  \
    {                                                                                                            \
        return attachLink;                                                                                       \
    } /* inline: a pointer copy */


#define SDW_MEMBERS_ParticleEmitter                                                                                   \
    ParticleEmitter();                                    /* inline (defined below): Emitter_Init(0), no pools yet */ \
    ParticleEmitter(Vec3f *slots, Particle *parts, u8 n); /* inline (defined below): the pools are given */
#define SDW_MEMBERS_TrailEmitter TrailEmitter();          /* inline (defined below): the pools are the inline buffers */
#define SDW_MEMBERS_WolfLaunchPath s32 Step(WolfArcScratch *s, s32 dt); /* inline, defined below */
#define SDW_MEMBERS_Wolf                                                                                                                                                                                                   \
    const s16 *SurfaceTuning(); /* inline accessor, defined below */                                                                                                                                                       \
    /* inline: clear flag bits; the constant mask is loaded into a register and complemented there */ /* which a written-out `flags &= ~mask` does not give */ \
    void ClearFlags(u32 mask)                                                                                                                                                                                              \
    {                                                                                                                                                                                                                      \
        flags &= ~mask;                                                                                                                                                                                                    \
    }                                                                                                                                                                                                                      \
    void ClearFxFlags(u32 mask)                                                                                                                                                                                            \
    {                                                                                                                                                                                                                      \
        fxFlags &= ~mask;                                                                                                                                                                                                  \
    }

#include "sdw_types.h"
#include "sdw_enums.h"
class Instance;
struct Animator;
u32 Anim_Start(Instance *inst, Animator *animator, u16 id, u32 opts); /* (used by the PlayAnim inline) */
#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "../objects/instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNOBJECT_FACING 1
#define SDW_INLINE_SCNOBJECT_SETFACING_S16 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_FACING
#undef SDW_INLINE_SCNOBJECT_SETFACING_S16
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
#define SDW_INLINE_SCNBODY_ANIMID 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_ANIMID
#define SDW_INLINE_SCNBODY_ANIMFLAGS_U16 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_ANIMFLAGS_U16
#define SDW_INLINE_SCNMOBILE_GROUNDY 1
#define SDW_INLINE_SCNMOBILE_SETSHADOWRADIUS_U8 1
#include "../engine/scn_mobile_inlines.h"
#undef SDW_INLINE_SCNMOBILE_GROUNDY
#undef SDW_INLINE_SCNMOBILE_SETSHADOWRADIUS_U8
#define SDW_INLINE_ALTMODEL_ISVALID 1
#include "../engine/alt_model_inlines.h"
#undef SDW_INLINE_ALTMODEL_ISVALID
#include "sdw_global_views.h"
#include "scenaric_props.h"

/* ---- callees ---- */
/* engine/scn_tools.cpp, engine/approach.cpp, engine/maths.cpp */
#include "../engine/scn_tools.h"
#include "../engine/approach.h"
#include "../engine/maths.h"
#include "scn_controllable.h"
#include "../engine/fixed_math.h"
#include "../objects/world_draw.h"
#include "../engine/scenaric.h"
#include "../engine/sound_mgr.h"
#include "../objects/camera.h"
#include "../engine/progress_inventory.h"
#include "../engine/fade.h"
#include "../engine/map.h"
#include "../engine/input.h"
#include "wolf_move.h"
#include "wolf_api.h"
#include "../app/app_main.h"
#include "../engine/game_state.h"
s32 Rand_Bounded(s32 bound);
/* game/scn_controllable.cpp */
/* engine/fixed_math.cpp, then the CRT */
extern "C" s16 Math_RadiansToAngle4096(float radians);
#include "../sdk/crt.h"
u16 Sound_Play(u16 soundId, void *owner, u16 volume, u8 flags, s32 rate);

/* ---- globals ---- */
extern Wolf *g_pWolf;
extern u8 g_sharedScratch[];          /* shared scratch; the move steps keep their vectors there */
extern const u16 g_wolfAltModelIds[]; /* {4, 64, 99, 124, 154} */
extern const IdleAnimEntry g_wolfGhostIdleAnims[]; /* ghost-costume idle anims (the tail of the table at
                                                     *): animId, then the idle count's range */
extern const Vec3s g_wolfPutDownOffsetNear;        /* (0,-2,0) */
extern const Vec3s g_wolfPutDownOffsetFront;       /* (0,-70,-92) */
extern const Vec3s g_wolfEquipPreviewOffset;       /* (0,-100,0) */
extern const u8 g_wolfClimbDirTable[]; /* climb-box direction (flags bits 27-30) -> heading / 0x200 */
extern "C" s16 g_sinTable4096[5122];   /* 4.12 sine, 4096 steps per turn */

#define g_padMasks (g_inputMap + 4) /* active-low button masks: [6] inventory 0xfbff, [8] view 0xefff */
extern "C" const s16 *g_pCosTable;  /* = g_sinTable4096 + 1024 */
extern u32 g_gameFlags;
extern u32 g_gameTime;              /* (u32 here: the Wolf code compares it unsigned) */
extern s32 g_dtMs;                  /* g_dt * 1000 >> 12 */
extern s32 g_dt;

/* The two camera flag bytes are bitfields. */

extern CamScriptState g_camScriptState;
#define g_camScriptReturnMode (g_camScriptState.returnMode)
/* the camera request the Wolf fills every frame (read by Camera_Update) */


#define SDW_ABS(v) ((v) >= 0 ? (v) : -(v))
/* signed difference a - b of two headings, in -2048..2047 */
#define SDW_ANGLE_DIFF(a, b) ((s16)((s16)(((a) - (b) + 2048) & 4095) - 2048))

/* ---- inline helpers ---- */

/* The tuning row of the current bank and surface. About 60 spellings of the direct expression were tried; none reverses
 * the order. */
inline const s16 *Wolf::SurfaceTuning()
{
    return g_wolfMoveBank0[mode].surfaceTuning[surface];
}

/* Whether the level has any zone of a type (a ZoneType, index into the seven lists; ZONE_PRINTS = footprints): the
 * constant type is loaded into a register and scaled in the address ( ; a narrow parameter type gives the scaled
 * form, int gives shl), and the comparison is materialised with setg before the test, which is what an inlined
 * `return count > 0` compiles to (also ZONE_DEATH, ZONE_SHADOW). */
inline s32 Zones_Exist(u8 type)
{
    return g_waterZones[type].count > 0;
}

/* One of the seven global zone lists by ZoneType (ZONE_WATER first); the constant type is loaded into a register first,
 * as in Zones_Exist. */
#define SDW_INLINE_FREE_ZONES_GET_U8 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_FREE_ZONES_GET_U8

#define SDW_INLINE_ZONELIST_FINDCONTAINING_VEC3S 1
#include "../engine/zone_list_inlines.h"
#undef SDW_INLINE_ZONELIST_FINDCONTAINING_VEC3S

/* One step along the launch trajectory (msg 0xC): while t < 0x100 advance t by dt (capped at 0x100) and put the point of
 * the per-axis quadratic p0 + (v * t >> 8) + (a * t * t >> 15) in s->target; returns whether it was still on the path. So
 * the inline takes the scratch pointer. */
inline s32 WolfLaunchPath::Step(WolfArcScratch *s, s32 dt)
{
    s32 t2;
    if (t < 0x100) {
        t = dt + t;
        if (t > 0x100)
            t = 0x100;
        t2 = t * t;
        s->target.x = x.p0 + ((t * x.v) >> 8) + ((t2 * x.a) >> 15);
        s->target.y = y.p0 + ((t * y.v) >> 8) + ((t2 * y.a) >> 15);
        s->target.z = z.p0 + ((t * z.v) >> 8) + ((t2 * z.a) >> 15);
        return 1;
    }
    return 0;
}

/* the same pair in ScnControllable_ScanInteractables, where these two were first found). */
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID

#define SDW_INLINE_FREE_SCENARIC_CLASSFLAGS_U16 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_FREE_SCENARIC_CLASSFLAGS_U16

/* The first box of the model's box list, or 0 without a list (the count is not checked).
 * ScnObject_GetFirstModelBox is an out-of-line copy. */
inline CollBox *ScnObject::GetFirstModelBox()
{
    ModelBoxList *list = inst_model->boxes;
    if (list)
        return list->boxes;
    return 0;
}

#define SDW_INLINE_FREE_BOX_CONTAINSPOINT_BOX_VEC3S 1
#include "../engine/coll_box_inlines.h"
#undef SDW_INLINE_FREE_BOX_CONTAINSPOINT_BOX_VEC3S

/* Whether the camera runs a scripted mode (CAM_SCRIPTED .. CAM_SCRIPT_TO_SCRIPT): its value is materialised in a
 * temporary before the test
 *. */
inline s32 Camera_IsScriptedOrReturning()
{
    return g_camMode == CAM_SCRIPT_BLEND_IN || g_camMode == CAM_SCRIPT_TO_SCRIPT || g_camMode == CAM_SCRIPTED ||
           g_camMode == CAM_SCRIPT_RETURN;
}

/* The emitter constructors, inlined into the factories. A plain ParticleEmitter starts with no pools (Emitter_Init(0));
 * the trail kind is a ParticleEmitter `base` followed by inline buffers for 16 particles, and constructs the base with
 * them (pools, count, Emitter_Reset - no Emitter_Init). DefusableMine_Create has the same inline shape for 1
 * particle, so the original was probably one template over the slot count. */
inline ParticleEmitter::ParticleEmitter()
{
    Emitter_Init(0);
}

inline ParticleEmitter::ParticleEmitter(Vec3f *slots, Particle *parts, u8 n)
{
    slotPool = slots;
    particles = parts;
    count = n;
    Emitter_Reset();
}

inline TrailEmitter::TrailEmitter() : base(slotBuf, particleBuf, 16) {}

#endif
