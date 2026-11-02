/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_GETPROP_VOID_U32) && !defined(SDW_INLINE_FREE_GETPROP_VOID_U32_DEFINED)
#define SDW_INLINE_FREE_GETPROP_VOID_U32_DEFINED
inline u32 GetProp(void *record, u32 offset)
{
    return *(u32 *)((u8 *)record + offset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_INDEXEDPROP_VOID_U16) && !defined(SDW_INLINE_FREE_INDEXEDPROP_VOID_U16_DEFINED)
#define SDW_INLINE_FREE_INDEXEDPROP_VOID_U16_DEFINED
inline u32 IndexedProp(void *record, u16 offset)
{
    return *(u32 *)((u8 *)record + offset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_PROPU32_VOID_U32) && !defined(SDW_INLINE_FREE_PROPU32_VOID_U32_DEFINED)
#define SDW_INLINE_FREE_PROPU32_VOID_U32_DEFINED
#if SDW_INLINE_FREE_PROPU32_VOID_U32 == 1
inline u32 PropU32(void *record, u32 fieldOffset)
{
    struct ReadWork {
        u32 result, offset;
    } read;
    read.offset = fieldOffset;
    read.result = *(u32 *)((u8 *)record + read.offset + 0x14);
    return read.result;
}
#elif SDW_INLINE_FREE_PROPU32_VOID_U32 == 2
inline u32 PropU32(void *rec, u32 off)
{
    return *(u32 *)((u8 *)rec + off + 0x14);
}
#endif
#endif

#if defined(SDW_INLINE_FREE_PROPERTY_VOID_U32) && !defined(SDW_INLINE_FREE_PROPERTY_VOID_U32_DEFINED)
#define SDW_INLINE_FREE_PROPERTY_VOID_U32_DEFINED
static inline u32 Property(void *record, u32 offset)
{
    return *(u32 *)((u8 *)record + offset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_PROPERTYU32_VOID_U32) && !defined(SDW_INLINE_FREE_PROPERTYU32_VOID_U32_DEFINED)
#define SDW_INLINE_FREE_PROPERTYU32_VOID_U32_DEFINED
inline u32 PropertyU32(void *record, u32 offset)
{
    return *(u32 *)((u8 *)record + offset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_READINTPROPERTY_VOID_U32) && !defined(SDW_INLINE_FREE_READINTPROPERTY_VOID_U32_DEFINED)
#define SDW_INLINE_FREE_READINTPROPERTY_VOID_U32_DEFINED
inline s32 ReadIntProperty(void *record, u32 offset)
{
    return *(s32 *)((u8 *)record + offset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_READPROPERTY_VOID_U32) && !defined(SDW_INLINE_FREE_READPROPERTY_VOID_U32_DEFINED)
#define SDW_INLINE_FREE_READPROPERTY_VOID_U32_DEFINED
#if SDW_INLINE_FREE_READPROPERTY_VOID_U32 == 1
inline u32 ReadProperty(void *record, u32 field)
{
    u32 offset = field;
    return *(u32 *)((u8 *)record + offset + 0x14);
}
#elif SDW_INLINE_FREE_READPROPERTY_VOID_U32 == 2
inline u32 ReadProperty(void *record, u32 offset)
{
    return *(u32 *)((u8 *)record + offset + 0x14);
}
#endif
#endif

#if defined(SDW_INLINE_FREE_SCN_GETPROPID_U16_U32) && !defined(SDW_INLINE_FREE_SCN_GETPROPID_U16_U32_DEFINED)
#define SDW_INLINE_FREE_SCN_GETPROPID_U16_U32_DEFINED
inline u32 Scn_GetPropId(u16 *props, u32 offset)
{
    return *(u32 *)((u8 *)props + offset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_SCN_GETPROPS32_VOID_U32) && !defined(SDW_INLINE_FREE_SCN_GETPROPS32_VOID_U32_DEFINED)
#define SDW_INLINE_FREE_SCN_GETPROPS32_VOID_U32_DEFINED
inline s32 Scn_GetPropS32(void *props, u32 offset)
{
    return *(s32 *)((u8 *)props + offset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_SCN_GETPROPU32_U16_U32) && !defined(SDW_INLINE_FREE_SCN_GETPROPU32_U16_U32_DEFINED)
#define SDW_INLINE_FREE_SCN_GETPROPU32_U16_U32_DEFINED
inline u32 Scn_GetPropU32(u16 *props, u32 offset)
{
    return *(u32 *)((u8 *)props + offset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_SCN_GETPROPU32_VOID_U32) && !defined(SDW_INLINE_FREE_SCN_GETPROPU32_VOID_U32_DEFINED)
#define SDW_INLINE_FREE_SCN_GETPROPU32_VOID_U32_DEFINED
inline u32 Scn_GetPropU32(void *props, u32 propOffset)
{
    return *(u32 *)((u8 *)props + propOffset + 0x14);
}
#endif

#if defined(SDW_INLINE_FREE_SETPROP_VOID_U32_U32) && !defined(SDW_INLINE_FREE_SETPROP_VOID_U32_U32_DEFINED)
#define SDW_INLINE_FREE_SETPROP_VOID_U32_U32_DEFINED
inline void SetProp(void *record, u32 offset, u32 value)
{
    *(u32 *)((u8 *)record + offset + 0x14) = value;
}
#endif
