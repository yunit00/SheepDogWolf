/*
 * The two `#pragma inline_depth` lines are there for that: depth 1 at the constructor, where the vtable and ??_G are
 * emitted (the destructor is expanded into ??_G, nothing nested in it), depth 0 for the rest. Without them and come out
 * with Free expanded (measured).
 *
 * Source shapes: GetVoiceAmplitude scopes the play/write cursors to the block that ends at their last use;
 * ReadOrPadSilence defers the refill loop behind a `bool refill` the optimiser removes. A function that can fail starts
 * with `hr = E_FAIL` and nests the successful steps with ONE return.
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
DSBPOSITIONNOTIFY
g_streamNotifyPosReserved[16];
DSBPOSITIONNOTIFY g_streamNotifyPos[16];  /* StreamSound_CreateFromWaveEx's notification table */
DSBPOSITIONNOTIFY g_streamNotifyPos2[16]; /* StreamSound_CreateFromFile's */

/* ================================================================ StreamSound (vtable) */

#pragma inline_depth(1)
StreamSound::StreamSound()
{
    waveIsExternal = 1;
    volumeCb = -3000;
    pDevice = 0;
    looping = 0;
    bufferBytes = 0;
    wave = 0;
    buffer = 0;
    panCb = 0;
    baseFrequency = 0;
    lipSyncEnabled = 0;
    pNotify = 0;
    eofReached = 0;
    bitsPerSample = blockAlign = channels = bytesPerSampleChannel = 0;
    samplesPerSec = 0;
    voiceAmplitude = 0;
    playProgress = writeOffset = lastPlayCursor = 0;
    notifySize = 0;
    stopped = 1;
}

#pragma inline_depth(0)

StreamSound::~StreamSound()
{
    Free();
}

/* the unused variant that opens its own WaveFile. It is CreateFromWaveEx's code with the two settings as locals: 2
 * seconds and 16 segments. No callers. */
s32 StreamSound::CreateFromFile(SoundDevice *device, char *path)
{
    DSBUFFERDESC desc;
    void *event;
    s32 hr = E_FAIL;
    u8 i;
    float fBlockAlign;
    float fSamplesPerSec;
    float seconds = 2.0f;
    u8 notifications = 16;

    if (device->initialized == 1) {
        pDevice = device;
        if ((hr = device->RegisterStreamEvent(this, &event)) >= 0) {
            if (wave && !waveIsExternal) {
                delete wave;
                wave = 0;
            }
            if (buffer) {
                buffer->Release();
                buffer = 0;
            }
            wave = new WaveFile;
            if ((hr = wave->Open(path, 0, 0)) >= 0) {
                stopped = 1;
                eofReached = 0;
                lipSyncEnabled = 0;
                lastPlayCursor = 0;
                playProgress = 0;
                fSamplesPerSec = (float)wave->format->nSamplesPerSec;
                fBlockAlign = (float)wave->format->nBlockAlign;
                blockAlign = (u8)fBlockAlign;
                bitsPerSample = (u8)wave->format->wBitsPerSample;
                channels = (u8)wave->format->nChannels;
                bytesPerSampleChannel = blockAlign / channels;
                samplesPerSec = (u32)fSamplesPerSec;
                voiceAmplitude = 0;
                notifySize = (u32)(fBlockAlign * fSamplesPerSec * seconds / notifications);
                notifySize -= notifySize % wave->format->nBlockAlign;
                bufferBytes = notifySize * notifications;
                memset(&desc, 0, sizeof desc);
                desc.dwSize = sizeof desc;
                desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLFREQUENCY |
                               DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
                desc.dwBufferBytes = bufferBytes;
                desc.lpwfxFormat = wave->format;
                if ((hr = pDevice->GetDSound()->CreateSoundBuffer(&desc, &buffer, 0)) >= 0) {
                    buffer->QueryInterface(IID_IDirectSoundNotify, (void **)&pNotify);
                    for (i = 0; i < notifications; i++) {
                        g_streamNotifyPos2[i].dwOffset = notifySize * (i + 1) - 1;
                        g_streamNotifyPos2[i].hEventNotify = event;
                    }
                    if ((hr = pNotify->SetNotificationPositions(notifications, g_streamNotifyPos2)) < 0)
                        pNotify->Release();
                    waveIsExternal = 0;
                    buffer->GetFrequency(&baseFrequency);
                }
            }
        }
    }
    return hr;
}

