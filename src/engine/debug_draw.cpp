/*
 * TlVertexFlat's colour word has two names, `diffuse` and `color` (an anonymous union: the two halves of the file use
 * one each; same offset and type).
 *
 * Each drawer writes its model-space vertices into the untransformed scratch buffer, sets the world / view / projection
 * transforms, has D3D transform them into the TL scratch buffer (ProcessVertices), colours them and queues lines in the
 * PolyBatcher's line batch or triangles as RenderPolys, expanding PolyBatcher::SubmitPoly in place.
 *  - DebugDraw_Camera expands it for its first three triangles only, each time with fewer draw helpers expanded, and
 *    calls it for the other nine. Both quad routines overwrite the vertex block RenderPoly's constructor allocated (
 *    news 0x60 bytes) with one of their own, so every call leaks 2 x 0x60 bytes.
 */
#include "sdw_types.h"
#include "sdw_enums.h"
#include "../sdk/d3d7.h"
#include "../sdk/ddraw.h"
class Mat44;
#include "../sdk/crt.h"
#define IsEqualGUID(a, b) (!memcmp((a), (b), 16)) /* sizeof(GUID); the type is incomplete here */

/* The line batch's vertex format, FVF 0xc4 (XYZRHW | DIFFUSE | SPECULAR), as in src/engine/polybatcher.cpp. Its empty
 * constructor is what makes `new TlVertexFlat[2]` run an (empty) element loop. The colour word has two names (an
 * anonymous union: one u32 at +0x10). */
struct TlVertexFlat {
    float x, y, z, rhw;
    union {
        u32 diffuse;
        u32 color;
    };
    u32 specular;
    TlVertexFlat() {}
};
struct XyzVertex { /* a D3DFVF_XYZ vertex */
    float x, y, z;
};
struct D3DXyzVertex { /* a D3DFVF_XYZ vertex: the untransformed scratch buffer */
    float x, y, z;
};
struct D3DTLVertex {
    float x, y, z, rhw;
    u32 color, specular;
    float u, v;
    D3DTLVertex() {}
};
class Texture;
enum TexStage { TEX_STAGE_0 };

#define SDW_MEMBERS_Mat44 Mat44();
#define SDW_MEMBERS_D3DApp                                                               \
    void CreateVB(D3DVERTEXBUFFERDESC *desc, SdwVertexBuffer **out);              \
    IDirect3DDevice7 *GetDevice();                                                       \
    void DrawPrimitiveInline(u32 type, u32 fvf, void *verts, u32 count);                 \
    void Render_DrawPrimitive(u32 type, u32 fvf, void *verts, u32 count); \
    inline void SetTexture(Texture *tex, TexStage stage);                                \
    void Render_SetStateFlags(u32 flags);                               \
    void Render_ClearStateFlags(u32 flags);
#define SDW_MEMBERS_RenderPoly                                              \
    RenderPoly();                            /* RenderPoly_Ctor */ \
    /* virtual ~RenderPoly() is generated */ /* RenderPoly_Dtor */
#define SDW_MEMBERS_PolyBatcher                                                                                   \
    /* inline: queue `count` lines (2 vertices each) while the line batch has room */                             \
    void AddLines(TlVertexFlat *verts, u32 count)                                                                 \
    {                                                                                                             \
        if (lineBatchCount <= batchCapacity) {                                                                    \
\
            memcpy((TlVertexFlat *)lineBatchVerts + lineBatchCount * 2, verts, count * sizeof(TlVertexFlat) * 2); \
            lineBatchCount += count;                                                                              \
        }                                                                                                         \
    }                                                                                                             \
    inline void SubmitPolys(RenderPoly *polys, u32 n); /* 's body over an array; defined below */         \
    void SubmitPoly(RenderPoly *poly);                                             \
    inline void SubmitTriA(RenderPoly *poly);          /* 's body; the twins are defined below */         \
    inline void SubmitTriB(RenderPoly *poly);                                                                     \
    inline void SubmitTriC(RenderPoly *poly);
#include "sdw_classes.h"
#define SDW_INLINE_D3DAPP_CREATEVB_D3DVERTEXBUFFERDESC_IDIRECT3DVERTEXBUFFER7 1
#define SDW_INLINE_D3DAPP_GETDEVICE 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_CREATEVB_D3DVERTEXBUFFERDESC_IDIRECT3DVERTEXBUFFER7
#undef SDW_INLINE_D3DAPP_GETDEVICE
#define SDW_INLINE_D3DAPP_DRAWPRIMITIVEINLINE_U32_U32_VOID_U32 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_DRAWPRIMITIVEINLINE_U32_U32_VOID_U32

