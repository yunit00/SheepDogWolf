/*
 * Everything here is drawn in the 512x240 virtual HUD space (the PlayStation's screen) and scaled by Screen::ScaleX/Y.
 * The fade and letterbox curtains are a 4x4 ARGB4444 texture (a PolyBatcher texture page reserved for it, locked and
 * refilled every frame) stretched over the screen.
 */
#include "sdw_enums.h"
#include "../sdk/ddraw.h"
#define SDW_MEMBERS_Texture void Surface_LockForWrite(DDSURFACEDESC2 *desc);


#include "sdw_enums.h"
#include "sdw_classes.h"
#define SDW_INLINE_CINE_ISACTIVE 1
#include "cine_inlines.h"
#undef SDW_INLINE_CINE_ISACTIVE
#define SDW_INLINE_UIQUAD_SETCOLOR_U32 1
#include "ui_quad_inlines.h"
#undef SDW_INLINE_UIQUAD_SETCOLOR_U32
#define SDW_INLINE_UIICON_SETCOLOR_U32 1
#include "ui_icon_inlines.h"
#undef SDW_INLINE_UIICON_SETCOLOR_U32

/* The DAV directory (DavHeader.dir): packed, its pointers sit at +6/+0xa/+0x12 (src/engine/load_dav.cpp), which the
 * struct generator cannot lay out, so it is declared here. */
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

/* ---- callees ---- */
typedef void (*MenuHandler)(u8 msg, MenuPage *self);
#include "fixed_math.h"
#include "interface.h"
#include "prompt.h"
#include "draw2d.h"
#include "scn_tools.h"
#include "scenaric_loop.h"
#include "text.h"
#include "input.h"
#include "game_state.h"
#include "screen.h"
#include "cine.h"
#include "scenaric.h"
#include "time.h"
u32 Rgb24_Lerp(u32 a, u32 b, s16 t);
void Draw2D_TexRect(float z, float x0, float y0, float x1, float y1, s32 texIndex, float uTL, float vTL, u32 cTL,
                    float uBL, float vBL, u32 cBL, float uTR, float vTR, u32 cTR, float uBR, float vBR,
                    u32 cBR);
void Dialogue_SetBoxActive(s32 active);
u8 Dialogue_Say(const char *text, s32 voiceId, ScnObject *speaker, u32 arg);
s32 Rand_Bounded(s32 bound);
u16 Text_CountWrappedLines(const char *s);
void Menu_BuildList(MenuPage *pages, Menu *menu, s8 count, const MenuHandler *handlers);
void Ui_DrawTextBox(TextBox *box, u16 lineCount);
void Ui_DrawMenuBox(MenuBox *box);
uptr *Res_GetValidatedIdList(u16 resId, u16 *outCount);
u16 Sound_Play(u16 soundId, void *owner, u16 volume, u8 flags, s32 rate);

/* ---- globals ---- */
extern u32 *g_screenLayerBase;
extern Wolf *g_pWolf;
extern u32 g_gameFlags;
extern s32 g_dt;
extern s16 g_dtRawMs;
extern u32 g_gameTime;
extern s32 g_dialogueCurText;
extern u16 *g_resTelescopeMaskOuter;
extern u16 *g_resTelescopeMaskInner;
extern u16 *g_resCannonMask;

/* inline: the constant mask is substituted but its `~` is left to run time. */
#define SDW_INLINE_FREE_GAME_CLEARFLAGS_U32 1
#include "game_state_inlines.h"
#undef SDW_INLINE_FREE_GAME_CLEARFLAGS_U32
/* inline: the virtual screen size (Screen::virtWidth / virtHeight are u16). */
#define SDW_INLINE_FREE_SCREENWIDTHU16 1
#include "screen_inlines.h"
#undef SDW_INLINE_FREE_SCREENWIDTHU16
#define SDW_INLINE_FREE_SCREENHEIGHTU16 1
#include "screen_inlines.h"
#undef SDW_INLINE_FREE_SCREENHEIGHTU16

/* An ARGB8888 colour as ARGB4444. The last term's redundant 16-bit mask is the original's. */
#define ARGB4444(c) ((((c) >> 4) & 0xf) | (((c) >> 8) & 0xf0) | (((c) >> 12) & 0xf00) | (((c) >> 16) & 0xf000 & 0xffff))

