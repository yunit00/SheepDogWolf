/*
 * Record formats: an id-list record is a u32 header (the id in the low word, the entry count in the high word) followed
 * by that many u32 entries, which Res_RelocateIdLists turns from file offsets into addresses. Whether the original
 * spelled them this way is not known.
 */
#include "sdw_classes.h"

uptr *g_warExportTable = 0; /* WAR type 0x82: id-list records                                   */
u32 g_warExportCount = 0;  /* record count                                                     */
uptr *g_idListBlob = 0;    /* DAV id-list records                                              */
u32 g_idListCount = 0;     /* record count                                                     */
uptr *g_warRelocTable = 0; /* WAR type 0x85: {u32 idAndCount; u32 ptr} pairs                   */
u32 g_warRelocCount = 0;   /* pair count                                                       */

/* walks nRecords id-list records and adds base to every entry (file offsets -> pointers). */
void Res_RelocateIdLists(u32 *blob, int nRecords, int base)
{
    u16 count;
    for (; nRecords != 0; nRecords--) {
        count = (u16)(*blob >> 16);
        blob++;
        for (; count != 0; count--) {
            *blob = *blob + base;
            blob++;
        }
    }
}

/* the id (low word) of the type-0x85 pair at index, clamped to the last pair; 0xffff when the table is empty. No callers. */
u16 WarReloc_GetIdAt(u16 index)
{
    u32 count = g_warRelocCount;
    uptr *p = g_warRelocTable;
    if (index >= count)
        index = (u16)(count - 1);
    if (index == 0xffff)
        return 0xffff;
    p += index * 2;
    return (u16)(*p & 0xffff);
}

/* finds the type-0x85 pair for id. Writes its 15-bit count through countOut; when *claim is non-zero on entry
 * marks the pair claimed; either way *claim receives the pair's previous claimed bit. Returns the pair's pointer word. */
u32 *WarReloc_Find(u16 id, u16 *countOut, int *claim)
{
    int wasClaimed;
    u32 count = g_warRelocCount;
    uptr *p = g_warRelocTable;
    while (count != 0 && (*p & 0xffff) != id) {
        p += 2;
        count--;
    }
    if (count == 0) {
        *claim = 0;
        *countOut = 0;
        return 0;
    }
    wasClaimed = (u16)(*p >> 16) >> 15;
    *countOut = (u16)((*p >> 16) & 0x7fff);
    if (*claim != 0)
        *p = *p | 0x80000000;
    *claim = wasClaimed;
    return (u32 *)p[1];
}

/* the entries of the WAR export-table record for id, and their count; NULL and 0 when there is none.
 * `result` is named for its slot (count -0xc, p -8, result -4, as in the original); `rec`, `found`, `entry` land elsewhere. */
uptr *Scn_FindIdList(u16 id, u16 *countOut)
{
    u32 count = g_warExportCount;
    uptr *p = g_warExportTable;
    uptr *result;
    while (count != 0 && (*p & 0xffff) != id) {
        p = p + (*p >> 16) + 1;
        count--;
    }
    if (count == 0)
        result = 0;
    else
        result = p;
    if (result == 0) {
        *countOut = 0;
        return 0;
    }
    *countOut = (u16)(*result >> 16);
    return result + 1;
}

/* the same search over the DAV id-list blob. Returns the ENTRIES (record + 1), not the record header. */
uptr *IdList_FindWithCount(u16 id, u16 *countOut)
{
    u32 count = g_idListCount;
    uptr *p = g_idListBlob;
    uptr *result;
    while (count != 0 && (*p & 0xffff) != id) {
        p = p + (*p >> 16) + 1;
        count--;
    }
    if (count == 0)
        result = 0;
    else
        result = p;
    if (result == 0) {
        *countOut = 0;
        return 0;
    }
    *countOut = (u16)(*result >> 16);
    return result + 1;
}

/* the DAV id-list blob and its record count; NULL (count untouched) when no level is loaded. */
uptr *IdList_GetBlob(u32 *countOut)
{
    if (g_idListBlob == 0)
        return 0;
    *countOut = g_idListCount;
    return g_idListBlob;
}

/* the record after rec (header + count entries); NULL stays NULL. */
uptr *IdList_Next(uptr *rec)
{
    if (rec == 0)
        return 0;
    return rec + (*rec >> 16) + 1;
}

/* a record's id, 0 for NULL. */
u16 IdList_GetId(u16 *rec)
{
    if (rec == 0)
        return 0;
    return *rec;
}

/* ---- level-file offsets are resolved, not relocated in place (src/include/sdw_fileptr.h) ---- */
#include "../sdk/crt.h"
#include "id_list.h"

u8 *g_warFileBase = 0; /* the loaded .WAR (Load_WAR) */
u8 *g_davFileBase = 0; /* the loaded .DAV (Load_DAV) */

/* SDW_OBJREF's handles: handle - 1 indexes this table of addresses. An address keeps its handle for the session (few
 * distinct objects are ever stored: the actors of the cinematics); the table grows in blocks of 256. */
static void **s_objRefs = 0;
static u32 s_objRefCount = 0, s_objRefCapacity = 0;

u32 SdwObjRef_Handle(void *p)
{
    u32 i;
    if (!p)
        return 0;
    for (i = s_objRefCount; i-- > 0;)
        if (s_objRefs[i] == p)
            return i + 1;
    if (s_objRefCount == s_objRefCapacity) {
        void **grown = (void **)malloc((s_objRefCapacity + 256) * sizeof(void *));
        if (s_objRefs) {
            memcpy(grown, s_objRefs, s_objRefCount * sizeof(void *));
            free(s_objRefs);
        }
        s_objRefs = grown;
        s_objRefCapacity += 256;
    }
    s_objRefs[s_objRefCount++] = p;
    return s_objRefCount;
}

void *SdwObjRef_Address(u32 handle)
{
    return handle && handle <= s_objRefCount ? s_objRefs[handle - 1] : 0;
}

/* The words of nRecords id-list records starting at `records` (a header word, then that many entries each). */
static u32 IdList_WordCount(const u32 *records, u32 nRecords)
{
    const u32 *p = records;
    for (; nRecords != 0; nRecords--)
        p += (*p >> 16) + 1;
    return (u32)(p - records);
}

/* A pointer-sized copy of nRecords id-list records, each entry relocated by base as Res_RelocateIdLists does (every
 * entry, zero included). The caller frees it (with the level). */
uptr *Res_WidenIdLists(const u32 *records, u32 nRecords, u8 *base)
{
    u32 words = IdList_WordCount(records, nRecords);
    uptr *wide = (uptr *)malloc((words ? words : 1) * sizeof(uptr));
    uptr *out = wide;
    u16 count;
    for (; nRecords != 0; nRecords--) {
        count = (u16)(*records >> 16);
        *out++ = *records++;
        for (; count != 0; count--)
            *out++ = (uptr)base + *records++;
    }
    return wide;
}

/* The same for the type-0x85 table's {u32 idAndCount; u32 offset} pairs. */
uptr *Res_WidenPairs(const u32 *pairs, u32 nPairs, u8 *base)
{
    uptr *wide = (uptr *)malloc((nPairs ? nPairs : 1) * 2 * sizeof(uptr));
    u32 i;
    for (i = 0; i < nPairs; i++) {
        wide[i * 2] = pairs[i * 2];
        wide[i * 2 + 1] = (uptr)base + pairs[i * 2 + 1];
    }
    return wide;
}
