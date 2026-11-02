#ifndef SDW_ENGINE_LOAD_WARMESHES_H
#define SDW_ENGINE_LOAD_WARMESHES_H

/* The functions and globals load_warmeshes.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct WarFile;

void Load_FreeWarMeshes(WarFile *war);
int Load_WarMeshes(const char *path, WarFile *war);
float Math_Angle128ToRadians(s8);
float Math_Angle4096ToRadians(s16);
float Math_U16ToUnitFloat(u16 value);
float Math_U8ToUnitFloat(u8);
float Tex_CornerUV(u32 offset, s32 size, s32 base);

#endif