/* Cine.h: the cinematic op-stride table (g_cineOpStride is the player's own copy). A header static, so every
 * file that includes Cine.h has one; nothing here reads it. */
static u8 s_cineOpStride[9] = {0, 8, 8, 4, 2, 2, 4, 2, 2};

/* start the level-exit fade (at least one second); Fade_Update sets LEVEL_EXIT_NEXT when it ends. */
void Fade_StartLevelExit(s32 ticks)
{
    if (!(g_gameFlags & GF_FADE_EXIT)) {
        if (ticks < 0x1000)
            ticks = 0x1000;
        Game_ClearFlags(GF_FADE_RESTART | GF_FADE_IN);
        Game_ClearFlags(GF_TRANSITION_IDLE);
        g_gameFlags |= GF_FADE_EXIT;
        g_fadeTimer = ticks;
    }
}

/* start the death / restart fade-out (stopping a cinematic unless GF_CINE_SURVIVES_RESTART);
 * Scenaric_ResetAll runs when it ends. */
void Fade_StartRestart(s32 ticks)
{
    if (!(g_gameFlags & (GF_FADE_RESTART | GF_FADE_EXIT))) {
        if (g_cinePlayer.IsActive() && !(g_gameFlags & GF_CINE_SURVIVES_RESTART))
            g_cinePlayer.Stop();
        if (ticks < 0x1000)
            ticks = 0x1000;
        Game_ClearFlags(GF_TRANSITION_IDLE);
        Game_ClearFlags(GF_FADE_IN);
        g_gameFlags |= GF_FADE_RESTART;
        g_fadeTimer = ticks;
    }
}

/* start the fade-in at a (re)start; half the time (and never at game time 0) it sets GF_FADE_VARIANT. */
void Fade_StartOut(s32 ticks)
{
    if (!(g_gameFlags & (GF_FADE_IN | GF_FADE_EXIT))) {
        if (ticks < 0x1000)
            ticks = 0x1000;
        Game_ClearFlags(GF_TRANSITION_IDLE);
        Game_ClearFlags(GF_FADE_RESTART);
        g_gameFlags |= GF_FADE_IN;
        g_fadeTimer = ticks;
        if (Rand_Bounded(100) < 50 && g_gameTime != 0)
            g_gameFlags |= GF_FADE_VARIANT;
        else
            Game_ClearFlags(GF_FADE_VARIANT);
    }
}

/* run the current fade: draw the curtain and step the timer by g_dt (g_dtRaw while a cinematic plays). */
void Fade_Update()
{
    s32 dt;
    if (g_gameFlags & (GF_FADE_RESTART | GF_FADE_IN | GF_FADE_EXIT)) {
        Game_ClearFlags(GF_TRANSITION_IDLE);
        if (g_cinePlayer.IsActive())
            dt = g_dtRaw;
        else
            dt = g_dt;
        if (g_gameFlags & (GF_FADE_RESTART | GF_FADE_EXIT)) {
            if (g_fadeTimer <= 0x1000)
                Fade_DrawOverlay(0, (0x1000 - g_fadeTimer) * 0x1f >> 12, 0);
            g_fadeTimer -= dt;
            if (g_fadeTimer <= 0) {
                g_screen.Clear(0);
                g_fadeTimer = 0;
                if (g_gameFlags & GF_FADE_EXIT) {
                    Game_ClearFlags(GF_FADE_EXIT);
                    g_levelExitFlags |= LEVEL_EXIT_NEXT;
                } else {
                    Game_ClearFlags(GF_FADE_RESTART);
                    g_gameFlags |= GF_TRANSITION_IDLE;
                    Scenaric_ResetAll();
                }
            }
        } else if (g_gameFlags & GF_FADE_IN) {
            if (g_fadeTimer <= 0x1000)
                Fade_DrawOverlay(0, g_fadeTimer * 0x1f >> 12, 0);
            else
                Fade_DrawOverlay(0, 0x1f, 0);
            g_fadeTimer -= dt;
            if (g_fadeTimer <= 0) {
                g_fadeTimer = 0;
                Game_ClearFlags(GF_FADE_IN);
                g_gameFlags |= GF_TRANSITION_IDLE;
            }
        }
    }
}
