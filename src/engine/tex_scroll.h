#ifndef SDW_ENGINE_TEX_SCROLL_H
#define SDW_ENGINE_TEX_SCROLL_H

/* The functions and globals tex_scroll.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct DavBitmapRec;
struct TexScroll;
class Texture;

/* a scrolling rect of a texture page (the level's scrolls, the rolling carpet's belt): TexScroll_Setup copies the rect
 * into the backup it returns, TexScroll_Step moves the rect's texels by the scroll's step */
Texture *TexScroll_Setup(TexScroll *scroll, const DavBitmapRec *rec, s8 step);
void TexScroll_Step(TexScroll *scroll, Texture *backup);
void TexScroll_AddRect(const DavBitmapRec *rec, s8 step);
void TexScroll_FreeAll();
void TexScroll_Init();
void TexScroll_UpdateAll();

#endif
