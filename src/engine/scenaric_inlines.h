/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_ISHELD_SCNOBJECT_VEC3S) && !defined(SDW_INLINE_FREE_ISHELD_SCNOBJECT_VEC3S_DEFINED)
#define SDW_INLINE_FREE_ISHELD_SCNOBJECT_VEC3S_DEFINED
inline s32 IsHeld(ScnObject *obj, Vec3s *holder)
{
    if (!obj->InstFlags(INST_F_ATTACHED))
        return 0;
    holder->x = obj->attachLink->parentObj->pos.x;
    holder->y = obj->attachLink->parentObj->pos.y;
    holder->z = obj->attachLink->parentObj->pos.z;
    return 1;
}
#endif

#if defined(SDW_INLINE_FREE_SCENARIC_CLASSFLAGS_U16) && !defined(SDW_INLINE_FREE_SCENARIC_CLASSFLAGS_U16_DEFINED)
#define SDW_INLINE_FREE_SCENARIC_CLASSFLAGS_U16_DEFINED
inline u32 Scenaric_ClassFlags(u16 classId)
{
    return g_scenaricClassRegistry[classId].classFlags;
}
#endif

#if defined(SDW_INLINE_FREE_ZONE_GETLIST_U8) && !defined(SDW_INLINE_FREE_ZONE_GETLIST_U8_DEFINED)
#define SDW_INLINE_FREE_ZONE_GETLIST_U8_DEFINED
inline ZoneList *Zone_GetList(u8 type)
{
    return &g_waterZones[type];
}
#endif

#if defined(SDW_INLINE_FREE_ZONES_GET_U8) && !defined(SDW_INLINE_FREE_ZONES_GET_U8_DEFINED)
#define SDW_INLINE_FREE_ZONES_GET_U8_DEFINED
inline ZoneList *Zones_Get(u8 type)
{
    return &g_waterZones[type];
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ATTACHTO_SCNOBJECT_U8_CONST_VEC3S_VEC3S_U32_U32) && \
    !defined(SDW_INLINE_SCNOBJECT_ATTACHTO_SCNOBJECT_U8_CONST_VEC3S_VEC3S_U32_U32_DEFINED)
#define SDW_INLINE_SCNOBJECT_ATTACHTO_SCNOBJECT_U8_CONST_VEC3S_VEC3S_U32_U32_DEFINED
inline void ScnObject::AttachTo(ScnObject *parent, u8 joint, const Vec3s *offset, Vec3s *rotation, u32 arg, uptr arg2)
{
    AttachTo(parent, joint, (Vec3s *)offset, rotation, arg, arg2);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_BROADCASTAROUND_S32_S32_U16_U32_VOID) && \
    !defined(SDW_INLINE_SCNOBJECT_BROADCASTAROUND_S32_S32_U16_U32_VOID_DEFINED)
#define SDW_INLINE_SCNOBJECT_BROADCASTAROUND_S32_S32_U16_U32_VOID_DEFINED
inline void ScnObject::BroadcastAround(s32 below, s32 above, u16 radius, u32 msg, void *arg)
{
    s16 minY;
    s16 maxY;
    maxY = pos.y + above;
    minY = pos.y - below;
    Scenaric_BroadcastInRadius(CLASSID_NONE, minY, maxY, radius, msg, arg, 0);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_DROP_VEC3S) && !defined(SDW_INLINE_SCNOBJECT_DROP_VEC3S_DEFINED)
