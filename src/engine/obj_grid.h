#ifndef SDW_ENGINE_OBJ_GRID_H
#define SDW_ENGINE_OBJ_GRID_H

/* The functions and globals obj_grid.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct ListNode;

extern ListNode ***g_objGridCells; /* dimX*dimZ list handles (each points at the cell's head node) */
extern u16 g_objGridDimX;
extern u16 *g_pWarObjGrid;         /* WAR type 0x84 */
void ObjGrid_CellFromXZ(s16 x, s16 z, s16 *cellXOut, s16 *cellZOut);           /* clamped to the grid */
void ObjGrid_CellFromXZ_Unclamped(s16 x, s16 z, s16 *cellXOut, s16 *cellZOut);
void ObjGrid_Free();
ListNode **ObjGrid_GetCellList(s16 cellX, s16 cellZ);                          /* the cell's list head */
void ObjGrid_Init(u16 *gridHeader);

#endif
