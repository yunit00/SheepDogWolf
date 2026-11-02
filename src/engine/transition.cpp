#include "sdw_enums.h"
#include "../engine/timer.h"
#include "../sdk/d3d7.h"
#include "../sdk/crt.h"
#define SDW_MEMBERS_Vec3f \
    Vec3f() {} /* empty but user-declared: the two empty static initialisers */
#define SDW_MEMBERS_Mat44 Mat44();
#define SDW_MEMBERS_RenderPoly RenderPoly(); /* RenderPoly_Ctor */
#define SDW_MEMBERS_D3DApp                                               \
    void Render_SetStateFlags(u32 flags);               \
    void Render_ClearStateFlags(u32 flags);               \
    void DrawPrimitiveInline(u32 type, u32 fvf, void *verts, u32 count); \
    void SetTextureInline(Texture *texture, volatile u32 stage);         \
    void ClearInline();                                                  \
    void BeginSceneInline();
#define SDW_MEMBERS_PolyBatcher                       \
    void SubmitPoly(RenderPoly *poly); \
    void SubmitPolyInline(RenderPoly *poly);
#define SDW_MEMBERS_HoleFX HoleFX();
#include "sdw_classes.h"

/* The DAV directory (DavHeader.dir): packed, its pointers sit at +6/+0xa/+0x12, which the struct generator cannot lay
 * out, so it is declared here (as in src/objects/lightspot.cpp and the other DAV readers). */
#include "sdw_fileptr.h"
#pragma pack(push, 1)
struct DavDirectory {
    u16 indexCount;
    u16 bitmapCount;
    u16 unk04;
    SDW_DAVPTR(u16) indices;
    SDW_DAVPTR(DavBitmapRec) bitmaps; /* 10-byte records */
    u32 fileSize;
    SDW_DAVPTR(u32) idLists;
};
#pragma pack(pop)

#include "fixed_math.h"
#include "load_warmeshes.h"
#include "scenaric.h"
#include "game_state.h"
#include "screen.h"
#include "draw2d.h"
#include "stream_player.h"
#include "progress.h"
#include "../platform/platform.h"
void Scenaric_RegisterClass_2(u16 classId, ScnObject *(*factory)(void *), u32 classFlags, u16 iconIdA, u16 iconIdB);

extern u32 g_gameFlags;

/* ---- the level transition: the HoleFX iris ----
 * Two modes, chosen by Transition_Init from whether HoleFX got its capture surface: non-blocking (the game keeps
 * running and each frame's Transition_Update draws one step of the iris) and blocking (the whole zoom runs inside one
 * call, with its own Clear / BeginScene / EndFrame / Present loop). Each phase lasts 3 seconds of g_transitionTimer. */

/* The static initialisers (Timer, with its atexit destructor), (HoleFX, likewise), and
 * (the two Vec3f, whose empty constructors compile to nothing) come from these definitions, in this order. */
Timer g_transitionTimer; /* bucket 940 */
HoleFX g_holeFX;         /* bucket 767 */
Vec3f
    g_transitionCenterPos; /* bucket 820  (g_transitionBasePos) the iris start: viewport centre at the near plane */
Vec3f
    g_transitionDelta; /* bucket 345  per-axis end-minus-start, times 2/9 (so delta * t*t/2 is the whole move at t = 3) */
/* Zero-initialised, so their type is unknowable: opaque words. */
u32 g_transitionUnk6D4120[2]; /* bucket 74 */
/* g_transitionAlphaOrigin and g_transIrisAlphaDelta share bucket 948: inside a bucket the LATER definition gets the
 * lower address, so the base is defined first. */
float g_transitionAlphaOrigin; /* bucket 948  (g_transitionAlphaBase) */
float g_transIrisAlphaDelta;   /* bucket 948  (g_transitionAlphaDelta) */
u8 g_transIrisPhase;           /* bucket 959  (g_transitionPhase) 0 idle, 1 out, 2 in */
u8 g_transPhaseRunning;        /* bucket 974  (g_transitionPhaseStarted) */
u8 g_transitionIsNonBlocking;  /* bucket 990  (g_transitionNonBlocking) */

#define SDW_INLINE_FREE_GAME_CLEARFLAGS_U32 1
#include "game_state_inlines.h"
#undef SDW_INLINE_FREE_GAME_CLEARFLAGS_U32

inline void D3DApp::ClearInline()
{
    SDW_RD(pD3DDevice)->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
}
inline void D3DApp::BeginSceneInline()
{
    SDW_RD(pD3DDevice)->BeginScene();
}

