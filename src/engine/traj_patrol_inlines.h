/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_TRAJPATROL_REVERSE) && !defined(SDW_INLINE_TRAJPATROL_REVERSE_DEFINED)
#define SDW_INLINE_TRAJPATROL_REVERSE_DEFINED
inline void TrajPatrol::Reverse()
{
    forceAdvance = 1;
    forward ^= 1;
}
#endif
