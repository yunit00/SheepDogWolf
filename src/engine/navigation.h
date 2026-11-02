#ifndef SDW_NAVIGATION_H
#define SDW_NAVIGATION_H
#include "sdw_classes.h"

/* The navigation functions of src/engine/navigation.cpp (Sam's node graph and its A* search), declared for their */
/* SamEdgeNormal comes from sdw_classes.h. */
#include "navigation_api.h"
void NavPath_BeginSearch(void *search, s16 goalX, s16 goalZ, NavNode *start);
#endif
