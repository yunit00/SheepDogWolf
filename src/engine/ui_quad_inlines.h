/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_UIQUAD_SETCOLOR_U32) && !defined(SDW_INLINE_UIQUAD_SETCOLOR_U32_DEFINED)
#define SDW_INLINE_UIQUAD_SETCOLOR_U32_DEFINED
inline void UiQuad::SetColor(u32 rgb)
{
    color = (color & 0xff000000) | rgb;
}
#endif

#if defined(SDW_INLINE_UIQUAD_SETFADELEVEL_U8) && !defined(SDW_INLINE_UIQUAD_SETFADELEVEL_U8_DEFINED)
#define SDW_INLINE_UIQUAD_SETFADELEVEL_U8_DEFINED
inline void UiQuad::SetFadeLevel(u8 level)
{
    drawFlags = (drawFlags & (u16)~UIQUAD_FADE_MASK) | (level << 6);
}
#endif
