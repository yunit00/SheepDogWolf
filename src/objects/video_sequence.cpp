/*
 * The player is g_vidPlayer here. No other object refers to it. The lists stay uninitialised for the same reason (a
 * zero-initialised global would be placed after the player). SoundDevice::Init is declared (void *, u32, u8), as
 * src/engine/sound_device.cpp defines it (same code; an HWND__ * spelling would decorate to a name nothing defines).
 */
#define SDW_MEMBERS_VideoPlayer VideoPlayer();
#define SDW_MEMBERS_SoundDevice SoundDevice();
#include "sdw_classes.h"
FmvList g_fmvListCredits; /* played by Progress_GotoScene(-8) after the Ending scene */
FmvList g_fmvListIntro;   /* the three intro clips played at start-up */
VideoPlayer g_vidPlayer;  /* the DirectShow FMV player (renamed, see the header) */
#include "../engine/stream_player.h"
#include "../engine/draw2d.h"
#include "../engine/sound_mgr.h"
extern HWND__ *g_hGameWindow;
#include "../sdk/win32.h"
#include "../sdk/crt.h"

u8 Video_PlaySequence(FmvList *list)
{
    (void)list;
    return 1; /* DirectShow/Indeo FMVs have no SDL3 decoder; skip them without disturbing SDL audio. */
#if 0 /* the original DirectShow player, kept for an SDL3 version: it deletes and recreates the sound device */
    u8 resultValue;
    u32 index;
    u32 rateData;
    char pathValue[SDW_PATH_MAX];
    u8 bits;
    resultValue = 1;
    index = 0;
    if (g_pStreamPlayer)
        g_pStreamPlayer->StopAndFree();
    Sound_ShutdownChannels();
    rateData = g_pSoundSystem->sampleRate;
    bits = g_pSoundSystem->bitsPerSample;
    delete g_pSoundSystem;
    CoInitialize(0);
    while (resultValue == 1 && index < list->count) {
        /* the player blits into DirectDraw's primary surface: with another renderer there is none, and the videos are
         * skipped */
        if (!g_vidPlayer.Init(g_pD3DAppMain))
            break;
        strcpy(pathValue, g_pathDemoDir);
        strcat(pathValue, "\\");
        strcat(pathValue, list->clips[index]);
        resultValue = g_vidPlayer.PlayFile(pathValue, 1);
        ++index;
    }
    CoUninitialize();
    g_pSoundSystem = new SoundDevice;
    g_pSoundSystem->Init(g_hGameWindow, rateData, bits);
    return resultValue;
#endif
}
