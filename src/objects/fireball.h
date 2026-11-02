#ifndef SDW_OBJECTS_FIREBALL_H
#define SDW_OBJECTS_FIREBALL_H

/* The functions and globals fireball.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class ScnObject;

extern ScnObject *g_pFireBall;
ScnObject *FireBall_Create(void *record);

#endif
