#include "sdw_types.h"

#define SDW_MEMBERS_PolyTri PolyTri();

#define SDW_MEMBERS_RenderPoly                                              \
    RenderPoly(); \
    RenderPoly(const RenderPoly &src);                       /* RenderPoly_CopyCtor */
#define SDW_MEMBERS_BsPolyFlat BsPolyFlat();
#define SDW_MEMBERS_BsPolyGouraud BsPolyGouraud();
#define SDW_MEMBERS_BsPolyTexFlat BsPolyTexFlat();
#define SDW_MEMBERS_BsPolyTexGouraud BsPolyTexGouraud();
#define SDW_MEMBERS_BsPolyBlendFlat BsPolyBlendFlat();
#define SDW_MEMBERS_BsPolyBlendGouraud BsPolyBlendGouraud();
class BsPolyFlat;
class BsPolyGouraud;
class BsPolyTexFlat;
class BsPolyTexGouraud;
class BsPolyBlendFlat;
class BsPolyBlendGouraud;
#include "sdw_enums.h"
#include "sdw_classes.h"
#define SDW_INLINE_POLYTRI_OPERATOR_CONST_POLYTRI 1
#include "poly_tri_inlines.h"
#undef SDW_INLINE_POLYTRI_OPERATOR_CONST_POLYTRI

#include "../sdk/crt.h"

/* ---- the .BS polygon records (BsDecode_* fills them; RenderPoly::InitFromBs* converts them) ---- */

/* A RenderPoly's vertices: 0x18-byte D3DFVF_XYZRHW|DIFFUSE|SPECULAR, or 0x20-byte with a UV (D3DTLVERTEX). The vertex
 * block is always allocated as three textured vertices; the empty constructor is what makes `new[]` loop over them
 * without calling anything. */
struct PolyVertex {
    float x, y, z, rhw;
    u32 color, specular;
};
struct PolyTexVertex {
    float x, y, z, rhw;
    u32 color, specular;
    float u, v;
    PolyTexVertex() {}
};

/* ======================================================================== RenderPoly */

/* the vertex block is always three textured (0x20-byte) vertices. */
RenderPoly::RenderPoly()
{
    verts = (float *)new PolyTexVertex[3];
}

RenderPoly::RenderPoly(const RenderPoly &src)
{
    if ((src.type & ~RPOLY_F_8000) >= RPOLY_TEXTURED_BASE)
        memcpy(verts, src.verts, 0x60);
    else
        memcpy(verts, src.verts, 0x48);
    sortZ = src.sortZ;
    poly = src.poly;
    type = src.type;
}

RenderPoly::~RenderPoly()
{
    if (verts)
        delete[] (PolyTexVertex *)verts;
}

/* operator=: textured polys (type >= 4, bit 0x8000 ignored) copy 3 x 0x20 bytes of vertex, others 3 x 0x18. */
void RenderPoly::Assign(const RenderPoly *src)
{
    if ((src->type & ~RPOLY_F_8000) >= RPOLY_TEXTURED_BASE)
        memcpy(verts, src->verts, 0x60);
    else
        memcpy(verts, src->verts, 0x48);
    sortZ = src->sortZ;
    poly = src->poly;
    type = src->type;
}

void RenderPoly::InitFromBsFlat(const BsPolyFlat *src)
{
    type = RPOLY_OPAQUE;
    poly.idx[0] = src->idx[0];
    poly.idx[1] = src->idx[1];
    poly.idx[2] = src->idx[2];
    ((PolyVertex *)verts)[0].color = ((PolyVertex *)verts)[1].color = ((PolyVertex *)verts)[2].color = src->colour;
    ((PolyVertex *)verts)[0].specular = ((PolyVertex *)verts)[1].specular = ((PolyVertex *)verts)[2].specular =
        0xff000000;
}

