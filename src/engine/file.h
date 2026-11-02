#ifndef SDW_ENGINE_FILE_H
#define SDW_ENGINE_FILE_H

/* The functions and globals file.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct FileHandle;

s32 File_Close(FileHandle *h);
void *File_LoadWhole(const char *name);
s32 File_Open(const char *name, FileHandle *fh);
s32 File_Read(FileHandle *h, void *buf, s32 len);
s32 File_ReadChecked(FileHandle *fh, void *buf, s32 len);
u8 File_SaveWhole(const char *path, const void *buf, s32 len);
void File_Seek(FileHandle *h, s32 pos);

#endif
