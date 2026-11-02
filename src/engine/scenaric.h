#ifndef SDW_ENGINE_SCENARIC_H
#define SDW_ENGINE_SCENARIC_H

/* The functions and globals scenaric.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct Animator;
class Heap;
struct ScnClassRegEntry;
class ScnObject;
class ZoneList;

extern u32 g_levelExitFlags;
extern void *g_levelHeapStorage;
extern ScnClassRegEntry g_scenaricClassRegistry[];
extern Heap g_scenaricLevelHeap;
extern ZoneList g_waterZones[];                             /* the seven global zone lists, water first */
void Anim_FireSoundEvent(Animator *anim, ScnObject *owner);
ScnObject *Install_CinematicResource(u32 resIdx);
ScnObject *Install_ScenaricResource(u32 resIdx);
void Level_ExitUpdate();
s32 ObjGrid_QueryBoxesInRectXZ(s32 minX, s32 minZ, s32 maxX, s32 maxZ, ScnObject **out);
s32 ObjGrid_QueryPointsInRectXZ(s32 minX, s32 minZ, s32 maxX, s32 maxZ, ScnObject **out);         /* max 64 */
void Render_CalcPivotOffset(Vec3s *rot, Vec3s *pivot, Vec3s *outOffset, Vec3s *scaleOrNull);
void Scenaric_AllocLevelHeap();
ScnObject *Scenaric_CreateObject(u16 classId, void *record);
void Scenaric_InitLevelState();
ScnObject *ScnGenericBody_Create(void *record);
ScnObject *ScnGenericLogic_Create(void *record);
void *Scn_BuildRecordFromExport(u16 gameResId, u16 *outRecord, u16 rotOrFlags, const Vec3s *pos);
void Zone_GetFlowHeading(const Box *zone, s16 *heading, s32 *speed);
void Zone_GetFlowVelocity(const Box *zone, Vec3s *outVel); /* water current from the zone's flag bits */
void Zones_ResolveGlobalLists();

#endif
