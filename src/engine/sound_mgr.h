#ifndef SDW_ENGINE_SOUND_MGR_H
#define SDW_ENGINE_SOUND_MGR_H

/* The functions and globals sound_mgr.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

int Load_SND(char *path);
void Sound_AllocChannel(u16 soundId, void *owner, u16 *outHandle);
void Sound_InitSilentLoop();
s32 Sound_IsPlaying(u16 handle);
s32 Sound_IsSampleIdPlaying(u16);
void Sound_MixerTick();
void Sound_PauseAll();
void Sound_ResumeAll();
void Sound_SetDistanceFalloff(u32 nearDist, u32 farDist, float farGain, float nearGain);
void Sound_SetRate(u16 handle, s32 fixed4_12);
void Sound_SetVolume(u16, u16);
void Sound_ShutdownChannels();
void Sound_StartSilentLoop();
void Sound_Stop(u16 handle, void *owner);
void Sound_StopAll();
void Sound_StopSilentLoop();
void Sound_UpdateChannelGain(u16 ch);

#endif
