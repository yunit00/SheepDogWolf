#ifndef SDW_ENGINE_INPUT_H
#define SDW_ENGINE_INPUT_H

/* The functions and globals input.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct ControlConfig;
class Pad;
struct PadFrame;

void Input_ApplyControlConfig(ControlConfig *cfg);
void Input_ApplyRemap(u16 slotC, u16 slotD, u16 slotE, u16 slotF, u16 slotB, u16 slotA);
u16 Input_GetMapping(u8 slot);
void Input_Init();
void Input_Poll();
void Input_SetMode(u8 mode);
void Input_SwapMapping(u8 slot, u16 value);
void Input_Unacquire();
void PadFrame_Clear(PadFrame *frame);
s32 Pad_AnalogToStick(u8, u8, s32 *, s32 *);
void Pad_ClearFrames(Pad *pad);
void Pad_DetectType(Pad *pad);
void Pad_InitPair(Pad *p1, Pad *p2);
bool Pad_MenuHeld(s32 activeLowMask);
bool Pad_MenuPressed(s32 activeLowMask);
bool Pad_MenuRepeat(s32 activeLowMask);
void Pad_ReadRaw(Pad *pad);
void Pad_StickToDeadzonedAxes(u8 rawX, u8 rawY, int *outX, int *outY);

/* The functions and globals input.cpp defines, declared once for every file that uses them. */

class Pad;

extern u16 g_inputMap[16];
extern u8 g_inputMapAux[16];        /* entries 8..15: the glyph of each remappable action's button */
extern Pad g_pad;
extern Pad g_pad2;
extern u8 g_rumbleSeqFallingRock[];
extern u8 g_rumbleSeqImpact[];      /* rumble motor pattern of the run crash */
extern u8 g_rumbleSeqKill[];        /* {0xc8,0x40,0x80,0x20,0xff}: the rumble of a fatal fall / zap */

#endif
