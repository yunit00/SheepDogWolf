/*
 * Vdx7 0x10 { vtbl; u8 ok; u32 *entries; Vdx7Record *records (10 bytes each); }: the .DAV companion table of a Black
 * Sheep file. It reads the file through a local BsStream (declared here).
 */
#include "sdw_types.h"

#define SDW_MEMBERS_BsStream BsStream(const char *path); /* BsStream_Open */
#define SDW_MEMBERS_Vdx7 Vdx7(const char *path);         /* Vdx7_Open */
#include "sdw_classes.h"

#include "../sdk/crt.h"

/* ------------------------------------------------------------------------------------------------ Vdx7 */

/* header "VDX7", four skipped u32s, then the u32 at +0x14 is where the counts live: u16 nEntries, u16 nRecords,
 * u16 (unused), u32 offset of the entry table (u16 each), u32 offset of the record table (five u16s each). */
Vdx7::Vdx7(const char *path)
{
    BsStream bs(path);
    u32 back;
    char head[4];
    u32 e;
    u32 nrec;
    u32 nEntries;
    u32 ri;
    u32 addr;

    ok = 0;
    if (bs.ok == 1) {
        head[0] = bs.ReadU8(1);
        head[1] = bs.ReadU8(1);
        head[2] = bs.ReadU8(1);
        head[3] = bs.ReadU8(1);
        if (strncmp(head, "VDX7", 4) == 0) {
            bs.ReadU32(1);
            bs.ReadU32(1);
            bs.ReadU32(1);
            bs.ReadU32(1);
            bs.Seek(bs.ReadU32(1));
            nEntries = bs.ReadU16(1);
            entries = (u32 *)malloc(nEntries * 4);
            nrec = bs.ReadU16(1);
            records = (Vdx7Record *)malloc(nrec * 10);
            bs.ReadU16(1);
            addr = bs.ReadU32(1);
            back = bs.cursor;
            bs.Seek(addr);
            for (e = 0; e < nEntries; e++)
                entries[e] = bs.ReadU16(1);
            bs.Seek(back);
            addr = bs.ReadU32(1);
            bs.Seek(addr);
            for (ri = 0; ri < nrec; ri++) {
                records[ri].x = bs.ReadU16(1);
                records[ri].w = bs.ReadU16(1);
                records[ri].y = bs.ReadU16(1);
                records[ri].h = bs.ReadU16(1);
                records[ri].page = bs.ReadU16(1);
            }
            ok = 1;
        }
    }
}

Vdx7::~Vdx7()
{
    if (entries)
        free(entries);
    if (records)
        free(records);
}

/* Vdx7_ScalarDeletingDtor: generated from the virtual destructor. */
