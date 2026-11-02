#Changelog

## [Unreleased] -branch `sdl3-audio`

### Corrected

-**No sound in Windows build (x64).**SDL3 opened the device via WASAPI and the mixer worked, but no sound effects
  were created at all, and music and voices played silence. The reason is the WAV reading, not the audio output.
  -On Windows `WaveFile` called the real `winmm.dll` (`mmioOpenA`, `mmioGetInfo`, `mmioAdvance`...), and on Linux and
    macOS -own implementation of `mmio_posix_compat.cpp`.
-In the Windows SDK, the `MMIOINFO` structure is packed into 1 byte units (`mmsystem.h` includes `pshpack1.h`), and in
    `src/sdk/mmsystem.h` it was declared with normal alignment. In the 2001 32-bit game this was the same,
    on x64 -no:

    | x64 | sizeof | htask | cchBuffer | pchBuffer | pchNext | pchEndRead |
    |---------------------|-------:|------:|----------:|----------:|--------:|-----------:|
| Windows SDK (winmm.dll) |    100 |    20 |        28 |        32 |      40 |         48 |
    | old `src/sdk/mmsystem.h` |    112 |    24 |        32 |        40 |      48 |         56 |

  -Consequences, reproduced on the old code under Windows x64:
    -bank of level sound effects (`.SND`, opened from memory via `'MEM'`) -`Open` returns `E_FAIL`:
      winmm reads `pchBuffer` at offset 32, where the game wrote `cchBuffer`;
-music and voices (files from disk) -`Open` passes, but `Read` receives 0 bytes.
  -There was no error on Linux/macOS: there both the game and the mmio implementation used the same declaration.

### Changed

-**WAV is read via SDL3 (`SDL_LoadWAV_IO`) on all platforms**-same code instead of winmm on Windows and
  custom mmio on other systems.
  -New `src/platform/wav_sdl.cpp`: `Platform_WavLoad` /`Platform_WavFree` (declared in `platform.h`). File
opens via `SDL_IOFromFile` (on Linux/macOS the path goes through `Sdw_ResolvePath`: `\` → `/` and search without
    register sensitive), image from the `.SND` bank -via `SDL_IOFromConstMem`.
  -Formats that the mixer does not play directly (IMA/MS ADPCM, a-law/mu-law, float, 24/32 bit, more than two channels),
    are converted to 16-bit PCM (`SDL_ConvertAudioSamples`). Previously, such files were not played silently:
    `CreateSoundBuffer` only accepted PCM.
-Loading errors are written to the log: `SheepD3D: wav <path>: <SDL error>`.
-**`WaveFile` (`src/engine/wave_file.cpp`)**rewritten without mmio, external interface is the same: `Open`, `ResetFile`,
  `Read`, `Close`, `format`, `ckData.cksize`, `ckRiff.cksize`.
  -The data block is loaded entirely into `samples` (instead of `HMMIO hmmio`); `Read` copies from memory, `ResetFile`
    rewinds to the beginning.
  -`ckRiff.cksize` is still taken from the RIFF header: it is used by `StreamSound::ServiceNotify` to determine the end
clip. For decoded formats, the size of the canonical PCM file is used (data + 36 bytes).
  -Removed `ReadMmio` /`ReadRiffHeader`; added `ReadAt` -reading without position shift for the lipsync indicator.
-**`StreamSound::GetVoiceAmplitude`**(lip sync) reads the sample via `WaveFile::ReadAt` instead of a pair
  `mmioSeek`/`mmioRead`. A slight difference from the original: it took the sample at the offset in the *file*, that is, at the length
header (44 bytes, fractions of a millisecond) before the playback position. Now -exactly according to the position in the data.
-**`StaticSound::FillBuffer`**reads samples directly into a locked voice buffer. Intermediate 1-MiB
  `g_waveStagingBuffer` removed: decoded ADPCM could overflow it.

### Deleted

-`src/platform/mmio_posix_compat.cpp` is no longer needed.
-Removed `MMIOINFO`, `HMMIO`, `mmio*` declarations and unused constants from `src/sdk/mmsystem.h`. Remained
`WAVEFORMATEX`, `WAVE_FORMAT_PCM`, `mmioFOURCC`, `FOURCC_RIFF`. Accidentally calling winmm will now give a compilation error.
-`winmm` has been removed from the game library list in `CMakeLists.txt` (SDL3 still includes it for its timers).

### Verified

-Linux (clang) and Windows x64 (clang + MinGW, toolchain from `build_win64.sh`) are built. New exe does not import from
  `WINMM.dll` does not contain a single mmio function -only `timeBeginPeriod` /`timeEndPeriod` from SDL3 itself.
-Test program (not included in the patch): WAV from disk (backslashes, different case, `LIST` chunk before data) and from
  memory, 8/16 bit, mono/stereo, IMA ADPCM, float32, missing/broken/cropped file, `Read` in parts,
  `ResetFile`, `ReadAt`, repeat `Close`. Plus end-to-end run `WaveFile` → DirectSound layer → SDL3 mixer
  with writing the output to a file (SDL `disk` driver): the output is a 440 Hz tone with the original amplitude. Run as Linux
and as a Windows program (under Wine) -all checks have been passed.
-The same test with the old `WaveFile` under Windows reproduces the error: from memory -`E_FAIL`, from disk -0 bytes.
-Not tested on real game data or on real Windows: you need to run the level with effects, music and voice acting.

### Known limitations

-The entire music track is loaded into memory (`SDL_LoadWAV_IO` does not have a streaming mode). This is several tens of MB
to the track; loading occurs in the `StreamPlayer` loader stream, as before.
-On Windows, paths are transferred to SDL as UTF-8. Game paths are ASCII; if `exeDir` came from the launcher, it is also UTF-8
  (from `SDL_GetBasePath`) so that folders with non-ASCII names now open correctly. With `mmioOpenA` they don't
  opened.
-Build via `build_win64.sh` on systems with MinGW on msvcrt (for example, Ubuntu) crashes when linking to
`__imp__time32` from `src/compat/crt_compat.c`. This was before the patch: on Arch MinGW is compiled on ucrt, and there is no error there.