/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_SCNBODY_ANIMFLAGS_U16) && !defined(SDW_INLINE_SCNBODY_ANIMFLAGS_U16_DEFINED)
#define SDW_INLINE_SCNBODY_ANIMFLAGS_U16_DEFINED
inline s32 ScnBody::AnimFlags(u16 mask)
{
    return anim.flags & mask;
}
#endif

#if defined(SDW_INLINE_SCNBODY_ANIMID) && !defined(SDW_INLINE_SCNBODY_ANIMID_DEFINED)
#define SDW_INLINE_SCNBODY_ANIMID_DEFINED
inline u16 ScnBody::AnimId()
{
    return anim.animId;
}
#endif

#if defined(SDW_INLINE_SCNBODY_CURRENTANIM) && !defined(SDW_INLINE_SCNBODY_CURRENTANIM_DEFINED)
#define SDW_INLINE_SCNBODY_CURRENTANIM_DEFINED
inline u16 ScnBody::CurrentAnim()
{
    return anim.animId;
}
#endif

#if defined(SDW_INLINE_SCNBODY_GETANIMID) && !defined(SDW_INLINE_SCNBODY_GETANIMID_DEFINED)
#define SDW_INLINE_SCNBODY_GETANIMID_DEFINED
inline u16 ScnBody::GetAnimId()
{
    return anim.animId;
}
#endif

#if defined(SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32) && !defined(SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32_DEFINED)
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32_DEFINED
inline void ScnBody::PlayAnim(u16 id, s32 loop, s32 blend)
{
    u32 options = 0;
    if (loop)
        options |= ANIM_SET_LOOP;
    if (blend)
        options |= ANIM_SET_BLEND;
    Anim_Start(Inst(), &anim, id, options);
}
#endif
