#ifndef SDW_ENGINE_MATHS_H
#define SDW_ENGINE_MATHS_H

/* The functions and globals maths.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

extern Vec3s *g_pZeroVec3s;                               /* origin for the world-space static boxes */
u32 Int_ToBcd(s32 value);
s32 Rand_Range(s32, s32);
void Rand_Reset();
u16 Str_Concat2(char *dst, const char *a, const char *b);
u32 Str_Copy(char *dst, const char *src);
u16 Str_CopyAsciiN(char *, const char *, u16);
s32 Str_IsPrefixOf(const char *a, const char *b);
void Vec3i_SetLength(s32 *v, s32 len);            /* (src/engine/maths.cpp) */

/* The functions and globals maths.cpp defines, declared once for every file that uses them. */

extern const s32 g_zeroConst_5775d8[4];

#endif
