#include "navigation.h"
#include "sdw_enums.h"
/* Its opaque search storage is viewed through the generated NavSearch type. */
#include "../sdk/crt.h"
extern "C" {
NavNode g_samNavNodes[120]; /* bucket 581 */
u16 g_samNavNodeCountBuild; /* bucket 951  the count while BuildGraph runs */
u16 g_navGraphNodeCount;    /* bucket 1010 (g_samNavNodeCount) the finished graph's count */
int Coll_BoxGroundQuery(CollBox *, int *, ScnObject *, u8, ScnObject **);
}

static char s_navTrajectoryNotFound[] = "Trajectory not found\n";
static char s_navDumpSon[] = "  Son %d : %d\n";
static char s_navDumpInfluence[] = " Influence (%d,%d) (%d,%d)\n";
static char s_navDumpNode[] = "*Node %d (%d, %d)\n";
void __cdecl Debug_Printf(const char *, ...);
#include "id_list.h"

void NavPath_OpenListInsert(NavSearch *s, NavNode *node)
{
    struct {
        NavNode *tail, *current;
    } w;
    w.tail = 0;
    for (w.current = s->head; w.current; w.current = w.current->next) {
        if (node->f <= w.current->f) {
            node->prev = w.current->prev;
            node->next = w.current;
            if (w.current->prev)
                w.current->prev->next = node;
            else
                s->head = node;
            w.current->prev = node;
            return;
        }
        if (!w.current->next)
            w.tail = w.current;
    }
    if (!s->head) {
        s->head = node;
        node->prev = 0;
        node->next = 0;
    } else {
        w.tail->next = node;
        node->next = 0;
        node->prev = w.tail;
    }
}

NavNode *NavPath_ListPopHead(NavSearch *s)
{
    NavNode *node;
    if (s->head) {
        node = s->head;
        s->head = node->next;
        node->listState = NAV_LIST_NONE;
        if (s->head)
            s->head->prev = 0;
        return node;
    }
    return 0;
}

NavNode *NavPath_ListPopFirstWithState(NavSearch *s, u8 state)
{
    NavNode *node = s->head;
    while (node) {
        if (node->listState == state) {
            if (node->prev)
                node->prev->next = node->next;
            else
                s->head = node->next;
            if (node->next)
                node->next->prev = node->prev;
            node->listState = NAV_LIST_NONE;
            return node;
        }
        node = node->next;
    }
    return 0;
}

void NavPath_ListRemove(NavSearch *s, NavNode *node)
{
    if (node->prev)
        node->prev->next = node->next;
    else
        s->head = node->next;
    if (node->next)
        node->next->prev = node->prev;
    node->listState = NAV_LIST_NONE;
}

void SamNav_DebugDumpGraph()
{
    struct {
        NavNode *node;
        int edge, index;
    } w;
    for (w.index = 0; w.index < g_samNavNodeCountBuild; w.index++) {
        w.node = &g_samNavNodes[w.index];
        printf(s_navDumpNode, w.node - g_samNavNodes, w.node->x, w.node->z);
        printf(s_navDumpInfluence, w.node->bbox[0], w.node->bbox[1], w.node->bbox[2], w.node->bbox[3]);
        for (w.edge = 0; w.edge < w.node->nbSons; w.edge++)
            printf(s_navDumpSon, w.edge, w.node->sons[w.edge] - g_samNavNodes);
    }
}

void SamNav_ResetGraph()
{
    g_samNavNodeCountBuild = 0;
    g_navGraphNodeCount = 0;
}

/* Inline lookup shape preserves the two separate ushort scan slots in BuildGraph. */
/* It emits no additional out-of-line function. Height is deliberately not a key. */
static __forceinline void NavGraph_FindXZ(s16 x, s16 z, s16 &result)
{
    u16 i;
    for (i = 0; i < g_samNavNodeCountBuild; i++)
        if (g_samNavNodes[i].x == x && g_samNavNodes[i].z == z) {
            result = i;
            return;
        }
    result = -1;
}

