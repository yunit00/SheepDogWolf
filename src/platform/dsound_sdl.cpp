/* The DirectSound: the interfaces the game's sound code calls (src/sdk/dsound.h - IDirectSound,
 * IDirectSoundBuffer, IDirectSoundNotify), implemented on the SDL3 mixer (platform.h, Platform_Voice*). The game's
 * SoundDevice, StaticSound and StreamSound (src/engine/) keep their code; only SoundDevice creates this instead of
 * DirectSound, and its stream "events" are the mixer's notification tags (sound_device.cpp).
 *
 * A secondary buffer is a mixer voice holding the buffer's PCM; the primary buffer only accepts its format and Play.
 * Only what the game uses is implemented; the rest answers E_NOTIMPL. */
#include "sdw_types.h"
#include "../sdk/windef.h"
#include "../sdk/win32.h"
#include "../sdk/mmsystem.h"
#include "../sdk/dsound.h"
#include "../sdk/crt.h"

#include "platform.h"

class SdlSoundBuffer;

/* a buffer's position notifications (QueryInterface gives it out) */
class SdlSoundNotify : public IDirectSoundNotify {
public:
    explicit SdlSoundNotify(SdlSoundBuffer *owner) : owner(owner) {}
    HRESULT __stdcall QueryInterface(const GUID &riid, void **ppv);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall SetNotificationPositions(DWORD count, const DSBPOSITIONNOTIFY *pos);

private:
    SdlSoundBuffer *owner;
};

class SdlSoundBuffer : public IDirectSoundBuffer {
public:
    /* voice 0: the primary buffer */
    SdlSoundBuffer(int voice, void *data, u32 bytes, u32 frequency)
        : refs(1), voice(voice), data((u8 *)data), bytes(bytes), frequency(frequency), volume(0), pan(0), notify(this)
    {
    }

    HRESULT __stdcall QueryInterface(const GUID &riid, void **ppv)
    {
        (void)riid; /* the game only ever asks for IDirectSoundNotify */
        *ppv = &notify;
        refs++;
        return S_OK;
    }
    ULONG __stdcall AddRef() { return ++refs; }
    ULONG __stdcall Release()
    {
        ULONG left = --refs;
        if (!left) {
            Platform_VoiceDestroy(voice);
            delete this;
        }
        return left;
    }

    HRESULT __stdcall GetCaps(void *caps)
    {
        (void)caps;
        return E_NOTIMPL;
    }
    HRESULT __stdcall GetCurrentPosition(DWORD *play, DWORD *write)
    {
        DWORD pos = voice ? Platform_VoiceGetPosition(voice) : 0;
        if (play)
            *play = pos;
        if (write)
            *write = pos;
        return S_OK;
    }
    HRESULT __stdcall GetFormat(WAVEFORMATEX *fmt, DWORD size, DWORD *written)
    {
        (void)fmt;
        (void)size;
        (void)written;
        return E_NOTIMPL;
    }
    HRESULT __stdcall GetVolume(s32 *out)
    {
        *out = volume;
        return S_OK;
    }
    HRESULT __stdcall GetPan(s32 *out)
    {
        *out = pan;
        return S_OK;
    }
    HRESULT __stdcall GetFrequency(u32 *out)
    {
        *out = frequency;
        return S_OK;
    }
    HRESULT __stdcall GetStatus(DWORD *status)
    {
        *status = voice && Platform_VoiceIsPlaying(voice) ? DSBSTATUS_PLAYING : 0;
        return S_OK;
    }
    HRESULT __stdcall Initialize(IDirectSound *ds, const DSBUFFERDESC *desc)
    {
        (void)ds;
        (void)desc;
        return E_NOTIMPL;
    }
    /* the region [off, off + n) of the buffer, split in two where it wraps (a second pointer the caller does not want
     * cuts the first part short) */
    HRESULT __stdcall Lock(DWORD off, DWORD n, void **p1, DWORD *n1, void **p2, DWORD *n2, DWORD flags)
    {
        (void)flags;
        if (!voice || off >= bytes)
            return E_FAIL;
        if (n > bytes)
            n = bytes;
        *p1 = data + off;
        *n1 = off + n <= bytes ? n : bytes - off;
        if (p2) {
            *p2 = n > *n1 ? data : 0;
            if (n2)
                *n2 = n - *n1;
        }
        return S_OK;
    }
    HRESULT __stdcall Play(DWORD reserved, DWORD priority, DWORD flags)
    {
        (void)reserved;
        (void)priority;
        if (voice)
            Platform_VoicePlay(voice, (flags & DSBPLAY_LOOPING) != 0);
        return S_OK;
    }
    HRESULT __stdcall SetCurrentPosition(DWORD pos)
    {
        if (voice)
            Platform_VoiceSetPosition(voice, pos);
        return S_OK;
    }
    HRESULT __stdcall SetFormat(const WAVEFORMATEX *fmt)
    {
        (void)fmt; /* the primary buffer's: the mixer has its own output format */
        return S_OK;
    }
    HRESULT __stdcall SetVolume(LONG v)
    {
        volume = v;
        if (voice)
            Platform_VoiceSetVolume(voice, v);
        return S_OK;
    }
    HRESULT __stdcall SetPan(LONG p)
    {
        pan = p;
        if (voice)
            Platform_VoiceSetPan(voice, p);
        return S_OK;
    }
    HRESULT __stdcall SetFrequency(DWORD hz)
    {
        frequency = hz;
        if (voice)
            Platform_VoiceSetFrequency(voice, hz);
        return S_OK;
    }
    HRESULT __stdcall Stop()
    {
        if (voice)
            Platform_VoiceStop(voice);
        return S_OK;
    }
    HRESULT __stdcall Unlock(void *p1, DWORD n1, void *p2, DWORD n2)
    {
        (void)p1; /* the caller wrote into the voice's own memory */
        (void)n1;
        (void)p2;
        (void)n2;
        return S_OK;
    }
    HRESULT __stdcall Restore() { return S_OK; } /* the mixer never loses a buffer */