/* Phase 1, the iris closing on the old level. Non-blocking: one step per call; at t = 3 s the phase ends with
 * GF_UPDATE_OBJECTS | GF_RENDER_WORLD set. Blocking: runs the whole close in a render loop of its own with fog off. */
void Transition_OutUpdate()
{
    float kk;
    float now;
    u8 savedFog;
    float tt;
    float k;

    if (g_transitionIsNonBlocking == 1) {
        if (!g_transPhaseRunning) {
            g_transitionTimer.Start();
            Game_ClearFlags(GF_BIT0 | GF_UPDATE_OBJECTS);
            tt = 0.0f;
            g_transPhaseRunning = 1;
        }
        tt = g_transitionTimer.GetElapsed(TIMER_SECONDS);
        if (tt > 3.0f)
            tt = 3.0f;
        k = tt * tt / 2.0f;
        g_holeFX.pos[0] = g_transitionCenterPos.x;
        g_holeFX.pos[1] = g_transitionCenterPos.y;
        g_holeFX.pos[2] = g_transitionDelta.z * k + g_transitionCenterPos.z;
        g_holeFX.alpha = g_transitionAlphaOrigin;
        g_holeFX.UpdateVertices(0);
        g_holeFX.Draw(g_pPolyBin);
        if (tt == 3.0f) {
            g_transitionTimer.Stop();
            g_gameFlags |= GF_UPDATE_OBJECTS | GF_RENDER_WORLD;
            g_transPhaseRunning = 0;
            g_transIrisPhase = TRANSITION_IN;
        }
    } else {
        g_holeFX.CaptureScreen(0, 1);
        savedFog = g_pViewFrustum->fogEnabled;
        g_pViewFrustum->EnableFog(0);
        g_transitionTimer.Start();
        now = 0.0f;
        do {
            g_pD3DAppMain->ClearInline();
            g_pD3DAppMain->BeginSceneInline();
            kk = now * now / 2.0f;
            g_holeFX.pos[0] = g_transitionDelta.x * kk + g_transitionCenterPos.x;
            g_holeFX.pos[1] = g_transitionDelta.y * kk + g_transitionCenterPos.y;
            g_holeFX.pos[2] = g_transitionDelta.z * kk + g_transitionCenterPos.z;
            if (now >= 2.4f)
                kk = (now - 2.4f) * (now - 2.4f) / 2.0f;
            else
                kk = 0.0f;
            g_holeFX.alpha = g_transIrisAlphaDelta * kk + g_transitionAlphaOrigin;
            if (g_holeFX.alpha < 0.0f)
                g_holeFX.alpha = 0.0f;
            g_holeFX.UpdateVertices(0);
            g_holeFX.Draw(g_pPolyBin);
            g_pPolyBin->Render_EndFrame(1);
            g_pPolyBin->Render_Present(60);
            Platform_PumpEvents(); /* 3 s a phase outside the main loop: the window stays responsive */
            now = g_transitionTimer.GetElapsed(TIMER_SECONDS);
        } while (g_holeFX.alpha != 0.0f);
        g_transIrisPhase = TRANSITION_IN;
        g_pViewFrustum->EnableFog(savedFog);
        g_transitionTimer.Stop();
    }
}

/* Phase 2, the iris opening on the new level: the same curves run backwards (tt = 3 - elapsed). */
void Transition_InUpdate()
{
    float kk;
    u8 savedFog;
    float tt;
    float k;
    float time;

    if (g_transitionIsNonBlocking == 1) {
        if (!g_transPhaseRunning) {
            g_transitionTimer.Start();
            Game_ClearFlags(GF_BIT0 | GF_UPDATE_OBJECTS);
            tt = 0.0f;
            g_transPhaseRunning = 1;
        }
        tt = 3.0f - (float)g_transitionTimer.GetElapsed(TIMER_SECONDS);
        if (tt < 0.0f)
            tt = 0.0f;
        k = tt * tt / 2.0f;
        g_holeFX.pos[0] = g_transitionCenterPos.x;
        g_holeFX.pos[1] = g_transitionCenterPos.y;
        g_holeFX.pos[2] = g_transitionDelta.z * k + g_transitionCenterPos.z;
        g_holeFX.alpha = g_transitionAlphaOrigin;
        g_holeFX.UpdateVertices(0);
        g_holeFX.Draw(g_pPolyBin);
        if (tt == 0.0f) {
            g_transitionTimer.Stop();
            g_gameFlags |= GF_UPDATE_OBJECTS | GF_RENDER_WORLD;
            g_transPhaseRunning = 0;
            g_transIrisPhase = TRANSITION_IDLE;
        }
    } else {
        g_holeFX.CaptureScreen(1, 1);
        savedFog = g_pViewFrustum->fogEnabled;
        g_pViewFrustum->EnableFog(0);
        g_transitionTimer.Start();
        time = 3.0f;
        do {
            g_pD3DAppMain->ClearInline();
            g_pD3DAppMain->BeginSceneInline();
            kk = time * time / 2.0f;
            g_holeFX.pos[0] = g_transitionDelta.x * kk + g_transitionCenterPos.x;
            g_holeFX.pos[1] = g_transitionDelta.y * kk + g_transitionCenterPos.y;
            g_holeFX.pos[2] = g_transitionDelta.z * kk + g_transitionCenterPos.z;
            if (time >= 2.4f)
                kk = (time - 2.4f) * (time - 2.4f) / 2.0f;
            else
                kk = 0.0f;
            g_holeFX.alpha = g_transIrisAlphaDelta * kk + g_transitionAlphaOrigin;
            g_holeFX.UpdateVertices(0);
            g_holeFX.Draw(g_pPolyBin);
            g_pPolyBin->Render_EndFrame(1);
            g_pPolyBin->Render_Present(60);
            Platform_PumpEvents(); /* 3 s a phase outside the main loop: the window stays responsive */
            time = 3.0f - (float)g_transitionTimer.GetElapsed(TIMER_SECONDS);
        } while (time >= 0.0f);
        g_transIrisPhase = TRANSITION_IDLE;
        g_pViewFrustum->EnableFog(savedFog);
        g_transitionTimer.Stop();
    }
}

