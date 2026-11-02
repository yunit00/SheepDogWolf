#include "sdw_types.h"
#include "sdw_enums.h"


#define SDW_MEMBERS_ScnObject void SetPos(Vec3s *p);

#define SDW_MEMBERS_Mat44 Mat44(); /* Mat44_Ctor */
#include "sdw_classes.h"
#define SDW_INLINE_PROGRESS_SETLEVELDONE_S8 1
#include "progress_inlines.h"
#undef SDW_INLINE_PROGRESS_SETLEVELDONE_S8
#define SDW_INLINE_SCNOBJECT_SETPOS_VEC3S 1
#include "scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETPOS_VEC3S
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#define SDW_INLINE_SCNOBJECT_ISKEPT 1
#define SDW_INLINE_SCNOBJECT_SETKEPT_S32 1
#include "scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_ISKEPT
#undef SDW_INLINE_SCNOBJECT_SETKEPT_S32
#include "sdw_empty_call.h"

#include "../sdk/windef.h"
#include "../sdk/crt.h"

/* ---- globals (named in the tables) ---- */
#include "draw2d.h"
#include "stream_player.h"
#include "../objects/video_sequence.h"
#include "interface.h"
#include "../app/app_main.h"
#include "scenaric.h"
#include "sfx_volume.h"
#include "input.h"
#include "game_level.h"
#include "list.h"
#include "text.h"
#include "scn_tools.h"
#include "approach.h"
#include "lerp.h"
#include "progress_inventory.h"
extern u32 g_gameFlags;
extern s32 g_dt;
extern s32 g_frameCount2;
extern u32 *g_screenLayerBase;
extern Vec3s g_camPos;
extern "C" const s16 g_sinTable4096[];
extern "C" const s16 *g_pCosTable;


/* ---- functions ---- */
LONG Reg_CreateSubKey(HKEY *out, const char *name);
void Reg_CloseKey(HKEY *key);
DWORD Reg_ReadBinary(HKEY key, const char *name, void *buf, DWORD bufSize);
u8 Video_PlaySequence(FmvList *list);
void Progress_BuildScenePath(char *dest, s8 scene);
void Progress_ResetGlobal();
s32 Scenaric_FindByClass(u16 classId, ScnObject **out, s32 maximum);

void InvWheel_GetSlotPosColor(s32 *outXY, u8 *outRGBA, s16 angle, s32 slideY);

/* inline: a scene number that is a real level (0..31), materialised as 0/1 */
#define SDW_INLINE_FREE_ISLEVELSCENE_S8 1
#include "progress_inlines.h"
#undef SDW_INLINE_FREE_ISLEVELSCENE_S8

/* inline: the attract-demo alternation bit, stored as `on != 0` */
#define SDW_INLINE_PROGRESS_SETSECONDDEMONEXT_S32 1
#include "progress_inlines.h"
#undef SDW_INLINE_PROGRESS_SETSECONDDEMONEXT_S32

/* inline: runtime flag 0 as a 0/1 value, clear = 1 (the branchy 0/1 temporary) */
#define SDW_INLINE_PROGRESS_FIELDACFLAGCLEAR 1
#include "progress_inlines.h"
#undef SDW_INLINE_PROGRESS_FIELDACFLAGCLEAR

/* inline: the first node of a list */
#define SDW_INLINE_FREE_LIST_FIRST_LISTNODE 1
#include "list_inlines.h"
#undef SDW_INLINE_FREE_LIST_FIRST_LISTNODE

/* inline: clears game flags; the mask is complemented at run time */
#define SDW_INLINE_FREE_GAMEFLAGS_CLEAR_U32 1
#include "game_state_inlines.h"
#undef SDW_INLINE_FREE_GAMEFLAGS_CLEAR_U32

#define SDW_INLINE_FREE_SCENARIC_CLASSFLAGS_U16 1
#include "scenaric_inlines.h"
#undef SDW_INLINE_FREE_SCENARIC_CLASSFLAGS_U16

/* ---- this object's data ---- */
extern const float g_viewDistFar = 12000.0f;
extern const float g_viewDistNear = 4000.0f;
static u8 s_cineOpStride[9] = {0, 8, 8, 4, 2, 2, 4, 2, 2};
extern Progress g_progress;
Progress *g_pProgress = &g_progress;
Progress g_progress;

/* ======================================================================== Progress */

void Progress_ResetGlobal()
{
    g_pProgress->Reset();
}

/* clears the progress and reloads the options saved under the "CfgGame" registry value (7 bytes: the
 * three volumes, the sound mode, the two option bits, the view distance), or their defaults when it cannot be read. */
