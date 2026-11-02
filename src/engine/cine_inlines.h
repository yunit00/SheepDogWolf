/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_CINE_ISFINISHED) && !defined(SDW_INLINE_FREE_CINE_ISFINISHED_DEFINED)
#define SDW_INLINE_FREE_CINE_ISFINISHED_DEFINED
inline s32 Cine_IsFinished()
{
    s32 finished = g_cinePlayer.finished;
    return finished;
}
#endif

#if defined(SDW_INLINE_FREE_CINE_PLAY_U32_U32_BOX_BOX_VOID) && \
    !defined(SDW_INLINE_FREE_CINE_PLAY_U32_U32_BOX_BOX_VOID_DEFINED)
#define SDW_INLINE_FREE_CINE_PLAY_U32_U32_BOX_BOX_VOID_DEFINED
inline void Cine_Play(u32 id, u32 startFlags, Box *box, Box *sheep, void *text)
{
    g_cinePlayer.Start(id, startFlags, box, sheep, text, 0);
}
#endif

#if defined(SDW_INLINE_FREE_STARTCINE_U32_U32_BOX_BOX_VOID) && \
    !defined(SDW_INLINE_FREE_STARTCINE_U32_U32_BOX_BOX_VOID_DEFINED)
#define SDW_INLINE_FREE_STARTCINE_U32_U32_BOX_BOX_VOID_DEFINED
inline void StartCine(u32 id, u32 startFlags, Box *box, Box *sheepBox, void *text)
{
    g_cinePlayer.Start(id, startFlags, box, sheepBox, text, 0);
}
#endif

#if defined(SDW_INLINE_CINE_ISACTIVE) && !defined(SDW_INLINE_CINE_ISACTIVE_DEFINED)
#define SDW_INLINE_CINE_ISACTIVE_DEFINED
inline s32 Cine::IsActive()
{
    return active;
}
#endif

#if defined(SDW_INLINE_CINE_ISFINISHED) && !defined(SDW_INLINE_CINE_ISFINISHED_DEFINED)
#define SDW_INLINE_CINE_ISFINISHED_DEFINED
inline s32 Cine::IsFinished()
{
    return finished;
}
#endif

#if defined(SDW_INLINE_CINE_ISWOLFREADY) && !defined(SDW_INLINE_CINE_ISWOLFREADY_DEFINED)
#define SDW_INLINE_CINE_ISWOLFREADY_DEFINED
inline s32 Cine::IsWolfReady()
{
    return wolfReady;
}
#endif
