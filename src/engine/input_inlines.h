/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_PAD_SETACTUATOR_INT) && !defined(SDW_INLINE_PAD_SETACTUATOR_INT_DEFINED)
#define SDW_INLINE_PAD_SETACTUATOR_INT_DEFINED
inline void Pad::SetActuator(int on)
{
    actuatorEnable = on;
}
#endif