#include "draw2d.h"
#include "../app/app_main.h"
#include "fixed_math.h"
uptr *Res_GetValidatedIdList(u16 resId, u16 *outCount);

SdwVertexBuffer *g_pVBScratchTLTex = 0; /* 32 x FVF 0x1c4 */
SdwVertexBuffer *g_pVBScratchXYZ = 0;   /* 32 x FVF 0x002, the ProcessVertices source */
SdwVertexBuffer *g_pVBScratchTL = 0;    /* 32 x FVF 0x0c4, the ProcessVertices target */

/* creates the three 32-vertex scratch buffers (called once from the game-system init). The first two
 * descriptors carry dwSize 0x180, not sizeof(D3DVERTEXBUFFERDESC) = 0x10 as the third does. */
void Draw_CreateScratchVertexBuffers()
{
    D3DVERTEXBUFFERDESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = 0x180;
    desc.dwCaps = D3DVBCAPS_DONOTCLIP;
    desc.dwFVF = D3DFVF_XYZ;
    desc.dwNumVertices = 32;
    g_pD3DAppMain->CreateVB(&desc, &g_pVBScratchXYZ);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = 0x180;
    desc.dwCaps = D3DVBCAPS_DONOTCLIP;
    desc.dwFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
    desc.dwNumVertices = 32;
    g_pD3DAppMain->CreateVB(&desc, &g_pVBScratchTL);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwCaps = D3DVBCAPS_DONOTCLIP;
    desc.dwFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
    desc.dwNumVertices = 32;
    g_pD3DAppMain->CreateVB(&desc, &g_pVBScratchTLTex);
}

/* draws a CollBox (min +4, max +0xa) as a red wire cube. The argument is copied into a local first
 *, as if it arrived untyped; `box` is that local. Like the other two drawers, it never frees the two-vertex
 * line it allocates (0x30 bytes leaked per call). */
void DebugDraw_CollBox(void *boxData, Mat44 *world, Mat44 *view, Mat44 *proj, D3DApp *app, PolyBatcher *batcher)
{
    u32 sz;
    CollBox *box;
    TlVertexFlat *seg;
    void *vb;
    u32 idx;
    box = (CollBox *)boxData;
    g_pVBScratchXYZ->Lock(DDLOCK_WAIT, &vb, &sz);
    ((XyzVertex *)vb)[0].x = box->min.x;
    ((XyzVertex *)vb)[0].y = box->min.y;
    ((XyzVertex *)vb)[0].z = box->min.z;
    ((XyzVertex *)vb)[1].x = box->max.x;
    ((XyzVertex *)vb)[1].y = box->min.y;
    ((XyzVertex *)vb)[1].z = box->min.z;
    ((XyzVertex *)vb)[2].x = box->max.x;
    ((XyzVertex *)vb)[2].y = box->max.y;
    ((XyzVertex *)vb)[2].z = box->min.z;
    ((XyzVertex *)vb)[3].x = box->min.x;
    ((XyzVertex *)vb)[3].y = box->max.y;
    ((XyzVertex *)vb)[3].z = box->min.z;
    ((XyzVertex *)vb)[4].x = box->min.x;
    ((XyzVertex *)vb)[4].y = box->max.y;
    ((XyzVertex *)vb)[4].z = box->max.z;
    ((XyzVertex *)vb)[5].x = box->min.x;
    ((XyzVertex *)vb)[5].y = box->min.y;
    ((XyzVertex *)vb)[5].z = box->max.z;
    ((XyzVertex *)vb)[6].x = box->max.x;
    ((XyzVertex *)vb)[6].y = box->min.y;
    ((XyzVertex *)vb)[6].z = box->max.z;
    ((XyzVertex *)vb)[7].x = box->max.x;
    ((XyzVertex *)vb)[7].y = box->max.y;
    ((XyzVertex *)vb)[7].z = box->max.z;
    g_pVBScratchXYZ->Unlock();
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_WORLD, world);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_VIEW, view);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_PROJECTION, proj);
    g_pVBScratchTL->ProcessVertices(D3DVOP_TRANSFORM, 0, 8, g_pVBScratchXYZ, 0, SDW_RD(app->GetDevice()), D3DPV_DONOTCOPYDATA);
    g_pVBScratchTL->Lock(DDLOCK_WAIT, &vb, &sz);
    for (idx = 0; idx < 8; idx++)
        ((TlVertexFlat *)vb)[idx].diffuse = 0xff0000;
    seg = new TlVertexFlat[2];
    memcpy(&seg[0], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    g_pVBScratchTL->Unlock();
}

