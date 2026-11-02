#include "sdw_types.h"

static u8 g_cineOpStride_581750[9] = {0, 8, 8, 4, 2, 2, 4, 2, 2}; /* unreferenced here */
u8 g_sfxVolume;                                                   /* the sound-effect volume */

/* sets the sound-effect volume g_sfxVolume (Cine_Start mutes it with flags & 0x4000) */
void Sound_SetSfxVolume(u8 v)
{
    g_sfxVolume = v;
}

/* The two call signatures that share the one-byte RET (Stub_Ret). */
void Stub_Ret(u8 value) {}
void Stub_Ret() {}