#define SDW_INLINE_SCNOBJECT_DROP_VEC3S_DEFINED
inline void ScnObject::Drop(Vec3s *where)
{
    Detach();
    SetPosition(where);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ENABLEBOXCOLLIDE_S32) && !defined(SDW_INLINE_SCNOBJECT_ENABLEBOXCOLLIDE_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_ENABLEBOXCOLLIDE_S32_DEFINED
inline void ScnObject::EnableBoxCollide(s32 on)
{
    if (on)
        flags &= (u16)~SCN_OF_NO_BOX_COLLIDE;
    else
        flags |= SCN_OF_NO_BOX_COLLIDE;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ENABLETINT_S32) && !defined(SDW_INLINE_SCNOBJECT_ENABLETINT_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_ENABLETINT_S32_DEFINED
inline void ScnObject::EnableTint(s32 on)
{
    if (on)
        InstFlagsSet(&inst_flags, INST_F_TINT);
    else
        InstFlagsClear(&inst_flags, INST_F_TINT);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_FACING) && !defined(SDW_INLINE_SCNOBJECT_FACING_DEFINED)
#define SDW_INLINE_SCNOBJECT_FACING_DEFINED
#if SDW_INLINE_SCNOBJECT_FACING == 1
inline s16 ScnObject::Facing()
{
    return rot.y;
}
#elif SDW_INLINE_SCNOBJECT_FACING == 2
inline s16 ScnObject::Facing()
{
    s16 result = rot.y;
    return result;
}
#endif
#endif

#if defined(SDW_INLINE_SCNOBJECT_FIRSTBOX) && !defined(SDW_INLINE_SCNOBJECT_FIRSTBOX_DEFINED)
#define SDW_INLINE_SCNOBJECT_FIRSTBOX_DEFINED
inline CollBox *ScnObject::FirstBox()
{
    ModelBoxList *list = inst_model->boxes;
    if (list)
        return list->boxes;
    return 0;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_FLAGSCLEAR_U32) && !defined(SDW_INLINE_SCNOBJECT_FLAGSCLEAR_U32_DEFINED)
#define SDW_INLINE_SCNOBJECT_FLAGSCLEAR_U32_DEFINED
inline s32 ScnObject::FlagsClear(u32 mask)
{
    return !(flags & mask);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETCLASSID) && !defined(SDW_INLINE_SCNOBJECT_GETCLASSID_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETCLASSID_DEFINED
inline u16 ScnObject::GetClassId()
{
    return classId;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETFACING) && !defined(SDW_INLINE_SCNOBJECT_GETFACING_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETFACING_DEFINED
inline s16 ScnObject::GetFacing()
{
    return rot.y;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETFIRSTMODELBOXINLINE) && \
    !defined(SDW_INLINE_SCNOBJECT_GETFIRSTMODELBOXINLINE_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETFIRSTMODELBOXINLINE_DEFINED
/* The emitted copy stays local. */
inline CollBox *ScnObject::GetFirstModelBoxInline()
{
    ModelBoxList *list = inst_model->boxes;
    if (list)
        return (CollBox *)list->boxes;
    return 0;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETFLAGS) && !defined(SDW_INLINE_SCNOBJECT_GETFLAGS_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETFLAGS_DEFINED
inline u16 ScnObject::GetFlags()
{
    return flags;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETHEADING) && !defined(SDW_INLINE_SCNOBJECT_GETHEADING_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETHEADING_DEFINED
inline s16 ScnObject::GetHeading()
{
    return rot.y;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETMODELBOX) && !defined(SDW_INLINE_SCNOBJECT_GETMODELBOX_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETMODELBOX_DEFINED
inline CollBox *ScnObject::GetModelBox()
{
    ModelBoxList *list = inst_model->boxes;
    if (list)
        return list->boxes;
    return 0;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETMODELBOXES_COLLBOX_U32) && \
    !defined(SDW_INLINE_SCNOBJECT_GETMODELBOXES_COLLBOX_U32_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETMODELBOXES_COLLBOX_U32_DEFINED
inline void ScnObject::GetModelBoxes(CollBox **out, u32 *count)
{
    ModelBoxList *list = inst_model->boxes;
    if (!list) {
        *count = 0;
        *out = 0;
    } else {
        *count = list->count;
        *out = list->boxes;
    }
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETMODELBOXES_U32) && !defined(SDW_INLINE_SCNOBJECT_GETMODELBOXES_U32_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETMODELBOXES_U32_DEFINED
inline CollBox *ScnObject::GetModelBoxes(u32 *count)
{
    ModelBoxList *list = inst_model->boxes;
    if (!list) {
        *count = 0;
        return 0;
    }
    *count = list->count;
    return list->boxes;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_GETPARENT) && !defined(SDW_INLINE_SCNOBJECT_GETPARENT_DEFINED)
#define SDW_INLINE_SCNOBJECT_GETPARENT_DEFINED
inline ScnObject *ScnObject::GetParent()
{
    if (!InstFlags(INST_F_ATTACHED))
        return 0;
    return attachLink->parentObj;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_INWORLD) && !defined(SDW_INLINE_SCNOBJECT_INWORLD_DEFINED)
#define SDW_INLINE_SCNOBJECT_INWORLD_DEFINED
inline s32 ScnObject::InWorld()
{
    return (flags & SCN_OF_IN_WORLD) != 0;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_INSTFLAGS_U16) && !defined(SDW_INLINE_SCNOBJECT_INSTFLAGS_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_INSTFLAGS_U16_DEFINED
inline s32 ScnObject::InstFlags(u16 mask)
{
    return inst_flags & mask;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_INSTANCEFLAGS_U16) && !defined(SDW_INLINE_SCNOBJECT_INSTANCEFLAGS_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_INSTANCEFLAGS_U16_DEFINED
inline s32 ScnObject::InstanceFlags(u16 mask)
{
    return inst_flags & mask;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ISACTIVE) && !defined(SDW_INLINE_SCNOBJECT_ISACTIVE_DEFINED)
#define SDW_INLINE_SCNOBJECT_ISACTIVE_DEFINED
inline s32 ScnObject::IsActive()
{
    return (flags & (SCN_OF_HIDDEN2 | SCN_OF_HIDDEN)) == 0;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ISCOLLIDABLE) && !defined(SDW_INLINE_SCNOBJECT_ISCOLLIDABLE_DEFINED)
#define SDW_INLINE_SCNOBJECT_ISCOLLIDABLE_DEFINED
inline s32 ScnObject::IsCollidable()
{
    return !(flags & SCN_OF_NO_BOX_COLLIDE);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ISINWORLD) && !defined(SDW_INLINE_SCNOBJECT_ISINWORLD_DEFINED)
#define SDW_INLINE_SCNOBJECT_ISINWORLD_DEFINED
inline s32 ScnObject::IsInWorld()
{
    return (flags & SCN_OF_IN_WORLD) != 0;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ISKEPT) && !defined(SDW_INLINE_SCNOBJECT_ISKEPT_DEFINED)
#define SDW_INLINE_SCNOBJECT_ISKEPT_DEFINED
inline s32 ScnObject::IsKept()
{
    return (flags & SCN_OF_KEPT) != 0;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ISSOUNDPLAYING_U16) && !defined(SDW_INLINE_SCNOBJECT_ISSOUNDPLAYING_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_ISSOUNDPLAYING_U16_DEFINED
inline s32 ScnObject::IsSoundPlaying(u16 handle)
{
    return Sound_IsPlaying(handle);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_ISVISIBLE) && !defined(SDW_INLINE_SCNOBJECT_ISVISIBLE_DEFINED)
#define SDW_INLINE_SCNOBJECT_ISVISIBLE_DEFINED
inline s32 ScnObject::IsVisible()
{
    return (flags & (SCN_OF_HIDDEN2 | SCN_OF_HIDDEN)) == 0;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_PLAYSOUND_U16_U16_U8_S32) && \
    !defined(SDW_INLINE_SCNOBJECT_PLAYSOUND_U16_U16_U8_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_PLAYSOUND_U16_U16_U8_S32_DEFINED
inline u16 ScnObject::PlaySound(u16 id, u16 volume, u8 flags, s32 pitch)
{
    return Sound_Play(id, this, volume, flags, pitch);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_RECORD) && !defined(SDW_INLINE_SCNOBJECT_RECORD_DEFINED)
#define SDW_INLINE_SCNOBJECT_RECORD_DEFINED
inline void *ScnObject::Record()
{
    return record;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETATTACHMENT_U8_CONST_VEC3S_CONST_VEC3S_S32_CONST_VEC3S) && \
    !defined(SDW_INLINE_SCNOBJECT_SETATTACHMENT_U8_CONST_VEC3S_CONST_VEC3S_S32_CONST_VEC3S_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETATTACHMENT_U8_CONST_VEC3S_CONST_VEC3S_S32_CONST_VEC3S_DEFINED
inline void ScnObject::SetAttachment(u8 joint, const Vec3s *localOffset, const Vec3s *rotation, s32 rootRotation,
                                     const Vec3s *worldOffset)
{
    AttachLink_SetParams(attachLink, joint, localOffset, rotation, rootRotation, worldOffset);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETBOXCOLLIDE_S32) && !defined(SDW_INLINE_SCNOBJECT_SETBOXCOLLIDE_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETBOXCOLLIDE_S32_DEFINED
inline void ScnObject::SetBoxCollide(s32 on)
{
    if (on)
        flags &= (u16)~SCN_OF_NO_BOX_COLLIDE;
    else
        flags |= SCN_OF_NO_BOX_COLLIDE;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETCOLLIDABLE_S32) && !defined(SDW_INLINE_SCNOBJECT_SETCOLLIDABLE_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETCOLLIDABLE_S32_DEFINED
inline void ScnObject::SetCollidable(s32 on)
{
    if (on)
        flags &= (u16)~SCN_OF_NO_BOX_COLLIDE;
    else
        flags |= SCN_OF_NO_BOX_COLLIDE;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETCOLLISION_S32) && !defined(SDW_INLINE_SCNOBJECT_SETCOLLISION_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETCOLLISION_S32_DEFINED
inline void ScnObject::SetCollision(s32 on)
{
    if (on)
        flags &= (u16)~SCN_OF_NO_BOX_COLLIDE;
    else
        flags |= SCN_OF_NO_BOX_COLLIDE;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETCONTACTENABLED_S32) && !defined(SDW_INLINE_SCNOBJECT_SETCONTACTENABLED_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETCONTACTENABLED_S32_DEFINED
inline void ScnObject::SetContactEnabled(s32 on)
{
    if (on)
        flags &= (u16)~SCN_OF_NO_BOX_COLLIDE;
    else
        flags |= SCN_OF_NO_BOX_COLLIDE;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETDRAWMODE_U32) && !defined(SDW_INLINE_SCNOBJECT_SETDRAWMODE_U32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETDRAWMODE_U32_DEFINED
inline void ScnObject::SetDrawMode(u32 mode)
{
    partHeight = (u8)(mode >> 4);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETFACING_S16) && !defined(SDW_INLINE_SCNOBJECT_SETFACING_S16_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETFACING_S16_DEFINED
inline void ScnObject::SetFacing(s16 f)
{
    rot.y = f;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETFACING_U16) && !defined(SDW_INLINE_SCNOBJECT_SETFACING_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETFACING_U16_DEFINED
inline void ScnObject::SetFacing(u16 f)
{
    rot.y = f;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETFLAG40_S32) && !defined(SDW_INLINE_SCNOBJECT_SETFLAG40_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETFLAG40_S32_DEFINED
inline void ScnObject::SetFlag40(s32 on)
{
    if (on)
        flags |= SCN_OF_NO_DIST_CULL;
    else
        flags &= (u16)~SCN_OF_NO_DIST_CULL;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETHEADING_S16) && !defined(SDW_INLINE_SCNOBJECT_SETHEADING_S16_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETHEADING_S16_DEFINED
inline void ScnObject::SetHeading(s16 value)
{
    rot.y = value;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETINSTFLAG_U16_S32) && !defined(SDW_INLINE_SCNOBJECT_SETINSTFLAG_U16_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETINSTFLAG_U16_S32_DEFINED
inline void ScnObject::SetInstFlag(u16 mask, s32 on)
{
    if (on) {
        u16 *f = &inst_flags;
        *f |= mask;
    } else {
        u16 *f = &inst_flags;
        *f &= (u16)~mask;
    }
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETKEPT_S32) && !defined(SDW_INLINE_SCNOBJECT_SETKEPT_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETKEPT_S32_DEFINED
inline void ScnObject::SetKept(s32 on)
{
    if (on)
        flags |= SCN_OF_KEPT;
    else
        flags &= (u16)~SCN_OF_KEPT;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETMOVEMENTENABLED_S32) && \
    !defined(SDW_INLINE_SCNOBJECT_SETMOVEMENTENABLED_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETMOVEMENTENABLED_S32_DEFINED
inline void ScnObject::SetMovementEnabled(s32 on)
{
    if (on)
        flags &= (u16)~SCN_OF_NO_BOX_COLLIDE;
    else
        flags |= SCN_OF_NO_BOX_COLLIDE;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETNOCULL_S32) && !defined(SDW_INLINE_SCNOBJECT_SETNOCULL_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETNOCULL_S32_DEFINED
inline void ScnObject::SetNoCull(s32 on)
{
    if (on)
        flags |= SCN_OF_NO_DIST_CULL;
    else
        flags &= (u16)~SCN_OF_NO_DIST_CULL;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETNODISTCULL_S32) && !defined(SDW_INLINE_SCNOBJECT_SETNODISTCULL_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETNODISTCULL_S32_DEFINED
inline void ScnObject::SetNoDistCull(s32 on)
{
    if (on)
        flags |= SCN_OF_NO_DIST_CULL;
    else
        flags &= (u16)~SCN_OF_NO_DIST_CULL;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETPARTHEIGHT_U32) && !defined(SDW_INLINE_SCNOBJECT_SETPARTHEIGHT_U32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETPARTHEIGHT_U32_DEFINED
inline void ScnObject::SetPartHeight(u32 value)
{
    partHeight = value >> 4;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETPOS_VEC3S) && !defined(SDW_INLINE_SCNOBJECT_SETPOS_VEC3S_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETPOS_VEC3S_DEFINED
inline void ScnObject::SetPos(Vec3s *p)
{
    pos = *p;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETPOS_CONST_VEC3S) && !defined(SDW_INLINE_SCNOBJECT_SETPOS_CONST_VEC3S_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETPOS_CONST_VEC3S_DEFINED
inline void ScnObject::SetPos(const Vec3s *p)
{
    pos = *p;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETROTATION_VEC3S) && !defined(SDW_INLINE_SCNOBJECT_SETROTATION_VEC3S_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETROTATION_VEC3S_DEFINED
inline void ScnObject::SetRotation(Vec3s *rotation)
{
    rot = *rotation;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETROTATION_CONST_VEC3S) && \
    !defined(SDW_INLINE_SCNOBJECT_SETROTATION_CONST_VEC3S_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETROTATION_CONST_VEC3S_DEFINED
inline void ScnObject::SetRotation(const Vec3s &value)
{
    rot = value;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETSOUNDRATE_U16_S32) && !defined(SDW_INLINE_SCNOBJECT_SETSOUNDRATE_U16_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETSOUNDRATE_U16_S32_DEFINED
inline void ScnObject::SetSoundRate(u16 handle, s32 rate)
{
    Sound_SetRate(handle, rate);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32) && !defined(SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32_DEFINED
#if SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32 == 1
inline void ScnObject::SetTint(u32 color, s16 amount, s32 on)
{
    tintColor = color;
    tintAmount = amount;
    if (on)
        InstFlags_Set(&inst_flags, INST_F_TINT);
    else
        InstFlags_Clear(&inst_flags, INST_F_TINT);
}
#elif SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32 == 2
inline void ScnObject::SetTint(u32 color, s16 amount, s32 on)
{
    tintColor = color;
    tintAmount = amount;
    if (on)
        Bits16_Set(&inst_flags, INST_F_TINT);
    else
        Bits16_Clear(&inst_flags, INST_F_TINT);
}
#elif SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32 == 3
inline void ScnObject::SetTint(u32 color, s16 amount, s32 on)
{
    tintColor = color;
    tintAmount = amount;
    if (on)
        InstFlagsSet(&inst_flags, INST_F_TINT);
    else
        InstFlagsClear(&inst_flags, INST_F_TINT);
}
#endif
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETTINTOVERRIDE_S32) && !defined(SDW_INLINE_SCNOBJECT_SETTINTOVERRIDE_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETTINTOVERRIDE_S32_DEFINED
inline void ScnObject::SetTintOverride(s32 on)
{
    if (on)
        Flags16_Set(&inst_flags, INST_F_TINT);
    else
        Flags16_Clear(&inst_flags, INST_F_TINT);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETTINTED_S32) && !defined(SDW_INLINE_SCNOBJECT_SETTINTED_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETTINTED_S32_DEFINED
inline void ScnObject::SetTinted(s32 on)
{
    if (on)
        InstFlags_Set(&inst_flags, INST_F_TINT);
    else
        InstFlags_Clear(&inst_flags, INST_F_TINT);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32) && !defined(SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32_DEFINED
inline void ScnObject::SetUpdateMode(s32 mode)
{
    switch (mode) {
        case SCN_UPD_NORMAL:
            flags &= (u16)~SCN_OF_ALWAYS_UPDATE;
            flags &= (u16)~SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_ALWAYS:
            flags |= SCN_OF_ALWAYS_UPDATE;
            flags &= (u16)~SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_NEVER:
            flags &= (u16)~SCN_OF_ALWAYS_UPDATE;
            flags |= SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_CINE:
            flags |= SCN_OF_CINE_UPDATE;
            break;
    }
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U32) && !defined(SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U32_DEFINED
inline void ScnObject::SetUpdateMode(u32 mode)
{
    switch (mode) {
        case SCN_UPD_NORMAL:
            flags &= (u16)~SCN_OF_ALWAYS_UPDATE;
            flags &= (u16)~SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_ALWAYS:
            flags |= SCN_OF_ALWAYS_UPDATE;
            flags &= (u16)~SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_NEVER:
            flags &= (u16)~SCN_OF_ALWAYS_UPDATE;
            flags |= SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_CINE:
            flags |= SCN_OF_CINE_UPDATE;
            break;
    }
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8) && !defined(SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_U8_DEFINED
inline void ScnObject::SetUpdateMode(u8 mode)
{
    switch (mode) {
        case SCN_UPD_NORMAL:
            flags &= (u16)~SCN_OF_ALWAYS_UPDATE;
            flags &= (u16)~SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_ALWAYS:
            flags |= SCN_OF_ALWAYS_UPDATE;
            flags &= (u16)~SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_NEVER:
            flags &= (u16)~SCN_OF_ALWAYS_UPDATE;
            flags |= SCN_OF_NEVER_UPDATE;
            break;
        case SCN_UPD_CINE:
            flags |= SCN_OF_CINE_UPDATE;
            break;
    }
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SETVISIBLE_S32) && !defined(SDW_INLINE_SCNOBJECT_SETVISIBLE_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_SETVISIBLE_S32_DEFINED
inline void ScnObject::SetVisible(s32 on)
{
    if (on)
        flags &= (u16)~SCN_OF_HIDDEN;
    else
        flags |= SCN_OF_HIDDEN;
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_SOUNDISPLAYING_U16) && !defined(SDW_INLINE_SCNOBJECT_SOUNDISPLAYING_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_SOUNDISPLAYING_U16_DEFINED
inline s32 ScnObject::SoundIsPlaying(u16 handle)
{
    return Sound_IsPlaying(handle);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16) && \
    !defined(SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_DEFINED
inline void ScnObject::StartCamera(u16 rx, u16 ry, u16 rz, Vec3s *eye, u16 focal)
{
    Camera_StartScripted(this, &g_camera, rx, ry, rz, eye, focal, 0, 0x1000);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_U32) && \
    !defined(SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_U32_DEFINED)
#define SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_U32_DEFINED
inline void ScnObject::StartCamera(u16 rx, u16 ry, u16 rz, Vec3s *eye, u16 focal, u32 mode)
{
    Camera_StartScripted(this, &g_camera, rx, ry, rz, eye, focal, mode, 0x1000);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_U32_S32) && \
    !defined(SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_U32_S32_DEFINED)
#define SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16_U32_S32_DEFINED
inline void ScnObject::StartCamera(u16 x, u16 y, u16 z, Vec3s *point, u16 focal, u32 mode, s32 time)
{
    Camera_StartScripted(this, &g_camera, x, y, z, point, focal, mode, time);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_STARTCAMERABLENDED_U16_U16_U16_VEC3S_U16) && \
    !defined(SDW_INLINE_SCNOBJECT_STARTCAMERABLENDED_U16_U16_U16_VEC3S_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_STARTCAMERABLENDED_U16_U16_U16_VEC3S_U16_DEFINED
inline void ScnObject::StartCameraBlended(u16 rotX, u16 rotY, u16 rotZ, Vec3s *eye, u16 focal)
{
    Camera_StartScripted(this, &g_camera, rotX, rotY, rotZ, eye, focal, CAMSCR_BLEND_IN | CAMSCR_BLEND_OUT, 0x1000);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_STOPSOUND_U16) && !defined(SDW_INLINE_SCNOBJECT_STOPSOUND_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_STOPSOUND_U16_DEFINED
inline void ScnObject::StopSound(u16 handle)
{
    Sound_Stop(handle, this);
}
#endif

#if defined(SDW_INLINE_SCNOBJECT_STOPSOUNDHANDLE_U16) && !defined(SDW_INLINE_SCNOBJECT_STOPSOUNDHANDLE_U16_DEFINED)
#define SDW_INLINE_SCNOBJECT_STOPSOUNDHANDLE_U16_DEFINED
inline void ScnObject::StopSoundHandle(u16 handle)
{
    Sound_Stop(handle, this);
}
#endif