s32 StreamSound::CreateFromWave(SoundDevice *device, WaveFile *w)
{
    return CreateFromWaveEx(device, w, 3.0f, 16);
}

/* the live one (the stream player's loader thread passes 2.0 s): adopts the caller's WaveFile, sizes the buffer
 * to `seconds` of audio (at least 2) cut into `notifications` segments (at most 16), and arms one notification per segment
 * on the event the device handed out. */
s32 StreamSound::CreateFromWaveEx(SoundDevice *device, WaveFile *w, float seconds, u8 notifications)
{
    DSBUFFERDESC desc;
    void *event;
    s32 hr = E_FAIL;
    u8 i;
    float fBlockAlign;
    float fSamplesPerSec;

    if (device->initialized == 1) {
        pDevice = device;
        if ((hr = device->RegisterStreamEvent(this, &event)) >= 0) {
            if (wave && !waveIsExternal) {
                delete wave;
                wave = 0;
            }
            if (buffer) {
                buffer->Release();
                buffer = 0;
            }
            wave = w;
            if (wave->format) {
                stopped = 1;
                eofReached = 0;
                lipSyncEnabled = 0;
                lastPlayCursor = 0;
                playProgress = 0;
                if (seconds < 2.0f)
                    seconds = 2.0f;
                if (notifications > 16)
                    notifications = 16;
                fSamplesPerSec = (float)wave->format->nSamplesPerSec;
                fBlockAlign = (float)wave->format->nBlockAlign;
                blockAlign = (u8)fBlockAlign;
                bitsPerSample = (u8)wave->format->wBitsPerSample;
                channels = (u8)wave->format->nChannels;
                bytesPerSampleChannel = blockAlign / channels;
                samplesPerSec = (u32)fSamplesPerSec;
                voiceAmplitude = 0;
                notifySize = (u32)(fBlockAlign * fSamplesPerSec * seconds / notifications);
                notifySize -= notifySize % wave->format->nBlockAlign;
                bufferBytes = notifySize * notifications;
                memset(&desc, 0, sizeof desc);
                desc.dwSize = sizeof desc;
                desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLFREQUENCY |
                               DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
                desc.dwBufferBytes = bufferBytes;
                desc.lpwfxFormat = wave->format;
                if ((hr = pDevice->GetDSound()->CreateSoundBuffer(&desc, &buffer, 0)) >= 0) {
                    buffer->QueryInterface(IID_IDirectSoundNotify, (void **)&pNotify);
                    for (i = 0; i < notifications; i++) {
                        g_streamNotifyPos[i].dwOffset = notifySize * (i + 1) - 1;
                        g_streamNotifyPos[i].hEventNotify = event;
                    }
                    if ((hr = pNotify->SetNotificationPositions(notifications, g_streamNotifyPos)) < 0)
                        pNotify->Release();
                    waveIsExternal = 1;
                }
            }
        }
    }
    return hr;
}

/* the only path that gives the device's stream slot back. */
s32 StreamSound::Free()
{
    if (pDevice) {
        pDevice->UnregisterStreamEvent(this);
        pDevice = 0;
    }
    if (wave && !waveIsExternal) {
        delete wave;
        wave = 0;
    }
    if (pNotify) {
        pNotify->Release();
        pNotify = 0;
    }
    if (buffer) {
        buffer->Release();
        buffer = 0;
    }
    return S_OK;
}

/* the SDK's loop: every iteration calls Restore twice (the first result only decides the Sleep). */
s32 StreamSound::RestoreBuffer()
{
    DWORD status;
    s32 hr;

    if (buffer == 0)
        return S_OK;
    if ((hr = buffer->GetStatus(&status)) < 0)
        return hr;
    if (status & DSBSTATUS_BUFFERLOST) {
        do {
            hr = buffer->Restore();
            if (hr == DSERR_BUFFERLOST)
                Sleep(10);
        } while ((hr = buffer->Restore()) != 0);
        return FillBuffer();
    }
    return hr;
}

