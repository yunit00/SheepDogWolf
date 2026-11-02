/*
 * The game keeps the same keys and values as files in the user's data folder instead (src/platform/save_store.h),
 * so that it can run where there is no registry: the functions keep their shapes, an HKEY being a SaveKey there.
 */
#include "sdw_classes.h"
#include "../sdk/win32.h"

#include "../sdk/crt.h"
#define HKEY_LOCAL_MACHINE ((HKEY)(ULONG_PTR)(LONG)0x80000002) /* as winreg.h: sign-extended on 64-bit */
#define KEY_ALL_ACCESS 0x2001f
#define REG_BINARY 3

LONG Reg_OpenAppRoot(HKEY *key);
void Reg_CloseKey2(HKEY *key);

#include "../platform/save_store.h"
#define REG_SZ 1

static SaveKey *Reg_Store(HKEY key)
{
    return (SaveKey *)key;
}

#ifdef _WIN32
#define HKEY_CURRENT_USER ((HKEY)(ULONG_PTR)(LONG)0x80000001)
#define KEY_READ 0x20019
extern "C" __declspec(dllimport) LONG __stdcall RegEnumValueA(HKEY hKey, DWORD dwIndex, char *lpValueName,
                                                              DWORD *lpcchValueName, DWORD *lpReserved, DWORD *lpType,
                                                              BYTE *lpData, DWORD *lpcbData);

/* one registry key's values into the store's key `name` */
static void Reg_ImportValues(HKEY src, const char *name)
{
    SaveKey *dst = Save_OpenKey(name);
    char valueName[256];
    DWORD index, nameLen, type, size;
    u8 *data;
    if (!dst)
        return;
    for (index = 0;; index++) {
        nameLen = sizeof(valueName);
        size = 0;
        if (RegEnumValueA(src, index, valueName, &nameLen, 0, &type, 0, &size) != ERROR_SUCCESS)
            break;
        data = new u8[size ? size : 1];
        nameLen = sizeof(valueName);
        if (RegEnumValueA(src, index, valueName, &nameLen, 0, &type, data, &size) == ERROR_SUCCESS)
            Save_SetValue(dst, valueName, type, data, size);
        delete[] data;
    }
    Save_CloseKey(dst);
}

/* The first time the store is made on Windows, it takes over what is in the registry: the earlier key
 * (per user), else the original's, which Windows put in the user's VirtualStore. The registry is left as it is. */
static void Reg_ImportFromRegistry()
{
    static const char *const sources[] = {
        "SOFTWARE\\Infogrames\\Sheep, Dog'n Wolf DX7",
        "Software\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\WOW6432Node\\Infogrames\\Sheep, Dog'n Wolf DX7", /* 64-bit Windows */
        "Software\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\Infogrames\\Sheep, Dog'n Wolf DX7"};            /* 32-bit Windows */
    static const char *const subkeys[] = {"Progress", "Config", "Setup", "CfgGame"};
    HKEY src, sub;
    int i, k;
    for (i = 0; i < 3; i++) {
        if (RegOpenKeyExA(HKEY_CURRENT_USER, sources[i], 0, KEY_READ, &src) != ERROR_SUCCESS)
            continue;
        Reg_ImportValues(src, "");
        for (k = 0; k < 4; k++) {
            if (RegOpenKeyExA(src, subkeys[k], 0, KEY_READ, &sub) == ERROR_SUCCESS) {
                Reg_ImportValues(sub, subkeys[k]);
                RegCloseKey(sub);
            }
        }
        RegCloseKey(src);
        return;
    }
}
#endif

/* opens the app key, creating it (the first time on Windows, with what the registry holds) */
LONG Reg_OpenAppRoot(HKEY *key)
{
    int fresh = !Save_KeyExists("");
    SaveKey *root = Save_OpenKey("");
    *key = (HKEY)root;
    if (!root)
        return SAVE_ACCESS_DENIED;
#ifdef _WIN32
    if (fresh)
        Reg_ImportFromRegistry();
#endif
    return ERROR_SUCCESS;
}