void SamNav_BuildGraph(u16 listId)
{
    NavNode *neighbour_1;
    s16 *normalTo_37;
    int dz;
    int dx_9;
    s16 *normalFrom_9;
    int length_28;
    int height_8;
    CollBox probe_1;
    Trajectory *trajectory_1;
    s16 z_4;
    s16 x_4;
    uptr *list_19;
    s16 index_8;
    u16 edge_19;
    u16 route_5;
    u16 point_8;
    u16 count_28;
    NavNode *previous_10;
    NavNode *current_14;
    g_samNavNodeCountBuild = 0;
    g_navGraphNodeCount = 0;
    list_19 = Scn_FindIdList(listId, &count_28);
    if (!list_19) {
        Debug_Printf(s_navTrajectoryNotFound);
        return;
    }
    for (route_5 = 0; route_5 < count_28; route_5++) {
        trajectory_1 = (Trajectory *)list_19[route_5];
        if (trajectory_1->count < 2)
            continue;
        x_4 = trajectory_1->pts[0].x;
        z_4 = trajectory_1->pts[0].z;
        NavGraph_FindXZ(x_4, z_4, index_8);
        if (index_8 < 0) {
            index_8 = g_samNavNodeCountBuild;
            g_samNavNodeCountBuild++;
            probe_1.flags = 0;
            probe_1.min.x = x_4 - 20;
            probe_1.max.x = x_4 + 20;
            probe_1.min.z = z_4 - 20;
            probe_1.max.z = z_4 + 20;
            probe_1.min.y = trajectory_1->pts[0].y - 300;
            probe_1.max.y = trajectory_1->pts[0].y + 3000;
            /* Original first-point failure fallback differs from later points. */
            if (!Coll_BoxGroundQuery(&probe_1, &height_8, 0, CQ_STATIC, 0))
                height_8 = 32000;
            g_samNavNodes[index_8].nbSons = 0;
            g_samNavNodes[index_8].x = x_4;
            g_samNavNodes[index_8].z = z_4;
            g_samNavNodes[index_8].listState = NAV_LIST_NONE;
            g_samNavNodes[index_8].groundY = (s16)height_8;
        }
        previous_10 = &g_samNavNodes[index_8];
        for (point_8 = 1; point_8 < trajectory_1->count; point_8++) {
            x_4 = trajectory_1->pts[point_8].x;
            z_4 = trajectory_1->pts[point_8].z;
            NavGraph_FindXZ(x_4, z_4, index_8);
            if (index_8 < 0) {
                index_8 = g_samNavNodeCountBuild;
                g_samNavNodeCountBuild++;
                probe_1.flags = 0;
                probe_1.min.x = x_4 - 20;
                probe_1.max.x = x_4 + 20;
                probe_1.min.z = z_4 - 20;
                probe_1.max.z = z_4 + 20;
                probe_1.min.y = trajectory_1->pts[point_8].y - 300;
                probe_1.max.y = trajectory_1->pts[point_8].y + 3000;
                if (!Coll_BoxGroundQuery(&probe_1, &height_8, 0, CQ_STATIC, 0))
                    height_8 = trajectory_1->pts[point_8].y;
                g_samNavNodes[index_8].nbSons = 0;
                g_samNavNodes[index_8].x = x_4;
                g_samNavNodes[index_8].z = z_4;
                g_samNavNodes[index_8].listState = NAV_LIST_NONE;
                g_samNavNodes[index_8].groundY = (s16)height_8;
            }
            current_14 = &g_samNavNodes[index_8];
            for (edge_19 = 0; edge_19 < previous_10->nbSons; edge_19++)
                if (previous_10->sons[edge_19] == current_14)
                    break;
            if (edge_19 == previous_10->nbSons) {
                normalFrom_9 = previous_10->edgeNormal[previous_10->nbSons];
                normalTo_37 = current_14->edgeNormal[current_14->nbSons];
                dx_9 = current_14->x - previous_10->x;
                dz = current_14->z - previous_10->z;
                length_28 = (int)sqrt((double)dx_9 * dx_9 + dz * dz);
                length_28++;
                dx_9 = (dx_9 * 4096) / length_28;
                dz = (dz * 4096) / length_28;
                normalFrom_9[0] = (s16)dz;
                normalTo_37[0] = (s16)dz;
                normalFrom_9[1] = (s16)-dx_9;
                normalTo_37[1] = normalFrom_9[1];
                previous_10->sons[previous_10->nbSons] = current_14;
                previous_10->nbSons++;
                current_14->sons[current_14->nbSons] = previous_10;
                current_14->nbSons++;
            }
            previous_10 = current_14;
        }
    }
    for (index_8 = 0; index_8 < (s16)g_samNavNodeCountBuild; index_8++) {
        current_14 = &g_samNavNodes[index_8];
        current_14->bbox[2] = current_14->x;
        current_14->bbox[0] = current_14->bbox[2];
        current_14->bbox[3] = current_14->z;
        current_14->bbox[1] = current_14->bbox[3];
        for (edge_19 = 0; edge_19 < current_14->nbSons; edge_19++) {
            neighbour_1 = current_14->sons[edge_19];
            if (neighbour_1->x < current_14->bbox[0])
                current_14->bbox[0] = neighbour_1->x;
            if (neighbour_1->z < current_14->bbox[1])
                current_14->bbox[1] = neighbour_1->z;
            if (neighbour_1->x > current_14->bbox[2])
                current_14->bbox[2] = neighbour_1->x;
            if (neighbour_1->z > current_14->bbox[3])
                current_14->bbox[3] = neighbour_1->z;
        }
        current_14->bbox[0] -= 300;
        current_14->bbox[1] -= 300;
        current_14->bbox[2] += 300;
        current_14->bbox[3] += 300;
    }
    g_navGraphNodeCount = g_samNavNodeCountBuild;
}