void RenderPoly::InitFromBsGouraud(const BsPolyGouraud *src)
{
    type = RPOLY_OPAQUE;
    poly.idx[0] = src->idx[0];
    poly.idx[1] = src->idx[1];
    poly.idx[2] = src->idx[2];
    ((PolyVertex *)verts)[0].color = src->colour[0];
    ((PolyVertex *)verts)[1].color = src->colour[1];
    ((PolyVertex *)verts)[2].color = src->colour[2];
    ((PolyVertex *)verts)[0].specular = ((PolyVertex *)verts)[1].specular = ((PolyVertex *)verts)[2].specular =
        0xff000000;
}

/* type = texture slot + 4 */
void RenderPoly::InitFromBsTexFlat(const BsPolyTexFlat *src)
{
    PolyTexVertex *v;
    type = src->texIndex + RPOLY_TEXTURED_BASE;
    poly.idx[0] = src->idx[0];
    poly.idx[1] = src->idx[1];
    poly.idx[2] = src->idx[2];
    v = (PolyTexVertex *)verts;
    v[0].u = src->uv[0];
    v[0].v = src->uv[1];
    v[1].u = src->uv[2];
    v[1].v = src->uv[3];
    v[2].u = src->uv[4];
    v[2].v = src->uv[5];
    v[0].color = v[1].color = v[2].color = src->colour;
    v[0].specular = v[1].specular = v[2].specular = 0xff000000;
}

/* type bit 0x8000 marks Gouraud (every reader masks it off) */
void RenderPoly::InitFromBsTexGouraud(const BsPolyTexGouraud *src)
{
    PolyTexVertex *v;
    type = (src->texIndex + RPOLY_TEXTURED_BASE) | RPOLY_F_8000;
    poly.idx[0] = src->idx[0];
    poly.idx[1] = src->idx[1];
    poly.idx[2] = src->idx[2];
    v = (PolyTexVertex *)verts;
    v[0].u = src->uv[0];
    v[0].v = src->uv[1];
    v[1].u = src->uv[2];
    v[1].v = src->uv[3];
    v[2].u = src->uv[4];
    v[2].v = src->uv[5];
    v[0].color = src->colour[0];
    v[1].color = src->colour[1];
    v[2].color = src->colour[2];
    v[0].specular = v[1].specular = v[2].specular = 0xff000000;
}

/* the blend mode picks the sorted list and the alpha OR'd into the colour. No default: a mode above 3
 * leaves type unchanged and ORs an uninitialised alpha. */
void RenderPoly::InitFromBsBlendFlat(const BsPolyBlendFlat *src)
{
    u32 alpha;
    switch (src->blendMode) {
        case 0:
            type = RPOLY_BLEND;
            alpha = 0x80000000;
            break;
        case 1:
        case 2:
            type = RPOLY_ADD;
            alpha = 0;
            break;
        case 3:
            type = RPOLY_ADD;
            alpha = 0xbd000000;
            break;
    }
    poly.idx[0] = src->idx[0];
    poly.idx[1] = src->idx[1];
    poly.idx[2] = src->idx[2];
    ((PolyVertex *)verts)[0].color = ((PolyVertex *)verts)[1].color = ((PolyVertex *)verts)[2].color =
        src->colour | alpha;
    ((PolyVertex *)verts)[0].specular = ((PolyVertex *)verts)[1].specular = ((PolyVertex *)verts)[2].specular =
        0xff000000;
}

void RenderPoly::InitFromBsBlendGouraud(const BsPolyBlendGouraud *src)
{
    u32 alpha;
    switch (src->blendMode) {
        case 0:
            type = RPOLY_BLEND;
            alpha = 0x80000000;
            break;
        case 1:
        case 2:
            type = RPOLY_ADD;
            alpha = 0;
            break;
        case 3:
            type = RPOLY_ADD;
            alpha = 0xbd000000;
            break;
    }
    poly.idx[0] = src->idx[0];
    poly.idx[1] = src->idx[1];
    poly.idx[2] = src->idx[2];
    ((PolyVertex *)verts)[0].color = src->colour[0] | alpha;
    ((PolyVertex *)verts)[1].color = src->colour[1] | alpha;
    ((PolyVertex *)verts)[2].color = src->colour[2] | alpha;
    ((PolyVertex *)verts)[0].specular = ((PolyVertex *)verts)[1].specular = ((PolyVertex *)verts)[2].specular =
        0xff000000;
}
