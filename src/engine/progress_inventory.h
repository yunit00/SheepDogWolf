#ifndef SDW_ENGINE_PROGRESS_INVENTORY_H
#define SDW_ENGINE_PROGRESS_INVENTORY_H

/* The functions and globals progress_inventory.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class ScnObject;

void InvWheel_Draw(s16 angle, u8 crayonFrame, s32 slideY);
void InvWheel_DrawSlot(u16 classId, s32 x, s32 y, u32 color, u8 crayonFrame);
s32 InvWheel_IsRotating();
void InvWheel_Render();
void Inventory_Add(ScnObject *obj);
void Inventory_ClearSelection();
void Inventory_CommitAtCheckpoint();
u32 Inventory_CountClass(u16 classId);
void Inventory_DropUncommitted();
ScnObject *Inventory_GetSelectedObject();
void Inventory_Init();
u16 Inventory_NextClass(u16 classId);
u16 Inventory_PrevClass(u16 classId);
void Inventory_Remove(ScnObject *obj);
void Inventory_SelectClass(u16 classId);
ScnObject *Inventory_SelectNext();
ScnObject *Inventory_SelectPrev();
void Inventory_SetWheelOpen(s32 open);
void Inventory_UpdateFx();
ScnObject *ItemFly_GetObject();
void ItemFly_SetFromPos(const Vec3s *pos);
void ItemFly_Start(ScnObject *obj, u8 mode, const Vec3s *fromPos);
void ItemFly_Stop();

#endif
