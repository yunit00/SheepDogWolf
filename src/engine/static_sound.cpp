/*
 * As an object of its own it needs no inline_depth pragmas (~Sound and the WaveFile methods are in other objects, and
 * the calls between StaticSound's own methods are virtual).
 *
 * Shapes recovered by compiling: a function that can fail starts with `hr = E_FAIL` and nests the successful steps with
 * ONE return; Stop / SetPaused / RewindBuffer return nothing.
 */
#define SDW_MEMBERS_Sound Sound();             /* Sound_Construct */
#define SDW_MEMBERS_StaticSound StaticSound(); /* StaticSound_Construct */
#define SDW_MEMBERS_StreamSound StreamSound(); /* StreamSound_Construct */
#define SDW_MEMBERS_WaveFile WaveFile();       /* WaveFile_Construct */
#include "sdw_classes.h"

#include "../sdk/win32.h"
#include "../sdk/mmsystem.h"
#include "../sdk/dsound.h"
#include "../sdk/crt.h"

#define SOUND_VOLUME_FLOOR (-6000) /* the game's floor (-60 dB), not DirectSound's DSBVOLUME_MIN (-10000) */

/* ---- globals ---- */
/* (port) g_waveStagingBuffer, the original's 1 MiB staging buffer for StaticSound_FillBuffer, is gone: FillBuffer reads
 * into the locked buffer. */

/* ================================================================ StaticSound (vtable) */

StaticSound::StaticSound()
{
    looping = 0;
    bufferBytes = 0;
    wave = 0;
    buffer = 0;
    panCb = 0;
    baseFrequency = 0;
    waveIsExternal = 1;
    volumeCb = -3000;
}

/* - Free's body written out, then ~Sound. */
StaticSound::~StaticSound()
{
    if (wave && !waveIsExternal) {
        delete wave;
        wave = 0;
    }
    if (buffer) {
        buffer->Release();
        buffer = 0;
    }
}

/* opens the .wav at path itself (an owned WaveFile) and loads it whole into a new buffer. No callers. */
s32 StaticSound::CreateFromFile(SoundDevice *device, char *path)
{
    DSBUFFERDESC desc;
    s32 hr = E_FAIL;

    if (device->initialized == 1) {
        Free();
        wave = new WaveFile;
        if ((hr = wave->Open(path, 0, 0)) >= 0) {
            memset(&desc, 0, sizeof desc);
            desc.dwSize = sizeof desc;
            desc.dwFlags = device->GetBufferLocFlags() | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
            desc.dwBufferBytes = wave->ckData.cksize;
            desc.lpwfxFormat = wave->format;
            if ((hr = device->GetDSound()->CreateSoundBuffer(&desc, &buffer, 0)) >= 0) {
                bufferBytes = desc.dwBufferBytes;
                waveIsExternal = 0;
                hr = FillBuffer();
                buffer->GetFrequency(&baseFrequency);
            }
        }
    }
    return hr;
}

/* the SFX path: adopts a WaveFile the caller keeps (a g_sndBankWaves entry) and loads it into a new buffer. */
s32 StaticSound::CreateFromWave(SoundDevice *device, WaveFile *w)
{
    DSBUFFERDESC desc;
    s32 hr = E_FAIL;

    if (device->initialized == 1) {
        Free();
        wave = w;
        if (wave->format) {
            memset(&desc, 0, sizeof desc);
            desc.dwSize = sizeof desc;
            desc.dwFlags = device->GetBufferLocFlags() | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
            desc.dwBufferBytes = wave->ckData.cksize;
            desc.lpwfxFormat = wave->format;
            if ((hr = device->GetDSound()->CreateSoundBuffer(&desc, &buffer, 0)) >= 0) {
                bufferBytes = desc.dwBufferBytes;
                waveIsExternal = 1;
                hr = FillBuffer();
                buffer->GetFrequency(&baseFrequency);
            }
        }
    }
    return hr;
}

s32 StaticSound::Free()
{
    if (wave && !waveIsExternal) {
        delete wave;
        wave = 0;
    }
    if (buffer) {
        buffer->Release();
        buffer = 0;
    }
    return S_OK;
}

/* if DirectSound lost the buffer, restores it and reloads the sample. The loop leaves only on S_OK, and sleeps
 * 10 ms only for DSERR_BUFFERLOST, so any other failure spins it at full speed and never returns (StreamSound's version
 * is the SDK's do/while and calls Restore twice per turn; this one calls it once). */
s32 StaticSound::RestoreBuffer()
{
    DWORD status;
    s32 hr;

    if (buffer == 0)
        return S_OK;
    if ((hr = buffer->GetStatus(&status)) < 0)
        return hr;
    if (status & DSBSTATUS_BUFFERLOST) {
        for (;;) {
            hr = buffer->Restore();
            if (hr == DSERR_BUFFERLOST)
                Sleep(10);
            else if (hr == S_OK)
                break;
        }
        return FillBuffer();
    }
    return hr;
}

