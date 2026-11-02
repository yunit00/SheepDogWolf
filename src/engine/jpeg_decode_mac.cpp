/* macOS uses the maintained system image decoder, not the bundled IJG 6. */
#include <ImageIO/ImageIO.h>
#include <CoreGraphics/CoreGraphics.h>
#include <stdio.h>
#include <stdlib.h>
#include "sdw_types.h"

static CGImageSourceRef Source(void *handle) {
    FILE *f = (FILE *)handle;
    if (!f) return nullptr;
    long start = ftell(f);
    if (start < 0 || fseek(f, 0, SEEK_END)) return nullptr;
    long end = ftell(f);
    if (fseek(f, start, SEEK_SET) || end <= start || end - start > 64 * 1024 * 1024) return nullptr;
    size_t n = (size_t)(end - start);
    unsigned char *bytes = (unsigned char *)malloc(n);
    if (!bytes) return nullptr;
    if (fread(bytes, 1, n, f) != n) { free(bytes); return nullptr; }
    CFDataRef data = CFDataCreate(kCFAllocatorDefault, bytes, n);
    free(bytes);
    if (!data) return nullptr;
    CGImageSourceRef src = CGImageSourceCreateWithData(data, nullptr);
    CFRelease(data);
    return src;
}
u8 Jpeg_GetDimensions(void *file, u32 *width, u32 *height) {
    CGImageSourceRef src = Source(file);
    if (!src || !width || !height) { if (src) CFRelease(src); return 0; }
    CFDictionaryRef props = CGImageSourceCopyPropertiesAtIndex(src, 0, nullptr);
    int w = 0, h = 0;
    if (props) {
        CFNumberRef wn = (CFNumberRef)CFDictionaryGetValue(props, kCGImagePropertyPixelWidth);
        CFNumberRef hn = (CFNumberRef)CFDictionaryGetValue(props, kCGImagePropertyPixelHeight);
        if (wn && hn) { CFNumberGetValue(wn, kCFNumberIntType, &w); CFNumberGetValue(hn, kCFNumberIntType, &h); }
        CFRelease(props);
    }
    CFRelease(src);
    if (w <= 0 || h <= 0 || w > 512 || h > 512) return 0;
    *width = w; *height = h; return 1;
}
u8 Jpeg_DecodeToRgb555Flipped(void *file, u16 *dest, u32 width, u32 height) {
    if (!dest || !width || !height || width > 512 || height > 512) return 0;
    CGImageSourceRef src = Source(file);
    if (!src) return 0;
    CGImageRef image = CGImageSourceCreateImageAtIndex(src, 0, nullptr);
    CFRelease(src);
    if (!image) return 0;
    if (CGImageGetWidth(image) != width || CGImageGetHeight(image) != height) { CGImageRelease(image); return 0; }
    unsigned char *rgba = (unsigned char *)calloc((size_t)width * height, 4);
    CGColorSpaceRef color = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = rgba ? CGBitmapContextCreate(rgba, width, height, 8, width * 4, color,
                        kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big) : nullptr;
    CGColorSpaceRelease(color);
    if (!ctx) { free(rgba); CGImageRelease(image); return 0; }
    CGContextDrawImage(ctx, CGRectMake(0, 0, width, height), image);
    for (u32 y = 0; y < height; ++y)
        for (u32 x = 0; x < width; ++x) {
            const unsigned char *p = rgba + ((size_t)y * width + x) * 4;
            dest[(size_t)(height - 1 - y) * width + x] = (p[0] >> 3) << 10 | (p[1] >> 3) << 5 | (p[2] >> 3);
        }
    CGContextRelease(ctx); CGImageRelease(image); free(rgba); return 1;
}