/* the same red wire cube for a box given as two padded s16 vectors (min at [0..2], max at [4..6]). */
void DebugDraw_Bounds(s16 *minMax, Mat44 *world, Mat44 *view, Mat44 *proj, D3DApp *app, PolyBatcher *batcher)
{
    u32 sz;
    TlVertexFlat *seg;
    void *vb;
    u32 idx;
    g_pVBScratchXYZ->Lock(DDLOCK_WAIT, &vb, &sz);
    ((XyzVertex *)vb)[0].x = minMax[0];
    ((XyzVertex *)vb)[0].y = minMax[1];
    ((XyzVertex *)vb)[0].z = minMax[2];
    ((XyzVertex *)vb)[1].x = minMax[4];
    ((XyzVertex *)vb)[1].y = minMax[1];
    ((XyzVertex *)vb)[1].z = minMax[2];
    ((XyzVertex *)vb)[2].x = minMax[4];
    ((XyzVertex *)vb)[2].y = minMax[5];
    ((XyzVertex *)vb)[2].z = minMax[2];
    ((XyzVertex *)vb)[3].x = minMax[0];
    ((XyzVertex *)vb)[3].y = minMax[5];
    ((XyzVertex *)vb)[3].z = minMax[2];
    ((XyzVertex *)vb)[4].x = minMax[0];
    ((XyzVertex *)vb)[4].y = minMax[5];
    ((XyzVertex *)vb)[4].z = minMax[6];
    ((XyzVertex *)vb)[5].x = minMax[0];
    ((XyzVertex *)vb)[5].y = minMax[1];
    ((XyzVertex *)vb)[5].z = minMax[6];
    ((XyzVertex *)vb)[6].x = minMax[4];
    ((XyzVertex *)vb)[6].y = minMax[1];
    ((XyzVertex *)vb)[6].z = minMax[6];
    ((XyzVertex *)vb)[7].x = minMax[4];
    ((XyzVertex *)vb)[7].y = minMax[5];
    ((XyzVertex *)vb)[7].z = minMax[6];
    g_pVBScratchXYZ->Unlock();
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_WORLD, world);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_VIEW, view);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_PROJECTION, proj);
    g_pVBScratchTL->ProcessVertices(D3DVOP_TRANSFORM, 0, 8, g_pVBScratchXYZ, 0, SDW_RD(app->GetDevice()), D3DPV_DONOTCOPYDATA);
    g_pVBScratchTL->Lock(DDLOCK_WAIT, &vb, &sz);
    for (idx = 0; idx < 8; idx++)
        ((TlVertexFlat *)vb)[idx].diffuse = 0xff0000;
    seg = new TlVertexFlat[2];
    memcpy(&seg[0], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    g_pVBScratchTL->Unlock();
}

/* a red wire cube of edge 2/sqrt(3) * size centred on pos, plus a green segment from pos to pos + (size,0,0). */
void DebugDraw_PointMarker(Vec3s *pos, u16 size, Mat44 *world, Mat44 *view, Mat44 *proj, D3DApp *app,
                           PolyBatcher *batcher)
{
    u32 sz;
    float ext;
    TlVertexFlat *seg;
    XyzVertex lower;
    XyzVertex upper;
    void *vb;
    u32 idx;
    ext = 2.0f / (float)sqrt(3.0) * size;
    lower.x = pos->x - ext / 2.0f;
    lower.y = pos->y - ext / 2.0f;
    lower.z = pos->z - ext / 2.0f;
    upper.x = pos->x + ext / 2.0f;
    upper.y = pos->y + ext / 2.0f;
    upper.z = pos->z + ext / 2.0f;
    g_pVBScratchXYZ->Lock(DDLOCK_WAIT, &vb, &sz);
    ((XyzVertex *)vb)[0].x = lower.x;
    ((XyzVertex *)vb)[0].y = lower.y;
    ((XyzVertex *)vb)[0].z = lower.z;
    ((XyzVertex *)vb)[1].x = upper.x;
    ((XyzVertex *)vb)[1].y = lower.y;
    ((XyzVertex *)vb)[1].z = lower.z;
    ((XyzVertex *)vb)[2].x = upper.x;
    ((XyzVertex *)vb)[2].y = upper.y;
    ((XyzVertex *)vb)[2].z = lower.z;
    ((XyzVertex *)vb)[3].x = lower.x;
    ((XyzVertex *)vb)[3].y = upper.y;
    ((XyzVertex *)vb)[3].z = lower.z;
    ((XyzVertex *)vb)[4].x = lower.x;
    ((XyzVertex *)vb)[4].y = upper.y;
    ((XyzVertex *)vb)[4].z = upper.z;
    ((XyzVertex *)vb)[5].x = lower.x;
    ((XyzVertex *)vb)[5].y = lower.y;
    ((XyzVertex *)vb)[5].z = upper.z;
    ((XyzVertex *)vb)[6].x = upper.x;
    ((XyzVertex *)vb)[6].y = lower.y;
    ((XyzVertex *)vb)[6].z = upper.z;
    ((XyzVertex *)vb)[7].x = upper.x;
    ((XyzVertex *)vb)[7].y = upper.y;
    ((XyzVertex *)vb)[7].z = upper.z;
    ((XyzVertex *)vb)[8].x = pos->x;
    ((XyzVertex *)vb)[8].y = pos->y;
    ((XyzVertex *)vb)[8].z = pos->z;
    ((XyzVertex *)vb)[9].x = pos->x + size;
    ((XyzVertex *)vb)[9].y = pos->y;
    ((XyzVertex *)vb)[9].z = pos->z;
    g_pVBScratchXYZ->Unlock();
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_WORLD, world);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_VIEW, view);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_PROJECTION, proj);
    g_pVBScratchTL->ProcessVertices(D3DVOP_TRANSFORM, 0, 10, g_pVBScratchXYZ, 0, SDW_RD(app->GetDevice()), D3DPV_DONOTCOPYDATA);
    g_pVBScratchTL->Lock(DDLOCK_WAIT, &vb, &sz);
    for (idx = 0; idx < 8; idx++)
        ((TlVertexFlat *)vb)[idx].diffuse = 0xff0000;
    ((TlVertexFlat *)vb)[8].diffuse = 0xff00;
    ((TlVertexFlat *)vb)[9].diffuse = 0xff00;
    seg = new TlVertexFlat[2];
    memcpy(&seg[0], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[0], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[5], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[1], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[6], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[2], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[7], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[3], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[4], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    memcpy(&seg[0], &((TlVertexFlat *)vb)[8], sizeof(TlVertexFlat));
    memcpy(&seg[1], &((TlVertexFlat *)vb)[9], sizeof(TlVertexFlat));
    batcher->AddLines(seg, 1);
    g_pVBScratchTL->Unlock();
}

