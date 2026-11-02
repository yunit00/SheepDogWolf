#ifndef SDW_ENGINE_ID_LIST_H
#define SDW_ENGINE_ID_LIST_H

/* The functions and globals id_list.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

/* The id-list tables hold one word per header and per entry; an entry is the address of a record. The words are uptr:
 * u32 as in the original on a 32-bit target (the loaders relocate the file's tables in place), and
 * a pointer-sized copy the loaders make (Res_WidenIdLists), since an address may not fit in the file's 32 bits. */
extern uptr *g_idListBlob;
extern u32 g_idListCount;
extern u32 g_warExportCount;
extern uptr *g_warExportTable;                               /* WAR type 0x82 */
extern u32 g_warRelocCount;
extern uptr *g_warRelocTable;                                /* WAR type 0x85 */
uptr *IdList_FindWithCount(u16 id, u16 *countOut);
uptr *IdList_GetBlob(u32 *countOut);
u16 IdList_GetId(u16 *rec);
uptr *IdList_Next(uptr *rec);
void Res_RelocateIdLists(u32 *blob, int nRecords, int base);
uptr *Scn_FindIdList(u16 id, u16 *countOut);
u32 *WarReloc_Find(u16 id, u16 *countOut, s32 *claim);
uptr *Res_WidenIdLists(const u32 *records, u32 nRecords, u8 *base);  /* a pointer-sized, relocated copy (malloc'd) */
uptr *Res_WidenPairs(const u32 *pairs, u32 nPairs, u8 *base);        /* the same for the type-0x85 pair table */

#endif
