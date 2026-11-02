#ifndef SDW_ENGINE_TEX_TABLE_H
#define SDW_ENGINE_TEX_TABLE_H

/* The functions and globals tex_table.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class Mesh;
struct Model;

extern u16 g_texCount;
extern uptr g_texKeys[];                /* its keys: each mesh's resource address in the WAR blob */
extern Mesh *g_texObjects[];           /* the mesh cache */
void *Texture_FindByResource(Model *);

#endif
