/* The game's sound mixed with SDL3 (platform.h): a software version of the DirectSound secondary buffers the original
 * plays - each voice is its buffer's PCM, played by a cursor that advances at the voice's frequency, looping or once,
 * with DirectSound's volume and pan (hundredths of a decibel, the pan attenuating the other side), and position
 * notifications. SDL's audio callback mixes every playing voice into float stereo. */
#include <SDL3/SDL.h>

#include "platform.h"

#define AUDIO_RATE 44100
#define AUDIO_MAX_VOICES 256
#define AUDIO_MAX_NOTIFY 16

struct Voice {
    unsigned char *data;
    unsigned bytes, frames;     /* size, in bytes and in sample frames */
    int channels, bits, blockAlign;
    double position;            /* the cursor, in frames (fractional: resampling) */
    unsigned frequency;         /* the playback rate, Hz */
    float gainLeft, gainRight;  /* from the volume and pan */
    int volume, pan;            /* centibels */
    int playing, looping;
    unsigned notify[AUDIO_MAX_NOTIFY];
    int notifyCount, notifyTag;
};

static SDL_AudioStream *s_stream;
static Voice *s_voices[AUDIO_MAX_VOICES]; /* voice id - 1 */
static SDL_AtomicInt s_notified;
static float *s_mix;
static int s_mixFrames;
static int s_mixLogged, s_playLogged; /* the first callback and the first Play are logged */

static float Audio_Gain(int centibels)
{
    if (centibels <= -10000)
        return 0.0f;
    return SDL_powf(10.0f, centibels / 2000.0f); /* hundredths of a dB: 10^(dB / 20) */
}

static void Audio_UpdateGains(Voice *v)
{
    float gain = Audio_Gain(v->volume);
    v->gainLeft = gain * (v->pan > 0 ? Audio_Gain(-v->pan) : 1.0f);
    v->gainRight = gain * (v->pan < 0 ? Audio_Gain(v->pan) : 1.0f);
}

/* the sample of channel ch at frame f, -1..1 */
static float Audio_Sample(const Voice *v, unsigned f, int ch)
{
    const unsigned char *p = v->data + f * v->blockAlign;
    if (v->bits == 8)
        return (p[ch] - 128) / 128.0f;
    return (Sint16)(p[ch * 2] | p[ch * 2 + 1] << 8) / 32768.0f;
}

/* whether the cursor passed byte offset `at` going from `from` to `to` (bytes, `to` < `from` when it wrapped) */
static int Audio_Passed(unsigned from, unsigned to, int wrapped, unsigned at)
{
    if (!wrapped)
        return at >= from && at < to;
    return at >= from || at < to;
}

static void Audio_Notify(int tag)
{
    int old;
    if (tag < 0)
        return;
    do
        old = SDL_GetAtomicInt(&s_notified);
    while (!SDL_CompareAndSwapAtomicInt(&s_notified, old, old | 1 << tag));
}

/* mixes n frames of v into out, and fires its notifications */
static void Audio_MixVoice(Voice *v, float *out, int n)
{
    double step = (double)v->frequency / AUDIO_RATE;
    unsigned startByte = (unsigned)v->position * v->blockAlign;
    int wrapped = 0, i, k;
    for (i = 0; i < n; i++) {
        unsigned f = (unsigned)v->position, g = f + 1;
        float t = (float)(v->position - f), left, right;
        if (g >= v->frames)
            g = v->looping ? 0 : f;
        left = Audio_Sample(v, f, 0) + (Audio_Sample(v, g, 0) - Audio_Sample(v, f, 0)) * t;
        right = v->channels == 2 ? Audio_Sample(v, f, 1) + (Audio_Sample(v, g, 1) - Audio_Sample(v, f, 1)) * t : left;
        out[i * 2] += left * v->gainLeft;
        out[i * 2 + 1] += right * v->gainRight;
        v->position += step;
        if (v->position >= v->frames) {
            if (v->looping) {
                v->position -= v->frames;
                wrapped = 1;
            } else { /* a one-shot stops at its end, the cursor back at the start */
                v->playing = 0;
                v->position = 0;
                for (k = 0; k < v->notifyCount; k++)
                    if (v->notify[k] >= startByte)
                        Audio_Notify(v->notifyTag);
                return;
            }
        }
    }
    if (v->notifyTag >= 0) {
        unsigned endByte = (unsigned)v->position * v->blockAlign;
        for (k = 0; k < v->notifyCount; k++) {
            if (Audio_Passed(startByte, endByte, wrapped, v->notify[k])) {
                Audio_Notify(v->notifyTag);
                break;
            }
        }
    }
}