void NavPath_ClearSearch(void *s)
{
    NavNode *node;
    for (node = ((NavSearch *)s)->head; node; node = node->next)
        node->listState = NAV_LIST_NONE;
    ((NavSearch *)s)->active = 0;
    ((NavSearch *)s)->head = 0;
}

void NavPath_BeginSearch(void *s, s16 goalX, s16 goalZ, NavNode *start)
{
    struct {
        int distance, dz, dx;
    } w;
    ((NavSearch *)s)->head = 0;
    ((NavSearch *)s)->goalX = goalX;
    ((NavSearch *)s)->goalZ = goalZ;
    ((NavSearch *)s)->active = 0;
    if (!start)
        start = g_samNavNodes;
    ((NavSearch *)s)->active = 1;
    start->g = 0;
    w.dx = start->x - ((NavSearch *)s)->goalX;
    if (w.dx < 0)
        w.dx = -w.dx;
    w.dz = start->z - ((NavSearch *)s)->goalZ;
    if (w.dz < 0)
        w.distance = w.dx - w.dz;
    else
        w.distance = w.dx + w.dz;
    start->h = w.distance;
    start->f = start->g + start->h;
    start->parent = 0;
    start->listState = NAV_LIST_OPEN;
    NavPath_OpenListInsert((NavSearch *)s, start);
}

