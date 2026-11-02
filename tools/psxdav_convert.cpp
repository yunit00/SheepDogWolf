/* psxdav_convert: writes the VDX7 conversion of a PlayStation .DAV, the one the game makes when it loads the level
 * (src/engine/psx_dav.cpp), for tools/retexture.py. Built with the game (CMakeLists.txt).
 *
 * Usage: psxdav_convert <PlayStation .DAV> <output .DAV>
 * Exit status: 0 written, 1 not a PlayStation .DAV or not converted, 2 a file error. */
#include <cstdarg>
#include <cstdio>
#include <vector>

#include "../src/engine/psx_dav.h"
#include "../src/platform/platform.h"

void Platform_Log(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <PlayStation .DAV> <output .DAV>\n", argv[0]);
        return 2;
    }
    FILE *in = fopen(argv[1], "rb");
    if (!in) {
        fprintf(stderr, "%s: cannot open\n", argv[1]);
        return 2;
    }
    std::vector<char> data;
    char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        data.insert(data.end(), buf, buf + n);
    fclose(in);
    if (!PsxDav_Is(data.data(), (u32)data.size())) {
        fprintf(stderr, "%s: not a PlayStation .DAV\n", argv[1]);
        return 1;
    }
    u32 size;
    char *vdx7 = PsxDav_ToVdx7(data.data(), (u32)data.size(), &size);
    if (!vdx7)
        return 1;
    FILE *out = fopen(argv[2], "wb");
    if (!out || fwrite(vdx7, 1, size, out) != size) {
        fprintf(stderr, "%s: cannot write\n", argv[2]);
        delete[] vdx7;
        return 2;
    }
    fclose(out);
    delete[] vdx7;
    return 0;
}