/* ---- debug drawing continued: ---- */

/* The DAV directory (DavHeader.dir): packed, its pointers sit at +6/+0xa/+0x12 (Load_DAV, src/engine/load_dav.cpp),
 * which the struct generator cannot lay out, so it is declared here as in src/objects/lightspot.cpp. */
#include "sdw_fileptr.h"
#pragma pack(push, 1)
struct DavDirectory {
    u16 indexCount;
    u16 bitmapCount;
    u16 unk04;
    SDW_DAVPTR(u16) indices;
    SDW_DAVPTR(DavBitmapRec) bitmaps; /* 10-byte records */
    u32 fileSize;
    SDW_DAVPTR(u32) idLists;
};
#pragma pack(pop)

#define SDW_INLINE_D3DAPP_SETTEXTURE_TEXTURE_TEXSTAGE 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_SETTEXTURE_TEXTURE_TEXSTAGE

/* PolyBatcher_SubmitPoly over an array of polys, as the debug routines expand it: untextured triangles and the immediate
 * texture pages are batched (and drawn when a batch fills), everything else goes to the sorted list. */
inline void PolyBatcher::SubmitPolys(RenderPoly *polys, u32 n)
{
    u32 *num;
    void *batch;
    u32 *pflags;
    u32 idx;
    RenderPoly *p;
    u32 pi;

    for (pi = 0; pi < n; pi++) {
        p = &polys[pi];
        switch (p->type) {
            case RPOLY_OPAQUE:
                if (flatBatchCount <= batchCapacity) {
                    memcpy((u8 *)flatBatchVerts + flatBatchCount * 3 * 0x18, p->verts, 0x48);
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
                    p->sortZ = p->verts[2] + p->verts[8] + p->verts[14];
                sortedPolys[sortedCount].Assign(p);
                sortedList[sortedCount] = &sortedPolys[sortedCount];
                sortedCount++;
                break;
            case RPOLY_ADD:
                if (computeSortZ == 1)
                    p->sortZ = p->verts[2] + p->verts[8] + p->verts[14];
                sortedPolys[sortedCount].Assign(p);
                sortedList[sortedCount] = &sortedPolys[sortedCount];
                sortedCount++;
                break;
            default:
                idx = p->type - RPOLY_TEXTURED_BASE & ~RPOLY_F_8000;
                if (idx < immediateTexCount) {
                    batch = texBatchVerts[idx];
                    num = &texBatchCounts[idx];
                    pflags = &texStateFlags[idx];
                    if (*num <= batchCapacity) {
                        memcpy((D3DTLVertex *)batch + *num * 3, p->verts, 0x60);
                        (*num)++;
                    }
                    if (*num >= batchCapacity) {
                        if (idx != lastTextureIndex || textureDirty == 1) {
                            renderer->SetTexture(textures[idx], TEX_STAGE_0);
                            lastTextureIndex = idx;
                            textureDirty = 0;
                        }
                        renderer->Render_SetStateFlags(*pflags);
                        renderer->DrawPrimitiveInline(D3DPT_TRIANGLELIST,
                                                      D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                                      batch, batchCapacity * 3);
                        renderer->Render_ClearStateFlags(*pflags);
                        *num = 0;
                    }
                } else {
                    if (computeSortZ == 1)
                        p->sortZ = p->verts[2] + p->verts[10] + p->verts[18];
                    sortedPolys[sortedCount].Assign(p);
                    sortedList[sortedCount] = &sortedPolys[sortedCount];
                    sortedCount++;
                }
        }
    }
}

#define SUBMIT_TRI_BODY(FLATDRAW, TEXDRAW)                                                                             \
    switch (poly->type) {                                                                                              \
        case RPOLY_OPAQUE:                                                                                             \
            if (flatBatchCount <= batchCapacity) {                                                                     \
\
                memcpy((u8 *)flatBatchVerts + flatBatchCount * 3 * 0x18, poly->verts, 0x48);                           \
                flatBatchCount++;                                                                                      \
            }                                                                                                          \
            if (flatBatchCount >= batchCapacity) {                                                                     \
                renderer->Render_SetStateFlags(stateFlags);                                                            \
                renderer->FLATDRAW(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR,               \
                                   flatBatchVerts, batchCapacity * 3);                                                 \
                renderer->Render_ClearStateFlags(stateFlags);                                                          \
                flatBatchCount = 0;                                                                                    \
                textureDirty = 1;                                                                                      \
            }                                                                                                          \
            break;                                                                                                     \
        case RPOLY_BLEND:                                                                                              \
            if (computeSortZ == 1)                                                                                     \
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];                                       \
            sortedPolys[sortedCount].Assign(poly);                                                                     \
            sortedList[sortedCount] = &sortedPolys[sortedCount];                                                       \
            sortedCount++;                                                                                             \
            break;                                                                                                     \
        case RPOLY_ADD:                                                                                                \
            if (computeSortZ == 1)                                                                                     \
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];                                       \
            sortedPolys[sortedCount].Assign(poly);                                                                     \
            sortedList[sortedCount] = &sortedPolys[sortedCount];                                                       \
            sortedCount++;                                                                                             \
            break;                                                                                                     \
        default: {                                                                                                     \
            u32 idx;                                                \
            float *aV;                                                                                                 \
            aV = poly->verts;                                                                                          \
            idx = (poly->type - RPOLY_TEXTURED_BASE) & ~RPOLY_F_8000;                                                  \
            if (idx < immediateTexCount) {                                                                             \
                u32 *pCount;                                                                                           \
                D3DTLVertex *buf;                                                                                      \
                u32 *flags;                                                                                            \
\
                buf = (D3DTLVertex *)texBatchVerts[idx];                                                               \
                pCount = &texBatchCounts[idx];                                                                         \
                flags = &texStateFlags[idx];                                                                           \
                if (*pCount <= batchCapacity) {                                                                        \
                    memcpy(buf + *pCount * 3, aV, 0x60);                                                               \
                    (*pCount)++;                                                                                       \
                }                                                                                                      \
                if (*pCount >= batchCapacity) {                                                                        \
                    if (idx != lastTextureIndex || textureDirty == 1) {                                                \
                        renderer->SetTexture(textures[idx], TEX_STAGE_0);                                              \
                        lastTextureIndex = idx;                                                                        \
                        textureDirty = 0;                                                                              \
                    }                                                                                                  \
                    renderer->Render_SetStateFlags(*flags);                                                            \
                    renderer->TEXDRAW(D3DPT_TRIANGLELIST,                                                              \
                                      D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, buf,             \
                                      batchCapacity * 3);                                                              \
                    renderer->Render_ClearStateFlags(*flags);                                                          \
                    *pCount = 0;                                                                                       \
                }                                                                                                      \
            } else {                                                                                                   \
                if (computeSortZ == 1)                                                                                 \
                    poly->sortZ = aV[2] + aV[10] + aV[18];                                                             \
                sortedPolys[sortedCount].Assign(poly);                                                                 \
                sortedList[sortedCount] = &sortedPolys[sortedCount];                                                   \
                sortedCount++;                                                                                         \
            }                                                                                                          \
            break;                                                                                                     \
        }                                                                                                              \
    }
