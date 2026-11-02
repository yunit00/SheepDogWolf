/* g_pFireBall is defined as ScnObject *, the type its three readers declare, so the decorated names agree at link. */
#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
#include "../sdk/win32.h"
#include "../sdk/d3d7.h"
#include "../sdk/ddraw.h"
class Mat44;
#include "../sdk/crt.h"

struct FlameVertex {
    float x, y, z, rhw;
    u32 diffuse, specular;
    float u, v;
};
struct FlatVertex {
    float x, y, z, rhw;
    u32 diffuse, specular;
};
struct XformedVertex {
    float x, y, z, rhw;
};

#define SDW_MEMBERS_Mat44 Mat44();
#define SDW_MEMBERS_PolyBatcher    \
    void SubmitPoly(RenderPoly *); \
    void SubmitPolyInline(RenderPoly *);
#define SDW_MEMBERS_D3DApp                                        \
    void SetTransform(u32 state, Mat44 *m);                       \
    IDirect3DDevice7 *GetDevice();                                \
    void DrawTriangleList(void *vertices, s32 count);             \
    void DrawTexturedTriangleList(void *vertices, s32 count);     \
    void Render_SetTexture(Texture *texture, volatile u32 stage); \
    void Render_SetStateFlags(u32);                               \
    void Render_ClearStateFlags(u32);                             \
    void ClearStateFlagsInline(u32 flags);

class Instance;
struct Animator;
u32 Anim_Start(Instance *, Animator *, u16, u32);
#define SDW_MEMBERS_ScnObject       \
    static void *operator new(uptr); \
    void SetUpdateMode(s32 mode);

#define SDW_MEMBERS_RenderPoly RenderPoly(); /* virtual ~RenderPoly() is generated */
#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_D3DAPP_SETTRANSFORM_U32_MAT44 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_SETTRANSFORM_U32_MAT44
#define SDW_INLINE_D3DAPP_GETDEVICE 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_GETDEVICE
#define SDW_INLINE_D3DAPP_DRAWTRIANGLELIST_VOID_S32 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_DRAWTRIANGLELIST_VOID_S32
#define SDW_INLINE_D3DAPP_DRAWTEXTUREDTRIANGLELIST_VOID_S32 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_DRAWTEXTUREDTRIANGLELIST_VOID_S32
#define SDW_INLINE_D3DAPP_CLEARSTATEFLAGSINLINE_U32 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_CLEARSTATEFLAGSINLINE_U32
#define SDW_INLINE_SCNOBJECT_ISACTIVE 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_ISACTIVE
#define SDW_INLINE_SCNOBJECT_INSTFLAGS_U16 1
#define SDW_INLINE_SCNOBJECT_SETVISIBLE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_INSTFLAGS_U16
#undef SDW_INLINE_SCNOBJECT_SETVISIBLE_S32
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32
#define SDW_INLINE_SCNBODY_ANIMFLAGS_U16 1
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_ANIMFLAGS_U16
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32
/* It qualifies only this private parameter, not the device or texture data. */
inline void D3DApp::Render_SetTexture(Texture *texture, volatile u32 stage)
{
    SDW_RD(pD3DDevice)->SetTexture(stage, SDW_RDTEX(texture->surface));
}
extern Wolf *g_pWolf;
ScnObject *g_pFireBall;
#include "../engine/maths.h"
#include "../engine/draw2d.h"
#include "../engine/screen.h"
#include "../engine/load_warmeshes.h"
void FireBall::PostLoadInit()
{
    flameSprite.InitFromRes(DAV_IDI_IFBSMOK_);
    boundRadius = 600;
    Reset();
}
void FireBall::Reset()
{
    SetState(FIREBALL_ST_OFF);
}
void FireBall::Update()
{
    if (state == FIREBALL_ST_BURN && AnimFlags(ANIM_F_FINISHED))
        SetState(FIREBALL_ST_OFF);
    AdvanceAnim();
}
sptr FireBall::HandleMessage(ScnObject *, u32 msg, void *arg)
{
    switch (msg) {
        case MSG_FIREBALL_STRIKE:
            if (state == FIREBALL_ST_OFF && !g_pWolf->HandleMessage(this, MSG_WOLF_IS_DEAD, 0)) {
                ScnObject *victim = (ScnObject *)arg;
                victim->HandleMessage(this, MSG_KILL, (void *)KILL_GENERIC);
                Vec3s point = victim->pos;
                point.y -= 600;
                SetPosition(&point);
                SetState(FIREBALL_ST_BURN);
                return 1;
            }
    }
    return 0;
}
void FireBall::SetState(u8 next)
{
    state = next;
    switch (state) {
        case FIREBALL_ST_OFF:
            PlayAnim(AFANTB02_ANIM_STAND, 0, 0);
            if (IsActive())
                SetVisible(0);
            SetUpdateMode(SCN_UPD_NEVER);
            break;
        case FIREBALL_ST_BURN:
            flameTick = 0;
            if (!IsActive())
                SetVisible(1);
            PlayAnim(AFANTB02_ANIM_KILL, 1, 0);
            SetUpdateMode(SCN_UPD_ALWAYS);
            for (u8 i = 0; i < 3; ++i) {
                flameOffsets[i].x = (s16)Rand_Range(20, 20);
                flameOffsets[i].y = (s16)Rand_Range(500, 580);
                flameOffsets[i].z = (s16)Rand_Range(20, 20);
            }
    }
}
void FireBall::Render(Camera *view)
{
    RenderFacingCamera(view, 1, 0, 0);
    if (InstFlags(INST_F_DRAWN))
        RenderFlames(view);
}
ScnObject *FireBall_Create(void *record)
{
    FireBall *object = new FireBall;
    object = (FireBall *)object->Init(record, 0);
    g_pFireBall = object;
    return object;
}
RenderPoly g_fireBallFlamePoly;

