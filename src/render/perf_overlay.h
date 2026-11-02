/* The performance overlay: frame timings in the top-left corner of the game, toggled with F3.
 *
 * A frame runs:  [game work] -> Present (flush the draws, wait for a swapchain image, submit) -> [frame limiter wait]
 * and each part is timed separately, so that the overlay shows where the time goes, not only the frame rate:
 *
 *   FPS      frames per second, and the average and worst frame time
 *   GAME     the game's own CPU work: from the end of the limiter's wait to Present (update, collision, building draws)
 *   PRESENT  the renderer's Present: uploading and recording the draws, then waiting for the GPU / vsync to hand over
 *            a swapchain image; high here (with GAME low) means the GPU or vsync is the limit
 *   LIMITER  the frame limiter's busy-wait (D3DApp::Frame_LimitFps) up to the game's frame rate cap
 *   DRAWS    draw calls and vertices the renderer submitted per frame
 *
 * Values are averaged over half a second (the worst frame over the same window), so the text is readable.
 * The renderer draws the text (src/render/sdl3/sdl3_render_device.cpp, DrawPerfOverlay) with the 5x7 font below. */
#ifndef SDW_PERF_OVERLAY_H
#define SDW_PERF_OVERLAY_H

#include "sdw_types.h"

void PerfOverlay_Toggle(); /* F3 (src/platform/platform_sdl.cpp) */
void CamDebug_Toggle();     /* F5 (src/platform/platform_sdl.cpp) */
int PerfOverlay_Enabled();

/* the timing points: FrameStart when the limiter's wait ends (the game's work begins); PresentBegin / PresentEnd
 * around the renderer's Present, with the draw calls and vertices that frame submitted */
void PerfOverlay_FrameStart();
void PerfOverlay_PresentBegin(u32 draws, u32 vertices);
void PerfOverlay_PresentEnd();

/* the overlay's text, one line per row; the number of lines (0 before the first half second is measured) */
#define PERF_OVERLAY_MAX_LINES 5
#define PERF_OVERLAY_LINE_SIZE 48
int PerfOverlay_Lines(char lines[PERF_OVERLAY_MAX_LINES][PERF_OVERLAY_LINE_SIZE]);

/* the font: PERF_FONT_GLYPHS glyphs of 5 x 7 pixels; each row a byte, bit 4 the leftmost pixel */
#define PERF_FONT_WIDTH 5
#define PERF_FONT_HEIGHT 7
int PerfOverlay_GlyphCount();
const u8 *PerfOverlay_GlyphRows(int index);
int PerfOverlay_GlyphIndex(char c); /* -1 for a space or a character the font lacks (lower case uses upper case) */

#endif
