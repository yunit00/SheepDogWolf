#ifndef SDW_ENGINE_MAP_H
#define SDW_ENGINE_MAP_H

/* The functions and globals map.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

void Map_DrawWipeBar(u16 y, s16 height);
void Map_Init();
s32 Map_IsOpen();
u16 Map_ResolvePropExportId(void *levelRecord, u16 propOffset);
void Map_Update();
u8 SelectMenu_GetState();           /* returns g_selectMenuState */
s32 SelectMenu_HasItems();
void SelectMenu_SetState(u8 state); /* sets g_selectMenuState */
void SelectMenu_UpdateWipe();       /* select-menu wipe in (state 1) / out (state 6) */

#endif
