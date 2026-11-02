/*
 * Load_SND reads a level's .SND file whole (Bs_LoadFile) and keeps it in g_pSndFile: {u32 crc; "vdx7"; u16 count; u16 pad;
 * then per sample a 12-byte SndBankEntry {soundId, dataSize, loop} followed by dataSize bytes of RIFF/WAVE}. Each entry
 * header is copied to g_sndBankEntries[i] and the WaveFile g_sndWaveBank[i] is opened IN MEMORY on the image (Open(ptr, 1,
 * size): the 'MEM ' mmio path). Nothing checks the count against the 256 entries of the two arrays.
 * A voice is a SoundChannel (0x44 bytes, an embedded StaticSound at +0x20). Voice 0 is reserved for the silent loop that
 * keeps DirectSound awake (Sound_InitSilentLoop); handle 0 means "no voice". Sound_Play only claims and describes a voice;
 * the buffer is (re)created and started later by the mixer tick, which is what `owned` (+2) waits for: while it is still 1
 * the voice has not been started, and Sound_Stop/Sound_StopAll then forget the request instead of stopping the buffer.
 *
 * The mixer half ( -): Sound_MixerTick runs once per frame and is where a requested voice's buffer is rebuilt, its
 * gain (distance to the camera x volume x g_sfxVolume) and frequency (sample rate x rate) pushed to DirectSound and the
 * buffer started; Sound_AllocChannel chooses the voice; pause/resume, rate, volume and the silent loop on voice 0
 * complete it.
 * */
#define SDW_MEMBERS_WaveFile WaveFile();       /* WaveFile_Construct */
#define SDW_MEMBERS_StaticSound StaticSound(); /* StaticSound_Construct */
#include "sdw_enums.h"
#include "sdw_classes.h"

#include "../sdk/crt.h"

/* ---- the game's own helpers ---- */
#include "bs_io.h"
#include "fixed_math.h"
#include "progress.h"
#include "draw2d.h"
#include "../app/app_main.h"
#include "sfx_volume.h"
void Sound_SetRate(u16 handle, s32 fixed4_12);
void Sound_InitSilentLoop();
void Sound_AllocChannel(u16 soundId, void *owner, u16 *outHandle);
void Sound_SetDistanceFalloff(u32 nearDist, u32 farDist, float farGain, float nearGain);
void Sound_UpdateChannelGain(u16 ch);

/* ---- globals ---- */

#define g_camPos (g_camera.pos) /* the listener */

SndBankEntry g_sndBankEntries[256]; /* entry headers copied from the .SND image */

/* empty, no callers. */
void Load_EmptyStub_548dd0() {}

/* the static initialisers of the two arrays - for each, a root (the _initterm entries, ), the constructor loop, the
 * atexit registration and the destructor loop. */
WaveFile g_sndWaveBank[256];      /* one in-memory WaveFile per entry, stride 0x34 */
SoundChannel g_soundChannels[24]; /* the voices, stride 0x44; 0 is reserved */

u8 *g_pSndFile = 0;       /* the whole .SND image */
u16 g_sndBankCount = 0;   /* entries in the loaded bank */
u32 g_sndNearDist = 0;    /* positional gain = g_sndNearGain up to here (200) */
u32 g_sndFarDist = 0;     /* .. = g_sndFarGain from here on (5000) */
float g_sndNearGain = 0;  /* 1.0 */
float g_sndFarGain = 0;   /* 0.0 */
float g_sndGainSlope = 0; /* (near - far gain) / (near - far dist) */

/* loads the level's sound bank (.SND) and resets voices 1..23. 0, or -1 when the file is missing, empty or
 * not a "vdx7" bank. */
s32 Load_SND(char *path)
{
    u16 index;
    SndBankEntry *sndEntry;
    u16 ch;
    char *hdr;
    s32 length;
    u32 cursor;
    s32 result;

    result = -1;
    length = 0;
    g_pSndFile = (u8 *)Bs_LoadFile(path, (u32 *)&length);
    if (length > 0) {
        cursor = 4;
        hdr = (char *)g_pSndFile + cursor;
        cursor += 8;
        if (strncmp(hdr, "vdx7", strlen("vdx7")) == 0) {
            g_sndBankCount = *(u16 *)(hdr + 4);
            for (index = 0; index < g_sndBankCount; index++) {
                sndEntry = (SndBankEntry *)(g_pSndFile + cursor);
                memcpy(&g_sndBankEntries[index], sndEntry, 12);
                cursor += 12;
                g_sndWaveBank[index].Open((char *)g_pSndFile + cursor, 1, sndEntry->dataSize);
                cursor += sndEntry->dataSize;
            }
            for (ch = 1; ch < 24; ch++) {
                g_soundChannels[ch].paused = g_soundChannels[ch].active = g_soundChannels[ch].owned =
                    g_soundChannels[ch].keepBuffer = 0;
                g_soundChannels[ch].owner = 0;
                g_soundChannels[ch].soundId = 0;
                g_soundChannels[ch].sampleSlot = 0;
                g_soundChannels[ch].playFlags = SNDF_NO_RETRIGGER;
                g_soundChannels[ch].volume = g_soundChannels[ch].mixedGain = g_soundChannels[ch].rate = 1.0f;
                g_soundChannels[ch].baseSampleRate = 0;
            }
            Sound_InitSilentLoop();
            Sound_SetDistanceFalloff(200, 5000, 0.0f, 1.0f);
            result = 0;
        }
    }
    return result;
}

