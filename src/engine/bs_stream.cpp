/*
 * The BsStream part of the byte-stream code; BsFile is src/engine/bs_file.cpp.
 *
 * BsStream 0x14 { vtbl; u8 ok; u8 *data; u32 size; u32 cursor; }: a byte stream over a whole file image. Its only
 * virtual function is the destructor.
 */
#include "sdw_types.h"

#define SDW_MEMBERS_BsStream BsStream(const char *path); /* BsStream_Open */
#include "sdw_classes.h"

#include "bs_io.h"

/* ------------------------------------------------------------------------------------------------ BsStream */

/* loads the whole file; ok = 1 when it has at least one byte. */
BsStream::BsStream(const char *path)
{
    s32 len;

    cursor = 0;
    data = (u8 *)Bs_LoadFile(path, (u32 *)&len);
    if (len <= 0) {
        ok = 0;
    } else {
        ok = 1;
        size = len;
    }
}

BsStream::~BsStream()
{
    if (data)
        delete data;
}

/* bounds-checked absolute seek. */
bool BsStream::Seek(u32 pos)
{
    bool moved;

    if (pos >= 0 && pos < size) {
        cursor = pos;
        moved = 1;
    } else {
        moved = 0;
    }
    return moved;
}

/* bounds-checked relative seek. */
bool BsStream::Skip(int delta)
{
    bool moved;

    if (cursor + delta >= 0 && cursor + delta < size) {
        cursor += delta;
        moved = 1;
    } else {
        moved = 0;
    }
    return moved;
}

u8 BsStream::ReadU8(u8 advance)
{
    u8 v;

    v = data[cursor];
    if (advance == 1)
        cursor += 1;
    return v;
}

/* little-endian. */
u16 BsStream::ReadU16(u8 advance)
{
    u16 v;

    v = data[cursor];
    v += (u16)(data[cursor + 1] << 8);
    if (advance == 1)
        cursor += 2;
    return v;
}

/* little-endian. */
u32 BsStream::ReadU32(u8 advance)
{
    u32 v;

    v = data[cursor];
    v += data[cursor + 1] << 8;
    v += data[cursor + 2] << 16;
    v += data[cursor + 3] << 24;
    if (advance == 1)
        cursor += 4;
    return v;
}

/* BsStream_ScalarDeletingDtor: generated from the virtual destructor. */
