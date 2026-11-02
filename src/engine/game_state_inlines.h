/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_CLEARGAMEFLAGS_U32) && !defined(SDW_INLINE_FREE_CLEARGAMEFLAGS_U32_DEFINED)
#define SDW_INLINE_FREE_CLEARGAMEFLAGS_U32_DEFINED
inline void ClearGameFlags(u32 mask)
{
    g_gameFlags &= ~mask;
}
#endif

#if defined(SDW_INLINE_FREE_GAMEFLAGS_CLEAR_U32) && !defined(SDW_INLINE_FREE_GAMEFLAGS_CLEAR_U32_DEFINED)
#define SDW_INLINE_FREE_GAMEFLAGS_CLEAR_U32_DEFINED
inline void GameFlags_Clear(u32 mask)
{
    g_gameFlags &= ~mask;
}
#endif

#if defined(SDW_INLINE_FREE_GAME_CLEARFLAGS_U32) && !defined(SDW_INLINE_FREE_GAME_CLEARFLAGS_U32_DEFINED)
#define SDW_INLINE_FREE_GAME_CLEARFLAGS_U32_DEFINED
inline void Game_ClearFlags(u32 mask)
{
    g_gameFlags &= ~mask;
}
#endif

#if defined(SDW_INLINE_FREE_SETGAMEFLAGS_U32) && !defined(SDW_INLINE_FREE_SETGAMEFLAGS_U32_DEFINED)
#define SDW_INLINE_FREE_SETGAMEFLAGS_U32_DEFINED
inline void SetGameFlags(u32 mask)
{
    g_gameFlags |= mask;
}
#endif
