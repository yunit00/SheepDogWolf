/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_POLYBATCHER_SUBMITPOLYINLINE_RENDERPOLY) && \
    !defined(SDW_INLINE_POLYBATCHER_SUBMITPOLYINLINE_RENDERPOLY_DEFINED)
#define SDW_INLINE_POLYBATCHER_SUBMITPOLYINLINE_RENDERPOLY_DEFINED
#if SDW_INLINE_POLYBATCHER_SUBMITPOLYINLINE_RENDERPOLY == 1
inline void PolyBatcher::SubmitPolyInline(RenderPoly *poly)
{
    switch (poly->type) {
        case RPOLY_OPAQUE:
            if (flatBatchCount <= batchCapacity) {
                memcpy((FlatVertex *)flatBatchVerts + flatBatchCount * 3, poly->verts, 0x48);
                ++flatBatchCount;
            }
            if (flatBatchCount >= batchCapacity) {
                renderer->Render_SetStateFlags(stateFlags);
                renderer->DrawTriangleList(flatBatchVerts, batchCapacity * 3);
                renderer->ClearStateFlagsInline(stateFlags);
                flatBatchCount = 0;
                textureDirty = 1;
            }
            break;
        case RPOLY_BLEND:
            if (computeSortZ == 1)
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];
            sortedPolys[sortedCount].Assign(poly);
            sortedList[sortedCount] = &sortedPolys[sortedCount];
            ++sortedCount;
            break;
        case RPOLY_ADD:
            if (computeSortZ == 1)
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];
            sortedPolys[sortedCount].Assign(poly);
            sortedList[sortedCount] = &sortedPolys[sortedCount];
            ++sortedCount;
            break;
        default:
            FlameVertex *vertices = (FlameVertex *)poly->verts;
            u32 texture = (poly->type - RPOLY_TEXTURED_BASE) & ~RPOLY_F_8000;
            if (texture < immediateTexCount) {
                u32 *flags;
                {
                    FlameVertex *batch = (FlameVertex *)texBatchVerts[texture];
                    {
                        u32 *count = &texBatchCounts[texture];
                        flags = &texStateFlags[texture];
                        if (*count <= batchCapacity) {
                            memcpy(batch + *count * 3, vertices, 0x60);
                            ++*count;
                        }
                        if (*count >= batchCapacity) {
                            if (texture != lastTextureIndex || textureDirty == 1) {
                                renderer->Render_SetTexture(textures[texture], 0);
                                lastTextureIndex = texture;
                                textureDirty = 0;
                            }
                            renderer->Render_SetStateFlags(*flags);
                            renderer->DrawTexturedTriangleList(batch, batchCapacity * 3);
                            renderer->Render_ClearStateFlags(*flags);
                            *count = 0;
                        }
                    }
                }
            } else {
                if (computeSortZ == 1)
                    poly->sortZ = vertices[0].z + vertices[1].z + vertices[2].z;
                sortedPolys[sortedCount].Assign(poly);
                sortedList[sortedCount] = &sortedPolys[sortedCount];
                ++sortedCount;
            }
    }
}
#elif SDW_INLINE_POLYBATCHER_SUBMITPOLYINLINE_RENDERPOLY == 2
__forceinline void PolyBatcher::SubmitPolyInline(RenderPoly *poly)
{
    switch (poly->type) {
        case RPOLY_OPAQUE:
            if (flatBatchCount <= batchCapacity) {
                memcpy((FlatVertex *)flatBatchVerts + flatBatchCount * 3, poly->verts, 0x48);
                ++flatBatchCount;
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
            ++sortedCount;
            break;
        case RPOLY_ADD:
            if (computeSortZ == 1)
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];
            sortedPolys[sortedCount].Assign(poly);
            sortedList[sortedCount] = &sortedPolys[sortedCount];
            ++sortedCount;
            break;
        default:
            TlVertex *vertices = (TlVertex *)poly->verts;
            u32 texture = (poly->type - RPOLY_TEXTURED_BASE) & ~RPOLY_F_8000;
            if (texture < immediateTexCount) {
                u32 *flags;
                {
                    TlVertex *batch = (TlVertex *)texBatchVerts[texture];
                    {
                        u32 *count = &texBatchCounts[texture];
                        flags = &texStateFlags[texture];
                        if (*count <= batchCapacity) {
                            memcpy(batch + *count * 3, vertices, 0x60);
                            ++*count;
                        }
                        if (*count >= batchCapacity) {
                            if (texture != lastTextureIndex || textureDirty == 1) {
                                renderer->SetTextureInline(textures[texture], 0);
                                lastTextureIndex = texture;
                                textureDirty = 0;
                            }
                            renderer->Render_SetStateFlags(*flags);
                            renderer->DrawPrimitiveInline(
                                D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                batch, batchCapacity * 3);
                            renderer->Render_ClearStateFlags(*flags);
                            *count = 0;
                        }
                    }
                }
            } else {
                if (computeSortZ == 1)
                    poly->sortZ = vertices[0].z + vertices[1].z + vertices[2].z;
                sortedPolys[sortedCount].Assign(poly);
                sortedList[sortedCount] = &sortedPolys[sortedCount];
                ++sortedCount;
            }
    }
}
#elif SDW_INLINE_POLYBATCHER_SUBMITPOLYINLINE_RENDERPOLY == 3
inline void PolyBatcher::SubmitPolyInline(RenderPoly *poly)
{
    switch (poly->type) {
        case RPOLY_OPAQUE:
            if (flatBatchCount <= batchCapacity) {
                memcpy((FlatVertex *)flatBatchVerts + flatBatchCount * 3, poly->verts, 0x48);
                ++flatBatchCount;
            }
            if (flatBatchCount >= batchCapacity) {
                renderer->Render_SetStateFlags(stateFlags);
                renderer->DrawTriangleList(flatBatchVerts, batchCapacity * 3);
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
            ++sortedCount;
            break;
        case RPOLY_ADD:
            if (computeSortZ == 1)
                poly->sortZ = poly->verts[2] + poly->verts[8] + poly->verts[14];
            sortedPolys[sortedCount].Assign(poly);
            sortedList[sortedCount] = &sortedPolys[sortedCount];
            ++sortedCount;
            break;
        default:
            FlameVertex *vertices = (FlameVertex *)poly->verts;
            u32 texture = (poly->type - 4) & ~RPOLY_F_8000;
            if (texture < immediateTexCount) {
                u32 *flags;
                {
                    FlameVertex *batch = (FlameVertex *)texBatchVerts[texture];
                    {
                        u32 *count = &texBatchCounts[texture];
                        flags = &texStateFlags[texture];
                        if (*count <= batchCapacity) {
                            memcpy(batch + *count * 3, vertices, 0x60);
                            ++*count;
                        }
                        if (*count >= batchCapacity) {
                            if (texture != lastTextureIndex || textureDirty == 1) {
                                renderer->Render_SetTexture(textures[texture], 0);
                                lastTextureIndex = texture;
                                textureDirty = 0;
                            }
                            renderer->Render_SetStateFlags(*flags);
                            renderer->Render_DrawPrimitive(
                                D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1,
                                batch, batchCapacity * 3);
                            renderer->Render_ClearStateFlags(*flags);
                            *count = 0;
                        }
                    }
                }
            } else {
                if (computeSortZ == 1)
                    poly->sortZ = vertices[0].z + vertices[1].z + vertices[2].z;
                sortedPolys[sortedCount].Assign(poly);
                sortedList[sortedCount] = &sortedPolys[sortedCount];
                ++sortedCount;
            }
    }
}
#endif
#endif
