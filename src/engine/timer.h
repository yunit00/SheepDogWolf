/*
 * Timer's methods. Include it BEFORE sdw_classes.h (the member lists must be defined when the classes are). Two
 * instances exist: g_pTimer (heap, made by Time_Init) drives the gameplay clock, g_limiterTimer (* static) the frame
 * limiter.
 */
#ifndef SDW_TIMER_H
#define SDW_TIMER_H

#include "sdw_enums.h" /* enum TimerUnit, the unit the Get / Peek / Convert methods take */

#define SDW_MEMBERS_Timer Timer(); /* Timer_Construct: calibrates g_cpuHz once */

#include "timer_api.h"

#endif
