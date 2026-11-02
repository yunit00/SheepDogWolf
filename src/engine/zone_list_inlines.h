/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_ZONELIST_CLEAR) && !defined(SDW_INLINE_ZONELIST_CLEAR_DEFINED)
#define SDW_INLINE_ZONELIST_CLEAR_DEFINED
inline void ZoneList::Clear()
{
    boxes = 0;
    count = 0;
}
#endif

#if defined(SDW_INLINE_ZONELIST_CONTAINS_VEC3S) && !defined(SDW_INLINE_ZONELIST_CONTAINS_VEC3S_DEFINED)
#define SDW_INLINE_ZONELIST_CONTAINS_VEC3S_DEFINED
inline Box *ZoneList::Contains(Vec3s *point)
{
    return BoxList_FindContainingPoint(point, boxes, count);
}
#endif

#if defined(SDW_INLINE_ZONELIST_CONTAINSXZ_VEC3S) && !defined(SDW_INLINE_ZONELIST_CONTAINSXZ_VEC3S_DEFINED)
#define SDW_INLINE_ZONELIST_CONTAINSXZ_VEC3S_DEFINED
inline Box *ZoneList::ContainsXZ(Vec3s *point)
{
    return BoxList_FindContainingPointXZ(point, boxes, count);
}
#endif

#if defined(SDW_INLINE_ZONELIST_FIND_VEC3S) && !defined(SDW_INLINE_ZONELIST_FIND_VEC3S_DEFINED)
#define SDW_INLINE_ZONELIST_FIND_VEC3S_DEFINED
inline Box *ZoneList::Find(Vec3s *point)
{
    return BoxList_FindContainingPoint(point, boxes, count);
}
#endif

#if defined(SDW_INLINE_ZONELIST_FINDCONTAINING_VEC3S) && !defined(SDW_INLINE_ZONELIST_FINDCONTAINING_VEC3S_DEFINED)
#define SDW_INLINE_ZONELIST_FINDCONTAINING_VEC3S_DEFINED
inline Box *ZoneList::FindContaining(Vec3s *point)
{
    return BoxList_FindContainingPoint(point, boxes, count);
}
#endif

#if defined(SDW_INLINE_ZONELIST_FINDCONTAININGXZ_VEC3S) && !defined(SDW_INLINE_ZONELIST_FINDCONTAININGXZ_VEC3S_DEFINED)
#define SDW_INLINE_ZONELIST_FINDCONTAININGXZ_VEC3S_DEFINED
inline Box *ZoneList::FindContainingXZ(Vec3s *p)
{
    return BoxList_FindContainingPointXZ(p, boxes, count);
}
#endif

#if defined(SDW_INLINE_ZONELIST_LOAD_U16) && !defined(SDW_INLINE_ZONELIST_LOAD_U16_DEFINED)
#define SDW_INLINE_ZONELIST_LOAD_U16_DEFINED
inline void ZoneList::Load(u16 id)
{
    boxes = (Box **)Scn_FindIdList(id, &count);
}
#endif

#if defined(SDW_INLINE_ZONELIST_LOAD_U32) && !defined(SDW_INLINE_ZONELIST_LOAD_U32_DEFINED)
#define SDW_INLINE_ZONELIST_LOAD_U32_DEFINED
inline void ZoneList::Load(u32 id)
{
    boxes = (Box **)Scn_FindIdList((u16)id, &count);
}
#endif

#if defined(SDW_INLINE_ZONELIST_RESOLVE_U32) && !defined(SDW_INLINE_ZONELIST_RESOLVE_U32_DEFINED)
#define SDW_INLINE_ZONELIST_RESOLVE_U32_DEFINED
inline void ZoneList::Resolve(u32 id)
{
    boxes = (Box **)Scn_FindIdList((u16)id, &count);
}
#endif
