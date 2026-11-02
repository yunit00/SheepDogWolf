#ifndef SDW_ENGINE_INTERFACE_H
#define SDW_ENGINE_INTERFACE_H

/* The functions and globals interface.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class ScnObject;
struct UiFrame;

extern s16 g_cannonOriginX;
extern s16 g_cannonOriginY;
extern s16 g_cannonTileH;
extern s16 g_cannonTileW;
extern s16 g_letterboxTimer;                            /* ms, 0..1500 */
extern char *g_strCancel;
extern char *g_strUiString1C;
extern char *g_strUiString1E;
extern char *g_strUiString21;
extern char *g_strValidate;
extern s16 g_telescopeOriginX;
extern u16 g_telescopeOriginY;
extern s16 g_telescopeTileH;
extern s16 g_telescopeTileW;
extern char *g_uiFooterText;
extern ScnObject *g_voiceOwner_2;
extern u32 g_voicePending;
extern u32 g_voicePlaying;
u8 Dialogue_Show(void *text, s32 arg);
void Dialogue_StopVoice();
void Fade_DrawOverlay(bool white, u8 level, s16 *rect);
void Hud_DrawCannonMask();
void Hud_DrawTelescopeMask(bool);
void Interface_Init();
void Letterbox_Update();
void Menu_BuildConfirmMenu();
void **Res_FindBitmapGroup(void *bmpRecord, u16 *outCount);
void StringBank_RandomiseGlyphs();
char *Text_GetUiString(u8 index);
void Ui_BuildFrameQuads(UiFrame *out, s16 *rect, u16 frameResId, u16 inset);
void Ui_DrawFlatRect(u32 *, s32, s32, s32, s32, u32);
void Ui_DrawFrameQuads(UiFrame *frame, s32 unusedArg);
void Ui_DrawGouraudRect(u32 *layer, s32 x0, s32 y0, s32 x1, s32 y1, u32 c0, u32 c1, u32 c2, u32 c3);
void Ui_DrawPanelFill(u32 colorRGB, s16 *rect);
void Ui_DrawRectOutline(s16 *rect, u32 colorRGB);
void Ui_DrawSubtitleBox(const char *text, const s16 *rect, u16 frameStyle);

/* The functions and globals interface.cpp defines, declared once for every file that uses them. */

class AnimSprite;
struct DialogueShownFlags;
class Sprite;
class UiQuad;

extern AnimSprite g_animSpriteCrayon1;
extern UiQuad g_cannonMaskQuads[4];
extern DialogueShownFlags g_dialogueShownFlags; /* bit 0: Dialogue_Show printed the text this frame */
extern char g_menuFooterText[];
extern Sprite g_spriteCrayon2;
extern UiQuad g_telescopeMaskQuads[8];

#endif
