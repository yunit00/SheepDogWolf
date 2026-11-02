/*
 * The level's WAR header says which weather it has (g_weatherType: 1 rain, 2 snow); the loader then calls
 * Weather_InitRain or Weather_InitSnow, and Game_Frame calls Weather_RenderRain / Weather_RenderSnow after the scene.
 * Weather_InitRain / _InitSnow: see HALF_WIDTH_AT.
 */
#include "sdw_types.h"
class Mat44;


#define SDW_MEMBERS_Vec3f \
    Vec3f() {}                               /* empty but user-declared, as in the Weather class's own file */
#define SDW_MEMBERS_Mat44 Mat44();
#define SDW_MEMBERS_RenderPoly RenderPoly(); /* RenderPoly_Ctor */


#define SDW_MEMBERS_Weather Weather();
#include "sdw_classes.h"
#define SDW_INLINE_CAMERA_VIEWDIR 1
#include "../objects/camera_inlines.h"
#undef SDW_INLINE_CAMERA_VIEWDIR
#define SDW_INLINE_PROGRESS_CURRENTLEVEL 1
#include "progress_inlines.h"
#undef SDW_INLINE_PROGRESS_CURRENTLEVEL
#include "sdw_enums.h"
#include "scenaric_props.h"

#include "draw2d.h"
#include "progress.h"
#include "../app/app_main.h"

/* half the width of the view at distance d (tan(fov/2) / aspect * d): the half-width of the particle sphere's slice */
#define HALF_WIDTH_AT(d) (((void)0, g_pViewFrustum->tanHalfFov) / g_pViewFrustum->aspect * (d))

/* the one weather object. */
Weather g_weather;

/* empty; the level loader calls it before the per-type setup. */
void Weather_LevelInit() {}

/* g_weatherType 1: 200 drops in a sphere of radius 1000, textures 0xe1 / 0xe9 (IGLGOUT1/2), falling
 * straight down (0, 1, 0) within 0.085 rad at 1400..1600 units/s. */
void Weather_InitRain(Vec3s *camPos)
{
    Vec3f dir;

    g_weather.SetVolume(HALF_WIDTH_AT(610.0f), 1000.0f, 200);
    g_weather.SetTexture(DAV_IDI_IGLGOUT1, 16.0f, 32.0f, 0xffffff);
    g_weather.SetTexture2(DAV_IDI_IGLGOUT2, 32.0f, 32.0f, 0xffffff);
    dir.x = 0.0f;
    dir.y = 1.0f;
    dir.z = 0.0f;
    g_weather.SfxParticles_InitRandom(&dir, 0.085f, 1400.0f, 1600.0f);
}

/* empty; Game_Frame's pre-update hook for rain. */
void Weather_UpdateRain(Vec3s *camPos) {}

/* Game_Frame's rain hook after the scene: move the drops, then draw them against the view direction. */
void Weather_RenderRain(void *layer, Camera *cam)
{
    Vec3s nrm;
    float dirF[3];

    g_weather.Advance(&g_camera.viewMat);
    nrm = g_camera.ViewDir();
    dirF[0] = nrm.x;
    dirF[1] = nrm.y;
    dirF[2] = nrm.z;
    g_weather.DrawRain(&g_camera.viewMat, dirF);
}

/* g_weatherType 2: 300 flakes (80 on disc level 8) in a sphere of radius 1500, texture 0x20 (IGLNEIG1) for
 * both, drifting along (50, 250, -25) within 0.175 rad at 180..250 units/s. */
void Weather_InitSnow(Vec3s *camPos)
{
    u32 count;
    Vec3f dir;

    if (g_pProgress->CurrentLevel() == SCENE_LVL_08)
        count = 80;
    else
        count = 300;
    g_weather.SetVolume(HALF_WIDTH_AT(610.0f), 1500.0f, count); /* see Weather_InitRain */
    g_weather.SetTexture(DAV_IDI_IGLNEIG1, 24.0f, 24.0f, 0xffffff);
    g_weather.SetTexture2(DAV_IDI_IGLNEIG1, 24.0f, 24.0f, 0xffffff);
    dir.x = 50.0f;
    dir.y = 250.0f;
    dir.z = -25.0f;
    g_weather.SfxParticles_InitRandom(&dir, 0.175f, 180.0f, 250.0f);
}

/* empty; Game_Frame's pre-update hook for snow. */
void Weather_UpdateSnow(Vec3s *camPos) {}

/* Game_Frame's snow hook after the scene. */
void Weather_RenderSnow(void *layer, Camera *cam)
{
    g_weather.Advance(&g_camera.viewMat);
    g_weather.DrawSnow(&g_camera.viewMat);
}