/* on a position notification: advances playProgress by the play cursor's movement, refills the segment at
 * writeOffset, and stops the buffer once a non-looping file has run out and its tail has been played. */
s32 StreamSound::ServiceNotify()
{
    s32 hr = E_FAIL;
    void *ptr = 0;
    DWORD len;
    DWORD play;
    DWORD write;
    u32 delta;

    if (buffer) {
        if (buffer->GetCurrentPosition(&play, &write) >= 0) {
            if (play < lastPlayCursor)
                delta = bufferBytes - lastPlayCursor + play;
            else
                delta = play - lastPlayCursor;
            playProgress += delta;
            lastPlayCursor = play;
        }
        if ((hr = buffer->Lock(writeOffset, notifySize, &ptr, &len, 0, 0, 0)) >= 0) {
            hr = ReadOrPadSilence((u8 *)ptr, len);
            buffer->Unlock(ptr, len, 0, 0);
            ptr = 0;
            if (eofReached == 1 && playProgress >= wave->ckRiff.cksize) {
                stopped = 1;
                buffer->Stop();
                buffer->SetCurrentPosition(0);
            }
            writeOffset = (writeOffset + len) % bufferBytes;
        }
    }
    return hr;
}

/* restarts the stream from the top of the file; lipSync arms GetVoiceAmplitude for this clip. */
s32 StreamSound::Play(u32 lipSync)
{
    s32 hr = E_FAIL;

    if (buffer) {
        if ((hr = RestoreBuffer()) >= 0) {
            if ((hr = FillBuffer()) >= 0) {
                stopped = 0;
                lipSyncEnabled = lipSync;
                hr = buffer->Play(0, 0, DSBPLAY_LOOPING);
            }
        }
    }
    return hr;
}

void StreamSound::Stop()
{
    if (buffer) {
        stopped = 1;
        buffer->Stop();
        buffer->SetCurrentPosition(0);
    }
}

/* does not touch `stopped`, so IsPlaying stays true while paused. */
void StreamSound::SetPaused(u8 pause)
{
    if (buffer) {
        if (pause == 1)
            buffer->Stop();
        else
            buffer->Play(0, 0, DSBPLAY_LOOPING);
    }
}

/* StreamSound's own copy of Sound_RewindBuffer: folded into StaticSound's identical. The body is
 * StaticSound::RewindBuffer's. */
void StreamSound::RewindBuffer()
{
    if (buffer)
        buffer->SetCurrentPosition(0);
}

/* Sound_SetBufferPosition (StaticSound's twin folded into it) - frac of the buffer; a looping sound keeps only the
 * fractional part, a one-shot restarts from 0 when frac >= 1. */
s32 StreamSound::SetBufferPosition(float frac)
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

/* trusts the `stopped` flag instead of asking DirectSound. */
u8 StreamSound::IsPlaying()
{
    u8 playing = 0;

    if (buffer)
        playing = !stopped;
    return playing;
}

/* StreamSound's own copy of Sound_SetBufferVolume: folded into StaticSound's identical; its 6000.0f is StaticSound's. The
 * body is StaticSound::Sound_SetBufferVolume's. */
s32 StreamSound::Sound_SetBufferVolume(float volume)
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

/* Sound_SetBufferPan (StaticSound's twin folded into it) */
s32 StreamSound::SetBufferPan(float pan)
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

/* StreamSound's own copy of Sound_SetBufferFrequency: folded into StaticSound's identical. The body is
 * StaticSound::Sound_SetBufferFrequency's. */
s32 StreamSound::Sound_SetBufferFrequency(u32 hz)
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

