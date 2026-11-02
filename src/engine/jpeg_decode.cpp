/* The JPEG decoders of the bonus gallery's .SDW packs (PackJpeg, src/objects/pack_jpeg.cpp), over the bundled IJG
 * libjpeg 6 (src/jpeg/).
 *
 * In their own file so that they can use libjpeg's own header: jpeglib.h needs the system's <stdio.h>, which clashes
 * with the game's C runtime declarations (src/sdk/crt.h). They used to declare libjpeg's structures by hand with their
 * 32-bit sizes and offsets; on 64-bit those structures are larger, so libjpeg wrote past the game's copies on the stack
 * and opening a bonus picture crashed. */
#include <setjmp.h>
#include <stdio.h>

extern "C" {
#include "../jpeg/jpeglib.h"
}

#include "sdw_types.h"
#include "jpeg_mlt.h"

/* the IJG example's error manager: libjpeg's, plus where error_exit jumps back to */
struct JpegErrorMgr {
    jpeg_error_mgr pub;
    jmp_buf setjmpBuffer;
};

static void Jpeg_ErrorExit(j_common_ptr cinfo)
{
    longjmp(((JpegErrorMgr *)cinfo->err)->setjmpBuffer, 1);
}

/* decodes a JPEG from file (the FILE * of the game's fopen) into dest as RGB555 (r bits 10-14, g 5-9, b 0-4), last
 * row first. dest holds width x height pixels. 1 on success. */
u8 Jpeg_DecodeToRgb555Flipped(void *file, u16 *dest, u32 width, u32 height)
{
    jpeg_decompress_struct cinfo = {};
    JpegErrorMgr jerr;
    if (!file)
        return 0;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = Jpeg_ErrorExit;
    if (setjmp(jerr.setjmpBuffer)) {
        jpeg_destroy_decompress(&cinfo);
        return 0;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, (FILE *)file);
    jpeg_read_header(&cinfo, TRUE);
    cinfo.out_color_space = JCS_RGB; /* three bytes a pixel below, whatever the file holds */
    if (cinfo.image_width != width || cinfo.image_height != height || !dest) {
        jpeg_destroy_decompress(&cinfo); return 0;
    }
    jpeg_start_decompress(&cinfo);
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE,
                                                  cinfo.output_width * cinfo.output_components, 1);
    u16 *row = dest + (cinfo.output_height - 1) * cinfo.output_width;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        const JSAMPLE *p = buffer[0];
        for (JDIMENSION x = 0; x < cinfo.output_width; x++, p += 3)
            row[x] = (u16)((p[0] >> 3) << 10 | (p[1] >> 3) << 5 | (p[2] >> 3));
        row -= cinfo.output_width;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return 1;
}

/* the size of the JPEG at file's position, from its header. 1 on success. */
u8 Jpeg_GetDimensions(void *file, u32 *widthOut, u32 *heightOut)
{
    jpeg_decompress_struct cinfo = {};
    JpegErrorMgr jerr;
    if (!file)
        return 0;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = Jpeg_ErrorExit;
    if (setjmp(jerr.setjmpBuffer)) {
        jpeg_destroy_decompress(&cinfo);
        return 0;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, (FILE *)file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_calc_output_dimensions(&cinfo); /* the decoded size, without decoding every row as the original did */
    *widthOut = cinfo.output_width;
    *heightOut = cinfo.output_height;
    jpeg_destroy_decompress(&cinfo);
    return 1;
}
