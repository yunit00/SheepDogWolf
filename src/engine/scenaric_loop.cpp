/*
 * The scenaric update / render loop and the game frame.
 * */


#define SDW_MEMBERS_ScnObject                                                                                   \
    s32 InUpdateRange()                                                                                         \
    {                                                                                                           \
        if (!(GetFlags() & SCN_OF_NEVER_UPDATE) && (camDist2 < 9000000 || (GetFlags() & SCN_OF_ALWAYS_UPDATE))) \
            return 1;                                                                                           \
        return 0;                                                                                               \
    }                                                                                                           \
    s32 NoPlaneCull()                                                                                           \
    {                                                                                                           \
        return (flags & SCN_OF_NO_PLANE_CULL) != 0;                                                             \
    }                                                                                                           \
    s32 NotHidden()                                                                                             \
    {                                                                                                           \
        return (flags & (SCN_OF_HIDDEN2 | SCN_OF_HIDDEN)) == 0;                                                 \
    }                                                                                                           \
    s32 TestInstFlags(u16 mask)                                                                                 \
    {                                                                                                           \
        return inst_flags & mask;                                                                               \
    }                                                                                                           \
    inline s32 PassesRenderVisibility();        /* defined below: it needs Frustrum and Camera */
#define SDW_MEMBERS_D3DApp void ClearZBuffer();
#include "sdw_enums.h"
#include "sdw_classes.h"
#define SDW_INLINE_CINE_ISACTIVE 1
#define SDW_INLINE_CINE_ISWOLFREADY 1
#include "cine_inlines.h"
#undef SDW_INLINE_CINE_ISACTIVE
#undef SDW_INLINE_CINE_ISWOLFREADY
#define SDW_INLINE_INSTANCE_INST 1
#include "../objects/instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNOBJECT_GETFLAGS 1
#include "scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETFLAGS
#include "../sdk/d3d7.h"

inline void D3DApp::ClearZBuffer()
{
    SDW_RD(pD3DDevice)->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
}

/* ---- globals ---- */
static u8 g_cineOpStride_57ecd4[9] = {0, 8, 8, 4, 2, 2, 4, 2, 2}; /* Cine.h copy, unreferenced here */
#include "cine.h"
#include "stream_player.h"
#include "scn_tools.h"
#include "../app/app_main.h"
#include "draw2d.h"
#include "../objects/instance.h"
#include "input.h"
#include "game_state.h"
#include "prompt.h"
#include "sound_mgr.h"
#include "progress_inventory.h"
#include "../objects/world_draw.h"
#include "weather.h"
#include "fade.h"
#include "../objects/camera.h"
#include "tex_scroll.h"
#include "map.h"
#include "pause_menu.h"
#include "interface.h"
#include "../platform/platform.h"
#include "progress.h"
float g_cameraMouseDX, g_cameraMouseDY;
extern "C" char *getenv(const char *);
extern u8 g_sharedScratch[]; /* scratch shared with the collision code (u8[0x200], as the game declares it) */
#define g_sharedScratch ((s32 *)g_sharedScratch)
extern void (*g_worldObjDrawFns[5])(WorldObj *obj, Camera *cam); /* indexed by WorldObj.inst_kind */
extern u32 g_gameFlags; /* 0x4000 update the world, 0x8000 render it, 0x20 toggled by the pause openers */
extern u32 *g_screenLayerBase0; /* first argument of the weather renderers (a draw-layer handle); u32 *, as its definition declares it */
extern Wolf *g_pWolf;
#define GAME_STATE ((GameState *)&g_animDt)

/* ---- functions ---- */
void Dialogue_Reset();

/* level reset (from Fade_Update when the fade-out ends): stop the cinematic, prompt, dialogue, sounds and the
 * streamed voice, then Reset() every level object and every dynamic object past them in g_scnActive. */
void Scenaric_ResetAll()
{
    struct {
        ScnObject **p;
        u32 n;
        ScnObject *obj;
    } w;

    if (g_cinePlayer.IsActive())
        g_cinePlayer.Stop();
    Prompt_End();
    Dialogue_Reset();
    Sound_StopAll();
    g_pStreamPlayer->StopVoice();
    g_voiceOwner = 0;
    for (w.p = g_scnObjects, w.n = g_scnObjectCount; w.n != 0; w.n--, w.p++) {
        w.obj = *w.p;
        if (w.obj != 0)
            w.obj->Reset();
    }
    if (g_scnActiveHigh > g_scnObjectCount) {
        for (w.p = &g_scnActive[g_scnObjectCount], w.n = g_scnActiveHigh - g_scnObjectCount; w.n != 0; w.n--, w.p++) {
            w.obj = *w.p;
            if (w.obj != 0)
                w.obj->Reset();
        }
    }
}

