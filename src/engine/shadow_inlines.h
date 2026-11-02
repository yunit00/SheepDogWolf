/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_SHADOW_INVALIDATE) && !defined(SDW_INLINE_SHADOW_INVALIDATE_DEFINED)
#define SDW_INLINE_SHADOW_INVALIDATE_DEFINED
inline void Shadow::Invalidate()
{
    flags |= SHADOW_F_REPROJECT;
}
#endif

#if defined(SDW_INLINE_SHADOW_REPROJECT) && !defined(SDW_INLINE_SHADOW_REPROJECT_DEFINED)
#define SDW_INLINE_SHADOW_REPROJECT_DEFINED
inline void Shadow::Reproject()
{
    flags |= SHADOW_F_REPROJECT;
}
#endif

#if defined(SDW_INLINE_SHADOW_SETENABLED_S32) && !defined(SDW_INLINE_SHADOW_SETENABLED_S32_DEFINED)
#define SDW_INLINE_SHADOW_SETENABLED_S32_DEFINED
inline void Shadow::SetEnabled(s32 on)
{
    if (on)
        flags &= (u8)~SHADOW_F_HIDDEN;
    else
        flags |= SHADOW_F_HIDDEN;
}
#endif

#if defined(SDW_INLINE_SHADOW_SETFLAG4_S32) && !defined(SDW_INLINE_SHADOW_SETFLAG4_S32_DEFINED)
#define SDW_INLINE_SHADOW_SETFLAG4_S32_DEFINED
inline void Shadow::SetFlag4(s32 on)
{
    if (on)
        flags |= SHADOW_F_DYNAMIC;
    else
        flags &= (u8)~SHADOW_F_DYNAMIC;
}
#endif

#if defined(SDW_INLINE_SHADOW_SETVISIBLE_S32) && !defined(SDW_INLINE_SHADOW_SETVISIBLE_S32_DEFINED)
#define SDW_INLINE_SHADOW_SETVISIBLE_S32_DEFINED
inline void Shadow::SetVisible(s32 on)
{
    if (on)
        flags &= (u8)~SHADOW_F_HIDDEN;
    else
        flags |= SHADOW_F_HIDDEN;
}
#endif