extern SdwVertexBuffer *g_pVertexBufSrc, *g_pVertexBufXf;

#define g_screenW (g_screen.viewportWidth)

#define g_screenH (g_screen.viewportHeight)

/* The original first SubmitPoly is inline (including its flat-batch clear);
   the second submission stays out of line. The complete dispatcher matters. */
#define SDW_INLINE_POLYBATCHER_SUBMITPOLYINLINE_RENDERPOLY 1
#include "../engine/polybatcher_inlines.h"
#undef SDW_INLINE_POLYBATCHER_SUBMITPOLYINLINE_RENDERPOLY

void FireBall::RenderFlames(Camera *view)
{
    FlameVertex *poly;
    {
        void *locked;
        {
            Vec3f *points;
            {
                SpriteFrame *frame;
                {
                    XformedVertex *transformed;
                    {
                        u32 lockSize;
                        {
                            u32 i;
                            {
                                Mat44 world;
                                if (!flameSprite.frameCount)
                                    return;
                                ++flameTick;
                                if (flameTick / 2 >= flameSprite.frameCount) {
                                    flameTick = 0;
                                    for (i = 0; i < 3; ++i) {
                                        flameOffsets[i].x = (s16)Rand_Range(-100, 100);
                                        flameOffsets[i].y = (s16)Rand_Range(400, 580);
                                        flameOffsets[i].z = (s16)Rand_Range(-100, 100);
                                    }
                                }
                                g_pVertexBufSrc->Lock(DDLOCK_WAIT, &locked, &lockSize);
                                points = (Vec3f *)locked;
                                for (i = 0; i < 3; ++i) {
                                    points[i].x = (float)(pos.x + flameOffsets[i].x);
                                    points[i].y = (float)(pos.y + flameOffsets[i].y);
                                    points[i].z = (float)(pos.z + flameOffsets[i].z);
                                }
                                g_pVertexBufSrc->Unlock();
                                world.SetIdentity();
                                g_pD3DAppMain->SetTransform(D3DTRANSFORMSTATE_WORLD, &world);
                                g_pD3DAppMain->SetTransform(D3DTRANSFORMSTATE_VIEW, &view->viewMat);
                                g_pVertexBufXf->ProcessVertices(D3DVOP_TRANSFORM, 0, 3, g_pVertexBufSrc, 0,
                                                                SDW_RD(g_pD3DAppMain->GetDevice()), D3DPV_DONOTCOPYDATA);
                                g_pVertexBufXf->Lock(DDLOCK_WAIT, &locked, &lockSize);
                                frame = &flameSprite.frames[flameTick % flameSprite.frameCount];
                                transformed = (XformedVertex *)locked;
                                poly = (FlameVertex *)g_fireBallFlamePoly.verts;
                                g_fireBallFlamePoly.type = frame->texPage + RPOLY_TEXTURED_BASE;
                                poly[0].diffuse = poly[1].diffuse = poly[2].diffuse = 0x808080;
                                poly[0].specular = poly[1].specular = poly[2].specular = 0xff000000;
                                /* The original does not advance transformed: each pass reuses vertex 0. */
                                for (i = 0; i < 3; ++i) {
                                    float width = 30.0f * g_projMatrix.m[0][0] * transformed->rhw * (s32)g_screenW;
                                    float height = -30.0f * g_projMatrix.m[1][1] * transformed->rhw * (s32)g_screenH;
                                    poly[0].z = poly[1].z = poly[2].z = transformed->z;
                                    poly[0].rhw = poly[1].rhw = poly[2].rhw = transformed->rhw;
                                    poly[0].x = transformed->x;
                                    poly[0].y = transformed->y;
                                    poly[1].x = transformed->x;
                                    poly[1].y = height + transformed->y;
                                    poly[2].x = width + transformed->x;
                                    poly[2].y = transformed->y;
                                    poly[0].u = Tex_CornerUV(0, flameSprite.width, frame->u);
                                    poly[0].v = Tex_CornerUV(0, flameSprite.height, frame->v);
                                    poly[1].u = Tex_CornerUV(0, flameSprite.width, frame->u);
                                    poly[1].v = Tex_CornerUV(flameSprite.height, flameSprite.height, frame->v);
                                    poly[2].u = Tex_CornerUV(flameSprite.width, flameSprite.width, frame->u);
                                    poly[2].v = Tex_CornerUV(0, flameSprite.height, frame->v);
                                    g_pPolyBin->SubmitPolyInline(&g_fireBallFlamePoly);
                                    poly[0].x = width + poly[0].x;
                                    poly[0].y = height + poly[0].y;
                                    poly[0].u = Tex_CornerUV(flameSprite.width, flameSprite.width, frame->u);
                                    poly[0].v = Tex_CornerUV(flameSprite.height, flameSprite.height, frame->v);
                                    g_pPolyBin->SubmitPoly(&g_fireBallFlamePoly);
                                }
                                g_pVertexBufXf->Unlock();
                            }
                        }
                    }
                }
            }
        }
    }
}
