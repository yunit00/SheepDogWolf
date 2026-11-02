/*
 * The immediate primitives fill the static TL vertex arrays, apply the render-state flags (ApplyStateFlags, the inline
 * twin of Render_SetStateFlags), call IDirect3DDevice7::DrawPrimitive directly and restore the states with the
 * out-of-line Render_ClearStateFlags.
 */
#include "sdw_types.h"
#include "sdw_enums.h"
#include "../sdk/d3d7.h"
#include "../sdk/win32.h"
class Mat44;
#include "../sdk/crt.h"
struct FlatVertex { /* the vertex of an untextured RenderPoly (FVF 0xc4) */
    float x, y, z, rhw;
    u32 diffuse, specular;
};
struct TexVertex { /* a D3DTLVERTEX (FVF 0x1c4), the vertex of a textured RenderPoly */
    float x, y, z, rhw;
    u32 diffuse, specular;
    float u, v;
};
/* the immediate-mode vertex arrays: the SDK's D3DTLVERTEX-style empty inline constructor gives the (empty)
 * vector-constructor loops */
struct ImmFlatVertex {
    float x, y, z, rhw;
    u32 diffuse, specular;
    ImmFlatVertex() {}
};
struct ImmTexVertex {
    float x, y, z, rhw;
    u32 diffuse, specular;
    float u, v;
    ImmTexVertex() {}
};
class Texture;

#define SDW_MEMBERS_InputMgr                      \
    InputMgr(); /* InputMgr_Construct */ \
    /* ~InputMgr is the virtual destructor sdw_classes.h declares */
#define SDW_MEMBERS_TextResBank TextResBank();
#define SDW_MEMBERS_Mat44 Mat44();
#define SDW_MEMBERS_RenderPoly                                              \
    RenderPoly();                            /* RenderPoly_Ctor */ \
    /* virtual ~RenderPoly() is generated */ /* RenderPoly_Dtor */
#define SDW_MEMBERS_PolyBatcher                       \
    void SubmitPoly(RenderPoly *poly); \
    void SubmitPolyTri(RenderPoly *poly);             \
    void SubmitPolyRect(RenderPoly *poly);            \
    void InvalidateTexture()                          \
    {                                                 \
        textureDirty = 1;                             \
    } /* inline */
#define SDW_MEMBERS_D3DApp                                                             \
    void Render_SetStateFlags(u32 flags);                             \
    void Render_ClearStateFlags(u32 flags);                             \
    void Render_DrawPrimitive(u32 type, u32 fvf, void *verts, u32 count);              \
    void DrawPrimitiveInline(u32 type, u32 fvf, void *verts, u32 count);               \
    void SetTextureInline(Texture *tex, volatile u32 stage);                           \
    void SetStateFlagsInline(u32 flags);                                               \
    /* the immediate primitives' helpers */                                            \
    void BindTexture(Texture *tex, s32 stage);             /* inline, defined below */ \
    void DrawTLTriangles(u32 fvf, void *verts, u32 count); /* inline, defined below */ \
    void ApplyStateFlags(u32 flags);                       /* inline, defined below */
#include "sdw_classes.h"
#define SDW_INLINE_D3DAPP_DRAWPRIMITIVEINLINE_U32_U32_VOID_U32 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_DRAWPRIMITIVEINLINE_U32_U32_VOID_U32

#define SDW_INLINE_D3DAPP_SETTEXTUREINLINE_TEXTURE_VOLATILE_U32 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_SETTEXTUREINLINE_TEXTURE_VOLATILE_U32

/* Render_SetStateFlags as the immediate primitives expand it: each RenderStateFlags bit sets its device
 * states. */
#define SDW_INLINE_D3DAPP_SETSTATEFLAGSINLINE_U32 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_SETSTATEFLAGSINLINE_U32

/* The constructed globals are defined further down, in the order of their static initialisers. ---- */
char g_musicsPath[SDW_PATH_MAX];
SoundDevice *g_pSoundSystem;
char g_dirBonusGame[SDW_PATH_MAX];    /* exeDir + ".\Bonus\" */
/* g_matUnk6d5428 (constructed, below) */
u8 g_sharedScratch[512 / 4 * sizeof(void *)]; /* shared scratch (collision, camera, shadow, text...); room for the same number of pointers on 64-bit */
/* the second 512-byte scratch: the ping-pong vertex buffer of the Poly_Clip* chain and the second merge
 * buffer of the segment tests. C name, as its users declare it. Its last 18 bytes are the game-space
 * vertices of the triangle under test, collide.cpp's g_collSegTriVerts: not a global of its own (a separate 18-byte global
 * would be 4-aligned; is only 2-aligned), see src/engine/collide.cpp. (`extern "C" { }`: the one-line form
 * `extern "C" u8 x[N];` would only declare it.) */
