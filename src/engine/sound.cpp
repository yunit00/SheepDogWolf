/*
 * This object is the Sound part of the sound classes (the constructor and the destructor). Being an object of its own
 * is what makes StaticSound / StreamSound call Sound::Sound out of line.
 *
 * The code is a game-side rework of the DirectX SDK samples: Sound / StaticSound / StreamSound follow the DX8 dsutil
 * CSound / CStreamingSound.
 */
#define SDW_MEMBERS_Sound Sound(); /* Sound_Construct */
#include "sdw_classes.h"

#include "../sdk/win32.h"
#include "../sdk/mmsystem.h"
#include "../sdk/dsound.h"
#include "../sdk/crt.h"

/* ================================================================ Sound, the abstract base (vtable) */

Sound::Sound()
{
    looping = 0;
    waveIsExternal = 1;
    bufferBytes = 0;
    wave = 0;
    buffer = 0;
    volumeCb = 0;
    panCb = 0;
    baseFrequency = 0;
}

Sound::~Sound()
{
    if (wave && !waveIsExternal) {
        delete wave;
        wave = 0;
    }
    if (buffer) {
        buffer->Release();
        buffer = 0;
    }
}