u8 NavPath_StepSearch(void *s, NavNode **outNode)
{
    struct {
        int goalDistance, goalDz, goalDx, stepDistance, stepDz, stepDx, dz, dx;
        u32 cost;
        NavNode *son;
        u32 distance;
        NavNode *node;
        u8 unused[3], edge;
    } w;
    w.node = NavPath_ListPopFirstWithState((NavSearch *)s, NAV_LIST_OPEN);
    w.dx = w.node->x - ((NavSearch *)s)->goalX;
    if (w.dx < 0)
        w.dx = -w.dx;
    w.dz = w.node->z - ((NavSearch *)s)->goalZ;
    if (w.dz < 0)
        w.distance = w.dx - w.dz;
    else
        w.distance = w.dx + w.dz;
    if (!w.node) {
        NavPath_ClearSearch(s);
        return NAV_SEARCH_FAILED;
    }
    if (w.distance < 200) {
        *outNode = w.node;
        NavPath_ClearSearch(s);
        return NAV_SEARCH_FOUND;
    }
    for (w.edge = 0; w.edge < w.node->nbSons; w.edge++) {
        w.son = w.node->sons[w.edge];
        w.stepDx = w.son->x - w.node->x;
        if (w.stepDx < 0)
            w.stepDx = -w.stepDx;
        w.stepDz = w.son->z - w.node->z;
        if (w.stepDz < 0)
            w.stepDistance = w.stepDx - w.stepDz;
        else
            w.stepDistance = w.stepDx + w.stepDz;
        w.cost = w.node->g + w.stepDistance;
        if (w.son->listState) {
            if (w.son->g <= w.cost)
                continue;
            NavPath_ListRemove((NavSearch *)s, w.son);
        }
        w.son->parent = w.node;
        w.son->g = w.cost;
        w.goalDx = w.son->x - ((NavSearch *)s)->goalX;
        if (w.goalDx < 0)
            w.goalDx = -w.goalDx;
        w.goalDz = w.son->z - ((NavSearch *)s)->goalZ;
        if (w.goalDz < 0)
            w.goalDistance = w.goalDx - w.goalDz;
        else
            w.goalDistance = w.goalDx + w.goalDz;
        w.son->h = w.goalDistance;
        w.son->f = w.son->g + w.son->h;
        w.son->listState = NAV_LIST_OPEN;
        NavPath_OpenListInsert((NavSearch *)s, w.son);
    }
    w.node->listState = NAV_LIST_CLOSED;
    NavPath_OpenListInsert((NavSearch *)s, w.node);
    return NAV_SEARCH_RUNNING;
}

NavNode *SamNav_FindNearestNode(s16 x, s16 z, int *outDistanceSquared)
{
    struct {
        NavNode *node;
        int dz;
        NavNode *best;
        int dx;
        u8 pad[2];
        u16 i;
        int d2;
    } w;
    if (!g_navGraphNodeCount)
        return 0;
    w.best = 0;
    *outDistanceSquared = 1000000;
    for (w.i = 0; w.i < g_navGraphNodeCount; w.i++) {
        w.node = &g_samNavNodes[w.i];
        w.dx = w.node->x - x;
        w.dz = w.node->z - z;
        w.d2 = w.dx * w.dx + w.dz * w.dz;
        if (w.d2 < *outDistanceSquared) {
            *outDistanceSquared = w.d2;
            w.best = w.node;
        }
    }
    return w.best;
}

NavNode *SamNav_FindNearestNodeAhead(s16 x, s16 z, Vec3s *dir, int dirLength, int anyDir)
{
    struct {
        int candidateDistance;
        NavNode *node;
        int bestDistance;
        NavNode *candidate;
        int dz;
        NavNode *best;
        int dx;
        u8 pad[2];
        u16 i;
        int d2, projection;
    } w;
    if (!g_navGraphNodeCount)
        return 0;
    w.best = 0;
    w.candidate = 0;
    w.bestDistance = 1000000;
    w.candidateDistance = 1000000;
    for (w.i = 0; w.i < g_navGraphNodeCount; w.i++) {
        w.node = &g_samNavNodes[w.i];
        w.dx = w.node->x - x;
        w.dz = w.node->z - z;
        w.d2 = w.dx * w.dx + w.dz * w.dz;
        if (dirLength != 0 && w.d2 < 160000) {
            w.projection = ((dir->x * w.dx + dir->z * w.dz) * 32) / (dirLength * (int)sqrt((double)w.d2));
            if (w.projection > 5 || anyDir) {
                w.candidateDistance = w.d2;
                w.candidate = w.node;
            }
        }
        if (w.d2 < w.bestDistance) {
            w.bestDistance = w.d2;
            w.best = w.node;
        }
    }
    /* Original behavior: last qualifying candidate, not nearest; the stored */
    /* candidateDistance is never read. Zero-distance division is also preserved. */
    if (w.candidate)
        return w.candidate;
    return w.best;
}

