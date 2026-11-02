/*
 * The stride table is defined here (a header static); the pad word declarations are macros though nothing here reads
 * them. Cine_Start and Cine_Stop call Rewind and Sound_SetSfxVolume out of line: Rewind is defined after them and
 * Sound_SetSfxVolume is in the next object. LINK never binds a static symbol across objects; counted that way, the
 * object's layout is identical.
 */

#define SDW_MEMBERS_Cine Cine(); /* Cine_Construct */
#define SDW_MEMBERS_ScnObject \
    void AttachTo(ScnObject *parent, u8 joint, Vec3s *offset, Vec3s *rot, u32 arg, uptr arg2);

#include "sdw_types.h"
#include "sdw_enums.h"
class Instance;
struct Animator;
u32 Anim_Start(Instance *inst, Animator *animator, u16 id, u32 opts);
#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "../objects/instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32

/* this object's copy of the cinematic header's opcode-stride table, indexed by Cine_Rewind and Cine_InitRecordCursors */
static u8 g_cineOpStride3[9] = {0, 8, 8, 4, 2, 2, 4, 2, 2};
#include "../app/app_main.h"
#include "progress.h"
#include "draw2d.h"
#include "time.h"
#include "sfx_volume.h"
#include "scn_tools.h"
#include "interface.h"
#include "input.h"
#include "fade.h"
#include "../objects/camera.h"
#include "prompt.h"
extern Wolf *g_pWolf;
/* The pad words below are not objects of their own: g_padMasks is &g_inputMap[4] and the two button words are fields of
 * g_pad, all defined by the Input object. Spelled as the element / fields they are, they compile to the same addresses
 * and leave no undefined symbol for the link. */
#define g_padMasks (g_inputMap + 4)           /* active-low button masks; [10] = action */
#define g_padCurButtons (g_pad.cur.buttons)
#define g_padPrevButtons (g_pad.prev.buttons)
extern void *g_dialogueCurText;
extern u32 g_scrollTextFlags; /* ScrollText flags: 1 paging started, 2 finished, 0x10 page turned */
extern u32 g_gameTime;

void Dialogue_SetBoxActive(s32 active);
void Camera_StartScripted(ScnObject *owner, Camera *cam, u16 rotX, u16 rotY, u16 rotZ, Vec3s *pos, u16 focal, u32 mode,
                          s32 time);

/* starts cinematic id: freezes the Wolf (msg 0xe) unless flags & 0x200, takes the level's default boxes
 * when none are given and the flags ask for them, loads and resolves the script, with flags & 0x10 plays it relative
 * to the Wolf (worldOffset = Wolf position - his first position key; no key clears the flag), opens the letterbox,
 * with flags & 0x4000 mutes the sound effects, and rewinds. */
void Cine::Start(u32 id, u32 startFlags, Box *box, Box *sheep, void *text, const CineDialogue *dlg)
{
    Vec3s *key;
    cineId = id;
    flags = startFlags;
    dialogueText = text;
    active = 1;
    finished = 0;
    wolfReady = 0;
    if (g_pWolf != 0 && !(startFlags & CINE_NO_WOLF_FREEZE))
        wolfFrozen = g_pWolf->HandleMessage(0, MSG_FREEZE, 0);
    else
        wolfFrozen = 0;
    unk8dc = 0;
    if (sheep == 0 && (flags & CINE_AUTO_SHEEP_BBOX))
        sheep = FindCinSheepBBox();
    if (box == 0 && (flags & CINE_AUTO_CIN_BBOX))
        box = FindCinBBox();
    cineBox = box;
    sheepBox = sheep;
    SetDialogueParams(dlg);
    Load();
    key = FindFirstPosKeyFor(g_pWolf);
    if ((flags & CINE_RELATIVE_TO_WOLF) && key == 0) {
        flags &= ~CINE_RELATIVE_TO_WOLF;
    } else if (flags & CINE_RELATIVE_TO_WOLF) {
        Vec3s wp = g_pWolf->pos;
        worldOffset.x = wp.x - key->x;
        worldOffset.y = wp.y - key->y;
        worldOffset.z = wp.z - key->z;
    }
    if ((flags & CINE_LETTERBOX) || dialogueText != 0)
        Dialogue_SetBoxActive(1);
    if (flags & CINE_SUSPEND_ACTIVE_SCRIPT) {
        savedSfxVolume = g_sfxVolume;
        Sound_SetSfxVolume(0);
    }
    Rewind();
    attachCount = 0;
    cameraHeld = 1;
}

/* ends the cinematic: unfreezes the Wolf, Finish (restores the actors, the level exit with flags & 0x1000),
 * restores the sound effects and the letterbox, marks the player idle, releases the camera and closes any prompt */
void Cine::Stop()
{
    if (!(flags & CINE_NO_WOLF_FREEZE) && wolfFrozen)
        g_pWolf->HandleMessage(0, MSG_UNFREEZE, 0);
    Finish();
    if (flags & CINE_SUSPEND_ACTIVE_SCRIPT)
        Sound_SetSfxVolume(savedSfxVolume);
    if ((flags & CINE_LETTERBOX) || dialogueText != 0)
        Dialogue_SetBoxActive(0);
    active = 0;
    finished = 1;
    wolfReady = 0;
    if (hasCameraTrack && cameraHeld)
        Camera_ReleaseAny();
    Prompt_End();
}

/* clock to 0, every key cursor to its record's first key; endTriggerMs = the latest last-key time - 1 s. */
void Cine::Rewind()
{
    s32 last;
    s32 off, t;
    time = 0;
    startRawTime = g_rawTime;
    last = 0;
    ResetIterators();
    do {
        CineRecord *rec = scriptCursor;
        rec->keyCursor = (u16 *)(rec + 1);
        if (scriptCursor->count != 0)
            off = g_cineOpStride3[scriptCursor->opcode] * (scriptCursor->count - 1);
        else
            off = 0;
        t = *(u16 *)((u8 *)rec + off + 8) & 0x3fff;
        if (t > last)
            last = t;
    } while (AdvanceScriptCursor());
    endTriggerMs = last * 16 - 1000;
}

void Cine::InitRecordCursors()
{
    s32 off;
    ResetIterators();
    do {
        CineRecord *rec = scriptCursor;
        if (rec->count != 0)
            off = g_cineOpStride3[rec->opcode] * (rec->count - 1);
        else
            off = 0;
        rec->keyCursor = (u16 *)((u8 *)rec + off + 8);
    } while (AdvanceScriptCursor());
}

/* the one cinematic player. Defined after Cine_InitRecordCursors, which its initialiser follows in the image. */
Cine g_cinePlayer;