/* releases every voice's DirectSound buffer (channel 0 included) and frees the .SND image. */
void Sound_ShutdownChannels()
{
    u16 ch;

    for (ch = 0; ch < 24; ch++)
        g_soundChannels[ch].sound.Free();
    if (g_pSndFile) {
        delete g_pSndFile;
        g_pSndFile = 0;
    }
}

/* requests sample soundId for owner: volume 0..255, flags a SoundPlayFlags set (1 loop, 8 no retrigger: an
 * active voice already playing this id for this owner is returned instead), rate 4.12 fixed. Returns the voice handle,
 * 0 when nothing was started (id 0x158 is always refused; so is everything in sound modes 1 and 3, an id missing from the
 * bank, or no free voice). */
u16 Sound_Play(u16 soundId, void *owner, u16 volume, u8 flags, s32 rate)
{
    u16 handle;
    u16 slot;
    u8 known;

    handle = 0;
    slot = 0;
    known = 0;
    if (soundId == SND_RUMBLE)
        return handle;
    if (g_pProgress->optionBits.soundMode == SOUNDMODE_ALL || g_pProgress->optionBits.soundMode == SOUNDMODE_SFX_ONLY) {
        if (flags & SNDF_NO_RETRIGGER) {
            for (slot = 1; slot < 24; slot++) {
                if (g_soundChannels[slot].active == 1 && g_soundChannels[slot].owner == owner &&
                    g_soundChannels[slot].soundId == soundId)
                    return slot;
            }
        }
        slot = 0;
        while (slot < g_sndBankCount && !known) {
            if (g_sndBankEntries[slot].soundId == soundId)
                known = 1;
            else
                slot++;
        }
        if (!known) {
            handle = 0;
        } else {
            Sound_AllocChannel(soundId, owner, &handle);
            if (handle != 0) {
                g_soundChannels[handle].sound.Stop();
                g_soundChannels[handle].paused = 0;
                g_soundChannels[handle].active = 1;
                g_soundChannels[handle].owned = 1;
                g_soundChannels[handle].sound.looping =
                    ((flags & SNDF_LOOP) || g_sndBankEntries[slot].loop == 1) ? 1 : 0;
                g_soundChannels[handle].soundId = soundId;
                g_soundChannels[handle].sampleSlot = slot;
                g_soundChannels[handle].playFlags = flags;
                g_soundChannels[handle].owner = owner;
                g_soundChannels[handle].volume = volume / 255.0f;
                Sound_SetRate(handle, rate);
            }
        }
    }
    return handle;
}

/* 1 while voice `handle` is playing; 0 for handle 0. */
s32 Sound_IsPlaying(u16 handle)
{
    s32 result;

    result = 0;
    if (handle != 0)
        result = g_soundChannels[handle].active == 1;
    return result;
}

/* 1 if the first voice (1..23) holding soundId is active. Only the first match is looked at. */
s32 Sound_IsSampleIdPlaying(u16 soundId)
{
    u16 ch;
    s32 result;

    result = 0;
    ch = 1;
    while (ch < 24 && g_soundChannels[ch].soundId != soundId)
        ch++;
    if (ch < 24 && g_soundChannels[ch].active == 1)
        result = 1;
    return result;
}

/* stops voice `handle` if owner started it; a voice not yet started by the mixer is just forgotten. */
void Sound_Stop(u16 handle, void *owner)
{
    if (handle != 0 && g_soundChannels[handle].owner == owner) {
        if (!g_soundChannels[handle].owned)
            g_soundChannels[handle].sound.Stop();
        if (g_soundChannels[handle].owned) {
            g_soundChannels[handle].soundId = 0;
            g_soundChannels[handle].sampleSlot = 0;
        }
        g_soundChannels[handle].active = 0;
        g_soundChannels[handle].owned = 0;
    }
}

