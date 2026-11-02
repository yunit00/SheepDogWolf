#include "sdw_types.h"
#include "sdw_classes.h"

extern "C" {

const u8 *g_lerpColorTarget;
const Vec3s *g_lerpVecTarget;

void Lerp_SetColorTarget(const u8 *rgb)
{
    g_lerpColorTarget = rgb;
}

void Lerp_SetVecTarget(const Vec3s *p)
{
    g_lerpVecTarget = p;
}

/* a + t*(b - a), t in 4.12. */
s32 Lerp_u16(u16 a, u16 b, u16 t)
{
    return a + (t * (b - a) >> 12);
}

/* out = a + t*(*g_lerpVecTarget - a) per axis, t in 4.12. */
void Vec3s_LerpToTarget(Vec3s *out, const Vec3s *a, s16 t)
{
    out->x = a->x + (t * (g_lerpVecTarget->x - a->x) >> 12);
    out->y = a->y + (t * (g_lerpVecTarget->y - a->y) >> 12);
    out->z = a->z + (t * (g_lerpVecTarget->z - a->z) >> 12);
}

/* as Vec3s_LerpToTarget for two components (the Map panel's rectangles). */
void Vec2s_LerpToTarget(Vec2s *out, const Vec2s *a, s16 t)
{
    out->x = a->x + (t * (g_lerpVecTarget->x - a->x) >> 12);
    out->y = a->y + (t * (g_lerpVecTarget->y - a->y) >> 12);
}

void Vec3s_LerpToTarget_2(Vec3s *out, const Vec3s *a, s16 t)
{
    out->x = a->x + (t * (g_lerpVecTarget->x - a->x) >> 12);
    out->y = a->y + (t * (g_lerpVecTarget->y - a->y) >> 12);
    out->z = a->z + (t * (g_lerpVecTarget->z - a->z) >> 12);
}

/* two colour bytes each moved toward g_lerpColorTarget[0] / [1] by t (4.12); returns hi << 8 | lo. */
u16 Color16_LerpToTarget(u8 hi, u8 lo, s16 t)
{
    u16 c = (hi + (t * (g_lerpColorTarget[0] - hi) >> 12)) << 8;
    c += lo + (t * (g_lerpColorTarget[1] - lo) >> 12);
    return c;
}

/* 0xRRGGBB moved channel by channel toward g_lerpColorTarget[0..2] by t (the products are unsigned). */
u32 Rgb24_LerpToTarget(u32 rgb, s16 t)
{
    u32 c = (u8)((rgb >> 16) + (t * (g_lerpColorTarget[0] - (rgb >> 16)) >> 12)) << 16;
    c += (u8)(((rgb >> 8) & 0xff) + (t * (g_lerpColorTarget[1] - ((rgb >> 8) & 0xff)) >> 12)) << 8;
    c += (u8)((rgb & 0xff) + (t * (g_lerpColorTarget[2] - (rgb & 0xff)) >> 12));
    return c;
}

/* a + t * (b - a) per channel of two 0xRRGGBB colours. */
u32 Rgb24_Lerp(u32 a, u32 b, s16 t)
{
    u8 c[3];
    c[0] = (a >> 16) + (t * ((b >> 16) - (a >> 16)) >> 12);
    c[1] = ((a >> 8) & 0xff) + (t * (((b >> 8) & 0xff) - ((a >> 8) & 0xff)) >> 12);
    c[2] = (a & 0xff) + (t * ((b & 0xff) - (a & 0xff)) >> 12);
    return (c[0] << 16) + (c[1] << 8) + c[2];
}

} /* extern "C" */