extern "C" {
u8 g_clipTriBuffer[0x200 / 4 * sizeof(void *)]; /* also a merge buffer of triangle pointers (Collide_SegCells_Gather) */
}
/* 3072 bytes no instruction refers to. Genuinely opaque: nothing reads or writes them, so neither their type nor their
 * name can be recovered; kept as bytes. */
u8 g_unref6d5868[0xc00];
char g_pathScene[SDW_PATH_MAX];
D3DApp *g_pD3DAppMain;
/* g_textCatalog (constructed, below) */
char g_introDir[SDW_PATH_MAX];
u32 g_maxImmediateTriangles;
char g_pathWheelDir[SDW_PATH_MAX];
/* g_inputMgr (constructed, below) */
char g_voiceDir[SDW_PATH_MAX];
char g_pathEnding[SDW_PATH_MAX];
char g_pathDemoDir[SDW_PATH_MAX];
char g_levelPathFmt[SDW_PATH_MAX]; /* exeDir + ".\Levels\Lvl-%02d\Lvl-%02d" */
char g_dirReference[SDW_PATH_MAX];
char g_pathFendDir[SDW_PATH_MAX];
HWND__ *g_hGameWindow;
Frustrum *g_pViewFrustum;
PolyBatcher *g_pPolyBin;
/* g_projMatrix (constructed, below; g_projDepthBias and
 *                                             g_projDepthScale are two of its elements) */
/* g_draw2dImmVerts, g_draw2dImmTexCorners, g_d2dScratchPoly (constructed, below) */

/* ---- the constructed globals, in the order of their static initialisers ---- */
InputMgr g_inputMgr;
TextResBank g_textCatalog;
Mat44 g_matUnk6d5428;
Mat44 g_projMatrix;                    /* the projection matrix (Screen_SetProjection) */
RenderPoly g_d2dScratchPoly;           /* the scratch poly of the queued primitives */
ImmFlatVertex g_draw2dImmVerts[3];
ImmTexVertex g_draw2dImmTexCorners[3];

/* PolyBatcher_SubmitPoly as the triangle primitives expand it: the state set / clear stay calls, both
 * draws are expanded. */
inline void PolyBatcher::SubmitPolyTri(RenderPoly *poly)
{
    TexVertex *tri;
    u32 idx6;
    TexVertex *batch;
    u32 *num;
    u32 *pflags;

    switch (poly->type) {
        case RPOLY_OPAQUE:
            if (flatBatchCount <= batchCapacity) {
                memcpy((u8 *)flatBatchVerts + flatBatchCount * 3 * 0x18, poly->verts, 0x48);
                flatBatchCount++;
            }
            if (flatBatchCount >= batchCapacity) {
                renderer->Render_SetStateFlags(stateFlags);
                renderer->DrawPrimitiveInline(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                                              flatBatchVerts, batchCapacity * 3);
                renderer->Render_ClearStateFlags(stateFlags);
                flatBatchCount = 0;
                textureDirty = 1;
            }
            break;
        case RPOLY_BLEND:
            if (computeSortZ == 1)
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];
            sortedPolys[sortedCount].Assign(poly);
            sortedList[sortedCount] = &sortedPolys[sortedCount];
            sortedCount++;
            break;
        case RPOLY_ADD:
            if (computeSortZ == 1)
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];
            sortedPolys[sortedCount].Assign(poly);
            sortedList[sortedCount] = &sortedPolys[sortedCount];
            sortedCount++;
            break;
        default:
            tri = (TexVertex *)poly->verts;
            idx6 = poly->type - RPOLY_TEXTURED_BASE & ~RPOLY_F_8000;
            if (idx6 < immediateTexCount) {
                batch = (TexVertex *)texBatchVerts[idx6];
                num = &texBatchCounts[idx6];
                pflags = &texStateFlags[idx6];
                if (*num <= batchCapacity) {
                    memcpy(&batch[*num * 3], tri, 0x60);
                    (*num)++;
                }
                if (*num >= batchCapacity) {
                    if (idx6 != lastTextureIndex || textureDirty == 1) {
                        renderer->SetTextureInline(textures[idx6], 0);
                        lastTextureIndex = idx6;
                        textureDirty = 0;
                    }
                    renderer->Render_SetStateFlags(*pflags);
                    renderer->DrawPrimitiveInline(D3DPT_TRIANGLELIST,
                                                  D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, batch,
                                                  batchCapacity * 3);
                    renderer->Render_ClearStateFlags(*pflags);
                    *num = 0;
                }
            } else {
                if (computeSortZ == 1)
                    poly->sortZ = tri[0].z + tri[1].z + tri[2].z;
                sortedPolys[sortedCount].Assign(poly);
                sortedList[sortedCount] = &sortedPolys[sortedCount];
                sortedCount++;
            }
    }
}

