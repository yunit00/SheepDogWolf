#include "sdw_classes.h"

u16 g_texCount;
Mesh *g_texObjects[512];
uptr g_texKeys[512];

/* the cached mesh built for a model resource, or 0. */
void *Texture_FindByResource(Model *model)
{
    u32 i = 0;
    do {
        if (g_texKeys[i] == (uptr)model)
            return g_texObjects[i];
        i++;
    } while (i < g_texCount);
    return 0;
}
