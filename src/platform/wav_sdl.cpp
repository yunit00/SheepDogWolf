/* The game's .wav files read with SDL3 (platform.h, Platform_Wav*): what WaveFile (src/engine/wave_file.cpp) did with
 * WINMM's mmio functions - parse the RIFF/WAVE header and read the 'data' chunk - done by SDL_LoadWAV_IO, the same code
 * on every platform.
 *
 * Why: on 64-bit Windows the game called the real winmm.dll, whose MMIOINFO is byte-packed (mmsystem.h includes
 * pshpack1.h) while src/sdk/mmsystem.h declares it with natural alignment. From `htask` on every field was at another
 * offset, so the in-memory opens of the sound bank (the 'MEM ' IOProc: pchBuffer / cchBuffer) and the buffered reads
 * (pchNext / pchEndRead) got the wrong values and the mixer played silence. macOS and Linux used
 * their own mmio over stdio (the former src/platform/mmio_posix_compat.cpp), which shared the game's declaration, and
 * so worked.
 *
 * The whole data chunk is loaded at once (SDL_LoadWAV_IO has no streaming form): the sound bank's samples are small and
 * a music track is a few tens of MB at most. */
#include <SDL3/SDL.h>

#include "platform.h"

#ifndef SDL_PLATFORM_WINDOWS
/* the game's path as this system can open it: '\\' to '/', and each component looked up ignoring case
 * (crt_posix_compat.cpp) */
extern "C" void Sdw_ResolvePath(const char *path, char *out, size_t size);
#endif

static Uint32 Wav_ReadLE32(const Uint8 *p)
{
    return (Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}

int Platform_WavLoad(const char *path, const void *memory, unsigned memoryBytes, PlatformWav *out)
{
    SDL_IOStream *io;
    SDL_AudioSpec spec;
    Uint8 header[12];
    Uint8 *data = 0;
    Uint32 bytes = 0;
    Uint32 riffBytes = 0;
    const char *what = memory ? "(sound bank entry)" : path;

    SDL_zerop(out);
    if (memory) {
        io = SDL_IOFromConstMem(memory, memoryBytes);
    } else {
        if (!path)
            return 0;
#ifdef SDL_PLATFORM_WINDOWS
        io = SDL_IOFromFile(path, "rb"); /* SDL takes UTF-8 and opens the file with the wide-character API */
#else
        {
            char native[4096];
            Sdw_ResolvePath(path, native, sizeof(native));
            io = SDL_IOFromFile(native, "rb");
        }
#endif
    }
    if (!io) {
        SDL_Log("SheepD3D: wav %s: %s", what, SDL_GetError());
        return 0;
    }

    /* the RIFF chunk's size field: StreamSound::ServiceNotify ends a one-shot stream when its play progress reaches it
     * (WaveFile.ckRiff.cksize, as mmioDescend filled it) */
    if (SDL_ReadIO(io, header, sizeof(header)) == sizeof(header) && SDL_memcmp(header, "RIFF", 4) == 0 &&
        SDL_memcmp(header + 8, "WAVE", 4) == 0)
        riffBytes = Wav_ReadLE32(header + 4);
    if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
        SDL_Log("SheepD3D: wav %s: %s", what, SDL_GetError());
        SDL_CloseIO(io);
        return 0;
    }

    if (!SDL_LoadWAV_IO(io, true, &spec, &data, &bytes)) { /* closes io in every case */
        SDL_Log("SheepD3D: wav %s: %s", what, SDL_GetError());
        return 0;
    }

    /* The mixer (platform_audio_sdl.cpp) plays 8-bit unsigned or 16-bit signed little-endian, mono or stereo: what the
     * game's PCM files are. Anything else SDL hands back (ADPCM or a-law decoded, float, 24/32-bit, more channels) is
     * converted to 16-bit (stereo when it had more than two channels). */
    if (!((spec.format == SDL_AUDIO_U8 || spec.format == SDL_AUDIO_S16LE) && (spec.channels == 1 || spec.channels == 2))) {
        SDL_AudioSpec to = spec;
        Uint8 *converted = 0;
        int convertedBytes = 0;
        to.format = SDL_AUDIO_S16LE;
        if (to.channels > 2)
            to.channels = 2;
        if (!SDL_ConvertAudioSamples(&spec, data, (int)bytes, &to, &converted, &convertedBytes)) {
            SDL_Log("SheepD3D: wav %s: %s", what, SDL_GetError());
            SDL_free(data);
            return 0;
        }
        SDL_free(data);
        data = converted;
        bytes = (Uint32)convertedBytes;
        spec = to;
        riffBytes = 0; /* not the file's samples any more: see below */
    }

    /* the mixer needs at least one whole sample frame */
    bytes -= bytes % (Uint32)(spec.channels * SDL_AUDIO_BYTESIZE(spec.format));
    if (!bytes) {
        SDL_Log("SheepD3D: wav %s: no samples", what);
        SDL_free(data);
        return 0;
    }

    out->data = data;
    out->bytes = bytes;
    out->channels = spec.channels;
    out->bits = (int)SDL_AUDIO_BITSIZE(spec.format);
    out->rate = spec.freq;
    /* A PCM file's RIFF chunk holds its samples and at least 36 bytes of header. A smaller (or missing) size means the
     * samples are not the file's own: SDL decoded them (ADPCM, a-law and mu-law come back as 16-bit, larger than in the
     * file) - then the size is that of a canonical PCM file around them. */
    if (riffBytes < bytes + 36)
        riffBytes = bytes + 36;
    out->riffBytes = riffBytes;
    return 1;
}

void Platform_WavFree(void *data)
{
    SDL_free(data);
}
