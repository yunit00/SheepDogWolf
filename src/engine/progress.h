#ifndef SDW_ENGINE_PROGRESS_H
#define SDW_ENGINE_PROGRESS_H

/* The functions and globals progress.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class Progress;

extern Progress *g_pProgress;
extern Progress g_progress;
void Progress_BuildScenePath(char *dest, s8 scene);
void Progress_ResetGlobal();

#endif
