#ifndef SDW_OBJECTS_LIGHTSPOT_H
#define SDW_OBJECTS_LIGHTSPOT_H

/* The functions and globals lightspot.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"


class RenderPoly;
class ScnObject;

extern RenderPoly g_lightSpotBeamFace;           /* the one triangle every beam face is pushed through */
extern u16 g_lightSpotSamBoxCount;
extern Box **g_lightSpotSamBoxes;                /* id list 0x46: where Sam can be told about the robot */
extern SdwVertexBuffer *g_lightSpotVbXf;  /* D3DFVF_XYZRHW: ProcessVertices' destination */
extern SdwVertexBuffer *g_lightSpotXyzVb; /* D3DFVF_XYZ, 41 vertices: the cone in object space */
ScnObject *LightSpot_Create(void *record);

#endif