static void SDLCALL Audio_Callback(void *userdata, SDL_AudioStream *stream, int additional, int total)
{
    int frames = additional / (int)(2 * sizeof(float)), i;
    (void)userdata;
    (void)total;
    if (frames <= 0)
        return;
    if (frames > s_mixFrames) {
        float *grown = (float *)SDL_realloc(s_mix, frames * 2 * sizeof(float));
        if (!grown)
            return;
        s_mix = grown;
        s_mixFrames = frames;
    }
    if (!s_mixLogged) {
        s_mixLogged = 1;
        SDL_Log("SheepD3D: audio: the device asked for its first %d frames", frames);
    }
    SDL_memset(s_mix, 0, frames * 2 * sizeof(float));
    for (i = 0; i < AUDIO_MAX_VOICES; i++)
        if (s_voices[i] && s_voices[i]->playing)
            Audio_MixVoice(s_voices[i], s_mix, frames);
    for (i = 0; i < frames * 2; i++)
        s_mix[i] = SDL_clamp(s_mix[i], -1.0f, 1.0f);
    SDL_PutAudioStreamData(stream, s_mix, frames * 2 * (int)sizeof(float));
}

/* the voices are touched under the stream's lock, which the callback runs under */
static Voice *Audio_Lock(int voice)
{
    if (!s_stream || voice <= 0 || voice > AUDIO_MAX_VOICES)
        return 0;
    SDL_LockAudioStream(s_stream);
    if (!s_voices[voice - 1]) {
        SDL_UnlockAudioStream(s_stream);
        return 0;
    }
    return s_voices[voice - 1];
}

static void Audio_Unlock()
{
    SDL_UnlockAudioStream(s_stream);
}

int Platform_AudioOpen()
{
    SDL_AudioSpec spec;
    if (s_stream)
        return 1;
    if (!SDL_WasInit(SDL_INIT_AUDIO) && !SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("SheepD3D: audio: no audio subsystem: %s", SDL_GetError());
        return 0;
    }
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;
    spec.freq = AUDIO_RATE;
    s_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, Audio_Callback, 0);
    if (!s_stream) {
        SDL_Log("SheepD3D: audio: %s", SDL_GetError());
        return 0;
    }
    SDL_SetAtomicInt(&s_notified, 0);
    if (!SDL_ResumeAudioStreamDevice(s_stream))
        SDL_Log("SheepD3D: audio: resume: %s", SDL_GetError());
    {
        SDL_AudioDeviceID device = SDL_GetAudioStreamDevice(s_stream);
        SDL_AudioSpec deviceSpec;
        int deviceFrames = 0;
        SDL_zero(deviceSpec);
        SDL_GetAudioDeviceFormat(device, &deviceSpec, &deviceFrames);
        SDL_Log("SheepD3D: audio: driver %s, device \"%s\", %d Hz, %d channels, %d frames", SDL_GetCurrentAudioDriver(),
                SDL_GetAudioDeviceName(device), deviceSpec.freq, deviceSpec.channels, deviceFrames);
    }
    return 1;
}

void Platform_AudioClose()
{
    int i;
    if (!s_stream)
        return;
    SDL_DestroyAudioStream(s_stream); /* stops the callback */
    s_stream = 0;
    for (i = 0; i < AUDIO_MAX_VOICES; i++) {
        if (s_voices[i]) {
            SDL_free(s_voices[i]->data);
            SDL_free(s_voices[i]);
            s_voices[i] = 0;
        }
    }
}

