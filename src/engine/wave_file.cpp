#define SDW_MEMBERS_WaveFile WaveFile(); /* WaveFile_Construct */
#include "sdw_classes.h"

#include "../sdk/win32.h"
#include "../sdk/mmsystem.h"
#include "../sdk/crt.h"

/* The .wav is read by SDL3 (src/platform/wav_sdl.cpp, SDL_LoadWAV_IO), the same on every platform: the original's WINMM
 * mmio calls reached the real winmm.dll on Windows, whose byte-packed MMIOINFO did not match the game's declaration on
 * 64-bit, and every sound came out silent there. */
#include "../platform/platform.h"

/* the RIFF WAVE chunk ids, named here: the SDK has no constants for them (its samples write mmioFOURCC at each use) */
#define FOURCC_DATA mmioFOURCC('d', 'a', 't', 'a')
#define FOURCC_WAVE mmioFOURCC('W', 'A', 'V', 'E')

void *operator new(uptr);
void operator delete(void *);

/* ================================================================ WaveFile (vtable), the DX7 CWaveSoundRead */

WaveFile::WaveFile()
{
    format = 0;
    samples = 0;
    sampleBytes = 0;
    readPos = 0;
    memset(&ckData, 0, sizeof ckData);
    memset(&ckRiff, 0, sizeof ckRiff);
}

WaveFile::~WaveFile()
{
    Close();
    if (format) {
        delete format;
        format = 0;
    }
}

/* opens a .wav file (fromMemory 0) or `size` bytes of RIFF image at name (fromMemory 1: a .SND bank entry), reads its
 * format into `format` and its whole 'data' chunk into `samples`, and rewinds to the start of the samples. */
s32 WaveFile::Open(char *name, u8 fromMemory, u32 size)
{
    PlatformWav wav;
    s32 hr = E_FAIL;

    Close();
    if (format) {
        delete format;
        format = 0;
    }
    if (Platform_WavLoad(fromMemory ? 0 : name, fromMemory ? name : 0, size, &wav)) {
        /* the 18-byte WAVEFORMATEX the header parser made: PCM, as the voice plays it */
        format = (WAVEFORMATEX *)operator new(18);
        if (format) {
            format->wFormatTag = WAVE_FORMAT_PCM;
            format->nChannels = (u16)wav.channels;
            format->nSamplesPerSec = (u32)wav.rate;
            format->wBitsPerSample = (u16)wav.bits;
            format->nBlockAlign = (u16)(wav.channels * wav.bits / 8);
            format->nAvgBytesPerSec = format->nSamplesPerSec * format->nBlockAlign;
            format->cbSize = 0;
            samples = (u8 *)wav.data;
            sampleBytes = wav.bytes;
            /* the two chunk descriptors as mmioDescend filled them; only the sizes are read (ckRiff.cksize by
             * StreamSound::ServiceNotify, ckData.cksize by Read and the sounds' Create) */
            ckRiff.ckid = FOURCC_RIFF;
            ckRiff.cksize = wav.riffBytes;
            ckRiff.fccType = FOURCC_WAVE;
            ckRiff.dwDataOffset = 8;
            ckRiff.dwFlags = 0;
            ckData.ckid = FOURCC_DATA;
            ckData.fccType = 0;
            ckData.dwDataOffset = 0;
            ckData.dwFlags = 0;
            hr = ResetFile();
        } else {
            Platform_WavFree(wav.data);
        }
    }
    return hr;
}

/* goes back to the first sample and restores ckData.cksize (the bytes left to read). */
s32 WaveFile::ResetFile()
{
    if (!samples)
        return E_FAIL;
    readPos = 0;
    ckData.cksize = sampleBytes;
    return S_OK;
}

/* the DX7 WaveReadFile: copies min(size, ckData.cksize) bytes and takes them off ckData.cksize. */
s32 WaveFile::Read(u32 size, u8 *dest, u32 *read)
{
    u32 n;

    *read = 0;
    if (!samples)
        return E_FAIL;
    n = size;
    if (n > ckData.cksize)
        n = ckData.cksize;
    if (n > sampleBytes - readPos) /* cannot happen while cksize and readPos move together; a guard */
        n = sampleBytes - readPos;
    memcpy(dest, samples + readPos, n);
    readPos += n;
    ckData.cksize -= n;
    *read = n;
    return S_OK;
}

/* frees the samples (the original closed the mmio handle); neither frees `format` nor clears the chunk descriptors. A
 * second Close (Open closes first) does nothing. */
s32 WaveFile::Close()
{
    if (samples) {
        Platform_WavFree(samples);
        samples = 0;
    }
    sampleBytes = 0;
    readPos = 0;
    return S_OK;
}

/* the lip-sync meter's read (StreamSound::GetVoiceAmplitude): up to `size` bytes of samples at `offset`, without moving
 * the read position. The original seeked the mmio file there, read and seeked back - in FILE offsets, so it sampled a
 * header's length (44 bytes, a few samples) before the play position. Returns the bytes copied, 0 past the end. */
u32 WaveFile::ReadAt(u32 offset, u8 *dest, u32 size)
{
    if (!samples || offset >= sampleBytes)
        return 0;
    if (size > sampleBytes - offset)
        size = sampleBytes - offset;
    memcpy(dest, samples + offset, size);
    return size;
}
