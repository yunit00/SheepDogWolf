/*
 * The mesh layer: MeshAnimSeq and AnimMesh, and Mesh up to Mesh_TransformRange; this object is the AnimMesh part.
 *
 * A Mesh is one Black Sheep geometry frame turned into two D3D7 vertex buffers (vbPositions: D3DFVF_XYZ, vbTransformed:
 * D3DFVF_XYZRHW, the ProcessVertices destination) plus a RenderPoly per face. Mesh::BuildFromBsFile reads the frame's
 * vertices and its six face kinds (flat, Gouraud, textured flat / Gouraud, blended flat / Gouraud; a quad entry counts
 * as two triangles) into temporary BsPoly arrays and converts every one into a RenderPoly. An AnimMesh adds a part
 * hierarchy (MeshPart, one world matrix each) and animation sequences (MeshAnimSeq, frames of per-part poses);
 * AnimMesh::DrawAll steps the animation on a private wall-clock Timer and transforms each part's vertex range with its
 * own matrix.
 *
 * Shapes:
 *  - AnimMesh::BuildFromBsFile, GetFlag and Mesh::BuildFromBsFile return through a u8 local, so they are written
 *    single-exit with nested if / else.
 */
#include "sdw_types.h"

class Mat44;
#include "sdw_enums.h"
#include "../sdk/d3d7.h"
#include "../sdk/crt.h"

#include "timer.h"
#define SDW_MEMBERS_Mat44 Mat44();
#define SDW_MEMBERS_Vec3f \
    Vec3f() {}
#define SDW_MEMBERS_D3DApp                  \
    void SetTransform(u32 state, Mat44 *m); \
    IDirect3DDevice7 *GetDevice();          \
    long CreateVBWithResult(D3DVERTEXBUFFERDESC *desc, SdwVertexBuffer **out);
#define SDW_MEMBERS_MeshPart MeshPart();
#define SDW_MEMBERS_MeshAnimSeq MeshAnimSeq();
#define SDW_MEMBERS_BsPolyFlat BsPolyFlat();
#define SDW_MEMBERS_BsPolyGouraud BsPolyGouraud();
#define SDW_MEMBERS_BsPolyTexFlat BsPolyTexFlat();
#define SDW_MEMBERS_BsPolyTexGouraud BsPolyTexGouraud();
#define SDW_MEMBERS_BsPolyBlendFlat BsPolyBlendFlat();
#define SDW_MEMBERS_BsPolyBlendGouraud BsPolyBlendGouraud();
#define SDW_MEMBERS_RenderPoly RenderPoly();
#define SDW_MEMBERS_Mesh Mesh();
#define SDW_MEMBERS_AnimMesh AnimMesh();
#include "sdw_classes.h"
#define SDW_INLINE_D3DAPP_SETTRANSFORM_U32_MAT44 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_SETTRANSFORM_U32_MAT44
#define SDW_INLINE_D3DAPP_GETDEVICE 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_GETDEVICE
#define SDW_INLINE_D3DAPP_CREATEVBWITHRESULT_D3DVERTEXBUFFERDESC_IDIRECT3DVERTEXBUFFER7 1
#include "../app/d3dapp_inlines.h"
#undef SDW_INLINE_D3DAPP_CREATEVBWITHRESULT_D3DVERTEXBUFFERDESC_IDIRECT3DVERTEXBUFFER7

/* Bs_IsGeometryResource is a BsFile member (declared above). */

/* ---------------------------------------------------------------- AnimMesh */

/* a private Timer drives the animation in wall-clock milliseconds (stopped until the first update). */
AnimMesh::AnimMesh()
{
    animTimer = new Timer;
    animTimer->Stop();
    requestedSeq = -1;
    currentSeq = -1;
    frameIndex = 0;
    elapsedMs = 0;
    frameDurationMs = 1.0f;
    playing = 0;
}

AnimMesh::~AnimMesh() {}

/* vtable slot 1. Accepts only a type-4 (animated) frame of an opened file: reads the part table and the
 * animation names, then builds the geometry like any Mesh. */
u8 AnimMesh::BuildFromBsFile(D3DApp *app, BsFile *file)
{
    u8 result;

    if (file->frameType != WAR_RES_MODEL || !file->ok) {
        result = 0;
    } else {
        partCount = file->Type4_GetField18();
        parts = new MeshPart[partCount];
        file->Type4_ReadTable14(parts);
        seqCount = file->Type4_GetTableCount();
        sequences = new MeshAnimSeq[seqCount];
        file->ReadAnimNames(sequences, partCount);
        result = Mesh::BuildFromBsFile(app, file);
    }
    return result;
}

/* vtable slot 3. Empty: each part needs its own world matrix, so the whole-mesh transform does not apply. */
void AnimMesh::TransformAll() {}

/* vtable slot 5. Steps the animation, then builds the part matrices root first (a child's parent always has a
 * lower index) and transforms each part's vertex range under its own world matrix. */
