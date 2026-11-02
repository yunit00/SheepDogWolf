#ifndef SDW_ENGINE_JPEG_MLT_H
#define SDW_ENGINE_JPEG_MLT_H

/* The functions and globals jpeg_mlt.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct StringBank;

u8 Jpeg_DecodeToRgb555Flipped(void *, u16 *, u32, u32); /* jpeg_decode.cpp */
u8 Jpeg_GetDimensions(void *, u32 *, u32 *);
s32 Load_FreeMLT(StringBank *bank);
int Load_MLT(char *path, StringBank *bank);
void Mlt_MapLanguageToBlock(u16 *lang);

#endif
