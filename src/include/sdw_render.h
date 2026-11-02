/* The game's Direct3D call sites reach g_renderDevice, the RenderDevice of the chosen backend, through these:
 *
 *   SDW_RD(dev)->Method(...)   a device call; dev is the original's receiver (pD3DDevice, app->GetDevice()...)
 *   SdwVertexBuffer            the vertex buffer type (IDirect3DVertexBuffer7 / RdVertexBuffer)
 *   SdwTexture                 a texture page's handle (IDirectDrawSurface7 / RdTexture): Texture.surface, HoleFX.captureSurface
 *   SDW_RDTEX(surface)         a texture page's surface as the texture handle SetTexture takes
 *   SDW_RSF_SET_HOOK(flags)    first statement of each copy of Render_SetStateFlags / ClearStateFlags (the original
 *   SDW_RSF_CLEAR_HOOK(flags)  compiler inlined them, so the source has several): the copy's work goes to the backend
 *   SDW_FOG_HOOK(color, start, end), SDW_FOGCOLOR_HOOK(color)   the same for the fog helpers (frustrum.cpp)
 *   SDW_RD_CREATEVB(d3d, desc, out)   IDirect3D7::CreateVertexBuffer / RenderDevice::CreateVertexBuffer */
#ifndef SDW_RENDER_H
#define SDW_RENDER_H

#include "../render/render_device.h"
typedef RdVertexBuffer SdwVertexBuffer;
typedef RdTexture SdwTexture;
#define SDW_RD(dev) g_renderDevice
#define SDW_RDTEX(surface) ((RdTexture *)(surface))
#define SDW_RSF_SET_HOOK(flags)                                                                                        \
    {                                                                                                                  \
        g_renderDevice->SetStateFlags(flags);                                                                          \
        return;                                                                                                        \
    }
#define SDW_RSF_CLEAR_HOOK(flags)                                                                                      \
    {                                                                                                                  \
        g_renderDevice->ClearStateFlags(flags);                                                                        \
        return;                                                                                                        \
    }
#define SDW_FOG_HOOK(color, start, end)                                                                                \
    {                                                                                                                  \
        g_renderDevice->SetFog(color, start, end);                                                                     \
        return;                                                                                                        \
    }
#define SDW_FOGCOLOR_HOOK(color)                                                                                       \
    {                                                                                                                  \
        g_renderDevice->SetFogColor(color);                                                                            \
        return;                                                                                                        \
    }
#define SDW_RD_CREATEVB(d3d, desc, out)                                                                                \
    g_renderDevice->CreateVertexBuffer((desc)->dwCaps, (desc)->dwFVF, (desc)->dwNumVertices, (out))

#endif