/* per-frame update of the active objects. Every object's squared distance to the camera eye is stored in
 * camDist2 (render culling reads it too); an object is updated only within 3000 units (camDist2 < 9000000) unless it
 * has flag 0x4000, and never with 0x2000. While a cinematic runs and the Wolf has acknowledged it, an object with flag
 * 0x80 is updated only if it also has 0x20 - regardless of distance and of 0x2000. */
void Scenaric_UpdateAll()
{
    struct {
        ScnObject **p;
        u16 unused, i;
        s32 *d;
        ScnObject *obj;
    } w;

    w.p = g_scnActive;
    if (g_cinePlayer.IsWolfReady() && g_cinePlayer.IsActive()) {
        for (w.i = 0; w.i < g_scnActiveHigh; w.i++) {
            w.obj = *w.p++;
            if (w.obj != 0) {
                w.d = g_sharedScratch;
                w.d[0] = w.obj->pos.x - g_camera.pos.x;
                w.d[1] = w.obj->pos.y - g_camera.pos.y;
                w.d[2] = w.obj->pos.z - g_camera.pos.z;
                w.d[0] = w.d[0] * w.d[0];
                w.d[1] = w.d[1] * w.d[1];
                w.d[2] = w.d[2] * w.d[2];
                w.obj->camDist2 = w.d[0] + w.d[1] + w.d[2];
                if ((w.obj->InUpdateRange() && !(w.obj->flags & SCN_OF_IN_CINE_BOX)) ||
                    ((w.obj->flags & SCN_OF_IN_CINE_BOX) && (w.obj->GetFlags() & SCN_OF_CINE_UPDATE)))
                    w.obj->Update();
            }
        }
    } else {
        for (w.i = 0; w.i < g_scnActiveHigh; w.i++) {
            w.obj = *w.p++;
            if (w.obj != 0) {
                w.d = g_sharedScratch;
                w.d[0] = w.obj->pos.x - g_camera.pos.x;
                w.d[1] = w.obj->pos.y - g_camera.pos.y;
                w.d[2] = w.obj->pos.z - g_camera.pos.z;
                w.d[0] = w.d[0] * w.d[0];
                w.d[1] = w.d[1] * w.d[1];
                w.d[2] = w.d[2] * w.d[2];
                w.obj->camDist2 = w.d[0] + w.d[1] + w.d[2];
                if (w.obj->InUpdateRange())
                    w.obj->Update();
            }
        }
    }
}

inline s32 ScnObject::PassesRenderVisibility()
{
    if (camDist2 < (boundRadius + (double)g_pViewFrustum->viewDistance) *
                       (boundRadius + (double)g_pViewFrustum->viewDistance) &&
        (g_camera.viewMatS.rot[6] * pos.x + g_camera.viewMatS.rot[7] * pos.y + g_camera.viewMatS.rot[8] * pos.z >=
             g_camera.planeD - (boundRadius << 12) ||
         NoPlaneCull()) &&
        NotHidden())
        return 1;
    return 0;
}

/* render one object if PassesRenderVisibility. An attached object (instance flag 8) first runs its parent through here
 * (once per frame) and draws only if the parent published its matrix. Afterwards every child link of this object is
 * marked parent-done, whether or not the object itself was drawn. */
void Scn_RenderIfVisible(ScnObject *obj)
{
    struct {
        u16 unused, n;
        AttachLink **slot;
    } w;

    if (obj->PassesRenderVisibility()) {
        if (obj->TestInstFlags(INST_F_ATTACHED)) {
            if (!obj->attachLink->flags.parentDone)
                Scn_RenderIfVisible(obj->attachLink->parentObj);
            if (obj->attachLink->flags.matrixValid)
                obj->Render(&g_camera);
        } else {
            obj->Render(&g_camera);
        }
    }
    w.n = obj->attachedChildCount;
    w.slot = AttachLink_FindFirstChildSlot(obj->Inst());
    while (w.n != 0) {
        (*w.slot)->flags.parentDone = 1;
        w.slot++;
        w.n--;
    }
}

