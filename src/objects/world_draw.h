#ifndef SDW_OBJECTS_WORLD_DRAW_H
#define SDW_OBJECTS_WORLD_DRAW_H

/* The functions and globals world_draw.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct Animator;
class Camera;
class Instance;
struct Mat34s;
class Mat44;
struct Vec3i;
class WorldObj;

void Instance_DrawAnimated(Instance *instance, Animator *animation, Camera *camera, const Vec3s *scale);
void Instance_DrawAnimatedOnScreen(Instance *instance, Animator *animation, Camera *camera, sptr unused3, s32 unused4,
                                   s32 distance, const s16 *screenXY);
void Instance_DrawRigid(Instance *inst, Camera *view, const Vec3s *scale, const Mat34s *mat);
void Instance_DrawSecondary_Stub(Instance *inst, Camera *view, const Vec3s *scale, const Mat34s *mat);
void Instance_SetRigidTransforms(Instance *, const Camera *, const Vec3s *, const Mat44 *);
void Instance_UpdateVisibility(Instance *, const Vec3i *, u16, s32);
void Mat34s_FromEulerScaled(const Vec3s *rot, Mat34s *out, const Vec3s *scale);
void WorldObj_DrawSky(WorldObj *obj, Camera *cam);

#endif