/* Sound_Stop on voices 1..23 whoever owns them. */
void Sound_StopAll()
{
    u16 ch;

    for (ch = 1; ch < 24; ch++) {
        if (!g_soundChannels[ch].owned) {
            g_soundChannels[ch].sound.Stop();
        } else {
            g_soundChannels[ch].soundId = 0;
            g_soundChannels[ch].sampleSlot = 0;
        }
        g_soundChannels[ch].active = 0;
        g_soundChannels[ch].owned = 0;
    }
}

/* pauses every playing voice (1..23): the buffer is stopped where it is and `paused` remembers it. */
void Sound_PauseAll()
{
    u16 ch;

    for (ch = 1; ch < 24; ch++) {
        if (g_soundChannels[ch].active == 1) {
            g_soundChannels[ch].paused = 1;
            g_soundChannels[ch].sound.SetPaused(1);
        }
    }
}

/* restarts the voices Sound_PauseAll paused. */
void Sound_ResumeAll()
{
    u16 ch;

    for (ch = 1; ch < 24; ch++) {
        if (g_soundChannels[ch].paused == 1) {
            g_soundChannels[ch].sound.SetPaused(0);
            g_soundChannels[ch].paused = 0;
        }
    }
}

/* playback-rate multiplier of voice `handle`, 4.12 fixed (0x1000 = the sample's own rate); applied by the
 * next mixer tick. */
void Sound_SetRate(u16 handle, s32 fixed4_12)
{
    if (handle != 0)
        g_soundChannels[handle].rate = Math_Fixed12ToFloat_s32(fixed4_12);
}

/* volume of voice `handle`, 0..255. */
void Sound_SetVolume(u16 handle, u16 volume)
{
    if (handle != 0)
        g_soundChannels[handle].volume = volume / 255.0f;
}

/* once per frame. For each voice 1..23: one that is neither waiting to start nor still playing is marked
 * inactive; one waiting to start (owned == 1) gets its buffer re-created from the bank sample unless AllocChannel found
 * the buffer already holds it (keepBuffer); an active one gets its distance gain and its volume and frequency pushed to
 * DirectSound, and is (re)started if it is not playing. `owned` is cleared, so a voice is started once per Sound_Play. */
void Sound_MixerTick()
{
    /* slots: ch -2, frequency -8 */
    u16 ch;
    u32 frequency;

    for (ch = 1; ch < 24; ch++) {
        if (g_soundChannels[ch].owned == 0 && g_soundChannels[ch].sound.IsPlaying() == 0) {
            g_soundChannels[ch].active = 0;
        } else {
            if (g_soundChannels[ch].owned == 1 && g_soundChannels[ch].keepBuffer == 0) {
                g_soundChannels[ch].sound.Free();
                g_soundChannels[ch].sound.CreateFromWave(g_pSoundSystem,
                                                         &g_sndWaveBank[g_soundChannels[ch].sampleSlot]);
                frequency = g_soundChannels[ch].sound.baseFrequency;
                g_soundChannels[ch].baseSampleRate = frequency;
                g_soundChannels[ch].owned = 0;
            }
            if (g_soundChannels[ch].active == 1) {
                Sound_UpdateChannelGain(ch);
                g_soundChannels[ch].sound.Sound_SetBufferVolume(g_soundChannels[ch].mixedGain);
                g_soundChannels[ch].sound.Sound_SetBufferFrequency(
                    (u32)(g_soundChannels[ch].baseSampleRate * g_soundChannels[ch].rate));
            }
            if (g_soundChannels[ch].active == 1 && g_soundChannels[ch].sound.IsPlaying() == 0)
                g_soundChannels[ch].sound.Play();
            g_soundChannels[ch].owned = 0;
        }
    }
}

/* voice 0 plays the bank's first sample, looped, at volume 0 (-60 dB, see Sound_SetBufferVolume): run while
 * nothing else plays, it keeps DirectSound from going idle (the stream player starts and stops it). */
void Sound_InitSilentLoop()
{
    g_soundChannels[0].volume = 0.0f;
    g_soundChannels[0].soundId = (u16)g_sndBankEntries[0].soundId;
    g_soundChannels[0].playFlags = SNDF_LOOP;
    g_soundChannels[0].owner = 0;
    g_soundChannels[0].sound.CreateFromWave(g_pSoundSystem, &g_sndWaveBank[0]);
    g_soundChannels[0].sound.Sound_SetBufferVolume(0.0f);
    g_soundChannels[0].sound.looping = 1;
}

void Sound_StartSilentLoop()
{
    if (!g_soundChannels[0].sound.IsPlaying())
        g_soundChannels[0].sound.Play();
}

