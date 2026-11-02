#ifndef SDW_OBJECTS_SHEEP_H
#define SDW_OBJECTS_SHEEP_H

/* The functions and globals sheep.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct FlockScentSource;
class ScnObject;
class Sheep;

extern Sheep *g_pSheepOutOfZone;
extern u8 g_sheepCount;
extern u32 g_sheepMovedMask;
void Flock_AddScentSource(ScnObject *object, s32 heading, s32 range);
FlockScentSource *Flock_FindScentSource(ScnObject *);
Sheep *Flock_GetObjNearestAvailableSheep_Scent(ScnObject *);
Sheep *Flock_GetObjNearestAvailableSheep_Vision(ScnObject *, u32);
s32 Flock_IsTargetReserved(ScnObject *);
u32 Flock_NearestAvailableSheepCB(ScnObject *, ScnObject *, u32, ScnObject *); /* defined after its user */
void Flock_RemoveScentSource(ScnObject *object);
s32 Flock_ReserveTarget(ScnObject *);
void Flock_ResetGlobals();
void Flock_UnreserveTarget(ScnObject *);
ScnObject *Sheep_Create(void *record);

/* The functions and globals sheep.cpp defines, declared once for every file that uses them. */

class Sheep;

extern Sheep *g_sheepTable[];

#endif
