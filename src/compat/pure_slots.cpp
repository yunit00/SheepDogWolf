/* The base classes' default virtuals, defined once here: the pure ones are _purecall (calling one aborts), the others
 * (ScnObject::HandleMessage, ScnBody::Update, ScnBody::CustomCollide) have their default bodies. */
#include "sdw_classes.h"

extern "C" int __cdecl _purecall(void);

#ifndef _MSC_VER /* MSVC's runtime has its own */
#include <stdlib.h>
extern "C" int _purecall(void)
{
    abort();
}
#endif

#define PURE_BODY { _purecall(); }
#define PURE_BODY_RET { _purecall(); return 0; }

void ScnObject::PostLoadInit() PURE_BODY
void ScnObject::Update() PURE_BODY
void ScnObject::Render(Camera *) PURE_BODY
s32 ScnObject::CustomCollide(ScnObject *, CollBox *, Vec3s *, s32 *, s32 *, CollContact *, s32 *, u32) PURE_BODY_RET
void ScnObject::RenderScaled(Camera *, Vec3s *) PURE_BODY
sptr ScnObject::HandleMessage(ScnObject *, u32, void *) { return 0; }
void ScnBody::Update() { AdvanceAnim(); }
s32 ScnBody::CustomCollide(ScnObject *, CollBox *, Vec3s *, s32 *, s32 *, CollContact *, s32 *, u32) { return 0; }

s32 Sound::CreateFromWave(SoundDevice *, WaveFile *) PURE_BODY_RET
s32 Sound::CreateFromFile(SoundDevice *, char *) PURE_BODY_RET
s32 Sound::Free() PURE_BODY_RET
s32 Sound::RestoreBuffer() PURE_BODY_RET
s32 Sound::Play() PURE_BODY_RET
void Sound::Stop() PURE_BODY
void Sound::SetPaused(u8) PURE_BODY
void Sound::RewindBuffer() PURE_BODY
s32 Sound::SetBufferPosition(float) PURE_BODY_RET
u8 Sound::IsPlaying() PURE_BODY_RET
s32 Sound::Sound_SetBufferVolume(float) PURE_BODY_RET
s32 Sound::SetBufferPan(float) PURE_BODY_RET
s32 Sound::Sound_SetBufferFrequency(u32) PURE_BODY_RET
s32 Sound::FillBuffer() PURE_BODY_RET
