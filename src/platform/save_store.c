/* The settings and saves as files (save_store.h). Standard C, plus mkdir. */
#include "save_store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define SAVE_MKDIR(path) _mkdir(path)
#define SAVE_SEP '\\'
#else
#define SAVE_MKDIR(path) mkdir(path, 0755)
#define SAVE_SEP '/'
#endif

#define SAVE_APP_PATH "Infogrames" "\0" "Sheep, Dog'n Wolf DX7" "\0"
#define SAVE_PATH_MAX 1024

typedef struct SaveValue {
    char *name;
    unsigned type, size;
    unsigned char *data;
} SaveValue;

struct SaveKey {
    char path[SAVE_PATH_MAX]; /* <dir>/<key>.dat */
};

/* ---- the folder ---- */
static char s_dir[SAVE_PATH_MAX];

static int Save_Append(char *out, const char *part)
{
    size_t len = strlen(out);
    if (len + strlen(part) + 2 > SAVE_PATH_MAX)
        return 0;
    if (len && out[len - 1] != '/' && out[len - 1] != '\\')
        out[len++] = SAVE_SEP;
    strcpy(out + len, part);
    return 1;
}

const char *Save_Dir(void)
{
    const char *base, *part;
    if (s_dir[0])
        return s_dir;
    if ((base = getenv("SDW_SAVE_DIR")) && *base) {
        strncpy(s_dir, base, SAVE_PATH_MAX - 1);
        return s_dir;
    }
#if defined(_WIN32)
    base = getenv("APPDATA");
    if (base && *base)
        strncpy(s_dir, base, SAVE_PATH_MAX - 1);
#elif defined(__APPLE__)
    base = getenv("HOME");
    if (base && *base) {
        strncpy(s_dir, base, SAVE_PATH_MAX - 1);
        Save_Append(s_dir, "Library/Application Support");
    }
#else
    base = getenv("XDG_DATA_HOME");
    if (base && *base)
        strncpy(s_dir, base, SAVE_PATH_MAX - 1);
    else if ((base = getenv("HOME")) && *base) {
        strncpy(s_dir, base, SAVE_PATH_MAX - 1);
        Save_Append(s_dir, ".local/share");
    }
#endif
    if (!s_dir[0])
        strcpy(s_dir, "."); /* no home: beside the game */
    for (part = SAVE_APP_PATH; *part; part += strlen(part) + 1)
        Save_Append(s_dir, part);
    return s_dir;
}

/* creates the folder and its parents */
static void Save_MakeDir(void)
{
    char path[SAVE_PATH_MAX];
    char *p;
    strcpy(path, Save_Dir());
    for (p = path + 1; *p; p++) {
        if ((*p == '/' || *p == '\\') && p[-1] != ':') {
            char c = *p;
            *p = 0;
            SAVE_MKDIR(path);
            *p = c;
        }
    }
    SAVE_MKDIR(path);
}

static int Save_KeyPath(const char *name, char *out)
{
    strcpy(out, Save_Dir());
    if (!name || !*name)
        name = "_app";
    return Save_Append(out, name) && strlen(out) + 5 < SAVE_PATH_MAX && strcat(out, ".dat");
}

static int Save_FileExists(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

/* ---- the file ---- */
static void Save_PutU32(FILE *f, unsigned v)
{
    unsigned char b[4];
    b[0] = (unsigned char)v;
    b[1] = (unsigned char)(v >> 8);
    b[2] = (unsigned char)(v >> 16);
    b[3] = (unsigned char)(v >> 24);
    fwrite(b, 1, 4, f);
}

static int Save_GetU32(FILE *f, unsigned *v)
{
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4)
        return 0;
    *v = b[0] | (unsigned)b[1] << 8 | (unsigned)b[2] << 16 | (unsigned)b[3] << 24;
    return 1;
}

static void Save_FreeValues(SaveValue *values, unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        free(values[i].name);
        free(values[i].data);
    }
    free(values);
}

/* the key's values (none when the file is missing or damaged); a <key>.tmp left by an interrupted write is used when
 * the file itself is gone */
static SaveValue *Save_Load(const char *path, unsigned *count)
{
    char tmp[SAVE_PATH_MAX + 4];
    unsigned char magic[4];
    unsigned version, n, i;
    SaveValue *values = 0;
    FILE *f = fopen(path, "rb");
    *count = 0;
    if (!f) {
        strcpy(tmp, path);
        strcpy(tmp + strlen(tmp) - 4, ".tmp");
        if (!(f = fopen(tmp, "rb")))
            return 0;
    }
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "SDWK", 4) || !Save_GetU32(f, &version) || version != 1 ||
        !Save_GetU32(f, &n) || n > 100000 || !(values = (SaveValue *)calloc(n ? n : 1, sizeof(SaveValue)))) {
        fclose(f);
        return 0;
    }
    for (i = 0; i < n; i++) {
        unsigned nameLen;
        SaveValue *v = &values[i];
        if (!Save_GetU32(f, &nameLen) || nameLen > 4096 || !(v->name = (char *)malloc(nameLen + 1)) ||
            fread(v->name, 1, nameLen, f) != nameLen || !Save_GetU32(f, &v->type) || !Save_GetU32(f, &v->size) ||
            v->size > 0x4000000 || !(v->data = (unsigned char *)malloc(v->size ? v->size : 1)) ||
            fread(v->data, 1, v->size, f) != v->size)
            break;
        v->name[nameLen] = 0;
    }
    fclose(f);
    if (i != n) { /* damaged: keep what was read whole */
        free(values[i].name);
        free(values[i].data);
        values[i].name = 0;
        values[i].data = 0;
    }
    *count = i;
    return values;
}

