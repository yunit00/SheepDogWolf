#ifndef SDW_APP_APP_MAIN_H
#define SDW_APP_APP_MAIN_H

/* The functions and globals app_main.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class Camera;
struct CollTri;
class D3DApp;
class ScnObject;
class WorldObj;

extern void *g_animNameTable; /* WAR type 0x86 */
extern Camera g_camera;
extern u16 g_cineObjectCount; /* class-2 resources (cinematic objects) */
extern ScnObject *
    *g_cineObjects;        /* the cinematic-only objects (WAR kind 0x0A), indexed by selector-3/4 tracks */
extern u16 *g_pWarCollMap; /* WAR type 0x80: header, then one CollCell per cell */
extern CollTri *g_pWarCollTris;  /* WAR type 0x81 */
extern ScnObject **g_scnActive;
extern u16 g_scnActiveBaseCount;
extern u16 g_scnActiveCapacity;  /* scenaric object count + 10 */
extern u16 g_scnActiveHigh;      /* high-water mark of g_scnActive */
extern u16 g_scnObjectCount;     /* class-1 resources (scenaric objects) */
extern ScnObject **g_scnObjects;
extern u16 g_worldObjCount;      /* class-0 resources (meshes) */
extern WorldObj **g_worldObjs;
u16 App_GetLanguageMask();
s32 App_InitGameSystems();
void App_Shutdown();
void Main_Loop(D3DApp *app);
int Game_Main(int argc, char **argv); /* the entry point, from main (src/platform/platform_sdl.cpp) */
void App_GetExeDir(char *out);

/* The functions and globals app_main.cpp defines, declared once for every file that uses them. */

struct Dav;

extern Dav g_levelDav; /* the level's Dav */

#endif