void AnimMesh::DrawAll(Mat44 *world, Mat44 *view, Mat44 *proj)
{
    u32 firstVertex = 0;
    Mat44 idMat;
    float t;
    u32 part;
    u32 parentIdx;

    if (playing == 1 || currentSeq != requestedSeq)
        UpdateAnim();
    t = elapsedMs / frameDurationMs;
    app->SetTransform(D3DTRANSFORMSTATE_VIEW, view);
    app->SetTransform(D3DTRANSFORMSTATE_PROJECTION, proj);
    idMat.SetIdentity();
    parts->BuildMatrices(world, &idMat, t);
    app->SetTransform(D3DTRANSFORMSTATE_WORLD, parts->matrix);
    vbTransformed->ProcessVertices(1, firstVertex, parts->vertexCount, vbPositions, firstVertex, SDW_RD(app->GetDevice()), 0);
    firstVertex += parts->vertexCount;
    for (part = 1; part < partCount; part++) {
        parentIdx = parts[part].parentIndex;
        parts[part].BuildMatrices(parts[parentIdx].localMatrix, parts[parentIdx].scaleMatrix, t);
        app->SetTransform(D3DTRANSFORMSTATE_WORLD, parts[part].matrix);
        vbTransformed->ProcessVertices(1, firstVertex, parts[part].vertexCount, vbPositions, firstVertex,
                                       SDW_RD(app->GetDevice()), 0);
        firstVertex += parts[part].vertexCount;
    }
}

/* no callers in the binary. */
void AnimMesh::SetSequence(s32 seq, u8 loop, u8 blend)
{
    requestedSeq = seq;
    this->loop = loop;
    this->blend = blend;
}

/* empty, no callers. */
void AnimMesh::Stub_40bac3() {}

/* empty, no callers. */
void AnimMesh::Stub_40bace() {}

/* 0 -> the loop flag, 1 -> the blend flag. No callers. */
u8 AnimMesh::GetFlag(char which)
{
    u8 result;

    switch (which) {
        case 0:
            result = loop;
            break;
        case 1:
            result = blend;
            break;
        default:
            result = 0;
    }
    return result;
}

/* the animation state machine (see the AnimMesh.csv field notes). */
void AnimMesh::UpdateAnim()
{
    float delta;

    if (!playing) {
        currentSeq = requestedSeq;
        frameIndex = 0;
        ApplyFrame(&sequences[currentSeq].frames[frameIndex], 0.0f);
        frameDurationMs = sequences[currentSeq].frames[frameIndex].halfDurationMs * 2.0f;
        frameIndex++;
        elapsedMs = 0;
        playing = 1;
        animTimer->Start();
    } else {
        delta = animTimer->GetDelta(TIMER_MILLISECONDS);
        elapsedMs = delta + elapsedMs;
        if (currentSeq == requestedSeq) {
            if (elapsedMs >= frameDurationMs) {
                elapsedMs -= frameDurationMs;
                if (frameIndex >= sequences[currentSeq].frameCount) {
                    frameIndex = 0;
                    playing = loop;
                }
                ApplyFrame(&sequences[currentSeq].frames[frameIndex], 1.0f);
                frameDurationMs = sequences[currentSeq].frames[frameIndex].halfDurationMs * 2.0f;
                frameIndex++;
            }
        } else if (elapsedMs >= frameDurationMs) {
            elapsedMs -= frameDurationMs;
            currentSeq = requestedSeq;
            frameIndex = 0;
            if (blend == 1) {
                ApplyFrame(&sequences[currentSeq].frames[frameIndex], 1.0f);
            } else {
                CapturePose(&sequences[currentSeq].frames[frameIndex++]);
                SetBlendTarget(&sequences[currentSeq].frames[frameIndex]);
            }
            frameDurationMs = sequences[currentSeq].frames[frameIndex].halfDurationMs * 2.0f;
            frameIndex++;
        } else {
            currentSeq = requestedSeq;
            frameIndex = 0;
            if (blend == 1) {
                ApplyFrame(&sequences[currentSeq].frames[frameIndex], elapsedMs / frameDurationMs);
            } else {
                CapturePose(&sequences[currentSeq].frames[frameIndex++]);
                SetBlendTarget(&sequences[currentSeq].frames[frameIndex]);
            }
            frameDurationMs = sequences[currentSeq].frames[frameIndex].halfDurationMs * 2.0f;
            frameIndex++;
            elapsedMs = 0;
        }
    }
}

void AnimMesh::ApplyFrame(MeshAnimFrame *frame, float weight)
{
    MeshPartPose *poses = frame->poses;
    u32 k;

    for (k = 0; k < partCount; k++)
        parts[k].BlendThenSetTarget(&poses[k], weight);
}

void AnimMesh::CapturePose(MeshAnimFrame *frame)
{
    MeshPartPose *poses = frame->poses;
    u32 k;

    for (k = 0; k < partCount; k++)
        parts[k].SetPose(&poses[k]);
}

void AnimMesh::SetBlendTarget(MeshAnimFrame *frame)
{
    MeshPartPose *poses = frame->poses;
    u32 k;

    for (k = 0; k < partCount; k++)
        parts[k].SetTargetPose(&poses[k]);
}

/* ---------------------------------------------------------------- Mesh */