static int Save_Store(const char *path, const SaveValue *values, unsigned count)
{
    char tmp[SAVE_PATH_MAX + 4];
    unsigned i;
    int ok;
    FILE *f;
    strcpy(tmp, path);
    strcpy(tmp + strlen(tmp) - 4, ".tmp");
    Save_MakeDir();
    if (!(f = fopen(tmp, "wb")))
        return SAVE_ACCESS_DENIED;
    fwrite("SDWK", 1, 4, f);
    Save_PutU32(f, 1);
    Save_PutU32(f, count);
    for (i = 0; i < count; i++) {
        unsigned nameLen = (unsigned)strlen(values[i].name);
        Save_PutU32(f, nameLen);
        fwrite(values[i].name, 1, nameLen, f);
        Save_PutU32(f, values[i].type);
        Save_PutU32(f, values[i].size);
        fwrite(values[i].data, 1, values[i].size, f);
    }
    ok = !ferror(f);
    ok &= fclose(f) == 0;
    if (!ok) {
        remove(tmp);
        return SAVE_ACCESS_DENIED;
    }
    remove(path); /* Windows' rename does not replace */
    return rename(tmp, path) == 0 ? SAVE_OK : SAVE_ACCESS_DENIED;
}

/* value names compare as the registry's do: without case */
static int Save_SameName(const char *a, const char *b)
{
    for (; *a && *b; a++, b++) {
        char x = *a >= 'A' && *a <= 'Z' ? (char)(*a + 32) : *a;
        char y = *b >= 'A' && *b <= 'Z' ? (char)(*b + 32) : *b;
        if (x != y)
            return 0;
    }
    return *a == *b;
}

/* ---- the API ---- */
int Save_KeyExists(const char *name)
{
    char path[SAVE_PATH_MAX];
    return Save_KeyPath(name, path) && Save_FileExists(path);
}

SaveKey *Save_OpenKey(const char *name)
{
    SaveKey *key = (SaveKey *)calloc(1, sizeof(SaveKey));
    if (!key || !Save_KeyPath(name, key->path)) {
        free(key);
        return 0;
    }
    if (!Save_FileExists(key->path)) {
        unsigned count;
        SaveValue *values = Save_Load(key->path, &count); /* a .tmp, if any */
        int result = Save_Store(key->path, values, count);
        Save_FreeValues(values, count);
        if (result != SAVE_OK) {
            free(key);
            return 0;
        }
    }
    return key;
}

void Save_CloseKey(SaveKey *key)
{
    free(key);
}

int Save_QueryValue(SaveKey *key, const char *value, unsigned *type, void *data, unsigned *size)
{
    unsigned count, i;
    int result = SAVE_NOT_FOUND;
    SaveValue *values = Save_Load(key->path, &count);
    for (i = 0; i < count; i++) {
        if (!Save_SameName(values[i].name, value))
            continue;
        if (type)
            *type = values[i].type;
        if (data && *size < values[i].size)
            result = SAVE_MORE_DATA;
        else {
            if (data)
                memcpy(data, values[i].data, values[i].size);
            result = SAVE_OK;
        }
        *size = values[i].size;
        break;
    }
    Save_FreeValues(values, count);
    return result;
}

int Save_SetValue(SaveKey *key, const char *value, unsigned type, const void *data, unsigned size)
{
    unsigned count, i;
    int result;
    SaveValue *grown, *v = 0;
    SaveValue *values = Save_Load(key->path, &count);
    for (i = 0; i < count && !v; i++)
        if (Save_SameName(values[i].name, value))
            v = &values[i];
    if (!v) {
        if (!(grown = (SaveValue *)realloc(values, (count + 1) * sizeof(SaveValue)))) {
            Save_FreeValues(values, count);
            return SAVE_NO_MEMORY;
        }
        values = grown;
        v = &values[count++];
        memset(v, 0, sizeof(*v));
        if (!(v->name = (char *)malloc(strlen(value) + 1))) {
            Save_FreeValues(values, count);
            return SAVE_NO_MEMORY;
        }
        strcpy(v->name, value);
    }
    free(v->data);
    if (!(v->data = (unsigned char *)malloc(size ? size : 1))) {
        Save_FreeValues(values, count);
        return SAVE_NO_MEMORY;
    }
    memcpy(v->data, data, size);
    v->type = type;
    v->size = size;
    result = Save_Store(key->path, values, count);
    Save_FreeValues(values, count);
    return result;
}

int Save_DeleteKey(const char *name)
{
    char path[SAVE_PATH_MAX];
    if (!Save_KeyPath(name, path) || !Save_FileExists(path))
        return SAVE_NOT_FOUND;
    return remove(path) == 0 ? SAVE_OK : SAVE_ACCESS_DENIED;
}
