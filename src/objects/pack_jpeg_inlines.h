/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_PACKJPEG_GETCOUNT) && !defined(SDW_INLINE_PACKJPEG_GETCOUNT_DEFINED)
#define SDW_INLINE_PACKJPEG_GETCOUNT_DEFINED
inline u32 PackJpeg::GetCount()
{
    return count;
}
#endif
