/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_LAUNCHARC_INITCOEFFICIENTS_S32_S32_S32_S32_S32_S32_S32_S32_S32) && \
    !defined(SDW_INLINE_LAUNCHARC_INITCOEFFICIENTS_S32_S32_S32_S32_S32_S32_S32_S32_S32_DEFINED)
#define SDW_INLINE_LAUNCHARC_INITCOEFFICIENTS_S32_S32_S32_S32_S32_S32_S32_S32_S32_DEFINED
inline void LaunchArc::InitCoefficients(s32 x0, s32 x1, s32 x2, s32 y0, s32 y1, s32 y2, s32 z0, s32 z1, s32 z2)
{
    coeffs[0] = x0;
    coeffs[1] = -3 * x0 - x2 + 4 * x1;
    coeffs[2] = x0 + x2 - 2 * x1;
    coeffs[3] = y0;
    coeffs[4] = -3 * y0 - y2 + 4 * y1;
    coeffs[5] = y0 + y2 - 2 * y1;
    coeffs[6] = z0;
    coeffs[7] = -3 * z0 - z2 + 4 * z1;
    coeffs[8] = z0 + z2 - 2 * z1;
}
#endif