int Platform_VoiceCreate(unsigned bytes, int channels, int bits, int rate, void **data)
{
    Voice *v;
    int i;
    *data = 0;
    if (!s_stream || !bytes || (channels != 1 && channels != 2) || (bits != 8 && bits != 16))
        return 0;
    if (!(v = (Voice *)SDL_calloc(1, sizeof(Voice))))
        return 0;
    v->blockAlign = channels * bits / 8;
    v->bytes = bytes;
    v->frames = bytes / v->blockAlign;
    v->channels = channels;
    v->bits = bits;
    v->frequency = rate;
    v->notifyTag = -1;
    if (!v->frames || !(v->data = (unsigned char *)SDL_malloc(bytes))) {
        SDL_free(v);
        return 0;
    }
    SDL_memset(v->data, bits == 8 ? 0x80 : 0, bytes);
    Audio_UpdateGains(v);
    SDL_LockAudioStream(s_stream);
    for (i = 0; i < AUDIO_MAX_VOICES && s_voices[i]; i++)
        ;
    if (i < AUDIO_MAX_VOICES)
        s_voices[i] = v;
    SDL_UnlockAudioStream(s_stream);
    if (i == AUDIO_MAX_VOICES) {
        SDL_Log("SheepD3D: audio: all %d voices are in use", AUDIO_MAX_VOICES);
        SDL_free(v->data);
        SDL_free(v);
        return 0;
    }
    *data = v->data;
    return i + 1;
}

void Platform_VoiceDestroy(int voice)
{
    Voice *v = Audio_Lock(voice);
    if (!v)
        return;
    s_voices[voice - 1] = 0;
    Audio_Unlock();
    SDL_free(v->data);
    SDL_free(v);
}

void Platform_VoicePlay(int voice, int looping)
{
    Voice *v = Audio_Lock(voice);
    if (!v)
        return;
    v->looping = looping;
    v->playing = 1;
    if (!s_playLogged) {
        s_playLogged = 1;
        SDL_Log("SheepD3D: audio: first voice played: %u Hz, %d-bit, %d channels, volume %d, pan %d", v->frequency,
                v->bits, v->channels, v->volume, v->pan);
    }
    Audio_Unlock();
}

void Platform_VoiceStop(int voice)
{
    Voice *v = Audio_Lock(voice);
    if (!v)
        return;
    v->playing = 0;
    Audio_Unlock();
}

int Platform_VoiceIsPlaying(int voice)
{
    int playing;
    Voice *v = Audio_Lock(voice);
    if (!v)
        return 0;
    playing = v->playing;
    Audio_Unlock();
    return playing;
}

unsigned Platform_VoiceGetPosition(int voice)
{
    unsigned pos;
    Voice *v = Audio_Lock(voice);
    if (!v)
        return 0;
    pos = (unsigned)v->position * v->blockAlign;
    Audio_Unlock();
    return pos;
}

void Platform_VoiceSetPosition(int voice, unsigned byte)
{
    Voice *v = Audio_Lock(voice);
    if (!v)
        return;
    v->position = byte < v->bytes ? byte / v->blockAlign : 0;
    Audio_Unlock();
}

void Platform_VoiceSetVolume(int voice, int centibels)
{
    Voice *v = Audio_Lock(voice);
    if (!v)
        return;
    v->volume = centibels;
    Audio_UpdateGains(v);
    Audio_Unlock();
}

void Platform_VoiceSetPan(int voice, int centibels)
{
    Voice *v = Audio_Lock(voice);
    if (!v)
        return;
    v->pan = centibels;
    Audio_UpdateGains(v);
    Audio_Unlock();
}

void Platform_VoiceSetFrequency(int voice, unsigned hz)
{
    Voice *v = Audio_Lock(voice);
    if (!v)
        return;
    v->frequency = hz;
    Audio_Unlock();
}

void Platform_VoiceSetNotify(int voice, const unsigned *offsets, int count, int tag)
{
    int i;
    Voice *v = Audio_Lock(voice);
    if (!v)
        return;
    if (count > AUDIO_MAX_NOTIFY)
        count = AUDIO_MAX_NOTIFY;
    for (i = 0; i < count; i++)
        v->notify[i] = offsets[i];
    v->notifyCount = count;
    v->notifyTag = tag >= 0 && tag < 32 ? tag : -1;
    Audio_Unlock();
}

unsigned Platform_AudioTakeNotified()
{
    int old;
    do
        old = SDL_GetAtomicInt(&s_notified);
    while (!SDL_CompareAndSwapAtomicInt(&s_notified, old, 0));
    return (unsigned)old;
}
