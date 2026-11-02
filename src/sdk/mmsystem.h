/* Only what src/ uses is here: constants, types and functions (depends on sdw_types.h). */
#ifndef SDW_SDK_MMSYSTEM_H
#define SDW_SDK_MMSYSTEM_H

/* (port) WINMM's mmio functions and MMIOINFO are gone: WaveFile (src/engine/wave_file.cpp) reads the .wav files with
 * SDL3 (src/platform/wav_sdl.cpp, SDL_LoadWAV_IO) on every platform. On 64-bit Windows the real winmm.dll's MMIOINFO
 * is byte-packed (the SDK's mmsystem.h includes pshpack1.h) and the naturally aligned declaration that was here did not
 * match it from `htask` on, which left every sound silent. Nothing links winmm any more. */

#define FOURCC_RIFF mmioFOURCC('R', 'I', 'F', 'F')
#define WAVE_FORMAT_PCM 1
#define mmioFOURCC(ch0, ch1, ch2, ch3) \
    ((u32)(u8)(ch0) | ((u32)(u8)(ch1) << 8) | ((u32)(u8)(ch2) << 16) | ((u32)(u8)(ch3) << 24))

#include "windef.h"

struct MMCKINFO;

#pragma pack(push, 1)
struct WAVEFORMATEX { /* mmreg.h, packed: 0x12 bytes */
    u16 wFormatTag;
    u16 nChannels;
    u32 nSamplesPerSec;
    u32 nAvgBytesPerSec;
    u16 nBlockAlign;
    u16 wBitsPerSample;
    u16 cbSize;
};
#pragma pack(pop)

typedef UINT MMRESULT;

#endif
