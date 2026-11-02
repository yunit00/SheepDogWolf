/* The settings and saves: what the original keeps in the Windows registry (src/engine/registry.cpp), kept
 * as files in the user's data folder instead, so that it works wherever there is a file system.
 *
 * The folder: $SDW_SAVE_DIR when set, else Windows  %APPDATA%\Infogrames\Sheep, Dog'n Wolf DX7 macOS
 * ~/Library/Application Support/Infogrames/Sheep, Dog'n Wolf DX7 others   $XDG_DATA_HOME (or
 * ~/.local/share)/Infogrames/Sheep, Dog'n Wolf DX7
 *
 * The file (<key>.dat, "_app.dat" for the app key): "SDWK", then little-endian u32s: version (1), value count; then per
 * value: name length, name bytes, type, data size, data bytes. Written to <key>.tmp, then moved over the old file. */
#ifndef SDW_SAVE_STORE_H
#define SDW_SAVE_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SaveKey SaveKey;

/* the registry's result codes, which the callers compare against */
#define SAVE_OK 0
#define SAVE_NOT_FOUND 2
#define SAVE_ACCESS_DENIED 5
#define SAVE_NO_MEMORY 8
#define SAVE_MORE_DATA 234

const char *Save_Dir(void);
int Save_KeyExists(const char *name); /* name "": the app key */
/* opens the key, creating its file (and the folder) when missing, as RegCreateKeyEx; 0 when it cannot */
SaveKey *Save_OpenKey(const char *name);
void Save_CloseKey(SaveKey *key);
int Save_QueryValue(SaveKey *key, const char *value, unsigned *type, void *data, unsigned *size);
/* RegSetValueEx: replaces or adds the value and writes the file */
int Save_SetValue(SaveKey *key, const char *value, unsigned type, const void *data, unsigned size);
int Save_DeleteKey(const char *name); /* removes the key's file */

#ifdef __cplusplus
}
#endif

#endif