    int Voice() const { return voice; }

private:
    ULONG refs;
    int voice;
    u8 *data;
    u32 bytes, frequency;
    s32 volume, pan;
    SdlSoundNotify notify;
};

HRESULT __stdcall SdlSoundNotify::QueryInterface(const GUID &riid, void **ppv)
{
    return owner->QueryInterface(riid, ppv);
}
ULONG __stdcall SdlSoundNotify::AddRef()
{
    return owner->AddRef();
}
ULONG __stdcall SdlSoundNotify::Release()
{
    return owner->Release();
}

/* The notification "event" is SoundDevice's stream slot + 1 (sound_device.cpp): the mixer sets that
 * slot's bit when the cursor passes one of the offsets. */
HRESULT __stdcall SdlSoundNotify::SetNotificationPositions(DWORD count, const DSBPOSITIONNOTIFY *pos)
{
    unsigned offsets[16];
    DWORD i;
    int tag = count ? (int)(uptr)pos[0].hEventNotify - 1 : -1;
    if (count > 16)
        return E_FAIL;
    for (i = 0; i < count; i++)
        offsets[i] = pos[i].dwOffset;
    Platform_VoiceSetNotify(owner->Voice(), offsets, (int)count, tag);
    return S_OK;
}

class SdlDirectSound : public IDirectSound {
public:
    SdlDirectSound() : refs(1) {}

    HRESULT __stdcall QueryInterface(const GUID &riid, void **ppv)
    {
        (void)riid;
        *ppv = 0;
        return E_NOTIMPL;
    }
    ULONG __stdcall AddRef() { return ++refs; }
    ULONG __stdcall Release()
    {
        ULONG left = --refs;
        if (!left) {
            Platform_AudioClose();
            delete this;
        }
        return left;
    }

    HRESULT __stdcall CreateSoundBuffer(const DSBUFFERDESC *desc, IDirectSoundBuffer **out, void *outer)
    {
        const WAVEFORMATEX *fmt = desc->lpwfxFormat;
        void *data = 0;
        int voice;
        (void)outer;
        *out = 0;
        if (desc->dwFlags & DSBCAPS_PRIMARYBUFFER) {
            *out = new SdlSoundBuffer(0, 0, 0, 0);
            return S_OK;
        }
        if (!fmt || fmt->wFormatTag != WAVE_FORMAT_PCM)
            return E_FAIL; /* DirectSound's buffers are PCM too */
        voice = Platform_VoiceCreate(desc->dwBufferBytes, fmt->nChannels, fmt->wBitsPerSample, fmt->nSamplesPerSec, &data);
        if (!voice)
            return E_OUTOFMEMORY;
        *out = new SdlSoundBuffer(voice, data, desc->dwBufferBytes, fmt->nSamplesPerSec);
        return S_OK;
    }
    HRESULT __stdcall GetCaps(DSCAPS *caps)
    {
        DWORD size = caps->dwSize;
        memset(caps, 0, sizeof(*caps)); /* no hardware mixing: the buffers are created static */
        caps->dwSize = size;
        return S_OK;
    }
    HRESULT __stdcall DuplicateSoundBuffer(IDirectSoundBuffer *src, IDirectSoundBuffer **out)
    {
        (void)src;
        *out = 0;
        return E_NOTIMPL;
    }
    HRESULT __stdcall SetCooperativeLevel(HWND hwnd, DWORD level)
    {
        (void)hwnd;
        (void)level;
        return S_OK;
    }
    HRESULT __stdcall Compact() { return S_OK; }
    HRESULT __stdcall GetSpeakerConfig(DWORD *config)
    {
        *config = 0;
        return S_OK;
    }
    HRESULT __stdcall SetSpeakerConfig(DWORD config)
    {
        (void)config;
        return S_OK;
    }
    HRESULT __stdcall Initialize(const GUID *device)
    {
        (void)device;
        return S_OK;
    }

private:
    ULONG refs;
};

/* DirectSoundCreate's replacement: the default output, through SDL3 */
HRESULT Sdl_DirectSoundCreate(IDirectSound **out)
{
    *out = 0;
    if (!Platform_AudioOpen())
        return E_FAIL;
    *out = new SdlDirectSound;
    return S_OK;
}
