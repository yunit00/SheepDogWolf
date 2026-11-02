/* The C runtime names the game code calls with Visual C++'s spelling, backed by POSIX (macOS, Linux); and the lookup
 * that lets the game open its data on a case-sensitive file system. */
#include <dirent.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cstdio>

/* The game's path, as this system can open it: '\\' becomes '/', and on a case-sensitive file system each component
 * that does not exist as spelled is looked up ignoring case (the disc has "Lvl-01.war" where the code opens ".WAR").
 * Components that are not found at all are kept as spelled, so a file about to be created keeps its name. */
extern "C" void Sdw_ResolvePath(const char *path, char *out, size_t size)
{
    char native[4096];
    size_t i, len = 0;
    const char *p;
    int lost = 0;
    for (i = 0; path[i] && i + 1 < sizeof(native); i++)
        native[i] = path[i] == '\\' ? '/' : path[i];
    native[i] = 0;
    if (access(native, F_OK) == 0 || size == 0) {
        snprintf(out, size, "%s", native);
        return;
    }
    out[0] = 0;
    p = native;
    if (*p == '/') {
        snprintf(out, size, "/");
        len = 1;
        p++;
    }
    while (*p) {
        const char *end = strchr(p, '/');
        size_t n = end ? (size_t)(end - p) : strlen(p);
        char component[1024];
        if (n == 0 || n >= sizeof(component)) {
            p = end ? end + 1 : p + n;
            continue;
        }
        memcpy(component, p, n);
        component[n] = 0;
        if (!lost && strcmp(component, ".") != 0 && strcmp(component, "..") != 0) {
            char exact[4096];
            snprintf(exact, sizeof(exact), "%s%s%s", out, len && out[len - 1] != '/' ? "/" : "", component);
            if (access(exact, F_OK) != 0) {
                DIR *dir = opendir(len ? out : ".");
                struct dirent *entry;
                int found = 0;
                while (dir && (entry = readdir(dir)) != 0) {
                    if (strcasecmp(entry->d_name, component) == 0) {
                        snprintf(component, sizeof(component), "%s", entry->d_name);
                        found = 1;
                        break;
                    }
                }
                if (dir)
                    closedir(dir);
                lost = !found;
            }
        }
        len += snprintf(out + len, len < size ? size - len : 0, "%s%s", len && out[len - 1] != '/' ? "/" : "",
                        component);
        if (len >= size) {
            out[size - 1] = 0;
            return;
        }
        p = end ? end + 1 : p + n;
    }
}

/* fopen through Sdw_ResolvePath: src/sdk/crt.h makes the game's fopen this one */
extern "C" FILE *Sdw_fopen(const char *path, const char *mode)
{
    char native[4096];
    Sdw_ResolvePath(path, native, sizeof(native));
    return fopen(native, mode);
}

extern "C" int _open(const char *path, int flags, ...)
{
    char nativePath[4096];
    int native = 0;
    if (flags & 0x0002)
        native |= O_RDWR;
    else
        native |= O_RDONLY;
    if (flags & 0x0100)
        native |= O_CREAT;
    if (flags & 0x0200)
        native |= O_TRUNC;
    if (flags & 0x0400)
        native |= O_EXCL;
    Sdw_ResolvePath(path, nativePath, sizeof(nativePath));
    if (native & O_CREAT) {
        va_list args;
        va_start(args, flags);
        int mode = va_arg(args, int);
        va_end(args);
        return open(nativePath, native, mode);
    }
    return open(nativePath, native);
}

extern "C" int _close(int fd) { return close(fd); }
extern "C" int _read(int fd, void *buffer, unsigned int size) { return (int)read(fd, buffer, size); }
extern "C" int _write(int fd, const void *buffer, unsigned int size) { return (int)write(fd, buffer, size); }
extern "C" long _lseek(int fd, long offset, int origin) { return (long)lseek(fd, offset, origin); }
extern "C" int _isnan(double value) { return std::isnan(value) ? 1 : 0; }
#ifdef __APPLE__
extern "C" size_t _msize(void *ptr) { return malloc_size(ptr); }
#else
extern "C" size_t _msize(void *ptr) { return malloc_usable_size(ptr); }
#endif
extern "C" int _itoa(int value, char *out, int radix)
{
    char digits[sizeof(unsigned int) * 8 + 1];
    unsigned int magnitude;
    int n = 0, pos = 0;
    if (radix < 2 || radix > 36)
        return 0;
    if (value < 0 && radix == 10) {
        out[pos++] = '-';
        magnitude = (unsigned int)(-(long long)value);
    } else {
        magnitude = (unsigned int)value;
    }
    do {
        unsigned int digit = magnitude % (unsigned int)radix;
        digits[n++] = (char)(digit < 10 ? '0' + digit : 'a' + digit - 10);
        magnitude /= (unsigned int)radix;
    } while (magnitude);
    while (n)
        out[pos++] = digits[--n];
    out[pos] = 0;
    return pos;
}
extern "C" int fcloseall(void) { return 0; }