/* ------------------------------------------------------------------------------------------------------------------
 * From here on the original is optimized code (see the top of the file).
 */
#pragma optimize("g", on)

/* render pass over the scenaric objects. First the attachment links: clear both link bits, then run every
 * parent not reached yet through Scn_RenderIfVisible (which draws the parent's attached children on the way). Then
 * every active object without children that passes PassesRenderVisibility. Then the cinematic's tagged objects and the inventory
 * wheel. Visibility uses the camDist2 that Scenaric_UpdateAll last stored. */
void Scenaric_RenderAll()
{
    u16 i;
    ScnObject **p;
    ScnObject *obj;

    if (g_attachLinkCount > 0) {
        for (i = 0; i < g_attachLinkCount; i++)
            g_attachLinkPool[i]->flagsByte &= ~(ATTACH_F_MATRIX_VALID | ATTACH_F_PARENT_RENDERED);
        for (i = 0; i < g_attachLinkCount; i++)
            if (!g_attachLinkPool[i]->flags.parentDone)
                Scn_RenderIfVisible(g_attachLinkPool[i]->parentObj);
    }
    p = g_scnActive;
    for (i = 0; i < g_scnActiveHigh; i++, p++) {
        obj = *p;
        if (obj != 0 && obj->PassesRenderVisibility() && !(obj->attachedChildCount > 0))
            obj->Render(&g_camera);
    }
    if (g_cinePlayer.IsActive())
        g_cinePlayer.RenderTaggedObjects();
    InvWheel_Render();
}

/* the static world: sky meshes (resource type 0x26) first, then a z-buffer clear, then every other world
 * mesh through its draw function. No caller: Game_Frame has this body inlined; this is the out-of-line copy. */
void Game_RenderStaticWorld()
{
    u16 i;

    for (i = 0; i < g_worldObjCount; i++)
        if (g_worldObjs[i]->inst_mode == INST_MODE_WAR_SKY)
            WorldObj_DrawSky(g_worldObjs[i], &g_camera);
    g_pD3DAppMain->ClearZBuffer();
    for (i = 0; i < g_worldObjCount; i++)
        if (g_worldObjs[i]->inst_mode != INST_MODE_WAR_SKY)
            g_worldObjDrawFns[g_worldObjs[i]->inst_kind](g_worldObjs[i], &g_camera);
}

/* the in-game frame: update (flag 0x4000: weather, fade, objects, camera), render (flag 0x8000: underwater
 * overlay, static world, objects, weather), the inventory effects when both are on, then the cinematic handshake:
 * until the Wolf acknowledges (message 0x407) the cinematic does not advance; while one is active, the frustum's
 * cullDisabled is set. TexScroll_UpdateAll is a tail call. */
void Game_Frame()
{
    if (g_gameFlags & GF_UPDATE_OBJECTS) {
        switch (g_weatherType) {
            case WEATHER_RAIN:
                Weather_UpdateRain(&g_camera.pos);
                break;
            case WEATHER_SNOW:
                Weather_UpdateSnow(&g_camera.pos);
                break;
        }
        Fade_Update();
        Scenaric_UpdateAll();
        Camera_Update(&g_camera, g_camDebugMode, &g_pad, &g_pad2);
    }
    if (g_gameFlags & GF_RENDER_WORLD) {
        Camera_UpdateUnderwater();
        Game_RenderStaticWorld();
        Scenaric_RenderAll();
        switch (g_weatherType) {
            case WEATHER_RAIN:
                Weather_RenderRain(g_screenLayerBase0, &g_camera);
                break;
            case WEATHER_SNOW:
                Weather_RenderSnow(g_screenLayerBase0, &g_camera);
                break;
        }
    }
    if ((g_gameFlags & GF_UPDATE_OBJECTS) && (g_gameFlags & GF_RENDER_WORLD))
        Inventory_UpdateFx();
    if (g_cinePlayer.IsActive()) {
        if (g_cinePlayer.IsWolfReady())
            g_cinePlayer.Update();
        else
            g_cinePlayer.wolfReady = g_pWolf->HandleMessage(0, MSG_WOLF_IS_CINE_READY, 0);
        g_pViewFrustum->cullDisabled = 1;
    } else {
        g_pViewFrustum->cullDisabled = 0;
    }
    if (g_gameFlags & GF_UPDATE_OBJECTS)
        TexScroll_UpdateAll();
}

