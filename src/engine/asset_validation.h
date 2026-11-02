/* Bounds for the PC resource directories. Nested mesh/scenario formats have
 * additional invariants; this is deliberately not a claim to validate them all. */
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace AssetValidation {
inline uint16_t U16(const unsigned char *p) { return p[0] | uint16_t(p[1]) << 8; }
inline uint32_t U32(const unsigned char *p) { return U16(p) | uint32_t(U16(p + 2)) << 16; }
inline bool Range(size_t size, size_t at, size_t count, size_t stride = 1) {
    return at <= size && stride && count <= (size - at) / stride;
}
inline bool War(const unsigned char *b, size_t size) {
    if (!b || size < 16 || b[4] != 'V' || b[5] != '2' || b[6] != '.' || b[7] != '6') return false;
    uint32_t count = U32(b + 12);
    if (count > 65535 || !Range(size, 16, count, 4)) return false;
    size_t end = 16 + size_t(count) * 4;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t record = U32(b + 16 + i * 4);
        size_t at = record & 0xffffff;
        if (at < end || !Range(size, at, 4)) return false;
    }
    return true;
}
inline bool Dav(const unsigned char *b, size_t size) {
    if (!b || size < 24 || U32(b) != 0x37584456) return false;
    uint32_t dir = U32(b + 20);
    if (!Range(size, dir, 22)) return false;
    const unsigned char *d = b + dir;
    uint32_t meta = U32(d + 14);
    if (meta > size || !Range(meta, dir, 22) ||
        !Range(meta, U32(d + 6), U16(d), 2) ||
        !Range(meta, U32(d + 10), U16(d + 2), 10)) return false;
    size_t at = U32(d + 18);
    if (!Range(meta, at, 4)) return false;
    uint32_t lists = U32(b + at); at += 4;
    if (lists > (meta - at) / 4) return false;
    for (uint32_t i = 0; i < lists; ++i) {
        if (!Range(meta, at, 4)) return false;
        uint32_t n = U32(b + at) >> 16; at += 4;
        if (!Range(meta, at, n, 4)) return false;
        for (uint32_t j = 0; j < n; ++j)
            if (!Range(meta, U32(b + at + j * 4), 2)) return false;
        at += n * 4;
    }
    return true;
}
}
