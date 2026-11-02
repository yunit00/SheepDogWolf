/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_SCNMOBILE_GROUNDY) && !defined(SDW_INLINE_SCNMOBILE_GROUNDY_DEFINED)
#define SDW_INLINE_SCNMOBILE_GROUNDY_DEFINED
inline s16 ScnMobile::GroundY()
{
    return shadow.groundPos.y;
}
#endif

#if defined(SDW_INLINE_SCNMOBILE_SETSHADOWRADIUS_U8) && !defined(SDW_INLINE_SCNMOBILE_SETSHADOWRADIUS_U8_DEFINED)
#define SDW_INLINE_SCNMOBILE_SETSHADOWRADIUS_U8_DEFINED
inline void ScnMobile::SetShadowRadius(u8 radius)
{
    shadow.radius = radius;
}
#endif
