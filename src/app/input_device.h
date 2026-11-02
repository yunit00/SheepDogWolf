/*
 * Only these three files include it.
 */
#ifndef SDW_INPUT_DEVICE_H
#define SDW_INPUT_DEVICE_H
#include "sdw_types.h"

#include "sdw_enums.h" /* before sdk/win32.h, whose IDOK would otherwise replace the enum constant of that name */
#include "../sdk/win32.h"
#include "../sdk/dinput.h"
#include "../sdk/crt.h"

/* ---- the classes ---- */
#define SDW_MEMBERS_InputDevice InputDevice(); /* InputDevice_Construct */
#define SDW_MEMBERS_Joystick Joystick(u16 buttonCount, D3DApp *app, HRESULT *result); /* Joystick_Construct */
#define SDW_MEMBERS_Keyboard Keyboard(u16 buttonCount, D3DApp *app, HRESULT *result); /* Keyboard_Construct */
#include "sdw_classes.h"

#define JOY_CAPS (*(DIDEVCAPS *)caps)
#define KB_AXISKEY(i) (axisKeys[i])   /* 0 X-, 1 X+, 2 Y-, 3 Y+, 4 Z-, 5 Z+ */

/* ---- globals ---- */
#include "joystick.h"

BOOL __stdcall DI_JoystickAxisEnumCallback(const DIDEVICEOBJECTINSTANCEA *doi, void *ref);

#endif
