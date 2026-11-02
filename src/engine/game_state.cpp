#include "sdw_classes.h"
#include "sdw_enums.h"
#include "../sdk/crt.h"
#include "game_state.h"

/* The reverse mask's type, as src/engine/tex_scroll.cpp declares it. The generated headers do not have it; this local
 * definition is the same as that file's, so the decorated name agrees. */
struct TexScrollReverseBits {
    u16 list0 : 1;
    u16 list1 : 1;
    u16 list2 : 1;
    u16 list3 : 1;
};

/* ---- the game-state block (all initialised to zero so that they keep this order: see the header) ---- */
s32 g_animDt = 0;                                  /* g_dt * 1000 >> 2 (GameState.animDt) */
u16 g_texScrollListIds[4] = {0};                   /* WAR type-0x83 header: four id-list ids */
TexScrollReverseBits g_texScrollReverseMask = {0}; /* bit i reverses list i */
u16 g_weatherType = 0;                             /* 1 rain, 2 snow */
Dav *g_pDav = 0;                                   /* GameState.pDav */
s32 g_fadeTimer = 0;                               /* 1/4096 s (GameState.fadeTimer) */
u32 g_gameFlags = 0;                               /* GameState.flags */
u8 g_letterboxState = 0;                           /* GameState.cineState: 0 idle, 1 animating, 2 out */
u8 g_camDebugMode = 0;                             /* GameState.camDebugMode */
u16 g_gameStateUnref_6ddf7a = 0;
u8 g_gameStateUnref_6ddf7c[12] = {0};

/* The level's WAR header (type 0x83) was copied as one 12-byte block over g_texScrollListIds and the
 * two globals after it (list.cpp), and cleared the same way (load_war.cpp); a modern compiler need not keep them
 * adjacent, so they are set by name. NULL clears them. */
void Game_SetWarLevelHeader(const WarLevelHeader *h)
{
    static const WarLevelHeader zero = {0};
    if (!h)
        h = &zero;
    memcpy(g_texScrollListIds, h->texScrollListIds, sizeof(g_texScrollListIds));
    memcpy(&g_texScrollReverseMask, &h->texScrollReverseMask, sizeof(g_texScrollReverseMask));
    g_weatherType = h->weatherType;
}

/* So the fields are the named globals themselves here, and `this` is not used. */
#define flags g_gameFlags
#define cineState g_letterboxState
#define animDt g_animDt
#define pDav g_pDav
#define fadeTimer g_fadeTimer
#define camDebugMode g_camDebugMode

/* sets or clears bits of g_gameFlags (this is always &g_animDt, so +0x18 is g_gameFlags). */
void GameState::Game_SetFlags(u32 mask, s32 on)
{
    if (on)
        flags |= mask;
    else
        flags &= ~mask;
}

/* g_gameFlags = GF_TRANSITION_IDLE | GF_UPDATE_OBJECTS | GF_RENDER_WORLD (0xc010); clears the letterbox
 * state, g_animDt, g_pDav, g_fadeTimer and the camera debug mode. */
void GameState::Game_ResetState()
{
    flags = 0;
    flags |= GF_TRANSITION_IDLE | GF_UPDATE_OBJECTS | GF_RENDER_WORLD;
    cineState = 0;
    animDt = 0;
    pDav = 0;
    fadeTimer = 0;
    camDebugMode = 0;
}

/* menus may open only while no transition runs and no letterbox is up. */
s32 GameState::Game_CanOpenMenu()
{
    if ((flags & GF_TRANSITION_IDLE) && !cineState)
        return 1;
    return 0;
}

#undef flags
#undef cineState
#undef animDt
#undef pDav
#undef fadeTimer
#undef camDebugMode
