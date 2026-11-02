/*
 * Reg_OpenProgressKey and Card_SaveExists return a full int, and Card_*'s first parameter is the PS1 card file name the
 * callers push (g_mcardFileName) and these PC versions ignore.
 */
#include "sdw_types.h"
#include "sdw_enums.h"

#include "../sdk/windef.h"
#include "../sdk/crt.h"

/* ---- the game's own functions ---- */
LONG Reg_CreateSubKey(HKEY *out, const char *name);
void Reg_CloseKey(HKEY *key);
#include "registry.h"
DWORD Reg_ReadBinary(HKEY key, const char *name, void *buf, DWORD bufSize);
u8 Reg_WriteBinary(HKEY key, const char *name, const void *data, DWORD size);

/* ---- the registry "memory card" ---- */
HKEY g_regProgressKey; /* the open Progress subkey (no table name yet) */

/* opens (creates) the Progress subkey; 1 on success. */
s32 Reg_OpenProgressKey()
{
    if (Reg_CreateSubKey(&g_regProgressKey, "Progress") == 0)
        return 1;
    return 0;
}

void Reg_CloseProgressKey()
{
    Reg_CloseKey(&g_regProgressKey);
}

/* 1 when Progress\SdwSaves holds a non-empty binary value. */
s32 Card_SaveExists(
    const char *
        fileName) /* the file name is passed and ignored: Card_StateMachine pushes it */
{
    if (Reg_HasBinaryValue("Progress", "SdwSaves") == 1)
        return 1;
    return 0;
}

/* the PS1 card status query's answer on PC. */
s32 CardStub_Status()
{
    return CARD_PRESENT;
}

/* free blocks on a PS1 card. */
s32 CardStub_FreeBlocks()
{
    return 0xf;
}

s32 CardStub_Status2()
{
    return CARD_FORMAT_OK;
}

s32 CardStub_Status3(
    const char *fileName) /* the file name is passed and ignored: Card_CommitBlock pushes it */
{
    return CARD_PREWRITE_OK;
}

/* reads nBlocks 8 KB card blocks of SdwSaves into dest; CARD_READ_OK when the whole size was read, else
 * CARD_READ_SHORT and dest untouched. fileName (the PS1 card file) is not used. No callers. */
s32 Card_ReadWholeBlocks(const char *fileName, u32 nBlocks, void *dest)
{
    s32 result = CARD_READ_SHORT;
    u32 size = nBlocks << 13;
    u8 *buf = new u8[size];
    if (Reg_ReadBinary(g_regProgressKey, "SdwSaves", buf, size) == size) {
        memcpy(dest, buf, size);
        result = CARD_READ_OK;
    }
    delete buf;
    return result;
}

/* as Card_ReadWholeBlocks, for nBytes rounded up to 128-byte PS1 card frames. */
s32 Card_ReadBlocks(const char *fileName, u32 nBytes, void *dest)
{
    s32 result = CARD_READ_SHORT;
    u32 size = ((nBytes + 0x7f) >> 7) << 7;
    u8 *buf = new u8[size];
    if (Reg_ReadBinary(g_regProgressKey, "SdwSaves", buf, size) == size) {
        memcpy(dest, buf, size);
        result = CARD_READ_OK;
    }
    delete buf;
    return result;
}

/* writes nBlocks 8 KB blocks from src to SdwSaves: CARD_WRITE_OK or CARD_WRITE_FAILED. */
s32 Card_WriteBlocks(const char *fileName, u32 nBlocks, void *src)
{
    u32 size = nBlocks << 13;
    if (Reg_WriteBinary(g_regProgressKey, "SdwSaves", src, size) == 1)
        return CARD_WRITE_OK;
    return CARD_WRITE_FAILED;
}

/* as Card_WriteBlocks, for nBytes rounded up to 128-byte frames. */
s32 Card_WriteFrames(const char *fileName, u32 nBytes, void *src)
{
    u32 size = ((nBytes + 0x7f) >> 7) << 7;
    if (Reg_WriteBinary(g_regProgressKey, "SdwSaves", src, size) == 1)
        return CARD_WRITE_OK;
    return CARD_WRITE_FAILED;
}
