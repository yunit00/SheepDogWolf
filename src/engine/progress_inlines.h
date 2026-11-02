/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_ISLEVELSCENE_S8) && !defined(SDW_INLINE_FREE_ISLEVELSCENE_S8_DEFINED)
#define SDW_INLINE_FREE_ISLEVELSCENE_S8_DEFINED
inline s32 IsLevelScene(s8 scene)
{
    return scene >= SCENE_LVL_00 && scene < SCENE_LEVEL_COUNT;
}
#endif

#if defined(SDW_INLINE_PROGRESS_CURRENTLEVEL) && !defined(SDW_INLINE_PROGRESS_CURRENTLEVEL_DEFINED)
#define SDW_INLINE_PROGRESS_CURRENTLEVEL_DEFINED
inline s8 Progress::CurrentLevel()
{
    return currentLevel;
}
#endif

#if defined(SDW_INLINE_PROGRESS_FIELDACFLAGCLEAR) && !defined(SDW_INLINE_PROGRESS_FIELDACFLAGCLEAR_DEFINED)
#define SDW_INLINE_PROGRESS_FIELDACFLAGCLEAR_DEFINED
inline s32 Progress::FieldAcFlagClear()
{
    if (runtimeBits.fieldAcFlag)
        return 0;
    return 1;
}
#endif

#if defined(SDW_INLINE_PROGRESS_GETLANGUAGE) && !defined(SDW_INLINE_PROGRESS_GETLANGUAGE_DEFINED)
#define SDW_INLINE_PROGRESS_GETLANGUAGE_DEFINED
inline u8 Progress::GetLanguage()
{
    return language;
}
#endif

#if defined(SDW_INLINE_PROGRESS_GETLEVEL) && !defined(SDW_INLINE_PROGRESS_GETLEVEL_DEFINED)
#define SDW_INLINE_PROGRESS_GETLEVEL_DEFINED
inline s8 Progress::GetLevel()
{
    return currentLevel;
}
#endif

#if defined(SDW_INLINE_PROGRESS_GETLEVELINDEXA) && !defined(SDW_INLINE_PROGRESS_GETLEVELINDEXA_DEFINED)
#define SDW_INLINE_PROGRESS_GETLEVELINDEXA_DEFINED
inline s8 Progress::GetLevelIndexA()
{
    return levelIndexA;
}
#endif

#if defined(SDW_INLINE_PROGRESS_SETLEVELDONE_S8) && !defined(SDW_INLINE_PROGRESS_SETLEVELDONE_S8_DEFINED)
#define SDW_INLINE_PROGRESS_SETLEVELDONE_S8_DEFINED
/* Sets the bit within its byte of levelDoneBits; preserve the byte store. */
inline void Progress::SetLevelDone(s8 level)
{
    levelDoneBits[level >> 3] |= (u8)(1 << (level & 7));
}
#endif

#if defined(SDW_INLINE_PROGRESS_SETSCENEEXITTARGET_S8) && !defined(SDW_INLINE_PROGRESS_SETSCENEEXITTARGET_S8_DEFINED)
#define SDW_INLINE_PROGRESS_SETSCENEEXITTARGET_S8_DEFINED
inline void Progress::SetSceneExitTarget(s8 level)
{
    sceneExitTarget = level;
}
#endif

#if defined(SDW_INLINE_PROGRESS_SETSECONDDEMONEXT_S32) && !defined(SDW_INLINE_PROGRESS_SETSECONDDEMONEXT_S32_DEFINED)
#define SDW_INLINE_PROGRESS_SETSECONDDEMONEXT_S32_DEFINED
inline void Progress::SetSecondDemoNext(s32 on)
{
    runtimeBits.secondDemoNext = on ? 1 : 0;
}
#endif
