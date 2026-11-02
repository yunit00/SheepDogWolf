#ifndef SDW_ENGINE_FADE_H
#define SDW_ENGINE_FADE_H

/* The functions and globals fade.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

void Fade_StartLevelExit(s32 ticks);
void Fade_StartOut(s32 ticks);       /* the fade-in at a (re)start */
void Fade_StartRestart(s32 ticks);
void Fade_Update();

#endif