void Progress::Reset()
{
    s32 n;
    HKEY hkey;
    u8 config[7];
    s32 len;
    float k;
    u8 loaded;

    printf("\nINFO: GameState cleared\n");
    for (n = 0; n < 4; n++) {
        timeKeeperBits[n] = 0;
        levelDoneBits[n] = 0;
    }
    saveTimestamp = 0;
    saveSlot = 0;
    runtimeBits.cardOpen = 0;
    runtimeBits.saveSlotValid = 0;
    runtimeBits.autoSaveOn = 0;
    modeBits = 0;
    levelIndexA = SCENE_HUB;
    sceneExitTarget = SCENE_HUB;
    levelIndexB = SCENE_HUB;
    loaded = 0;
    len = 7;
    if (Reg_CreateSubKey(&hkey, "CfgGame") == 0 && Reg_ReadBinary(hkey, 0, config, len) == (DWORD)len) {
        Reg_CloseKey(&hkey);
        loaded = 1;
    }
    currentLevel = SCENE_LVL_00;
    runtimeBits.fieldAcFlag = 0;
    Progress_BuildScenePath(scenePath, currentLevel);
    language = GAME_LANG_ENGLISH;
    bonusFlags = 0;
    bonusPoints = 0;
    bonusCount = 0;
    sheepToCatch = 0x11;
    slotIcon = -1;
    if (loaded) {
        streamVolumeA = config[0];
        streamVolumeB = config[1];
        sfxVolume = config[2];
        optionBits.soundMode = config[3];
    } else {
        streamVolumeA = 0xc0;
        streamVolumeB = 0xff;
        sfxVolume = 0xff;
        optionBits.soundMode = 0;
    }
    Sound_SetSfxVolume(sfxVolume);
    if (loaded) {
        optionBits.setting = config[4];
        optionBits.gate = config[5];
    } else {
        optionBits.setting = 1;
        optionBits.gate = 1;
    }
    controls.preset = 0;
    controls.actuatorEnable = 1;
    controls.padIsAnalog = 1;
    controls.remap[0] = (u16)~PAD_CROSS;
    controls.remap[1] = (u16)~PAD_SQUARE;
    controls.remap[2] = (u16)~PAD_TRIANGLE;
    controls.remap[3] = (u16)~PAD_CIRCLE;
    controls.remap[4] = (u16)~PAD_R1;
    controls.remap[5] = (u16)~PAD_L1;
    field_aa = 0;
    field_ac = FieldAcFlagClear() ? 0 : 0x10;
    if (loaded)
        viewDistanceSetting = config[6];
    else
        viewDistanceSetting = 200;
    k = (float)(0xff - viewDistanceSetting) / 255.0f;
    g_pViewFrustum->SetViewDistance((1.0f - k) * g_viewDistNear + g_viewDistFar * k, 1);
}

/* the current level's TimeKeeper reward: its bit, and one bonus (two for level 16). No test that the bit
 * was already set; the only caller (TimeKeeper_HandleMessage) guards on the TimeKeeper's own state. */
void Progress::AwardTimeKeeper()
{
    timeKeeperBits[currentLevel >> 3] |= (u8)(1 << (currentLevel & 7));
    if (currentLevel == SCENE_LVL_16) {
        bonusCount++;
        bonusPoints++;
    }
    bonusCount++;
    bonusPoints++;
}

/* the level's TimeKeeper bit, masked but not normalised to 0/1 */
u32 Progress::IsTimeKeeperDone(s8 level)
{
    return timeKeeperBits[level >> 3] & (1 << (level & 7));
}

void Progress::SetBonusPoints(u16 points)
{
    bonusPoints = points;
}

/* loads the saved 0x2c-byte record and re-applies the options it holds */
void Progress::LoadRecord(const void *src)
{
    float t;

    memcpy(timeKeeperBits, src, 0x2c);
    Stub_Ret(sfxVolume);
    Stub_Ret(optionBits.setting);
    g_pStreamPlayer->ApplyVolume();
    Input_ApplyControlConfig(&controls);
    t = (float)(0xff - viewDistanceSetting) / 255.0f;
    g_pViewFrustum->SetViewDistance((1.0f - t) * g_viewDistNear + g_viewDistFar * t, 1);
}

/* the saved record: the 0x2c bytes from +0x84 */
void Progress::CopyRecord(void *dest)
{
    memcpy(dest, timeKeeperBits, 0x2c);
}

u8 Progress::IsLevelDone(s8 level)
{
    return (levelDoneBits[level >> 3] >> (level & 7)) & 1;
}