/* as the rectangle primitives expand it: as SubmitPolyTri, but the textured flush draws through the
 * out-of-line Render_DrawPrimitive. */
inline void PolyBatcher::SubmitPolyRect(RenderPoly *poly)
{
    TexVertex *tri;
    u32 idx6;
    TexVertex *batch;
    u32 *num;
    u32 *pflags;

    switch (poly->type) {
        case RPOLY_OPAQUE:
            if (flatBatchCount <= batchCapacity) {
                memcpy((u8 *)flatBatchVerts + flatBatchCount * 3 * 0x18, poly->verts, 0x48);
                flatBatchCount++;
            }
            if (flatBatchCount >= batchCapacity) {
                renderer->Render_SetStateFlags(stateFlags);
                renderer->DrawPrimitiveInline(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                                              flatBatchVerts, batchCapacity * 3);
                renderer->Render_ClearStateFlags(stateFlags);
                flatBatchCount = 0;
                textureDirty = 1;
            }
            break;
        case RPOLY_BLEND:
            if (computeSortZ == 1)
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];
            sortedPolys[sortedCount].Assign(poly);
            sortedList[sortedCount] = &sortedPolys[sortedCount];
            sortedCount++;
            break;
        case RPOLY_ADD:
            if (computeSortZ == 1)
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];
            sortedPolys[sortedCount].Assign(poly);
            sortedList[sortedCount] = &sortedPolys[sortedCount];
            sortedCount++;
            break;
        default:
            tri = (TexVertex *)poly->verts;
            idx6 = poly->type - RPOLY_TEXTURED_BASE & ~RPOLY_F_8000;
            if (idx6 < immediateTexCount) {
                batch = (TexVertex *)texBatchVerts[idx6];
                num = &texBatchCounts[idx6];
                pflags = &texStateFlags[idx6];
                if (*num <= batchCapacity) {
                    memcpy(&batch[*num * 3], tri, 0x60);
                    (*num)++;
                }
                if (*num >= batchCapacity) {
                    if (idx6 != lastTextureIndex || textureDirty == 1) {
                        renderer->SetTextureInline(textures[idx6], 0);
                        lastTextureIndex = idx6;
                        textureDirty = 0;
                    }
                    renderer->Render_SetStateFlags(*pflags);
                    renderer->Render_DrawPrimitive(D3DPT_TRIANGLELIST,
                                                   D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                                   batch, batchCapacity * 3);
                    renderer->Render_ClearStateFlags(*pflags);
                    *num = 0;
                }
            } else {
                if (computeSortZ == 1)
                    poly->sortZ = tri[0].z + tri[1].z + tri[2].z;
                sortedPolys[sortedCount].Assign(poly);
                sortedList[sortedCount] = &sortedPolys[sortedCount];
                sortedCount++;
            }
    }
}

#define DRAW2D_FLAT ((FlatVertex *)g_d2dScratchPoly.verts)

/* one untextured triangle; blendMode 0 = opaque (type 1), otherwise the poly type (2/3 sorted blends) with
 * the sort depth z. rhw is 1 / the near plane, specular opaque black. */
void Draw2D_FlatTri(float z, float x0, float y0, float x1, float y1, float x2, float y2, u32 color, u8 blendMode)
{
    float rhw;

    if (blendMode == 0) {
        g_d2dScratchPoly.type = RPOLY_OPAQUE;
    } else {
        g_d2dScratchPoly.type = blendMode;
        g_d2dScratchPoly.sortZ = z;
    }
    rhw = 1.0f / (g_pViewFrustum->nearZ);
    DRAW2D_FLAT[0].x = x0;
    DRAW2D_FLAT[0].y = y0;
    DRAW2D_FLAT[0].z = z;
    DRAW2D_FLAT[0].rhw = rhw;
    DRAW2D_FLAT[0].diffuse = color;
    DRAW2D_FLAT[0].specular = 0xff000000;
    DRAW2D_FLAT[1].x = x1;
    DRAW2D_FLAT[1].y = y1;
    DRAW2D_FLAT[1].z = z;
    DRAW2D_FLAT[1].rhw = rhw;
    DRAW2D_FLAT[1].diffuse = color;
    DRAW2D_FLAT[1].specular = 0xff000000;
    DRAW2D_FLAT[2].x = x2;
    DRAW2D_FLAT[2].y = y2;
    DRAW2D_FLAT[2].z = z;
    DRAW2D_FLAT[2].rhw = rhw;
    DRAW2D_FLAT[2].diffuse = color;
    DRAW2D_FLAT[2].specular = 0xff000000;
    g_pPolyBin->SubmitPolyTri(&g_d2dScratchPoly);
}

