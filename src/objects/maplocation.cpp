/* The UiQuad declarations follow the definitions (s32 x/y
 * parameters). */
#include "sdw_types.h"
#define SDW_MEMBERS_ScnObject            \
    static void *operator new(uptr size); \
    void SetUpdateMode(s32 mode);
#define SDW_MEMBERS_ZoneList void Load(u32 id);

#include "sdw_enums.h"
#include "scenaric_props.h"
#include "sdw_classes.h"
#include "../engine/id_list.h"
#include "../engine/scn_tools.h"
#define SDW_INLINE_SCNOBJECT_SETVISIBLE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETVISIBLE_S32
#define SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETUPDATEMODE_S32
#define SDW_INLINE_UIQUAD_SETFADELEVEL_U8 1
#include "../engine/ui_quad_inlines.h"
#undef SDW_INLINE_UIQUAD_SETFADELEVEL_U8
#define SDW_INLINE_UIQUAD_SETCOLOR_U32 1
#include "../engine/ui_quad_inlines.h"
#undef SDW_INLINE_UIQUAD_SETCOLOR_U32
extern Wolf *g_pWolf;
#define SDW_INLINE_FREE_PROPU32_VOID_U32 2
#include "../engine/scn_tools_inlines.h"
#undef SDW_INLINE_FREE_PROPU32_VOID_U32
#define SDW_INLINE_ZONELIST_LOAD_U32 1
#include "../engine/zone_list_inlines.h"
#undef SDW_INLINE_ZONELIST_LOAD_U32
#define SDW_INLINE_ZONELIST_CONTAINSXZ_VEC3S 1
#include "../engine/zone_list_inlines.h"
#undef SDW_INLINE_ZONELIST_CONTAINSXZ_VEC3S

/* MapLocation_AddSlot */
void MapLocation::AddSlot(s32 boxOffset, s32 xOffset, s32 yOffset)
{
    u32 p = PropU32(props, boxOffset);
    if (p) {
        slots[slotCount].zones.Load(p);
        if (slots[slotCount].zones.count > 0) {
            slots[slotCount].mapX = PropU32(props, xOffset) << 1;
            slots[slotCount].mapY = PropU32(props, yOffset);
            slotCount++;
        }
    }
}

/* MapLocation_Init */
void MapLocation::PostLoadInit()
{
    u16 p;
    void *q = record;
    props = q;
    slotCount = 0;
    AddSlot(0, 4, 8);
    AddSlot(0xc, 0x10, 0x14);
    AddSlot(0x18, 0x1c, 0x20);
    AddSlot(0x24, 0x28, 0x2c);
    AddSlot(0x30, 0x34, 0x38);
    AddSlot(0x3c, 0x40, 0x44);
    AddSlot(0x48, 0x4c, 0x50);
    AddSlot(0x54, 0x58, 0x5c);
    AddSlot(0x60, 0x64, 0x68);
    AddSlot(0x6c, 0x70, 0x74);
    AddSlot(0x78, 0x7c, 0x80);
    AddSlot(0x84, 0x88, 0x8c);
    AddSlot(0x90, 0x94, 0x98);
    AddSlot(0x9c, 0xa0, 0xa4);
    AddSlot(0xa8, 0xac, 0xb0);
    p = 1;
    markerBitmap = IdList_FindWithCount(DAV_IDI_IGLCOYO1, &p);
    SetVisible(0);
    SetUpdateMode(SCN_UPD_NEVER);
}

/* MapLocation_DrawWolfMarker */
s32 MapLocation::DrawWolfMarker(u8 fade, u32 color)
{
    s32 p;
    for (p = 0; p < slotCount; p++) {
        if (slots[p].zones.ContainsXZ(&g_pWolf->pos)) {
            marker.UiQuad_SetFromBitmap((const u16 *)*markerBitmap, slots[p].mapX, slots[p].mapY, 1, 1, 0x400, 0x400);
            marker.SetFadeLevel(fade);
            marker.SetColor(color);
            marker.UiQuad_Draw(0xb);
            return 1;
        }
    }
    return 0;
}

/* MapLocation_Create */
ScnObject *MapLocation_Create(void *record)
{
    MapLocation *obj = new MapLocation;
    obj = (MapLocation *)obj->Init(record);
    return obj;
}

/* ScnObject_Nop: the inherited Reset slot. */
inline void ScnObject::Reset() {}