inline void PolyBatcher::SubmitTriA(RenderPoly *poly) /* both draws expanded */
{
    SUBMIT_TRI_BODY(DrawPrimitiveInline, DrawPrimitiveInline)
}
inline void PolyBatcher::SubmitTriB(RenderPoly *poly) /* the textured draw a call */
{
    SUBMIT_TRI_BODY(DrawPrimitiveInline, Render_DrawPrimitive)
}
inline void PolyBatcher::SubmitTriC(RenderPoly *poly) /* both draws calls */
{
    SUBMIT_TRI_BODY(Render_DrawPrimitive, Render_DrawPrimitive)
}

/* a 400x400 quad standing at z 200 (x -200..200, y -500..-100) in world space, drawn with texture page
 * texIndex tinted by diffuse, as two textured triangles. */
void DebugDraw_WorldQuadTex(u16 texIndex, u32 diffuse, Mat44 *matProj, Mat44 *matView, D3DApp *app)
{
    u32 sz;
    u32 unused;
    D3DTLVertex *tl;

    g_pVBScratchXYZ->Lock(DDLOCK_WAIT, (void **)&tl, &sz);
    ((D3DXyzVertex *)tl)[0].x = -200.0f;
    ((D3DXyzVertex *)tl)[0].y = -500.0f;
    ((D3DXyzVertex *)tl)[0].z = 200.0f;
    ((D3DXyzVertex *)tl)[1].x = -200.0f;
    ((D3DXyzVertex *)tl)[1].y = -100.0f;
    ((D3DXyzVertex *)tl)[1].z = 200.0f;
    ((D3DXyzVertex *)tl)[2].x = 200.0f;
    ((D3DXyzVertex *)tl)[2].y = -100.0f;
    ((D3DXyzVertex *)tl)[2].z = 200.0f;
    ((D3DXyzVertex *)tl)[3].x = 200.0f;
    ((D3DXyzVertex *)tl)[3].y = -500.0f;
    ((D3DXyzVertex *)tl)[3].z = 200.0f;
    ((D3DXyzVertex *)tl)[4].x = -200.0f;
    ((D3DXyzVertex *)tl)[4].y = -500.0f;
    ((D3DXyzVertex *)tl)[4].z = 200.0f;
    ((D3DXyzVertex *)tl)[5].x = 200.0f;
    ((D3DXyzVertex *)tl)[5].y = -100.0f;
    ((D3DXyzVertex *)tl)[5].z = 200.0f;
    g_pVBScratchXYZ->Unlock();
    Mat44 world;
    world.SetIdentity();
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_WORLD, &world);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_VIEW, matView);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_PROJECTION, matProj);
    g_pVBScratchTLTex->ProcessVertices(D3DVOP_TRANSFORM, 0, 6, g_pVBScratchXYZ, 0, SDW_RD(app->GetDevice()),
                                       D3DPV_DONOTCOPYDATA);
    g_pVBScratchTLTex->Lock(DDLOCK_WAIT, (void **)&tl, &sz);
    tl[0].u = 0.0f;
    tl[0].v = 0.0f;
    tl[0].color = diffuse;
    tl[0].specular = 0xff000000;
    tl[1].u = 0.0f;
    tl[1].v = 1.0f;
    tl[1].color = diffuse;
    tl[1].specular = 0xff000000;
    tl[2].u = 1.0f;
    tl[2].v = 1.0f;
    tl[2].color = diffuse;
    tl[2].specular = 0xff000000;
    tl[3].u = 1.0f;
    tl[3].v = 0.0f;
    tl[3].color = diffuse;
    tl[3].specular = 0xff000000;
    tl[4].u = 0.0f;
    tl[4].v = 0.0f;
    tl[4].color = diffuse;
    tl[4].specular = 0xff000000;
    tl[5].u = 1.0f;
    tl[5].v = 1.0f;
    tl[5].color = diffuse;
    tl[5].specular = 0xff000000;
    RenderPoly polys[2];
    polys[0].type = texIndex + RPOLY_TEXTURED_BASE;
    polys[0].verts = (float *)new D3DTLVertex[3];
    memcpy(polys[0].verts, tl, 0x60);
    polys[1].type = texIndex + RPOLY_TEXTURED_BASE;
    polys[1].verts = (float *)new D3DTLVertex[3];
    memcpy(polys[1].verts, tl + 3, 0x60);
    g_pVBScratchTLTex->Unlock();
    g_pPolyBin->SubmitPolys(polys, 2);
}