/* rewinds the file and fills the whole buffer from the top. */
s32 StreamSound::FillBuffer()
{
    void *ptr;
    DWORD len;
    s32 hr;

    eofReached = 0;
    writeOffset = 0;
    playProgress = 0;
    lastPlayCursor = 0;
    wave->ResetFile();
    buffer->SetCurrentPosition(0);
    if ((hr = buffer->Lock(0, bufferBytes, &ptr, &len, 0, 0, 0)) < 0)
        return hr;
    hr = ReadOrPadSilence((u8 *)ptr, len);
    buffer->Unlock(ptr, len, 0, 0);
    writeOffset = len % bufferBytes;
    return hr;
}

/* fills size bytes at dest from the file; at the end of a looping file it rewinds and keeps reading, at the end of a
 * one-shot it pads with silence and from then on fills whole blocks with silence. */
s32 StreamSound::ReadOrPadSilence(u8 *dest, u32 size)
{
    s32 hr = S_OK;
    u8 silence;
    u32 read;
    u32 total;
    bool refill = 0; /* the loop is DEFERRED behind this flag, and the optimiser then removes the flag: that is
                                 * what puts the end-of-file / shared memset block before the refill loop, as the original
                                 * has it. No storage of its own survives. */

    if (wave->format)
        silence = (u8)(wave->format->wBitsPerSample == 8 ? 0x80 : 0);
    if (!eofReached) {
        hr = wave->Read(size, dest, &read);
        if (read < size) {
            if (!looping) {
                eofReached = 1;
                memset(dest + read, silence, size - read);
            } else {
                total = read;
                refill = 1;
            }
        }
    } else {
        memset(dest, silence, size);
    }

    if (refill) {
        while (total < size) {
            if ((hr = wave->ResetFile()) < 0)
                return hr;
            hr = wave->Read(size - total, dest + total, &read);
            if (hr < 0)
                return hr;
            total += read;
        }
    }
    return hr;
}

/* the lip-sync meter (game code, not SDK): samples the stream's file at the play position, keeps a moving average of
 * |sample| and gates it with hysteresis (opens above 500, closes below 400). refresh 0 returns the last value. */
u32 StreamSound::GetVoiceAmplitude(s32 refresh)
{
    u32 sum = 0;
    u16 window = 16;
    u16 i;
    u32 delta;
    u32 avg;
    char sample[2];
    s32 v;

    if (refresh) {
        if (wave && IsPlaying() && lipSyncEnabled && buffer) {
            /* The cursors live ONLY here. */
            {
                DWORD play;
                DWORD write;
                if (buffer->GetCurrentPosition(&play, &write) < 0)
                    goto invalid;
                if (play < lastPlayCursor)
                    delta = bufferBytes - lastPlayCursor + play;
                else
                    delta = play - lastPlayCursor;
                playProgress += delta;
                lastPlayCursor = play;
            }
            if (samplesPerSec < 44100)
                window = 8;
            for (i = 0; i < window - 1; i++)
                amplitudeWindow[i] = amplitudeWindow[i + 1];
            /* (port) the original saved the mmio position, seeked to playProgress, read and seeked back; the samples are
             * in memory now and WaveFile::ReadAt peeks without moving the read position. */
            if (!wave->samples)
                return 0;
            if (wave->ReadAt(playProgress, (u8 *)sample, bytesPerSampleChannel) > 0) {
                if (bytesPerSampleChannel > 1) {
                    /* the two bytes are sign-extended separately and OR'ed, so a low byte >= 0x80 fills the top bits and the magnitude comes
                     * out as a small negative number: the meter under-reads most 16-bit samples. */
                    v = (sample[1] << 8) | sample[0];
                    if (v < 0)
                        v = -v;
                    amplitudeWindow[window - 1] = v;
                } else {
                    v = sample[0] < 0 ? -sample[0] : sample[0];
                    amplitudeWindow[window - 1] = v << 7;
                }
            }
            for (i = 0; i < window - 1; i++)
                sum += amplitudeWindow[i];
            avg = sum / window;
            if (voiceAmplitude)
                voiceAmplitude = avg < 400 ? 0 : avg;
            else
                voiceAmplitude = avg > 500 ? avg : 0;
            return voiceAmplitude;
        }
    invalid:
        voiceAmplitude = 0;
    }
    return voiceAmplitude;
}
