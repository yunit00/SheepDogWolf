// The basic types.
#ifndef SDW_TYPES_H
#define SDW_TYPES_H
#include <stddef.h>
/* The Win32 stand-in headers keep the SDK's declaration spelling; the annotations mean nothing on 64-bit. */
#ifndef _MSC_VER
#define __declspec(attribute)
#define __stdcall
#define __cdecl
#define __forceinline inline
#endif
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;
typedef void (*fnptr)(void);
/* Pointer-sized integers, for numbers that travel in a pointer (message arguments) and pointers that travel in an
 * integer. uptr is size_t so that allocation functions can take it. */
typedef s64 sptr;
typedef size_t uptr;
#ifdef __cplusplus
static_assert(sizeof(void *) == 8, "a 64-bit target is required");
#endif
/* DirectDraw's DDSURFACEDESC2 (0x88 bytes: it holds a pointer), kept as raw bytes by D3DDeviceInfo.modes and
 * Texture.desc */
typedef struct SdwDdsd2Bytes { u8 bytes[0x88]; } SdwDdsd2Bytes;
typedef struct Vec3s { s16 x, y, z; } Vec3s;                 // vertical axis points DOWN
typedef struct Box { u32 flags; s16 min[3]; s16 max[3];          // zone / trigger / model box as stored in WAR files
#if defined(__cplusplus) && defined(SDW_MEMBERS_Box)
    SDW_MEMBERS_Box                                                 // member functions only
#endif
} Box;
/* the size of a buffer holding a path to the game's data. The original only built short relative paths (".\\Levels\\..."),
 * in buffers of 64 to 260 bytes; a game folder chosen in the launcher (--game-dir) is a full path, so every buffer a
 * data path goes through has this size. */
#define SDW_PATH_MAX 1024

#endif
