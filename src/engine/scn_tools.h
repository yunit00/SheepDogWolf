#ifndef SDW_ENGINE_SCN_TOOLS_H
#define SDW_ENGINE_SCN_TOOLS_H

/* The functions and globals scn_tools.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct CamSetup;
class CollBox;
struct GroundQuery;
struct Mat34s;
class ScnObject;
struct Trajectory;
class box;

extern void *g_voiceOwner;
Box *BoxList_FindContainingPoint(Vec3s *p, Box **list, u16 count);
Box *BoxList_FindContainingPointBelowTop(Vec3s *point, Box **boxes, u16 count);
Box *BoxList_FindContainingPointXZ(Vec3s *point, Box **boxes, u16 count);
Box *BoxList_FindOverlappingBox(CollBox *query, Box **boxes, u16 count);
Box *BoxList_FindOverlappingBoxXZ(Box *query, Box **boxes, u16 count);
s32 Box_GapXZ(CollBox *a, CollBox *b); /* max of the X and Z edge gaps (0 if overlapping) */
s32 Box_GroundQueryDome(GroundQuery *query, CollBox *box, Vec3s *position);
u16 Dav_FindResourceIndex(void *resource);
void Matrix_RollByDisplacement(Mat34s *matrix, Vec3s *delta, s32 radius);
void Scenaric_DrawClassIcon(u32 *layer, u16 classId, int x, int y, u32 unused, int useIconB,
                            u32 colorRGB);
ScnObject *Scenaric_FindByIdList(u16 listId);
ScnObject *Scenaric_FindByRecord(void *record);
void Scenaric_GetClassIcons(u16 classId, void **iconAOut, void **iconBOut);
Box *Scn_GetPropBox(void *props, s32 propOffset);
CamSetup *Scn_GetPropCamera(void *props, s32 propOffset);
void **Scn_GetPropIdList(void *props, u32 propOffset, u16 *countOut);
ScnObject *Scn_GetPropObject(void *props, s32 propOffset);
void Scn_GetPropString8(void *props, s32 propOffset, char *out9);
Trajectory *Scn_GetPropTrajectory(void *props, s32 propOffset);
s32 Vec3s_DistSq(Vec3s *a, Vec3s *b);
s32 Vec3s_ManhattanDistXZ(Vec3s *a, Vec3s *b);
void Vec3s_ScaleByDt(const Vec3s *vel, Vec3s *out);
s32 Voice_PlayStream(u32 voiceId, void *owner);
void Voice_StopStream(void *owner);

#endif