/* picks the scene that follows the current one and goes there. A level (0..31) left with success marks
 * its first completion in levelIndexA, raises slotIcon, and returns to the Scene hub (-3); level 16 also marks itself
 * done (and takes one from sheepToCatch the first time) and goes on to Fend (-4), level 17 to the Ending (-5).
 * A level left without success goes to the hub, or to the title (-1) from level 0. The non-level scenes:
 * Wheel/title (-1) -> Intro (-2) until level 0 is done, then the hub; Intro -> level 0; hub -> sceneExitTarget on
 * success, else the title; Fend -> level 17; Ending -> the credits (-8); credits and the demos -> the title, the demos
 * flipping which one plays next. Going to the title (-1) resets the progress. */
void Progress::Level_FinishScene(s32 success)
{
    s8 next;

    levelIndexA = SCENE_HUB;
    if (IsLevelScene(currentLevel)) {
        if (success) {
            if (!g_pProgress->IsLevelDone(currentLevel))
                levelIndexA = currentLevel;
            if (currentLevel > slotIcon)
                slotIcon = currentLevel;
            if (currentLevel == SCENE_LVL_16) {
                if (!IsLevelDone(currentLevel))
                    sheepToCatch = sheepToCatch - 1;
                SetLevelDone(currentLevel);
                next = SCENE_FEND;
            } else if (currentLevel == SCENE_LVL_17) {
                next = SCENE_ENDING;
            } else {
                next = SCENE_HUB;
            }
        } else {
            if (currentLevel == SCENE_LVL_00)
                next = SCENE_WHEEL;
            else
                next = SCENE_HUB;
        }
    } else {
        switch (currentLevel) {
            case SCENE_INTRO:
                next = SCENE_LVL_00;
                break;
            case SCENE_FEND:
                next = SCENE_LVL_17;
                break;
            case SCENE_DEMO_A:
                next = SCENE_WHEEL;
                SetSecondDemoNext(1);
                break;
            case SCENE_DEMO_B:
                next = SCENE_WHEEL;
                SetSecondDemoNext(0);
                break;
            case SCENE_END_FMV:
                next = SCENE_WHEEL;
                break;
            case SCENE_ENDING:
                next = SCENE_END_FMV;
                break;
            case SCENE_WHEEL:
                if (g_pProgress->IsLevelDone(SCENE_LVL_00))
                    next = SCENE_HUB;
                else
                    next = SCENE_INTRO;
                break;
            case SCENE_HUB:
                if (success)
                    next = sceneExitTarget;
                else
                    next = SCENE_WHEEL;
                break;
            default:
                next = SCENE_WHEEL;
                break;
        }
    }
    if (next == SCENE_WHEEL)
        g_pProgress->Reset();
    GotoScene(next);
}

/* the attract demo: the two demo scenes (-6, -7) alternate */
void Progress::StartAttractDemo()
{
    s8 next;

    levelIndexA = SCENE_HUB;
    if (runtimeBits.secondDemoNext)
        next = SCENE_DEMO_B;
    else
        next = SCENE_DEMO_A;
    GotoScene(next);
}

/* the path prefix of a scene's files */
void Progress_BuildScenePath(char *dest, s8 scene)
{
    switch (scene) {
        case SCENE_WHEEL:
            strcpy(dest, g_pathWheelDir);
            break;
        case SCENE_INTRO:
            strcpy(dest, g_introDir);
            break;
        case SCENE_HUB:
            strcpy(dest, g_pathScene);
            break;
        case SCENE_FEND:
            strcpy(dest, g_pathFendDir);
            break;
        case SCENE_ENDING:
            strcpy(dest, g_pathEnding);
            break;
        case SCENE_DEMO_A:
            sprintf(dest, g_pathDemoDir, 3);
            break;
        case SCENE_DEMO_B:
            sprintf(dest, g_pathDemoDir, 4);
            break;
        default:
            sprintf(dest, g_levelPathFmt, scene, scene);
            break;
    }
}

/* scene -8 plays the credits and finishes at once; any other scene is loaded (a level also counts one
 * more level entry in modeBits). */
s32 Progress::GotoScene(s8 scene)
{
    if (scene == SCENE_END_FMV) {
        Video_PlaySequence(&g_fmvListCredits);
        levelIndexB = currentLevel;
        currentLevel = scene;
        Level_FinishScene(0);
    } else {
        if (IsLevelScene(scene))
            modeBits++;
        Progress_BuildScenePath(scenePath, scene);
        levelIndexB = currentLevel;
        currentLevel = scene;
        sceneExitTarget = SCENE_HUB;
        Game_FreeLevel();
        Game_ReloadLevel();
    }
    return 1;
}