/* distance/height heuristic; performs no collision queries. */
NavNode *SamNav_FindNearestReachableNode(s16 fromX, s16 fromZ, s16 x, s16 y, s16 z)
{
    int sonDz_8;
    int sonDx;
    int dz;
    int dx_9;
    int acceptable_28;
    u32 distance_19;
    int fromDz_12;
    NavNode *node;
    NavNode *best_6;
    int reverseDz_3;
    NavNode *nearest_29;
    int fromDx_18;
    u16 i_15;
    u32 bestDistance_7;
    u32 sonDistance_28;
    u32 nearestDistance_37;
    u16 edge_19;
    int reverseDx_4;
    NavNode *son_14;
    fromDx_18 = x - fromX;
    fromDz_12 = z - fromZ;
    if (!g_navGraphNodeCount)
        return 0;
    best_6 = 0;
    nearest_29 = 0;
    bestDistance_7 = 1000000;
    nearestDistance_37 = 1000000;
    for (i_15 = 0; i_15 < g_navGraphNodeCount; i_15++) {
        node = &g_samNavNodes[i_15];
        dx_9 = node->x - x;
        if (dx_9 < 0)
            dx_9 = -dx_9;
        dz = node->z - z;
        if (dz < 0)
            distance_19 = dx_9 - dz;
        else
            distance_19 = dx_9 + dz;
        reverseDx_4 = x - node->x;
        reverseDz_3 = z - node->z;
        if (distance_19 < bestDistance_7) {
            acceptable_28 = 1;
            if ((node->groundY - y >= 0 ? node->groundY - y : -(node->groundY - y)) > 150) {
                acceptable_28 = 0;
                for (edge_19 = 0; edge_19 < node->nbSons; edge_19++) {
                    son_14 = node->sons[edge_19];
                    if ((son_14->groundY - y >= 0 ? son_14->groundY - y : -(son_14->groundY - y)) <= 150) {
                        sonDx = son_14->x - x;
                        if (sonDx < 0)
                            sonDx = -sonDx;
                        sonDz_8 = son_14->z - z;
                        if (sonDz_8 < 0)
                            sonDistance_28 = sonDx - sonDz_8;
                        else
                            sonDistance_28 = sonDx + sonDz_8;
                        if (sonDistance_28 < 200)
                            acceptable_28 = 1;
                    }
                }
            } else if (distance_19 > 2000)
                acceptable_28 = 0;
            if (acceptable_28) {
                bestDistance_7 = distance_19;
                best_6 = node;
            }
        }
        if (distance_19 < nearestDistance_37) {
            nearestDistance_37 = distance_19;
            nearest_29 = node;
        }
    }
    if (!best_6)
        return nearest_29;
    return best_6;
}

/* positive dot prefers the approach-side half-plane of the target. */
NavNode *SamNav_FindNodeTowards(s16 fromX, s16 fromZ, s16 x, s16 z)
{
    int dz;
    int dx_9;
    u32 distance_19;
    int fromDz_12;
    NavNode *node;
    NavNode *approachSide_26;
    int reverseDz_3;
    NavNode *nearest_29;
    int fromDx_18;
    NavNode *nearby_10;
    u16 i_15;
    u32 nearbyDistance_27;
    u32 approachDistance_3;
    int isApproachSide_2;
    u32 nearestDistance_37;
    int isNear_5;
    int reverseDx_4;
    fromDx_18 = x - fromX;
    fromDz_12 = z - fromZ;
    if (!g_navGraphNodeCount)
        return 0;
    nearby_10 = 0;
    approachSide_26 = 0;
    nearest_29 = 0;
    nearbyDistance_27 = 1000000;
    approachDistance_3 = 1000000;
    nearestDistance_37 = 1000000;
    for (i_15 = 0; i_15 < g_navGraphNodeCount; i_15++) {
        node = &g_samNavNodes[i_15];
        dx_9 = node->x - x;
        if (dx_9 < 0)
            dx_9 = -dx_9;
        dz = node->z - z;
        if (dz < 0)
            distance_19 = dx_9 - dz;
        else
            distance_19 = dx_9 + dz;
        reverseDx_4 = x - node->x;
        reverseDz_3 = z - node->z;
        isNear_5 = distance_19 < 300;
        isApproachSide_2 = fromDx_18 * reverseDx_4 + fromDz_12 * reverseDz_3 > 0 && distance_19 < 600;
        if (distance_19 < nearestDistance_37) {
            nearestDistance_37 = distance_19;
            nearest_29 = node;
        }
        if (isApproachSide_2 && distance_19 < approachDistance_3) {
            approachDistance_3 = distance_19;
            approachSide_26 = node;
        }
        if (isNear_5 && distance_19 < nearbyDistance_27) {
            nearbyDistance_27 = distance_19;
            nearby_10 = node;
        }
    }
    if (nearby_10)
        return nearby_10;
    if (approachSide_26)
        return approachSide_26;
    return nearest_29;
}