/* the same 400x400 world quad as DebugDraw_WorldQuadTex, showing the DAV bitmap of resource resId (its
 * first id): untinted, UVs from the bitmap's rectangle on its texture page (texels / 256). */
void DebugDraw_WorldQuadBitmap(u16 resId, Mat44 *matProj, Mat44 *matView, D3DApp *app)
{
    u32 bytesz;
    u16 tx9;
    DavBitmapRec *r9;
    void **idlk;
    u16 num1;
    u16 ty0;
    D3DTLVertex *tl4;
    u16 wid8;
    u16 hgt6;

    idlk = (void **)Res_GetValidatedIdList(resId, &num1);
    r9 = &g_levelDav.header->dir->bitmaps[*(u16 *)idlk[0]];
    g_pVBScratchXYZ->Lock(DDLOCK_WAIT, (void **)&tl4, &bytesz);
    ((D3DXyzVertex *)tl4)[0].x = -200.0f;
    ((D3DXyzVertex *)tl4)[0].y = -500.0f;
    ((D3DXyzVertex *)tl4)[0].z = 200.0f;
    ((D3DXyzVertex *)tl4)[1].x = -200.0f;
    ((D3DXyzVertex *)tl4)[1].y = -100.0f;
    ((D3DXyzVertex *)tl4)[1].z = 200.0f;
    ((D3DXyzVertex *)tl4)[2].x = 200.0f;
    ((D3DXyzVertex *)tl4)[2].y = -100.0f;
    ((D3DXyzVertex *)tl4)[2].z = 200.0f;
    ((D3DXyzVertex *)tl4)[3].x = 200.0f;
    ((D3DXyzVertex *)tl4)[3].y = -500.0f;
    ((D3DXyzVertex *)tl4)[3].z = 200.0f;
    ((D3DXyzVertex *)tl4)[4].x = -200.0f;
    ((D3DXyzVertex *)tl4)[4].y = -500.0f;
    ((D3DXyzVertex *)tl4)[4].z = 200.0f;
    ((D3DXyzVertex *)tl4)[5].x = 200.0f;
    ((D3DXyzVertex *)tl4)[5].y = -100.0f;
    ((D3DXyzVertex *)tl4)[5].z = 200.0f;
    g_pVBScratchXYZ->Unlock();
    Mat44 world;
    world.SetIdentity();
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_WORLD, &world);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_VIEW, matView);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_PROJECTION, matProj);
    g_pVBScratchTLTex->ProcessVertices(D3DVOP_TRANSFORM, 0, 6, g_pVBScratchXYZ, 0, SDW_RD(app->GetDevice()),
                                       D3DPV_DONOTCOPYDATA);
    g_pVBScratchTLTex->Lock(DDLOCK_WAIT, (void **)&tl4, &bytesz);
    wid8 = r9->width;
    tx9 = r9->u;
    ty0 = r9->v;
    hgt6 = r9->height;
    tl4[0].u = tx9 / 256.0f;
    tl4[0].v = (ty0 + hgt6) / 256.0f;
    tl4[0].color = 0xffffffff;
    tl4[0].specular = 0xff000000;
    tl4[1].u = tx9 / 256.0f;
    tl4[1].v = ty0 / 256.0f;
    tl4[1].color = 0xffffffff;
    tl4[1].specular = 0xff000000;
    tl4[2].u = (wid8 + tx9) / 256.0f;
    tl4[2].v = ty0 / 256.0f;
    tl4[2].color = 0xffffffff;
    tl4[2].specular = 0xff000000;
    tl4[3].u = (wid8 + tx9) / 256.0f;
    tl4[3].v = (ty0 + hgt6) / 256.0f;
    tl4[3].color = 0xffffffff;
    tl4[3].specular = 0xff000000;
    tl4[4].u = tx9 / 256.0f;
    tl4[4].v = (ty0 + hgt6) / 256.0f;
    tl4[4].color = 0xffffffff;
    tl4[4].specular = 0xff000000;
    tl4[5].u = (wid8 + tx9) / 256.0f;
    tl4[5].v = ty0 / 256.0f;
    tl4[5].color = 0xffffffff;
    tl4[5].specular = 0xff000000;
    RenderPoly tri[2];
    tri[0].type = r9->page + RPOLY_TEXTURED_BASE;
    tri[0].verts = (float *)new D3DTLVertex[3];
    memcpy(tri[0].verts, tl4, 0x60);
    tri[1].type = r9->page + RPOLY_TEXTURED_BASE;
    tri[1].verts = (float *)new D3DTLVertex[3];
    memcpy(tri[1].verts, tl4 + 3, 0x60);
    g_pVBScratchTLTex->Unlock();
    g_pPolyBin->SubmitPolys(tri, 2);
}

