/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_UIICON_SETCOLOR_U32) && !defined(SDW_INLINE_UIICON_SETCOLOR_U32_DEFINED)
#define SDW_INLINE_UIICON_SETCOLOR_U32_DEFINED
inline void UiIcon::SetColor(u32 rgb)
{
    mainQuad.SetColor(rgb);
}
#endif

#if defined(SDW_INLINE_UIICON_SETENABLED_S32) && !defined(SDW_INLINE_UIICON_SETENABLED_S32_DEFINED)
#define SDW_INLINE_UIICON_SETENABLED_S32_DEFINED
inline void UiIcon::SetEnabled(s32 on)
{
    enabled = on;
}
#endif

#if defined(SDW_INLINE_UIICON_SETHIGHLIGHTED_S32) && !defined(SDW_INLINE_UIICON_SETHIGHLIGHTED_S32_DEFINED)
#define SDW_INLINE_UIICON_SETHIGHLIGHTED_S32_DEFINED
inline void UiIcon::SetHighlighted(s32 on)
{
    highlighted = on;
}
#endif
