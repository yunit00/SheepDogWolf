#include "../src/engine/jpeg_mlt.h"
#include "../src/engine/asset_validation.h"
#include <cassert>
#include <cstdio>
#include <vector>

int main(int argc, char **argv) {
    int checked = 0;
    for (int a = 1; a < argc; ++a) {
        FILE *f = fopen(argv[a], "rb");
        assert(f);
        unsigned char header[12];
        assert(fread(header, 1, 12, f) == 12);
        unsigned count = AssetValidation::U32(header + 8);
        assert(count && count < 4096);
        std::vector<unsigned char> directory(count * 24);
        assert(fread(directory.data(), 1, directory.size(), f) == directory.size());
        for (unsigned i = 0; i < count; ++i) {
            unsigned offset = AssetValidation::U32(directory.data() + i * 24);
            assert(!fseek(f, offset, SEEK_SET));
            u32 w = 0, h = 0;
            assert(Jpeg_GetDimensions(f, &w, &h));
            assert(w && h && w <= 512 && h <= 512 && !(w & 1) && !(h & 1));
            std::vector<u16> pixels(w * h + 2, 0x7bad);
            assert(!fseek(f, offset, SEEK_SET));
            assert(Jpeg_DecodeToRgb555Flipped(f, pixels.data() + 1, w, h));
            assert(pixels.front() == 0x7bad && pixels.back() == 0x7bad);
            assert(!fseek(f, offset, SEEK_SET));
            assert(!Jpeg_DecodeToRgb555Flipped(f, pixels.data() + 1, w + 1, h));
            ++checked;
        }
        fclose(f);
    }
    FILE *bad = tmpfile();
    assert(bad);
    fputs("not an image", bad); rewind(bad);
    u32 w = 0, h = 0;
    assert(!Jpeg_GetDimensions(bad, &w, &h));
    fclose(bad);
    printf("Decoded %d gallery JPEGs; guard pixels, dimension mismatch and malformed input passed\n", checked);
}