/* an untextured axis-aligned rectangle as two triangles: (x0,y0) (x1,y0) (x1,y1), then the middle vertex
 * moved to (x0,y1). */
void Draw2D_FlatRect(float z, float x0, float y0, float x1, float y1, u32 color, u8 blendMode)
{
    float rhw;

    if (blendMode == 0) {
        g_d2dScratchPoly.type = RPOLY_OPAQUE;
    } else {
        g_d2dScratchPoly.type = blendMode;
        g_d2dScratchPoly.sortZ = z;
    }
    rhw = 1.0f / (g_pViewFrustum->nearZ);
    DRAW2D_FLAT[0].x = x0;
    DRAW2D_FLAT[0].y = y0;
    DRAW2D_FLAT[0].z = z;
    DRAW2D_FLAT[0].rhw = rhw;
    DRAW2D_FLAT[0].diffuse = color;
    DRAW2D_FLAT[0].specular = 0xff000000;
    DRAW2D_FLAT[1].x = x1;
    DRAW2D_FLAT[1].y = y0;
    DRAW2D_FLAT[1].z = z;
    DRAW2D_FLAT[1].rhw = rhw;
    DRAW2D_FLAT[1].diffuse = color;
    DRAW2D_FLAT[1].specular = 0xff000000;
    DRAW2D_FLAT[2].x = x1;
    DRAW2D_FLAT[2].y = y1;
    DRAW2D_FLAT[2].z = z;
    DRAW2D_FLAT[2].rhw = rhw;
    DRAW2D_FLAT[2].diffuse = color;
    DRAW2D_FLAT[2].specular = 0xff000000;
    g_pPolyBin->SubmitPolyRect(&g_d2dScratchPoly);
    DRAW2D_FLAT[1].x = x0;
    DRAW2D_FLAT[1].y = y1;
    g_pPolyBin->SubmitPoly(&g_d2dScratchPoly);
}

/* Draw2D_FlatTri with a colour per vertex. No callers. */
void Draw2D_GouraudTri(float z, float x0, float y0, u32 c0, float x1, float y1, u32 c1, float x2, float y2, u32 c2,
                       u8 blendMode)
{
    float rhw;

    if (blendMode == 0) {
        g_d2dScratchPoly.type = RPOLY_OPAQUE;
    } else {
        g_d2dScratchPoly.type = blendMode;
        g_d2dScratchPoly.sortZ = z;
    }
    rhw = 1.0f / (g_pViewFrustum->nearZ);
    DRAW2D_FLAT[0].x = x0;
    DRAW2D_FLAT[0].y = y0;
    DRAW2D_FLAT[0].z = z;
    DRAW2D_FLAT[0].rhw = rhw;
    DRAW2D_FLAT[0].diffuse = c0;
    DRAW2D_FLAT[0].specular = 0xff000000;
    DRAW2D_FLAT[1].x = x1;
    DRAW2D_FLAT[1].y = y1;
    DRAW2D_FLAT[1].z = z;
    DRAW2D_FLAT[1].rhw = rhw;
    DRAW2D_FLAT[1].diffuse = c1;
    DRAW2D_FLAT[1].specular = 0xff000000;
    DRAW2D_FLAT[2].x = x2;
    DRAW2D_FLAT[2].y = y2;
    DRAW2D_FLAT[2].z = z;
    DRAW2D_FLAT[2].rhw = rhw;
    DRAW2D_FLAT[2].diffuse = c2;
    DRAW2D_FLAT[2].specular = 0xff000000;
    g_pPolyBin->SubmitPolyTri(&g_d2dScratchPoly);
}

/* Draw2D_FlatRect with a colour per corner: (x0,y0) cTL, (x1,y0) cTR, (x1,y1) cBR, then the middle vertex
 * moved to (x0,y1) cBL. */
