/* Only what src/ uses is here: constants, types and functions (depends on sdw_types.h). */
#ifndef SDW_SDK_CRT_H
#define SDW_SDK_CRT_H

#include "sdw_types.h"
#include <stddef.h>

#define NULL 0 /* C++: stddef.h / stdio.h / windef.h */
#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0
#define _O_BINARY 0x8000
#define _O_CREAT 0x0100
#define _O_RDWR 0x0002
#define _P_NOWAIT 1
#define _P_WAIT 0
#define _S_IREAD 0000400
#define _S_IWRITE 0000200


struct FILE;

#include <stdarg.h> /* the compiler's: the stand-in below is only right for 32-bit cdecl (x64 passes 8-byte slots) */

/* ---- CRT ---- */
struct div_t {
    int quot;
    int rem;
};

/* ---- libjpeg 6 (a library: declared as far as these callers use it) ---- */
typedef int jmp_buf[16];

extern "C" int _close(int fd);
extern "C" int _isnan(double);
extern "C" long _lseek(int fd, long offset, int origin);
extern "C" size_t __cdecl _msize(void *);
extern "C" int _open(const char *path, int oflag, ...);
extern "C" int _read(int fd, void *buf, unsigned int n);
extern "C" int __cdecl _setjmp(jmp_buf env);
extern "C" int _spawnv(int mode, const char *path, const char *const *argv);
extern "C" int _write(int fd, const void *buf, unsigned int n);
extern "C" int abs(int v);                                                   /* CRT, */
extern "C" double __cdecl atan(double x);
extern "C" double atan2(double, double);
extern "C" int atexit(void (*fn)(void));
extern "C" double __cdecl cos(double);               /* CRT */
extern "C" div_t div(int num, int denom);
extern "C" __declspec(noreturn) void exit(int code);
extern "C" double __cdecl exp(double x);
extern "C" double fabs(double);
extern "C" int fclose(FILE *f);
extern "C" int fcloseall();
extern "C" double floor(double x); /* CRT */
extern "C" double fmod(double, double);
extern "C" FILE *fopen(const char *name, const char *mode);
#ifndef _WIN32
/* on a case-sensitive file system the game's names may not match its data's case: its fopen looks the name up
 * (src/platform/crt_posix_compat.cpp) */
extern "C" FILE *Sdw_fopen(const char *name, const char *mode);
#define fopen Sdw_fopen
#endif
extern "C" size_t fread(void *buf, size_t size, size_t count, FILE *f);
extern "C" void free(void *p);
extern "C" int fseek(FILE *f, long offset, int origin);
extern "C" long ftell(FILE *f);
extern "C" size_t fwrite(const void *buf, size_t size, size_t count, FILE *f);
extern "C" char *itoa(int value, char *out, int radix);
extern "C" __declspec(noreturn) void __cdecl longjmp(jmp_buf env, int value);
extern "C" void *malloc(size_t size);
extern "C" int memcmp(const void *a, const void *b, size_t n);
extern "C" void *memcpy(void *d, const void *s, size_t n);     /* CRT, */
extern "C" void *memset(void *p, int c, size_t n);
extern "C" double __cdecl pow(double, double);
extern "C" int printf(const char *fmt, ...);
extern "C" void qsort(void *base, size_t n, size_t size,
                      int (*compare)(const void *, const void *)); /* CRT, */
extern "C" int rand(void);                                         /* CRT, */
/* rand() in the range of the game's CRT (MSVC: 0..0x7fff), which the game scales by 32767: other C libraries return up
 * to 0x7fffffff */
static inline int Crt_Rand15()
{
    return rand() & 0x7fff;
}
extern "C" double __cdecl sin(double);                             /* CRT */
extern "C" int sprintf(char *buf, const char *fmt, ...);
extern "C" int snprintf(char *buf, size_t size, const char *fmt, ...);
extern "C" double sqrt(double);                                    /* CRT */
extern "C" void srand(unsigned int seed);                          /* CRT, */
extern "C" char *strcat(char *dst, const char *src);
extern "C" char *strchr(const char *s, int c);                     /* CRT */
extern "C" int strcmp(const char *a, const char *b);
extern "C" char *strcpy(char *d, const char *s);                   /* CRT, */
extern "C" size_t strlen(const char *s);
extern "C" int strncmp(const char *a, const char *b, size_t n);
extern "C" char *strncpy(char *dst, const char *src, size_t n);
extern "C" char *strrchr(const char *, int);
extern "C" int swprintf(unsigned short *, const unsigned short *, ...);
extern "C" double __cdecl tan(double x);
extern "C" long time(long *t);           /* CRT, */

#endif
