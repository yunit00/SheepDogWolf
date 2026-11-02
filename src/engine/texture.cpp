/*
 * The Texture object: an IDirectDrawSurface7 texture page, created blank, from a .bmp file, or from a .bmp file converted
 * to a chosen 16-bit pixel format, with its accessors and Lock / Unlock wrappers.
 *
 * The Texture constructors follow the DirectX 7 SDK's d3dtextr.cpp (TextureContainer::CreateFromBitmap): the device's
 * caps decide power-of-two and square sizes, a HAL / T&L HAL device gets a managed texture and anything else a
 * system-memory one, and IDirect3DDevice7::EnumTextureFormats picks the pixel format through
 * Texture_EnumPixelFormatCallback, which reads the wanted format index from g_texRequestedFormat:
 * 0 = R5G6B5, 1 = A1R5G5B5, 2 = A4R4G4B4. Result codes written to *result: 0 ok, 1 no device, 2 bitmap not loaded,
 * 3 no matching pixel format (or unknown format index), 4 out of memory creating the surface, 5 no DC.
 *
 * Defects visible here (no in-game consequence established): the .bmp constructor never DeleteObject's the DIB section
 * LoadImageA returns, and its GetDC failure path Releases the surface without clearing `surface`, so the destructor
 * Releases it a second time; every early return after GetDDInterface leaks the IDirectDraw7 reference. The converting
 * constructor sets up nothing when the temporary load fails (*result != 0): `surface` is left uninitialised for the
 * destructor, and the temporary Texture is leaked, as are it and its lock on the later early returns.
 */

#include "sdw_enums.h"
#include "../sdk/win32.h"
#include "../sdk/ddraw.h"
#include "../sdk/d3d7.h"
#include "../sdk/crt.h"

struct IDirectInputDevice8A;

#define SDW_MEMBERS_Texture                                                                                    \
    Texture(D3DApp *app, u32 width, u32 height, u32 format, s32 *result); \
    Texture(const char *fileName, D3DApp *app, u32 width, u32 height, s32 *result); \
    Texture(const char *fileName, D3DApp *app, u32 width, u32 height, u32 format, s32 *result); \
    void Surface_LockForRead(DDSURFACEDESC2 *desc); \
    long Surface_LockForWrite(DDSURFACEDESC2 *desc); \
    void Surface_LockReadWrite(DDSURFACEDESC2 *desc);
#include "sdw_classes.h"

/* ======================================================================== Texture */

/* the SDK's C++ inline (guiddef.h), as in src/app/d3dapp.cpp */
inline int IsEqualGUID(const GUID &rguid1, const GUID &rguid2)
{
    return !memcmp(&rguid1, &rguid2, sizeof(GUID));
}

/* the pixel-format index the constructors ask Texture_EnumPixelFormatCallback for (0 R5G6B5, 1 A1R5G5B5,
 * 2 A4R4G4B4). Only this file uses it. */
u32 g_texRequestedFormat;

HRESULT __stdcall Texture_EnumPixelFormatCallback(DDPIXELFORMAT *pddpf, void *pOutPixelFormat);
u16 Texture_CountMaskBits(u32 mask);

#define DESC (*(DDSURFACEDESC2 *)&desc)

/* ---- the page is the renderer's (src/render/render_device.h), which does what the DirectDraw code
 * below does in the original. The .bmp constructors are not supported: only PolyBatcher::CreateTexture
 * uses them, and nothing calls it. ---- */

/* a blank texture page of (at least) width x height in the requested format. */
Texture::Texture(D3DApp *app, u32 width, u32 height, u32 format, s32 *result)
{
    surface = NULL;
    memset(&desc, 0, sizeof(desc));
    if (!app->deviceReady) {
        *result = TEXRES_NO_DEVICE;
        return;
    }
    *result = g_renderDevice->CreateTexture(width, height, format, &surface, &this->width, &this->height);
    if (*result == TEXRES_NO_PIXEL_FORMAT)
        return; /* as the original: the size is set, the format is not */
    this->format = format;
    DESC.dwSize = sizeof(DDSURFACEDESC2); /* the record other code reads the size from (pause_menu.cpp) */
    DESC.dwWidth = this->width;
    DESC.dwHeight = this->height;
}