void Draw2D_GouraudRect(float z, float x0, float y0, float x1, float y1, u32 cTL, u32 cBL, u32 cTR, u32 cBR,
                        u8 blendMode)
{
    float rhw;

    if (blendMode == 0) {
        g_d2dScratchPoly.type = RPOLY_OPAQUE;
    } else {
        g_d2dScratchPoly.type = blendMode;
        g_d2dScratchPoly.sortZ = z;
    }
    rhw = 1.0f / (g_pViewFrustum->nearZ);
    DRAW2D_FLAT[0].x = x0;
    DRAW2D_FLAT[0].y = y0;
    DRAW2D_FLAT[0].z = z;
    DRAW2D_FLAT[0].rhw = rhw;
    DRAW2D_FLAT[0].diffuse = cTL;
    DRAW2D_FLAT[0].specular = 0xff000000;
    DRAW2D_FLAT[1].x = x1;
    DRAW2D_FLAT[1].y = y0;
    DRAW2D_FLAT[1].z = z;
    DRAW2D_FLAT[1].rhw = rhw;
    DRAW2D_FLAT[1].diffuse = cTR;
    DRAW2D_FLAT[1].specular = 0xff000000;
    DRAW2D_FLAT[2].x = x1;
    DRAW2D_FLAT[2].y = y1;
    DRAW2D_FLAT[2].z = z;
    DRAW2D_FLAT[2].rhw = rhw;
    DRAW2D_FLAT[2].diffuse = cBR;
    DRAW2D_FLAT[2].specular = 0xff000000;
    g_pPolyBin->SubmitPolyRect(&g_d2dScratchPoly);
    DRAW2D_FLAT[1].x = x0;
    DRAW2D_FLAT[1].y = y1;
    DRAW2D_FLAT[1].diffuse = cBL;
    g_pPolyBin->SubmitPoly(&g_d2dScratchPoly);
}

/* a textured triangle: poly type = texture page + 4, sort depth z, per-vertex uv and colour. */
void Draw2D_TexTri(float z, float x0, float y0, float x1, float y1, float x2, float y2, s32 texIndex, float u0,
                   float v0, u32 c0, float u1, float v1, u32 c1, float u2, float v2, u32 c2)
{
    float rhw;
    TexVertex *v;

    g_d2dScratchPoly.type = texIndex + RPOLY_TEXTURED_BASE;
    g_d2dScratchPoly.sortZ = z;
    rhw = 1.0f / (g_pViewFrustum->nearZ);
    v = (TexVertex *)g_d2dScratchPoly.verts;
    v[0].x = x0;
    v[0].y = y0;
    v[0].z = z;
    v[0].rhw = rhw;
    v[0].diffuse = c0;
    v[0].specular = 0xff000000;
    v[1].x = x1;
    v[1].y = y1;
    v[1].z = z;
    v[1].rhw = rhw;
    v[1].diffuse = c1;
    v[1].specular = 0xff000000;
    v[2].x = x2;
    v[2].y = y2;
    v[2].z = z;
    v[2].rhw = rhw;
    v[2].diffuse = c2;
    v[2].specular = 0xff000000;
    v[0].u = u0;
    v[0].v = v0;
    v[1].u = u1;
    v[1].v = v1;
    v[2].u = u2;
    v[2].v = v2;
    g_pPolyBin->SubmitPolyTri(&g_d2dScratchPoly);
}

/* the general textured rectangle (texture page texIndex, uv and colour per corner) as two triangles:
 * TL, TR, BR, then the middle vertex moved to BL. cBL is never read: the BL corner keeps cTR's colour. */
void Draw2D_TexRect(float z, float x0, float y0, float x1, float y1, s32 texIndex, float uTL, float vTL, u32 cTL,
                    float uBL, float vBL, u32 cBL, float uTR, float vTR, u32 cTR, float uBR, float vBR, u32 cBR)
{
    float rhw;
    TexVertex *v;

    g_d2dScratchPoly.type = texIndex + RPOLY_TEXTURED_BASE;
    g_d2dScratchPoly.sortZ = z;
    rhw = 1.0f / (g_pViewFrustum->nearZ);
    v = (TexVertex *)g_d2dScratchPoly.verts;
    v[0].x = x0;
    v[0].y = y0;
    v[0].z = z;
    v[0].rhw = rhw;
    v[0].diffuse = cTL;
    v[0].specular = 0xff000000;
    v[1].x = x1;
    v[1].y = y0;
    v[1].z = z;
    v[1].rhw = rhw;
    v[1].diffuse = cTR;
    v[1].specular = 0xff000000;
    v[2].x = x1;
    v[2].y = y1;
    v[2].z = z;
    v[2].rhw = rhw;
    v[2].diffuse = cBR;
    v[2].specular = 0xff000000;
    v[0].u = uTL;
    v[0].v = vTL;
    v[1].u = uTR;
    v[1].v = vTR;
    v[2].u = uBR;
    v[2].v = vBR;
    g_pPolyBin->SubmitPolyRect(&g_d2dScratchPoly);
    v[1].x = x0;
    v[1].y = y1;
    v[1].u = uBL;
    v[1].v = vBL;
    g_pPolyBin->SubmitPoly(&g_d2dScratchPoly);
}

/* an untextured triangle drawn at once through the device (not queued) with the given RenderStateFlags,
 * from the static vertex array g_draw2dImmVerts. */