void Sound_StopSilentLoop()
{
    g_soundChannels[0].sound.Stop();
}

/* no callers. */
void Sound_StartStopSilentLoop_Dead()
{
    Sound_StartSilentLoop();
    Sound_StopSilentLoop();
}

/* picks the voice for soundId/owner, in *outHandle (0 = none). In order: a voice never used (soundId and
 * sampleSlot 0); an idle voice already holding soundId - this owner's, else the first such (both keep the buffer); the
 * first idle voice; else the first playing voice that does not loop, which is stolen. keepBuffer tells the mixer
 * whether the DirectSound buffer must be rebuilt. With 23 looping voices playing there is no voice (handle 0). */
void Sound_AllocChannel(u16 soundId, void *owner, u16 *outHandle)
{
    u16 candidate;
    u16 start;
    u8 found;
    u8 rebuild;

    start = 1;
    candidate = 0;
    found = 0;
    rebuild = 1;
    *outHandle = 1;
    while (!found && *outHandle < 24) {
        if (g_soundChannels[*outHandle].soundId == 0 && g_soundChannels[*outHandle].sampleSlot == 0)
            found = 1;
        else
            (*outHandle)++;
    }
    if (!found) {
        *outHandle = 1;
        while (!found && *outHandle < 24) {
            if (g_soundChannels[*outHandle].soundId == soundId && g_soundChannels[*outHandle].active == 0) {
                if (g_soundChannels[*outHandle].owner == owner) {
                    found = 1;
                    rebuild = 0;
                } else {
                    if (candidate == 0)
                        candidate = *outHandle;
                    (*outHandle)++;
                }
            } else {
                (*outHandle)++;
            }
        }
        if (!found) {
            if (candidate != 0) {
                *outHandle = candidate;
                rebuild = 0;
            } else {
                *outHandle = 1;
                while (!found && *outHandle < 24) {
                    if (g_soundChannels[*outHandle].active == 0) {
                        found = 1;
                    } else {
                        if (candidate == 0 && g_soundChannels[*outHandle].sound.looping == 0)
                            candidate = *outHandle;
                        (*outHandle)++;
                    }
                }
                if (!found)
                    *outHandle = candidate;
                rebuild = 1;
            }
        }
    }
    if (*outHandle != 0)
        g_soundChannels[*outHandle].keepBuffer = rebuild == 0 ? 1 : 0;
}

/* positional gain: nearGain up to nearDist, farGain from farDist on, linear in between. */
void Sound_SetDistanceFalloff(u32 nearDist, u32 farDist, float farGain, float nearGain)
{
    g_sndNearDist = nearDist;
    g_sndFarDist = farDist;
    g_sndFarGain = farGain;
    g_sndNearGain = nearGain;
    /* signed: nearDist - farDist is negative (200 - 5000), and as an unsigned difference it wraps to about 4.3e9, which
     * made the slope ~0 and every positional sound beyond nearDist silent */
    g_sndGainSlope = (nearGain - farGain) / ((s32)nearDist - (s32)farDist);
}

/* mixedGain = distance gain * volume * g_sfxVolume/255. The distance gain applies only to positional voices
 * (flag 2, not 0x10): the distance from the owner's position (ScnObject.pos) to the camera, horizontal only with flag 4. */
void Sound_UpdateChannelGain(u16 ch)
{
    float gain;
    Vec3s src;
    u32 len;
    Vec3s delta;

    gain = 1.0f;
    if ((g_soundChannels[ch].playFlags & SNDF_POSITIONAL) && !(g_soundChannels[ch].playFlags & SNDF_NO_ATTENUATION)) {
        src = ((ScnObject *)g_soundChannels[ch].owner)->pos;
        if (g_soundChannels[ch].playFlags & SNDF_DIST_HORIZONTAL) {
            delta.x = src.x - g_camPos.x;
            delta.z = src.z - g_camPos.z;
            len = (u32)sqrt(delta.x * delta.x + delta.z * delta.z);
        } else {
            delta.x = src.x - g_camPos.x;
            delta.y = src.y - g_camPos.y;
            delta.z = src.z - g_camPos.z;
            len = (u32)sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
        }
        if (len <= g_sndNearDist)
            gain = g_sndNearGain;
        else if (len >= g_sndFarDist)
            gain = g_sndFarGain;
        else
            gain = (s32)(len - g_sndFarDist) * g_sndGainSlope + g_sndFarGain;
    }
    g_soundChannels[ch].mixedGain = gain * g_soundChannels[ch].volume * (g_sfxVolume / 255.0f);
}
