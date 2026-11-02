#ifndef SDW_ENGINE_WEATHER_H
#define SDW_ENGINE_WEATHER_H

/* The functions and globals weather.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class Camera;

void Weather_InitRain(Vec3s *camPos);
void Weather_InitSnow(Vec3s *camPos);
void Weather_LevelInit();
void Weather_RenderRain(void *layer, Camera *cam);
void Weather_RenderSnow(void *layer, Camera *cam);
void Weather_UpdateRain(Vec3s *camPos);            /* empty */
void Weather_UpdateSnow(Vec3s *camPos);            /* empty */

#endif