void Draw2D_FlatTri_Immediate(float z, float x0, float y0, float x1, float y1, float x2, float y2, u32 renderFlags,
                              u32 color)
{
    float rhw;

    rhw = 1.0f / (g_pViewFrustum->nearZ);
    g_draw2dImmVerts[0].x = x0;
    g_draw2dImmVerts[0].y = y0;
    g_draw2dImmVerts[0].z = z;
    g_draw2dImmVerts[0].rhw = rhw;
    g_draw2dImmVerts[0].diffuse = color;
    g_draw2dImmVerts[0].specular = 0xff000000;
    g_draw2dImmVerts[1].x = x1;
    g_draw2dImmVerts[1].y = y1;
    g_draw2dImmVerts[1].z = z;
    g_draw2dImmVerts[1].rhw = rhw;
    g_draw2dImmVerts[1].diffuse = color;
    g_draw2dImmVerts[1].specular = 0xff000000;
    g_draw2dImmVerts[2].x = x2;
    g_draw2dImmVerts[2].y = y2;
    g_draw2dImmVerts[2].z = z;
    g_draw2dImmVerts[2].rhw = rhw;
    g_draw2dImmVerts[2].diffuse = color;
    g_draw2dImmVerts[2].specular = 0xff000000;
    g_pD3DAppMain->SetStateFlagsInline(renderFlags);
    g_pD3DAppMain->DrawPrimitiveInline(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                                       g_draw2dImmVerts, 3);
    g_pD3DAppMain->Render_ClearStateFlags(renderFlags);
}

/* an untextured rectangle drawn at once: (x0,y0) (x1,y0) (x1,y1), then the middle vertex moved to (x0,y1).
 * The local vertex array is never used (only its empty constructor loop remains). */
void Draw2D_FlatRect_Immediate(float z, float x0, float y0, float x1, float y1, u32 renderFlags, u32 color)
{
    ImmFlatVertex verts[3];
    float rhw;

    rhw = 1.0f / (g_pViewFrustum->nearZ);
    g_draw2dImmVerts[0].x = x0;
    g_draw2dImmVerts[0].y = y0;
    g_draw2dImmVerts[0].z = z;
    g_draw2dImmVerts[0].rhw = rhw;
    g_draw2dImmVerts[0].diffuse = color;
    g_draw2dImmVerts[0].specular = 0xff000000;
    g_draw2dImmVerts[1].x = x1;
    g_draw2dImmVerts[1].y = y0;
    g_draw2dImmVerts[1].z = z;
    g_draw2dImmVerts[1].rhw = rhw;
    g_draw2dImmVerts[1].diffuse = color;
    g_draw2dImmVerts[1].specular = 0xff000000;
    g_draw2dImmVerts[2].x = x1;
    g_draw2dImmVerts[2].y = y1;
    g_draw2dImmVerts[2].z = z;
    g_draw2dImmVerts[2].rhw = rhw;
    g_draw2dImmVerts[2].diffuse = color;
    g_draw2dImmVerts[2].specular = 0xff000000;
    g_pD3DAppMain->SetStateFlagsInline(renderFlags);
    g_pD3DAppMain->DrawPrimitiveInline(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                                       g_draw2dImmVerts, 3);
    g_draw2dImmVerts[1].x = x0;
    g_draw2dImmVerts[1].y = y1;
    g_pD3DAppMain->DrawPrimitiveInline(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,
                                       g_draw2dImmVerts, 3);
    g_pD3DAppMain->Render_ClearStateFlags(renderFlags);
}

/* ---- the immediate primitives, continued ---- */

#define SDW_INLINE_D3DAPP_BINDTEXTURE_TEXTURE_S32 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_BINDTEXTURE_TEXTURE_S32

inline void D3DApp::DrawTLTriangles(u32 fvf, void *verts, u32 count)
{
    if (count)
        SDW_RD(pD3DDevice)->DrawPrimitive(D3DPT_TRIANGLELIST, fvf, verts, count, 0);
}

/* inline twin of Render_SetStateFlags (as src/fx/holefx.cpp) */
inline void D3DApp::ApplyStateFlags(u32 flags)
{ SDW_RSF_SET_HOOK(flags)
    if (flags & RSF_ANTIALIAS)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
    if (flags & RSF_BLEND_ALPHA) {
        pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
        pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVSRCALPHA);
        pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA);
    }
    if (flags & RSF_BLEND_ADD) {
        pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
        pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVSRCALPHA);
        pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
    }
    if (flags & RSF_ALPHATEST) {
        pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
        pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 8);
        pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_LESSEQUAL);
    }
    if (flags & RSF_CLIPPLANE)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE, TRUE);
    if (flags & RSF_DITHER)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
    if (flags & RSF_LIGHTING)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
    if (flags & RSF_SPECULAR)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);
    if (flags & RSF_COLORVERTEX)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_COLORVERTEX, TRUE);
    if (flags & RSF_CULL_CW)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
    if (flags & RSF_CULL_CCW)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
    if (flags & RSF_ZTEST)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
    if (flags & RSF_ZWRITE_ON)
        if (pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE))
            pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
    if (flags & RSF_ZWRITE_OFF)
        if (pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE))
            pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
    if (flags & RSF_TEXTURED) {
        pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    } else {
        pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    if (flags & RSF_FILTER_LINEAR) {
        pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
        pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
    }
    if (flags & RSF_FOG)
        pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
}

