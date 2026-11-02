/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_SCREENHEIGHTS16) && !defined(SDW_INLINE_FREE_SCREENHEIGHTS16_DEFINED)
#define SDW_INLINE_FREE_SCREENHEIGHTS16_DEFINED
inline s16 ScreenHeightS16()
{
    return 240;
}
#endif

#if defined(SDW_INLINE_FREE_SCREENHEIGHTS32) && !defined(SDW_INLINE_FREE_SCREENHEIGHTS32_DEFINED)
#define SDW_INLINE_FREE_SCREENHEIGHTS32_DEFINED
inline s32 ScreenHeightS32()
{
    return 240;
}
#endif

#if defined(SDW_INLINE_FREE_SCREENHEIGHTU16) && !defined(SDW_INLINE_FREE_SCREENHEIGHTU16_DEFINED)
#define SDW_INLINE_FREE_SCREENHEIGHTU16_DEFINED
inline u16 ScreenHeightU16()
{
    return 240;
}
#endif

#if defined(SDW_INLINE_FREE_SCREENWIDTHS16) && !defined(SDW_INLINE_FREE_SCREENWIDTHS16_DEFINED)
#define SDW_INLINE_FREE_SCREENWIDTHS16_DEFINED
inline s16 ScreenWidthS16()
{
    return 512;
}
#endif

#if defined(SDW_INLINE_FREE_SCREENWIDTHS32) && !defined(SDW_INLINE_FREE_SCREENWIDTHS32_DEFINED)
#define SDW_INLINE_FREE_SCREENWIDTHS32_DEFINED
inline s32 ScreenWidthS32()
{
    return 512;
}
#endif

#if defined(SDW_INLINE_FREE_SCREENWIDTHU16) && !defined(SDW_INLINE_FREE_SCREENWIDTHU16_DEFINED)
#define SDW_INLINE_FREE_SCREENWIDTHU16_DEFINED
inline u16 ScreenWidthU16()
{
    return 512;
}
#endif

#if defined(SDW_INLINE_SCREEN_LAYERS4_U16) && !defined(SDW_INLINE_SCREEN_LAYERS4_U16_DEFINED)
#define SDW_INLINE_SCREEN_LAYERS4_U16_DEFINED
inline u32 *Screen::Layers4(u16 index)
{
    return (u32 *)scratch4 + index;
}
#endif

#if defined(SDW_INLINE_SCREEN_LAYERS60_U16) && !defined(SDW_INLINE_SCREEN_LAYERS60_U16_DEFINED)
#define SDW_INLINE_SCREEN_LAYERS60_U16_DEFINED
inline u32 *Screen::Layers60(u16 index)
{
    return (u32 *)scratch60 + index;
}
#endif