/* Helper:. */
int SamNav_IsGoalNearOrNotAhead(Vec3s *position, Vec3s *toWaypoint, int waypointDistance, Vec3s *goal)
{
    struct {
        int result, dz, dx, distanceSquared, projection;
    } w;
    w.dx = goal->x - position->x;
    w.dz = goal->z - position->z;
    w.distanceSquared = w.dx * w.dx + w.dz * w.dz;
    if (w.distanceSquared < 90000)
        return 1;
    else if (w.distanceSquared < 360000) {
        if (waypointDistance == 0)
            return 0;
        w.projection = ((toWaypoint->x * w.dx + toWaypoint->z * w.dz) * 32) /
                       (waypointDistance * (int)sqrt((double)w.distanceSquared));
        w.result = w.projection < 27;
        return w.result;
    } else
        return 0;
}

/* Helper:. */
int SamNav_DistToEdge(Vec3s *point, SamEdgeNormal *normal, short x0, short z0, short x1, short z1, Vec3s *closest)
{
    struct {
        int endResult, endDz, endDx, startResult, startDz, startDx;
        int pz, px, projection, ez, ex, lengthSquared, distance;
    } w;
    w.px = point->x - x0;
    w.pz = point->z - z0;
    w.ex = x1 - x0;
    w.ez = z1 - z0;
    w.projection = w.ex * w.px + w.ez * w.pz;
    if (w.projection <= 0) {
        closest->x = x0;
        closest->z = z0;
        w.startDx = x0 - point->x;
        if (w.startDx < 0)
            w.startDx = -w.startDx;
        w.startDz = z0 - point->z;
        if (w.startDz < 0)
            w.startResult = w.startDx - w.startDz;
        else
            w.startResult = w.startDx + w.startDz;
        return w.startResult;
    }
    w.lengthSquared = w.ex * w.ex + w.ez * w.ez;
    if (w.projection >= w.lengthSquared) {
        closest->x = x1;
        closest->z = z1;
        w.endDx = x1 - point->x;
        if (w.endDx < 0)
            w.endDx = -w.endDx;
        w.endDz = z1 - point->z;
        if (w.endDz < 0)
            w.endResult = w.endDx - w.endDz;
        else
            w.endResult = w.endDx + w.endDz;
        return w.endResult;
    }
    w.projection = (normal->x * w.px + normal->z * w.pz) >> 12;
    w.distance = w.projection >= 0 ? w.projection : -w.projection;
    if (w.distance <= 100) {
        closest->x = point->x - ((normal->x * w.projection) >> 12);
        closest->z = point->z - ((normal->z * w.projection) >> 12);
    }
    return w.distance;
}

NavNode *SamNav_FindEdgeNear(Vec3s *position, u8 *edgeOut, Vec3s *closest)
{
    struct {
        u16 edge, index;
        NavNode *node;
        int distance;
    } w;
    for (w.index = 0; w.index < g_navGraphNodeCount; w.index++) {
        w.node = &g_samNavNodes[w.index];
        if (position->x >= w.node->bbox[0] && position->x <= w.node->bbox[2] && position->z >= w.node->bbox[1] &&
            position->z <= w.node->bbox[3]) {
            for (w.edge = 0; w.edge < w.node->nbSons; w.edge++) {
                w.distance = SamNav_DistToEdge(position, (SamEdgeNormal *)w.node->edgeNormal[w.edge], w.node->x,
                                               w.node->z, w.node->sons[w.edge]->x, w.node->sons[w.edge]->z, closest);
                if (w.distance <= 100) {
                    *edgeOut = (u8)w.edge;
                    return w.node;
                }
            }
        }
    }
    return 0;
}