/* one Gouraud-shaded untextured triangle, drawn at once with the given render-state flags. */
void Draw2D_GouraudTri_Immediate(float z, float x0, float y0, u32 c0, float x1, float y1, u32 c1, float x2, float y2,
                                 u32 c2, u32 renderFlags)
{
    float rhw = 1.0f / (g_pViewFrustum->nearZ); /* the parentheses load the divisor first (fld / fdivr) */
    g_draw2dImmVerts[0].x = x0;
    g_draw2dImmVerts[0].y = y0;
    g_draw2dImmVerts[0].z = z;
    g_draw2dImmVerts[0].rhw = rhw;
    g_draw2dImmVerts[0].diffuse = c0;
    g_draw2dImmVerts[0].specular = 0xff000000;
    g_draw2dImmVerts[1].x = x1;
    g_draw2dImmVerts[1].y = y1;
    g_draw2dImmVerts[1].z = z;
    g_draw2dImmVerts[1].rhw = rhw;
    g_draw2dImmVerts[1].diffuse = c1;
    g_draw2dImmVerts[1].specular = 0xff000000;
    g_draw2dImmVerts[2].x = x2;
    g_draw2dImmVerts[2].y = y2;
    g_draw2dImmVerts[2].z = z;
    g_draw2dImmVerts[2].rhw = rhw;
    g_draw2dImmVerts[2].diffuse = c2;
    g_draw2dImmVerts[2].specular = 0xff000000;
    g_pD3DAppMain->ApplyStateFlags(renderFlags);
    g_pD3DAppMain->DrawTLTriangles(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR, g_draw2dImmVerts, 3);
    g_pD3DAppMain->Render_ClearStateFlags(renderFlags);
}

/* a Gouraud-shaded untextured rectangle: (TL, TR, BR), then vertex 1 becomes the bottom-left corner. */
void Draw2D_GouraudRect_Immediate(float z, float x0, float y0, float x1, float y1, u32 cTL, u32 cBL, u32 cTR, u32 cBR,
                                  u32 renderFlags)
{
    float rhw = 1.0f / (g_pViewFrustum->nearZ); /* the parentheses load the divisor first (fld / fdivr) */
    g_draw2dImmVerts[0].x = x0;
    g_draw2dImmVerts[0].y = y0;
    g_draw2dImmVerts[0].z = z;
    g_draw2dImmVerts[0].rhw = rhw;
    g_draw2dImmVerts[0].diffuse = cTL;
    g_draw2dImmVerts[0].specular = 0xff000000;
    g_draw2dImmVerts[1].x = x1;
    g_draw2dImmVerts[1].y = y0;
    g_draw2dImmVerts[1].z = z;
    g_draw2dImmVerts[1].rhw = rhw;
    g_draw2dImmVerts[1].diffuse = cTR;
    g_draw2dImmVerts[1].specular = 0xff000000;
    g_draw2dImmVerts[2].x = x1;
    g_draw2dImmVerts[2].y = y1;
    g_draw2dImmVerts[2].z = z;
    g_draw2dImmVerts[2].rhw = rhw;
    g_draw2dImmVerts[2].diffuse = cBR;
    g_draw2dImmVerts[2].specular = 0xff000000;
    g_pD3DAppMain->ApplyStateFlags(renderFlags);
    g_pD3DAppMain->DrawTLTriangles(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR, g_draw2dImmVerts, 3);
    g_draw2dImmVerts[1].x = x0;
    g_draw2dImmVerts[1].y = y1;
    g_draw2dImmVerts[1].diffuse = cBL;
    g_pD3DAppMain->DrawTLTriangles(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR, g_draw2dImmVerts, 3);
    g_pD3DAppMain->Render_ClearStateFlags(renderFlags);
}

/* one textured, Gouraud-shaded triangle: binds tex to stage 0 and leaves the batcher's texture binding
 * marked stale, since the direct SetTexture bypassed it. */
