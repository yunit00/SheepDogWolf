#ifndef SDW_OBJECTS_INSTANCE_H
#define SDW_OBJECTS_INSTANCE_H

/* The functions and globals instance.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct Aabb;
struct Animator;
struct AttachLink;
class Camera;
class Instance;
class ScnObject;
class WorldObj;

extern u16 g_attachLinkCount;
void Animator_Init(Instance *instance, Animator *animation, void *poseBuffers, s32 keepBounds);
AttachLink *AttachLink_Alloc(Instance *parent, ScnObject *owner, u8 partIndex, const Vec3s *offset, const Vec3s *rot,
                             s32 rootRotation, const Vec3s *worldOffset);
AttachLink **AttachLink_FindFirstChildSlot(Instance *parentInst);
void AttachLink_Free(AttachLink *link);
void AttachLink_InitPool();
void AttachLink_SetParams(AttachLink *link, u8 joint, const Vec3s *localOffset, const Vec3s *rotation, s32 rootRotation,
                          const Vec3s *worldOffset);
s8 Cull_IsAabbVisible(const Aabb *aabb, const Camera *camera);
void *Dav_GetResourcePtr(u16 index);
void Instance_InitFromWarRecord(Instance *instance, u16 *record, u8 mode, s32 keepTransform);
void *WorldObj_CreateFromResource(u32 resIdx); /* (32-bit index: the callee reads a dword) */
void WorldObj_Free(WorldObj *object);

/* The functions and globals instance.cpp defines, declared once for every file that uses them. */

struct AttachLink;

extern u8 g_aabbEdgePairs[16][2];
extern AttachLink *g_attachLinkPool[10];

#endif
