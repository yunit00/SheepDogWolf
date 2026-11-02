/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_BITS16_CLEAR_U16_U16) && !defined(SDW_INLINE_FREE_BITS16_CLEAR_U16_U16_DEFINED)
#define SDW_INLINE_FREE_BITS16_CLEAR_U16_U16_DEFINED
inline void Bits16_Clear(u16 *p, u16 mask)
{
    *p &= (u16)~mask;
}
#endif

#if defined(SDW_INLINE_FREE_BITS16_SET_U16_U16) && !defined(SDW_INLINE_FREE_BITS16_SET_U16_U16_DEFINED)
#define SDW_INLINE_FREE_BITS16_SET_U16_U16_DEFINED
inline void Bits16_Set(u16 *p, u16 mask)
{
    *p |= mask;
}
#endif

#if defined(SDW_INLINE_FREE_FLAGS16_CLEAR_U16_U16) && !defined(SDW_INLINE_FREE_FLAGS16_CLEAR_U16_U16_DEFINED)
#define SDW_INLINE_FREE_FLAGS16_CLEAR_U16_U16_DEFINED
inline void Flags16_Clear(u16 *word, u16 mask)
{
    *word &= (u16)~mask;
}
#endif

#if defined(SDW_INLINE_FREE_FLAGS16_SET_U16_U16) && !defined(SDW_INLINE_FREE_FLAGS16_SET_U16_U16_DEFINED)
#define SDW_INLINE_FREE_FLAGS16_SET_U16_U16_DEFINED
inline void Flags16_Set(u16 *word, u16 mask)
{
    *word |= mask;
}
#endif

#if defined(SDW_INLINE_FREE_INSTFLAGSCLEAR_U16_U16) && !defined(SDW_INLINE_FREE_INSTFLAGSCLEAR_U16_U16_DEFINED)
#define SDW_INLINE_FREE_INSTFLAGSCLEAR_U16_U16_DEFINED
inline void InstFlagsClear(u16 *word, u16 mask)
{
    *word &= (u16)~mask;
}
#endif

#if defined(SDW_INLINE_FREE_INSTFLAGSSET_U16_U16) && !defined(SDW_INLINE_FREE_INSTFLAGSSET_U16_U16_DEFINED)
#define SDW_INLINE_FREE_INSTFLAGSSET_U16_U16_DEFINED
inline void InstFlagsSet(u16 *word, u16 mask)
{
    *word |= mask;
}
#endif

#if defined(SDW_INLINE_FREE_INSTFLAGS_CLEAR_U16_U16) && !defined(SDW_INLINE_FREE_INSTFLAGS_CLEAR_U16_U16_DEFINED)
#define SDW_INLINE_FREE_INSTFLAGS_CLEAR_U16_U16_DEFINED
inline void InstFlags_Clear(u16 *word, u16 mask)
{
    *word &= (u16)~mask;
}
#endif

#if defined(SDW_INLINE_FREE_INSTFLAGS_SET_U16_U16) && !defined(SDW_INLINE_FREE_INSTFLAGS_SET_U16_U16_DEFINED)
#define SDW_INLINE_FREE_INSTFLAGS_SET_U16_U16_DEFINED
inline void InstFlags_Set(u16 *word, u16 mask)
{
    *word |= mask;
}
#endif

#if defined(SDW_INLINE_FREE_INSTANCECLEARFLAGS_INSTANCEBASE_U16) && \
    !defined(SDW_INLINE_FREE_INSTANCECLEARFLAGS_INSTANCEBASE_U16_DEFINED)
#define SDW_INLINE_FREE_INSTANCECLEARFLAGS_INSTANCEBASE_U16_DEFINED
inline void InstanceClearFlags(InstanceBase *instance, u16 mask)
{
    instance->inst_flags &= (u16)~mask;
}
#endif

#if defined(SDW_INLINE_FREE_INSTANCEHASFLAGS_INSTANCEBASE_U16) && \
    !defined(SDW_INLINE_FREE_INSTANCEHASFLAGS_INSTANCEBASE_U16_DEFINED)
#define SDW_INLINE_FREE_INSTANCEHASFLAGS_INSTANCEBASE_U16_DEFINED
inline u32 InstanceHasFlags(InstanceBase *instance, u16 mask)
{
    return instance->inst_flags & mask;
}
#endif

#if defined(SDW_INLINE_FREE_INSTANCESETFLAGS_INSTANCEBASE_U16) && \
    !defined(SDW_INLINE_FREE_INSTANCESETFLAGS_INSTANCEBASE_U16_DEFINED)
#define SDW_INLINE_FREE_INSTANCESETFLAGS_INSTANCEBASE_U16_DEFINED
inline void InstanceSetFlags(InstanceBase *instance, u16 mask)
{
    instance->inst_flags |= mask;
}
#endif

#if defined(SDW_INLINE_INSTANCE_INST) && !defined(SDW_INLINE_INSTANCE_INST_DEFINED)
#define SDW_INLINE_INSTANCE_INST_DEFINED
inline Instance *Instance::Inst()
{
    return this;
}
#endif
