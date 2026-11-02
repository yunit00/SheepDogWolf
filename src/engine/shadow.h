#ifndef SDW_ENGINE_SHADOW_H
#define SDW_ENGINE_SHADOW_H

/* The functions and globals shadow.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class ScnObject;
class Shadow;

s16 ObjGrid_QueryGroundY(Vec3s *pos, Vec3s *outNormal, s16 minY, ScnObject *exclude); /* object tops */
void Shadow_FreeLevel();
void Shadow_Init(Shadow *shadow, u8 radius);
void Shadow_LoadLevel();
void Shadow_Render(Shadow *shadow);
void Shadow_Update(Shadow *shadow, Vec3s *pos, ScnObject *owner);

#endif
