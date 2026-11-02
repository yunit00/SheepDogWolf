#include "sdw_classes.h"

#include "../sdk/crt.h"

/* The DAV directory (DavHeader.dir): packed, its pointers sit at +6/+0xa/+0x12 (Load_DAV), which
 * the struct generator cannot lay out, so it is declared here. */
#include "sdw_fileptr.h"
#pragma pack(push, 1)
struct DavDirectory {
    u16 indexCount;  /* entries of `indices`; Res_GetValidatedIdList bounds an entry's index by it */
    u16 bitmapCount; /* records in `bitmaps`; Res_GetValidatedIdList bounds an entry's value by it */
    u16 unk04;
    SDW_DAVPTR(u16) indices;          /* relocated by Load_DAV; the id-list entries point into it */
    SDW_DAVPTR(DavBitmapRec) bitmaps; /* relocated by Load_DAV; 10-byte records (TexAtlas_GetPage) */
    u32 fileSize;          /* size of the whole .DAV file */
    SDW_DAVPTR(u32) idLists;          /* relocated by Load_DAV: {u32 count; id-list records} -> g_idListBlob */
};
#pragma pack(pop)

/* ---- globals ---- */
#include "id_list.h"
#include "file.h"
#include "bs_io.h"
#include "psx_dav.h"
#include "asset_validation.h"

/* ---- functions ---- */
void Debug_Printf(const char *fmt, ...); /* a no-op stub */
s32 Dav_Free(Dav *dav);                  /* below */

/* the texture page of a DAV bitmap record. */
u32 TexAtlas_GetPage(DavBitmapRec *rec)
{
    return rec->page;
}

/* reads the .DAV file at path into dav: a 0x44-byte probe read finds the directory and the file size, then the whole file
 * is read into dav->blob and its internal offsets become pointers. 0, or -1 (dav freed) on failure. The version check is
 * inverted: "Bad version" is printed when the magic DOES match (Debug_Printf is a no-op anyway). When the file does not
 * open, the failure path frees `probe` before anything was assigned to it. */
s32 Load_DAV(const char *path, Dav *dav)
{
    FileHandle file = {-1, 0, 0};
    s32 deadWord;
    u32 headerSize;
    u32 davSize;
    char ver[5];
    DavHeader *probe = 0;
    s32 unused = 0;

    if (File_Open(path, &file) <= 0) {
        Debug_Printf("Load_DAV: DAV File not found\n");
        goto fail;
    }
    headerSize = 0x44;
    if (file.size < (s32)headerSize)
        goto fail;
    probe = (DavHeader *)malloc(headerSize);
    if (!probe)
        goto fail;
    if (File_Read(&file, probe, headerSize) < 0) {
        Debug_Printf("Load_DAV: Read Error\n");
        goto fail;
    }
    if (PsxDav_Is(probe, headerSize)) {
        /* the PlayStation's .DAV: its VDX7 conversion, of which the part before the pages, as below */
        u32 size;
        u8 *image = (u8 *)Bs_LoadFile(path, &size);
        free(probe);
        probe = 0;
        File_Close(&file);
        if (!image)
            goto fail;
        davSize = ((DavDirectory *)(image + ((DavHeader *)image)->dir.off))->fileSize;
        dav->blob = (u8 *)malloc(davSize);
        memcpy(dav->blob, image, davSize);
        delete[] image;
        goto loaded;
    }
    /* the game keeps file offsets (sdw_fileptr.h): the probe's directory is at that offset from the probe */
    if (strncmp(probe->magic, "VDX7", 4) ||
        !AssetValidation::Range(headerSize, probe->dir.off, sizeof(DavDirectory)))
        goto fail;
    davSize = ((DavDirectory *)((u8 *)probe + probe->dir.off))->fileSize;
    if (davSize < headerSize || davSize > (u32)file.size || davSize > 64 * 1024 * 1024)
        goto fail;
    if (probe != 0) {
        free(probe);
        probe = 0;
    }
    dav->blob = (u8 *)malloc(davSize);
    if (!dav->blob)
        goto fail;
    File_Seek(&file, 0);
    if (File_Read(&file, dav->blob, davSize) < 0) {
        Debug_Printf("Load_DAV: Read Error\n");
        goto fail;
    }
    File_Close(&file);
loaded:
    if (!AssetValidation::Dav(dav->blob, davSize))
        goto fail;
    dav->header = (DavHeader *)dav->blob;
    sprintf(ver, "VDX7");
    if (strncmp(dav->header->magic, ver, 4) == 0)
        Debug_Printf(" Bad version: Viewer:%s, Dav&War:%s\n", ver, dav->header);
    /* The directory's pointers stay file offsets, resolved from g_davFileBase (sdw_fileptr.h), and the
     * id lists get a pointer-sized copy instead of being relocated in place. */
    g_davFileBase = dav->blob;
    {
        u32 *lists = dav->header->dir->idLists; /* {u32 count; id-list records} */
        g_idListCount = *lists;
        if (g_idListBlob)
            free(g_idListBlob);
        g_idListBlob = Res_WidenIdLists(lists + 1, g_idListCount, dav->blob);
    }
    return 0;

fail:
    File_Close(&file);
    if (probe != 0) {
        free(probe);
        probe = 0;
    }
    Dav_Free(dav);
    return -1;
}

/* frees the DAV image. Always 0. */
s32 Dav_Free(Dav *dav)
{
    if (dav->blob != 0 && dav->blob == g_davFileBase) { /* the id lists' copy goes with the image it was made from */
        if (g_idListBlob)
            free(g_idListBlob);
        g_idListBlob = 0;
        g_idListCount = 0;
        g_davFileBase = 0;
    }
    if (dav->blob != 0) {
        free(dav->blob);
        dav->blob = 0;
    }
    return 0;
}