/* the frame while a level is loaded (from Main_Loop): the three menu openers - Select (pad mask 0xfffe) steps
 * the select-menu state 0 -> 1 or 3 -> 4; Esc opens the pause menu, Pause/P the other pause menu, both only in select
 * state 0 or 7 and both toggling g_gameFlags 0x20 - then either the map screen (states 2..5) or the select-menu wipe
 * plus Game_Frame plus, while paused, the pause-menu update; then the letterbox, the streamed voice and (tail call) the
 * sound mixer. */
void Game_Frame_2()
{
    static unsigned inputTraceFrame;
    if (getenv("SDW_INPUT_TRACE") && (++inputTraceFrame % 60) == 0)
        Platform_Log("[input] active=%d capture=%d flags=%x cine=%d ready=%d id=%u letterbox=%d stream=%d wolf=%d wf=%x pad=%x prev=%x connected=%d keyActive=%d prompt=%d\n",
            Platform_IsActive(), Platform_MouseCameraCaptured(), g_gameFlags,
            g_cinePlayer.IsActive(), g_cinePlayer.IsWolfReady(), g_cinePlayer.cineId,
            g_letterboxState, g_pStreamPlayer->state, g_pWolf ? g_pWolf->state : -1,
            g_pWolf ? g_pWolf->flags : 0, g_pad.cur.buttons, g_pad.prev.buttons,
            g_pad.IsConnected(), g_inputMgr.current ? g_inputMgr.current->allowHeld : -1, g_promptActive);

    if (g_pad.IsConnected() && Pad_MenuPressed((u16)~PAD_SELECT) && !g_cinePlayer.IsActive() &&
        !g_pStreamPlayer->IsBusy() && GAME_STATE->Game_CanOpenMenu() && !Game_IsPaused()) {
        if (SelectMenu_GetState() == SEL_CLOSED)
            SelectMenu_SetState(SEL_OPEN_WIPE);
        else if (SelectMenu_GetState() == SEL_INTERACTIVE)
            SelectMenu_SetState(SEL_CLOSE_HOLD);
    }
    if (g_pad.IsConnected() && g_inputMgr.escPressed && !g_cinePlayer.IsActive() && !Map_IsOpen() &&
        !g_pStreamPlayer->IsBusy() && GAME_STATE->Game_CanOpenMenu() &&
        (SelectMenu_GetState() == SEL_CLOSED || SelectMenu_GetState() == SEL_SUPPRESSED) && !Game_IsPaused()) {
        g_gameFlags ^= GF_PAUSE_TOGGLE;
        PauseMenu_Open();
    }
    if (g_pad.IsConnected() && g_inputMgr.pausePressed && !g_cinePlayer.IsActive() && !Map_IsOpen() &&
        !g_pStreamPlayer->IsBusy() && GAME_STATE->Game_CanOpenMenu() && !Game_IsPaused() &&
        (SelectMenu_GetState() == SEL_CLOSED || SelectMenu_GetState() == SEL_SUPPRESSED)) {
        g_gameFlags ^= GF_PAUSE_TOGGLE;
        PausedMenu_Open();
    }
    // A script may lock walking while allowing a look lesson. Pointer visibility is
    // independent: cinematic/frozen gameplay does not need a desktop cursor.
    bool inGame = g_pProgress && g_pProgress->currentLevel != SCENE_WHEEL;
    bool pointerVisible = !inGame || Game_IsPaused() || Map_IsOpen() ||
        (g_gameFlags & GF_ITEM_WHEEL_OPEN);
    bool mouseEnabled = inGame && (g_gameFlags & GF_UPDATE_OBJECTS) &&
        !(g_gameFlags & (GF_PAUSED | GF_ITEM_WHEEL_OPEN | GF_FADE_EXIT | GF_FADE_RESTART)) &&
        !Map_IsOpen() && !g_cinePlayer.IsActive() && g_pWolf &&
        (g_pWolf->CanControl() || g_pWolf->CanLook());
    Platform_MouseCameraFrame(mouseEnabled, pointerVisible, &g_cameraMouseDX, &g_cameraMouseDY);
    if (SelectMenu_GetState() >= SEL_OPEN_HOLD && SelectMenu_GetState() <= SEL_CLOSE_WIPE) {
        Map_Update();
    } else {
        SelectMenu_UpdateWipe();
        Game_Frame();
        if (Game_IsPaused())
            PauseMenu_Update();
    }
    Letterbox_Update();
    g_pStreamPlayer->Update();
    Sound_MixerTick();
}
#pragma optimize("", on)
