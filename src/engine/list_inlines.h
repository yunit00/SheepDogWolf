/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_LIST_FIRST_LISTNODE) && !defined(SDW_INLINE_FREE_LIST_FIRST_LISTNODE_DEFINED)
#define SDW_INLINE_FREE_LIST_FIRST_LISTNODE_DEFINED
inline ListNode *List_First(ListNode **list)
{
    return *list;
}
#endif
