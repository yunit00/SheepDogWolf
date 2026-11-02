#ifndef SDW_ENGINE_REGISTRY_H
#define SDW_ENGINE_REGISTRY_H

/* The functions and globals registry.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

u8 Reg_DeleteAppKeys();
u8 Reg_HasBinaryValue(const char *subkey, const char *valueName);
/* the own settings: string values of the app key (src/platform/save_store.h) */
u8 Reg_ReadAppString(const char *name, char *out, u32 outSize);
u8 Reg_WriteAppString(const char *name, const char *value);

#endif
