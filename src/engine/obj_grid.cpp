#include "sdw_classes.h"

#include "../sdk/crt.h"
#include "list.h"

s16 g_objGridOriginX = 0;             /* object grid origin X */
s16 g_objGridOriginZ = 0;             /* object grid origin Z (z) */
u16 g_objGridShift = 0;               /* log2 of the cell size */
u16 g_objGridDimX = 0;                /* cells along X */
u16 g_objGridDimZ = 0;                /* cells along Z */
ListNode ***g_objGridCells = 0;       /* dimX*dimZ list heads, one per cell */
ListNode **g_objGridOverflowList = 0; /* the list every object outside the grid shares */
u16 *g_pWarObjGrid = 0;               /* WAR type 0x84 (set by the loader) */

void ObjGrid_Init(u16 *header)
{
    struct {
        s32 z, x;
        ListNode ***cell;
    } w;

    g_objGridDimX = *header++;
    g_objGridDimZ = *header++;
    g_objGridOriginX = *header++;
    g_objGridOriginZ = *header++;
    g_objGridShift = *header++;
    g_objGridCells = (ListNode ***)malloc((u32)g_objGridDimX * g_objGridDimZ * sizeof(ListNode **));
    w.cell = g_objGridCells;
    for (w.z = 0; w.z < g_objGridDimZ; ++w.z) {
        for (w.x = 0; w.x < g_objGridDimX; ++w.x) {
            List_Create(w.cell);
            ++w.cell;
        }
    }
    List_Create(&g_objGridOverflowList);
}

/* frees the cell array on level teardown. The lists themselves live in the level heap and go with it. */
void ObjGrid_Free()
{
    if (g_objGridCells) {
        free(g_objGridCells);
        g_objGridCells = 0;
    }
}

/* the cell a point falls in, WITHOUT clamping: the caller gets negative or out-of-range cells and
 * ObjGrid_GetCellList turns those into the overflow list. */
void ObjGrid_CellFromXZ_Unclamped(s16 x, s16 z, s16 *cellX, s16 *cellZ)
{
    *cellX = (x - g_objGridOriginX) >> g_objGridShift;
    *cellZ = (z - g_objGridOriginZ) >> g_objGridShift;
}

/* object-grid cell of a point, clamped to the grid (ObjGrid_CellFromXZ_Unclamped is the same
 * without the clamp). */
void ObjGrid_CellFromXZ(s16 x, s16 z, s16 *cellX, s16 *cellZ)
{
    *cellX = (x - g_objGridOriginX) >> g_objGridShift;
    *cellZ = (z - g_objGridOriginZ) >> g_objGridShift;
    if (*cellX < 0)
        *cellX = 0;
    else if (*cellX >= g_objGridDimX)
        *cellX = g_objGridDimX - 1;
    if (*cellZ < 0)
        *cellZ = 0;
    else if (*cellZ >= g_objGridDimZ)
        *cellZ = g_objGridDimZ - 1;
}

/* the list head of a cell, or the shared overflow list when the cell is outside the grid. */
ListNode **ObjGrid_GetCellList(s16 cellX, s16 cellZ)
{
    if ((cellX < 0) | (cellZ < 0) | (cellX >= g_objGridDimX) | (cellZ >= g_objGridDimZ))
        return g_objGridOverflowList;
    else
        return g_objGridCells[cellX + cellZ * g_objGridDimX];
}