/* a camera gizmo. A cube of side 80 around the camera's position, rotated by its orientation, drawn as 12
 * untextured triangles with a colour ramp over the corners, plus one line from the cube's front face (z 40) to z 70
 * along the view axis. The camera's position is read as the s32 viewPos, its angles from rot (12-bit). */
void DebugDraw_Camera(Camera *cam, Mat44 *matView, Mat44 *matProj, D3DApp *app, PolyBatcher *batcher)
{
    u32 sz;
    TlVertexFlat *seg;
    TlVertexFlat *vtx;
    u32 i;

    g_pVBScratchXYZ->Lock(DDLOCK_WAIT, (void **)&vtx, &sz);
    ((D3DXyzVertex *)vtx)[0].x = -40.0f;
    ((D3DXyzVertex *)vtx)[0].y = -40.0f;
    ((D3DXyzVertex *)vtx)[0].z = -40.0f;
    ((D3DXyzVertex *)vtx)[1].x = 40.0f;
    ((D3DXyzVertex *)vtx)[1].y = -40.0f;
    ((D3DXyzVertex *)vtx)[1].z = -40.0f;
    ((D3DXyzVertex *)vtx)[2].x = 40.0f;
    ((D3DXyzVertex *)vtx)[2].y = 40.0f;
    ((D3DXyzVertex *)vtx)[2].z = -40.0f;
    ((D3DXyzVertex *)vtx)[3].x = -40.0f;
    ((D3DXyzVertex *)vtx)[3].y = 40.0f;
    ((D3DXyzVertex *)vtx)[3].z = -40.0f;
    ((D3DXyzVertex *)vtx)[4].x = -40.0f;
    ((D3DXyzVertex *)vtx)[4].y = 40.0f;
    ((D3DXyzVertex *)vtx)[4].z = 40.0f;
    ((D3DXyzVertex *)vtx)[5].x = -40.0f;
    ((D3DXyzVertex *)vtx)[5].y = -40.0f;
    ((D3DXyzVertex *)vtx)[5].z = 40.0f;
    ((D3DXyzVertex *)vtx)[6].x = 40.0f;
    ((D3DXyzVertex *)vtx)[6].y = -40.0f;
    ((D3DXyzVertex *)vtx)[6].z = 40.0f;
    ((D3DXyzVertex *)vtx)[7].x = 40.0f;
    ((D3DXyzVertex *)vtx)[7].y = 40.0f;
    ((D3DXyzVertex *)vtx)[7].z = 40.0f;
    ((D3DXyzVertex *)vtx)[8].x = 0.0f;
    ((D3DXyzVertex *)vtx)[8].y = 0.0f;
    ((D3DXyzVertex *)vtx)[8].z = 40.0f;
    ((D3DXyzVertex *)vtx)[9].x = 0.0f;
    ((D3DXyzVertex *)vtx)[9].y = 0.0f;
    ((D3DXyzVertex *)vtx)[9].z = 70.0f;
    g_pVBScratchXYZ->Unlock();
    Mat44 camMat;
    camMat.SetRotYXZ(-Math_Angle4096ToRadians_2(cam->rot.x), -Math_Angle4096ToRadians_2(cam->rot.y),
                     Math_Angle4096ToRadians_2(cam->rot.z));
    camMat.m[3][0] = (float)cam->viewPos[0];
    camMat.m[3][1] = (float)cam->viewPos[1];
    camMat.m[3][2] = (float)cam->viewPos[2];
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_WORLD, &camMat);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_VIEW, matView);
    SDW_RD(app->pD3DDevice)->SetTransform(D3DTRANSFORMSTATE_PROJECTION, matProj);
    g_pVBScratchTL->ProcessVertices(D3DVOP_TRANSFORM, 0, 10, g_pVBScratchXYZ, 0, SDW_RD(app->GetDevice()), 0);
    g_pVBScratchTL->Lock(DDLOCK_WAIT, (void **)&vtx, &sz);
    for (i = 0; i < 8; i++)
        vtx[i].color = 0xff << i * 2;
    vtx[8].color = 0;
    vtx[9].color = 0xff0000;
    RenderPoly poly;
    seg = new TlVertexFlat[2];
    poly.type = RPOLY_OPAQUE;
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[0], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[1], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[2], 0x18);
    batcher->SubmitTriA(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[0], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[2], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[3], 0x18);
    batcher->SubmitTriB(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[4], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[7], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[2], 0x18);
    batcher->SubmitTriC(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[3], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[4], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[2], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[0], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[1], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[6], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[0], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[6], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[5], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[0], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[5], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[4], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[0], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[4], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[3], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[2], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[7], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[6], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[2], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[6], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[1], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[4], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[5], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[6], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&((TlVertexFlat *)poly.verts)[0], &vtx[4], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[1], &vtx[6], 0x18);
    memcpy(&((TlVertexFlat *)poly.verts)[2], &vtx[7], 0x18);
    batcher->SubmitPoly(&poly);
    memcpy(&seg[0], &vtx[8], 0x18);
    memcpy(&seg[1], &vtx[9], 0x18);
    batcher->AddLines(seg, 1);
    delete seg;
    g_pVBScratchTL->Unlock();
}

void Draw_StaticInit_Empty() {}