void Reg_CloseKey2(HKEY *key)
{
    Save_CloseKey(Reg_Store(*key));
    *key = 0;
}

/* deletes the four subkeys, then the app key; 1 if that last delete succeeded */
u8 Reg_DeleteAppKeys()
{
    Save_DeleteKey("Progress");
    Save_DeleteKey("Config");
    Save_DeleteKey("Setup");
    Save_DeleteKey("CfgGame");
    return Save_DeleteKey("") == SAVE_OK;
}

LONG Reg_CreateSubKey(HKEY *out, const char *name)
{
    HKEY root = 0;
    LONG result = Reg_OpenAppRoot(&root);
    Reg_CloseKey2(&root);
    if (result != ERROR_SUCCESS)
        return result;
    *out = (HKEY)Save_OpenKey(name);
    return *out ? ERROR_SUCCESS : SAVE_ACCESS_DENIED;
}

void Reg_CloseKey(HKEY *key)
{
    Save_CloseKey(Reg_Store(*key));
    *key = 0;
}

/* no callers */
u8 Reg_DeleteSubKey(const char *name)
{
    return Save_DeleteKey(name) == SAVE_OK;
}

u8 Reg_SubKeyExists(const char *name)
{
    return Save_KeyExists("") && Save_KeyExists(name);
}

/* 1 if the subkey holds a non-empty binary value `valueName` (default "0") */
u8 Reg_HasBinaryValue(const char *subkey, const char *valueName)
{
    SaveKey *key;
    unsigned type = 0, size = 0;
    u8 exists = 0;
    if (!valueName)
        valueName = "0";
    if (!Reg_SubKeyExists(subkey) || !(key = Save_OpenKey(subkey)))
        return 0;
    if (Save_QueryValue(key, valueName, &type, 0, &size) == SAVE_OK && type == REG_BINARY && size != 0)
        exists = 1;
    Save_CloseKey(key);
    return exists;
}

/* the binary value `name` (default "0") into buf: its byte count, or 0 with buf zero-filled when it is
 * missing, not binary or larger than bufSize */
DWORD Reg_ReadBinary(HKEY key, const char *name, void *buf, DWORD bufSize)
{
    unsigned size = 0, type = 0;
    if (!name)
        name = "0";
    if (!key)
        return 0;
    size = bufSize;
    if (Save_QueryValue(Reg_Store(key), name, &type, buf, &size) != SAVE_OK || type != REG_BINARY) {
        memset(buf, 0, bufSize);
        size = 0;
    }
    return size;
}

/* writes data as the binary value `name` (default "0"); 1 on success */
u8 Reg_WriteBinary(HKEY key, const char *name, const void *data, DWORD size)
{
    if (!name)
        name = "0";
    return key && Save_SetValue(Reg_Store(key), name, REG_BINARY, data, size) == SAVE_OK;
}

/* This port's own settings beside the game's (the renderer the launcher picked): a string value of the app key.
 * 1 when read (out holds it, NUL-terminated) / written. */
u8 Reg_ReadAppString(const char *name, char *out, u32 outSize)
{
    HKEY key = 0;
    unsigned type = 0, size = outSize - 1;
    u8 ok = 0;
    if (Reg_OpenAppRoot(&key) == ERROR_SUCCESS &&
        Save_QueryValue(Reg_Store(key), name, &type, out, &size) == SAVE_OK && type == REG_SZ) {
        out[size] = 0;
        ok = 1;
    }
    Reg_CloseKey2(&key);
    return ok;
}

u8 Reg_WriteAppString(const char *name, const char *value)
{
    HKEY key = 0;
    u8 ok = 0;
    if (Reg_OpenAppRoot(&key) == ERROR_SUCCESS)
        ok = Save_SetValue(Reg_Store(key), name, REG_SZ, value, (unsigned)strlen(value) + 1) == SAVE_OK;
    Reg_CloseKey2(&key);
    return ok;
}
