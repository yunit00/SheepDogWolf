#ifndef SDW_ENGINE_GAME_STATE_H
#define SDW_ENGINE_GAME_STATE_H

/* The functions and globals game_state.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct Dav;

extern s32 g_animDt;        /* also the start of the GameState block */
extern u8 g_camDebugMode;
extern s32 g_fadeTimer;     /* 1/4096 s */
extern u8 g_letterboxState; /* 0 idle, 1 animating, 2 fully extended */
extern Dav *g_pDav;
extern u16 g_weatherType;

/* The functions and globals game_state.cpp defines, declared once for every file that uses them. */

extern u16 g_texScrollListIds[4]; /* WAR type 0x83 (the table names the block by its first field) */

struct WarLevelHeader;
void Game_SetWarLevelHeader(const WarLevelHeader *h); /* Sets the three WAR header globals (NULL clears) */

#endif
