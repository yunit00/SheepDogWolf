#ifndef SDW_GAME_WOLF_API_H
#define SDW_GAME_WOLF_API_H

/* The functions and globals wolf.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class ScnObject;

extern u8 g_wolfInstanceCount;
ScnObject *Wolf_Create(void *record);

/* The functions and globals wolf.cpp defines, declared once for every file that uses them. */

struct EmitterFadeParams;
struct EmitterRiseParams;
struct WolfMoveBank;

extern EmitterRiseParams g_wolfBubbleParams;
extern WolfMoveBank g_wolfMoveBank0[]; /* [0] Ralph on foot, [1] carrying (g_wolfMoveBank1) */
extern EmitterFadeParams g_wolfWakeParams;

#endif
