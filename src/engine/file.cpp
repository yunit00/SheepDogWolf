/*
 * File I/O helpers: a FileHandle {fd, size, remaining} over the CRT's low-level _open/_lseek/_read/_write/_close, the
 * whole-file load / save used by the pad recorder (src/engine/input.cpp).
 */
#include "sdw_classes.h"
#include "../sdk/crt.h"

/* ---- the game's own helpers ---- */
void Debug_Printf(const char *fmt, ...); /* an empty stub in the retail build */
#include "crc32.h"

s32 File_Read(FileHandle *h, void *buf, s32 len);
s32 File_Close(FileHandle *h);

/* ---- globals defined here ---- */
s32 g_fileLastSize = 1;  /* size of the last file File_Open opened */
s32 g_fileBytesRead = 1;
char g_filePath[SDW_PATH_MAX] = {0}; /* base directory + name, built by File_Open */
char g_fileBaseDir[4] = {0};  /* prefix File_Open puts before every name; never written: "" */

/* opens g_fileBaseDir + name read-only (O_BINARY), fills h and returns the size, or -1. */
s32 File_Open(const char *name, FileHandle *h)
{
    h->fd = -1;
    h->size = h->remaining = 0;
    if (!name || strlen(name) + strlen(g_fileBaseDir) >= sizeof(g_filePath))
        return -1;
    strcpy(g_filePath, g_fileBaseDir);
    strcat(g_filePath, name);
    h->fd = _open(g_filePath, _O_BINARY);
    if (h->fd == -1)
        return -1;
    h->size = _lseek(h->fd, 0, SEEK_END);
    if (h->size < 0) {
        File_Close(h);
        return -1;
    }
    h->remaining = h->size;
    _lseek(h->fd, 0, SEEK_SET);
    g_fileLastSize = h->size;
    g_fileBytesRead = 1;
    return h->size;
}

/* creates / opens path for writing (O_RDWR | O_CREAT, S_IREAD | S_IWRITE); 0, or -1 on failure. */
s8 File_Create(const char *path, FileHandle *h)
{
    h->fd = _open(path, _O_RDWR | _O_CREAT, _S_IWRITE | _S_IREAD);
    if (h->fd == -1)
        return -1;
    return 0;
}

/* reads len bytes at offset; len or -1. */
s32 File_ReadAt(FileHandle *h, void *buf, s32 offset, s32 len)
{
    if (h->fd < 0 || offset < 0 || len < 0 || offset > h->size || len > h->size - offset)
        return -1;
    _lseek(h->fd, offset, SEEK_SET);
    if (_read(h->fd, buf, len) != len)
        return -1;
    h->remaining = h->size - offset - len;
    return len;
}

void File_Seek(FileHandle *h, s32 pos)
{
    if (h->fd >= 0 && pos >= 0 && pos <= h->size && _lseek(h->fd, pos, SEEK_SET) == pos)
        h->remaining = h->size - pos;
}

/* reads a u32 CRC, then len bytes that must hash (Crc32) to it; len or -1. */
s32 File_ReadChecked(FileHandle *h, void *buf, s32 len)
{
    u32 crc;
    s32 result;

    result = File_Read(h, &crc, 4);
    if (result != 4)
        return -1;
    result = File_Read(h, buf, len);
    if (result != len)
        return -1;
    if (crc != Crc32((u8 *)buf, len))
        return -1;
    return result;
}

s32 File_Read(FileHandle *h, void *buf, s32 len)
{
    if (h->fd < 0 || len < 0 || len > h->remaining || (!buf && len))
        return -1;
    if (_read(h->fd, buf, len) != len)
        return -1;
    h->remaining -= len;
    g_fileBytesRead += len;
    return len;
}

/* 0 on a full write, else -1. */
s32 File_Write(FileHandle *h, const void *buf, s32 len)
{
    if (_write(h->fd, buf, len) != len)
        return -1;
    return 0;
}

s32 File_Close(FileHandle *h)
{
    if (h->fd >= 0)
        _close(h->fd);
    h->fd = -1;
    return 0;
}

/* the whole file in a malloc'd block (no CRC), or 0. */
void *File_LoadWhole(const char *name)
{
    FileHandle file;
    s32 ret;
    void *data;

    Debug_Printf("***********LOADFILE CALLED (no chksum)*********\n");
    if (File_Open(name, &file) <= 0)
        goto error;
    data = malloc(file.size);
    File_Read(&file, data, file.size);
    File_Close(&file);
    return data;
error:
    Debug_Printf("Error in _LoadFile\n");
    return 0;
}

/* writes len bytes to path; 0, or -1 (0xff) on failure. */
u8 File_SaveWhole(const char *path, const void *buf, s32 len)
{
    FileHandle file;
    s32 status;

    if (File_Create(path, &file))
        return -1;
    if (File_Write(&file, buf, len))
        return -1;
    File_Close(&file);
    return 0;
}