void Draw2D_TexTri_Immediate(float z, float x0, float y0, float x1, float y1, float x2, float y2, u32 renderFlags,
                             Texture *tex, float u0, float v0, u32 c0, float u1, float v1, u32 c1, float u2, float v2,
                             u32 c2)
{
    float rhw = 1.0f / (g_pViewFrustum->nearZ); /* the parentheses load the divisor first (fld / fdivr) */
    g_draw2dImmTexCorners[0].x = x0;
    g_draw2dImmTexCorners[0].y = y0;
    g_draw2dImmTexCorners[0].z = z;
    g_draw2dImmTexCorners[0].rhw = rhw;
    g_draw2dImmTexCorners[0].diffuse = c0;
    g_draw2dImmTexCorners[0].specular = 0xff000000;
    g_draw2dImmTexCorners[1].x = x1;
    g_draw2dImmTexCorners[1].y = y1;
    g_draw2dImmTexCorners[1].z = z;
    g_draw2dImmTexCorners[1].rhw = rhw;
    g_draw2dImmTexCorners[1].diffuse = c1;
    g_draw2dImmTexCorners[1].specular = 0xff000000;
    g_draw2dImmTexCorners[2].x = x2;
    g_draw2dImmTexCorners[2].y = y2;
    g_draw2dImmTexCorners[2].z = z;
    g_draw2dImmTexCorners[2].rhw = rhw;
    g_draw2dImmTexCorners[2].diffuse = c2;
    g_draw2dImmTexCorners[2].specular = 0xff000000;
    g_draw2dImmTexCorners[0].u = u0;
    g_draw2dImmTexCorners[0].v = v0;
    g_draw2dImmTexCorners[1].u = u1;
    g_draw2dImmTexCorners[1].v = v1;
    g_draw2dImmTexCorners[2].u = u2;
    g_draw2dImmTexCorners[2].v = v2;
    g_pD3DAppMain->BindTexture(tex, 0);
    g_pD3DAppMain->ApplyStateFlags(renderFlags);
    g_pD3DAppMain->DrawTLTriangles(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                   g_draw2dImmTexCorners, 3);
    g_pD3DAppMain->Render_ClearStateFlags(renderFlags);
    g_pPolyBin->InvalidateTexture();
}

/* a textured rectangle: (TL, TR, BR), then vertex 1 becomes the bottom-left corner. Only its position and
 * uv are rewritten: cBL is never read, so the bottom-left corner keeps the top-right colour. */
void Draw2D_TexRect_Immediate(float z, float x0, float y0, float x1, float y1, u32 renderFlags, Texture *texture,
                              float uTL, float vTL, u32 cTL, float uBL, float vBL, u32 cBL, float uTR, float vTR,
                              u32 cTR, float uBR, float vBR, u32 cBR)
{
    float rhw = 1.0f / (g_pViewFrustum->nearZ); /* the parentheses load the divisor first (fld / fdivr) */
    g_draw2dImmTexCorners[0].x = x0;
    g_draw2dImmTexCorners[0].y = y0;
    g_draw2dImmTexCorners[0].z = z;
    g_draw2dImmTexCorners[0].rhw = rhw;
    g_draw2dImmTexCorners[0].diffuse = cTL;
    g_draw2dImmTexCorners[0].specular = 0xff000000;
    g_draw2dImmTexCorners[1].x = x1;
    g_draw2dImmTexCorners[1].y = y0;
    g_draw2dImmTexCorners[1].z = z;
    g_draw2dImmTexCorners[1].rhw = rhw;
    g_draw2dImmTexCorners[1].diffuse = cTR;
    g_draw2dImmTexCorners[1].specular = 0xff000000;
    g_draw2dImmTexCorners[2].x = x1;
    g_draw2dImmTexCorners[2].y = y1;
    g_draw2dImmTexCorners[2].z = z;
    g_draw2dImmTexCorners[2].rhw = rhw;
    g_draw2dImmTexCorners[2].diffuse = cBR;
    g_draw2dImmTexCorners[2].specular = 0xff000000;
    g_draw2dImmTexCorners[0].u = uTL;
    g_draw2dImmTexCorners[0].v = vTL;
    g_draw2dImmTexCorners[1].u = uTR;
    g_draw2dImmTexCorners[1].v = vTR;
    g_draw2dImmTexCorners[2].u = uBR;
    g_draw2dImmTexCorners[2].v = vBR;
    g_pD3DAppMain->BindTexture(texture, 0);
    g_pD3DAppMain->ApplyStateFlags(renderFlags);
    g_pD3DAppMain->DrawTLTriangles(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                   g_draw2dImmTexCorners, 3);
    g_draw2dImmTexCorners[1].x = x0;
    g_draw2dImmTexCorners[1].y = y1;
    g_draw2dImmTexCorners[1].u = uBL;
    g_draw2dImmTexCorners[1].v = vBL;
    g_pD3DAppMain->DrawTLTriangles(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                   g_draw2dImmTexCorners, 3);
    g_pD3DAppMain->Render_ClearStateFlags(renderFlags);
    g_pPolyBin->InvalidateTexture();
}