Texture::Texture(const char *fileName, D3DApp *app, u32 width, u32 height, s32 *result)
{
    surface = NULL;
    memset(&desc, 0, sizeof(desc));
    *result = TEXRES_NO_BITMAP;
}

Texture::Texture(const char *fileName, D3DApp *app, u32 width, u32 height, u32 format, s32 *result)
{
    surface = NULL;
    memset(&desc, 0, sizeof(desc));
    *result = TEXRES_NO_BITMAP;
}

Texture::~Texture()
{
    if (surface && g_renderDevice)
        g_renderDevice->ReleaseTexture(surface);
}

u32 Texture::GetWidth()
{
    return width;
}

u32 Texture::GetHeight()
{
    return height;
}

u32 Texture::GetFormat()
{
    return format;
}

SdwTexture *Texture::GetSurface()
{
    return surface;
}

/* The lock functions fill the fields of the DDSURFACEDESC2 their callers read: lpSurface, lPitch, dwWidth, dwHeight. */
static long Texture_Lock(SdwTexture *surface, u32 access, DDSURFACEDESC2 *desc)
{
    RdLockedRect locked;
    long hr;
    desc->dwSize = sizeof(DDSURFACEDESC2);
    hr = g_renderDevice->LockTexture(surface, access, &locked);
    if (hr >= 0) {
        desc->lpSurface = locked.pixels;
        desc->lPitch = locked.pitch;
        desc->dwWidth = locked.width;
        desc->dwHeight = locked.height;
    }
    return hr;
}

void Texture::Surface_LockForRead(DDSURFACEDESC2 *desc)
{
    Texture_Lock(surface, RD_TEXLOCK_READ, desc);
}

long Texture::Surface_LockForWrite(DDSURFACEDESC2 *desc)
{
    return Texture_Lock(surface, RD_TEXLOCK_WRITE, desc);
}

void Texture::Surface_LockReadWrite(DDSURFACEDESC2 *desc)
{
    Texture_Lock(surface, RD_TEXLOCK_READWRITE, desc);
}

void Texture::Surface_Unlock()
{
    g_renderDevice->UnlockTexture(surface);
}

/* IDirect3DDevice7::EnumTextureFormats callback: takes the first plain 16-bit RGB format whose channel widths
 * fit g_texRequestedFormat (5-6-5; 5-5-5 with alpha; 4-4-4 with alpha), copies it out and stops. An unknown index
 * stops at the first 16-bit RGB format without copying anything. */
HRESULT __stdcall Texture_EnumPixelFormatCallback(DDPIXELFORMAT *pddpf, void *pOutPixelFormat)
{
    HRESULT ret;
    ret = DDENUMRET_OK;
    if (pddpf->dwRGBBitCount == 16 && pddpf->dwFourCC == 0 &&
        !(pddpf->dwFlags & (DDPF_LUMINANCE | DDPF_BUMPLUMINANCE | DDPF_BUMPDUDV))) {
        u16 rBits = Texture_CountMaskBits(pddpf->dwRBitMask);
        u16 greenBits = Texture_CountMaskBits(pddpf->dwGBitMask);
        switch (g_texRequestedFormat) {
            case TEXFMT_RGB565:
                if (rBits == 5 && greenBits == 6) {
                    memcpy(pOutPixelFormat, pddpf, sizeof(DDPIXELFORMAT));
                    ret = DDENUMRET_CANCEL;
                }
                break;
            case TEXFMT_ARGB1555:
                if (rBits == 5 && greenBits == 5 && pddpf->dwRGBAlphaBitMask) {
                    memcpy(pOutPixelFormat, pddpf, sizeof(DDPIXELFORMAT));
                    ret = DDENUMRET_CANCEL;
                }
                break;
            case TEXFMT_ARGB4444:
                if (rBits == 4 && greenBits == 4 && pddpf->dwRGBAlphaBitMask) {
                    memcpy(pOutPixelFormat, pddpf, sizeof(DDPIXELFORMAT));
                    ret = DDENUMRET_CANCEL;
                }
                break;
            default:
                ret = DDENUMRET_CANCEL;
        }
    }
    return ret;
}

/* the number of set bits in mask. */
u16 Texture_CountMaskBits(u32 mask)
{
    u16 n;
    n = 0;
    while (mask) {
        mask &= mask - 1;
        n++;
    }
    return n;
}

/* Texture_ScalarDeletingDtor: generated by the compiler from the virtual destructor. */