s32 StaticSound::Play()
{
    s32 hr = E_FAIL;

    if (buffer) {
        if ((hr = RestoreBuffer()) >= 0) {
            if (looping == 1)
                hr = buffer->Play(0, 0, DSBPLAY_LOOPING);
            else
                hr = buffer->Play(0, 0, 0);
        }
    }
    return hr;
}

/* stops and rewinds. */
void StaticSound::Stop()
{
    if (buffer) {
        buffer->Stop();
        buffer->SetCurrentPosition(0);
    }
}

/* pause stops without rewinding; resume plays on from the cursor. */
void StaticSound::SetPaused(u8 pause)
{
    if (buffer) {
        if (pause == 1)
            buffer->Stop();
        else if (looping == 1)
            buffer->Play(0, 0, DSBPLAY_LOOPING);
        else
            buffer->Play(0, 0, 0);
    }
}

void StaticSound::RewindBuffer()
{
    if (buffer)
        buffer->SetCurrentPosition(0);
}

/* The body is StreamSound::SetBufferPosition's. */
s32 StaticSound::SetBufferPosition(float frac)
{
    s32 hr = E_FAIL;
    float pos;
    DWORD offset;

    if (buffer) {
        if (frac < 0.0f)
            return E_FAIL;
        if (looping == 1) {
            pos = frac - (s32)frac;
        } else {
            pos = frac;
            if (frac >= 1.0f)
                pos = 0.0f;
        }
        offset = (DWORD)(bufferBytes * pos);
        hr = buffer->SetCurrentPosition(offset);
    }
    return hr;
}

u8 StaticSound::IsPlaying()
{
    DWORD status = 0;
    u8 playing = 0;

    if (buffer) {
        buffer->GetStatus(&status);
        playing = (u8)(status & DSBSTATUS_PLAYING);
    }
    return playing;
}

/* Sound_SetBufferVolume (StreamSound's twin folded into it) - 0..1 maps linearly onto -60..0 dB. */
s32 StaticSound::Sound_SetBufferVolume(float volume)
{
    s32 old = volumeCb;
    s32 hr = E_FAIL;

    if (buffer) {
        volumeCb = (s32)(volume * 6000.0f - 6000.0f);
        if (volumeCb < SOUND_VOLUME_FLOOR)
            volumeCb = SOUND_VOLUME_FLOOR;
        else if (volumeCb > DSBVOLUME_MAX)
            volumeCb = DSBVOLUME_MAX;
        if ((hr = buffer->SetVolume(volumeCb)) < 0)
            volumeCb = old;
    }
    return hr;
}

/* StaticSound's own copy of Sound_SetBufferPan: folded into StreamSound's identical. Its 10000.0f is the one this object
 * owns. The body is StreamSound::SetBufferPan's. */
s32 StaticSound::SetBufferPan(float pan)
{
    s32 old = panCb;
    s32 hr = E_FAIL;

    if (buffer) {
        if (pan < 0.0f) {
            panCb = (s32)(pan * 10000.0f);
            if (panCb < DSBPAN_LEFT)
                panCb = DSBPAN_LEFT;
        } else {
            panCb = (s32)(pan * 10000.0f);
            if (panCb > DSBPAN_RIGHT)
                panCb = DSBPAN_RIGHT;
        }
        if ((hr = buffer->SetPan(panCb)) < 0)
            panCb = old;
    }
    return hr;
}

/* Sound_SetBufferFrequency (StreamSound's twin folded into it) */
s32 StaticSound::Sound_SetBufferFrequency(u32 hz)
{
    u32 old = baseFrequency;
    s32 hr = E_FAIL;

    if (buffer) {
        baseFrequency = hz;
        if (baseFrequency < DSBFREQUENCY_MIN)
            baseFrequency = DSBFREQUENCY_MIN;
        else if (baseFrequency > DSBFREQUENCY_MAX)
            baseFrequency = DSBFREQUENCY_MAX;
        if ((hr = buffer->SetFrequency(baseFrequency)) < 0)
            baseFrequency = old;
    }
    return hr;
}

/* reads the whole data chunk into the buffer and rewinds the file. (port) The original read it into the 1 MiB
 * g_waveStagingBuffer and copied bufferBytes from there; the samples now go straight into the locked buffer, so a sample
 * SDL decoded to more than 1 MiB (ADPCM is 4x larger as 16-bit PCM) cannot overrun the staging buffer. */
s32 StaticSound::FillBuffer()
{
    void *ptr1;
    void *ptr2;
    DWORD len1;
    DWORD len2;
    u32 read;
    s32 hr;

    if ((hr = buffer->Lock(0, bufferBytes, &ptr1, &len1, &ptr2, &len2, 0)) >= 0) {
        hr = wave->Read(len1, (u8 *)ptr1, &read);
        if (hr >= 0 && read < len1)
            memset((u8 *)ptr1 + read, wave->format->wBitsPerSample == 8 ? 0x80 : 0, len1 - read);
        buffer->Unlock(ptr1, len1, 0, 0);
        wave->ResetFile();
    }
    return hr;
}