/* Called by the level loader: sets up g_holeFX (radius = viewport height / 8, cubic curves) and picks the
 * mode. Both modes start the iris at the viewport centre on the near plane; the blocking one also moves it in x / y
 * toward (w/10, h/6) and to 10 x near, and fades alpha from 1 at 50/9 per unit of (tt - 2.4)^2 / 2. */
void Transition_Init()
{
    float tx;
    float endY;
    float tz;

    g_holeFX.Init(g_pD3DAppMain, g_pViewFrustum, g_pViewFrustum->viewportHeight / 8.0f, 1);
    g_transitionIsNonBlocking = (s32)(g_holeFX.hasCaptureSurface == HOLEFX_MASK);
    if (g_transitionIsNonBlocking == 1) {
        g_transitionCenterPos.x = g_pViewFrustum->viewportWidth / 2.0f;
        g_transitionCenterPos.y = g_pViewFrustum->viewportHeight / 2.0f;
        g_transitionCenterPos.z = g_pViewFrustum->nearZ;
        g_transitionAlphaOrigin = 1.0f;
        g_transitionDelta.z = (g_pViewFrustum->farZ - g_transitionCenterPos.z) * 2.0f / 9.0f;
    } else {
        g_holeFX.SetShading(12.0f, -0.3f);
        g_transitionCenterPos.x = g_pViewFrustum->viewportWidth / 2.0f;
        g_transitionCenterPos.y = g_pViewFrustum->viewportHeight / 2.0f;
        g_transitionCenterPos.z = g_pViewFrustum->nearZ;
        g_transitionAlphaOrigin = 1.0f;
        tx = g_pViewFrustum->viewportWidth / 10.0f;
        endY = g_pViewFrustum->viewportHeight / 6.0f;
        tz = g_pViewFrustum->nearZ * 10.0f;
        g_transitionDelta.x = (tx - g_transitionCenterPos.x) * 2.0f / 9.0f;
        g_transitionDelta.y = (endY - g_transitionCenterPos.y) * 2.0f / 9.0f;
        g_transitionDelta.z = (tz - g_transitionCenterPos.z) * 2.0f / 9.0f;
        g_transIrisAlphaDelta = -50.0f / 9.0f;
    }
}

/* Starts the out phase unless a transition is already running; in non-blocking mode it first clears the
 * g_gameFlags bits GF_BIT0 | GF_UPDATE_OBJECTS, and it always raises LEVEL_EXIT_TRANSITION. */
void Transition_Start()
{
    if (!g_transIrisPhase) {
        if (g_transitionIsNonBlocking == 1)
            Game_ClearFlags(GF_BIT0 | GF_UPDATE_OBJECTS);
        g_transIrisPhase = TRANSITION_OUT;
        g_levelExitFlags |= LEVEL_EXIT_TRANSITION;
    }
}

/* Runs one step of the current phase; returns 1 while a transition is running. */
s32 Transition_Update()
{
    s32 busy;

    switch (g_transIrisPhase) {
        case TRANSITION_IDLE:
            busy = 0;
            break;
        case TRANSITION_OUT:
            Transition_OutUpdate();
            busy = 1;
            break;
        case TRANSITION_IN:
            Transition_InUpdate();
            busy = 1;
            break;
    }
    return busy;
}
