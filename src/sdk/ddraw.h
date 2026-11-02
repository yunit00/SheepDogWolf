/* Only what src/ uses is here: constants, types and functions (depends on sdw_types.h). */
#ifndef SDW_SDK_DDRAW_H
#define SDW_SDK_DDRAW_H

#define DDBD_16 0x00000400l
#define DDBD_24 0X00000200l
#define DDBD_32 0x00000100l
#define DDBLT_WAIT 0x01000000l
#define DDCAPS2_CANRENDERWINDOWED 0x00080000l
#define DDENUMRET_CANCEL 0
#define DDENUMRET_OK 1
#define DDENUM_ATTACHEDSECONDARYDEVICES 0x00000001L
#define DDENUM_DETACHEDSECONDARYDEVICES 0x00000002L
#define DDENUM_NONDISPLAYDEVICES 0x00000004L
#define DDERR_OUTOFVIDEOMEMORY ((HRESULT)0x8876017CL) /* MAKE_DDHRESULT(380) */
#define DDERR_SURFACELOST ((HRESULT)0x887601C2L)      /* MAKE_DDHRESULT(450) */
#define DDERR_INVALIDMODE ((HRESULT)0x88760078L)      /* MAKE_DDHRESULT(120) */
#define DDFLIP_WAIT 0x00000001l
#define DDLOCK_NOSYSLOCK 0x00000800L
#define DDLOCK_READONLY 0x00000010L
#define DDLOCK_WAIT 0x00000001L
#define DDLOCK_WRITEONLY 0x00000020L
#define DDPF_BUMPDUDV 0x00080000l
#define DDPF_BUMPLUMINANCE 0x00040000l
#define DDPF_LUMINANCE 0x00020000l
#define DDPF_ZBUFFER 0x00000400l
#define DDSCAPS2_HINTDYNAMIC 0x00000004L
#define DDSCAPS2_TEXTUREMANAGE 0x00000010L
#define DDSCAPS_3DDEVICE 0x00002000l
#define DDSCAPS_BACKBUFFER 0x00000004l
#define DDSCAPS_COMPLEX 0x00000008l
#define DDSCAPS_FLIP 0x00000010l
#define DDSCAPS_OFFSCREENPLAIN 0x00000040l
#define DDSCAPS_PRIMARYSURFACE 0x00000200l
#define DDSCAPS_SYSTEMMEMORY 0x00000800l
#define DDSCAPS_TEXTURE 0x00001000l
#define DDSCAPS_VIDEOMEMORY 0x00004000l
#define DDSCAPS_ZBUFFER 0x00020000l
#define DDSCL_EXCLUSIVE 0x00000010l
#define DDSCL_FULLSCREEN 0x00000001l
#define DDSCL_NORMAL 0x00000008l
#define DDSD_BACKBUFFERCOUNT 0x00000020l
#define DDSD_CAPS 0x00000001l
#define DDSD_HEIGHT 0x00000002l
#define DDSD_PIXELFORMAT 0x00001000l
#define DDSD_TEXTURESTAGE 0x00100000l
#define DDSD_WIDTH 0x00000004l

#include "windef.h"

struct DDCAPS;
struct DDCOLORKEY;
struct DDPIXELFORMAT;
struct DDSCAPS2;
struct DDSURFACEDESC2;
struct IDirectDrawClipper;
struct IDirectDrawSurface7;

struct DDSCAPS2 {
    DWORD dwCaps, dwCaps2, dwCaps3, dwCaps4;
};

struct DDPIXELFORMAT { /* 0x20 bytes */
    DWORD dwSize, dwFlags, dwFourCC, dwRGBBitCount;
    DWORD dwRBitMask, dwGBitMask, dwBBitMask, dwRGBAlphaBitMask;
};

struct DDCOLORKEY {
    DWORD dwColorSpaceLowValue, dwColorSpaceHighValue;
};

struct DDSURFACEDESC2 { /* 0x7c bytes */
    DWORD dwSize, dwFlags, dwHeight, dwWidth;
    union {
        LONG lPitch;
        DWORD dwLinearSize;
    };
    DWORD dwBackBufferCount;
    union {
        DWORD dwMipMapCount;
        DWORD dwRefreshRate;
    };
    DWORD dwAlphaBitDepth, dwReserved;
    void *lpSurface;
    DDCOLORKEY ddckCKDestOverlay, ddckCKDestBlt, ddckCKSrcOverlay, ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat; /* dwRGBBitCount +0x54 */
    DDSCAPS2 ddsCaps;
    DWORD dwTextureStage;
};

struct DDCAPS { /* 0x17c bytes; only dwSize and dwCaps2 are used */
    DWORD dwSize, dwCaps, dwCaps2;
    u8 rest[0x17c - 0xc];
};

typedef HRESULT(__stdcall *LPDDENUMMODESCALLBACK2)(DDSURFACEDESC2 *desc, void *context);

typedef BOOL(__stdcall *LPDDENUMCALLBACKEXA)(GUID *guid, char *description, char *name, void *context, void *monitor);

struct IDirectDrawSurface7 : IUnknown {
    virtual HRESULT __stdcall AddAttachedSurface(IDirectDrawSurface7 *surface) = 0;
    virtual HRESULT __stdcall AddOverlayDirtyRect(RECT *rect) = 0;
    virtual HRESULT __stdcall Blt(RECT *dst, IDirectDrawSurface7 *src, RECT *srcRect, DWORD flags,
                                  void *fx) = 0;
    virtual HRESULT __stdcall BltBatch(void *batch, DWORD count, DWORD flags) = 0;
    virtual HRESULT __stdcall BltFast(DWORD x, DWORD y, IDirectDrawSurface7 *src, RECT *srcRect,
                                      DWORD trans) = 0;
    virtual HRESULT __stdcall DeleteAttachedSurface(DWORD flags, IDirectDrawSurface7 *surface) = 0;
    virtual HRESULT __stdcall EnumAttachedSurfaces(void *context, void *callback) = 0;
    virtual HRESULT __stdcall EnumOverlayZOrders(DWORD flags, void *context, void *callback) = 0;
    virtual HRESULT __stdcall Flip(IDirectDrawSurface7 *target, DWORD flags) = 0;
    virtual HRESULT __stdcall GetAttachedSurface(DDSCAPS2 *caps, IDirectDrawSurface7 **surface) = 0;
    virtual HRESULT __stdcall GetBltStatus(DWORD flags) = 0;
    virtual HRESULT __stdcall GetCaps(DDSCAPS2 *caps) = 0;
    virtual HRESULT __stdcall GetClipper(IDirectDrawClipper **clipper) = 0;
    virtual HRESULT __stdcall GetColorKey(DWORD flags, DDCOLORKEY *key) = 0;
    virtual HRESULT __stdcall GetDC(HDC *hdc) = 0;
    virtual HRESULT __stdcall GetFlipStatus(DWORD flags) = 0;
    virtual HRESULT __stdcall GetOverlayPosition(LONG *x, LONG *y) = 0;
    virtual HRESULT __stdcall GetPalette(void **palette) = 0;
    virtual HRESULT __stdcall GetPixelFormat(DDPIXELFORMAT *format) = 0;
    virtual HRESULT __stdcall GetSurfaceDesc(DDSURFACEDESC2 *desc) = 0;
    virtual HRESULT __stdcall Initialize(void *dd, DDSURFACEDESC2 *desc) = 0;
    virtual HRESULT __stdcall IsLost() = 0;
    virtual HRESULT __stdcall Lock(RECT *rect, DDSURFACEDESC2 *desc, DWORD flags, void *event) = 0;
    virtual HRESULT __stdcall ReleaseDC(HDC hdc) = 0;
    virtual HRESULT __stdcall Restore() = 0;
    virtual HRESULT __stdcall SetClipper(IDirectDrawClipper *clipper) = 0;
    virtual HRESULT __stdcall SetColorKey(DWORD flags, DDCOLORKEY *key) = 0;
    virtual HRESULT __stdcall SetOverlayPosition(LONG x, LONG y) = 0;
    virtual HRESULT __stdcall SetPalette(void *palette) = 0;
    virtual HRESULT __stdcall Unlock(RECT *rect) = 0;
    virtual HRESULT __stdcall UpdateOverlay(RECT *src, IDirectDrawSurface7 *dst, RECT *dstRect, DWORD flags,
                                            void *fx) = 0;
    virtual HRESULT __stdcall UpdateOverlayDisplay(DWORD flags) = 0;
    virtual HRESULT __stdcall UpdateOverlayZOrder(DWORD flags, IDirectDrawSurface7 *ref) = 0;
    virtual HRESULT __stdcall GetDDInterface(void **dd) = 0;
};

struct IDirectDrawClipper : IUnknown {
    virtual HRESULT __stdcall GetClipList(RECT *rect, void *clipList, DWORD *size) = 0;
    virtual HRESULT __stdcall GetHWnd(HWND *hWnd) = 0;
    virtual HRESULT __stdcall Initialize(void *dd, DWORD flags) = 0;
    virtual HRESULT __stdcall IsClipListChanged(BOOL *changed) = 0;
    virtual HRESULT __stdcall SetClipList(void *clipList, DWORD flags) = 0;
    virtual HRESULT __stdcall SetHWnd(DWORD flags, HWND hWnd) = 0;
};

struct IDirectDraw7 : IUnknown {
    virtual HRESULT __stdcall Compact() = 0;
    virtual HRESULT __stdcall CreateClipper(DWORD flags, IDirectDrawClipper **clipper, void *outer) = 0;
    virtual HRESULT __stdcall CreatePalette(DWORD flags, void *entries, void **palette, void *outer) = 0;
    virtual HRESULT __stdcall CreateSurface(DDSURFACEDESC2 *desc, IDirectDrawSurface7 **surface,
                                            void *outer) = 0;
    virtual HRESULT __stdcall DuplicateSurface(IDirectDrawSurface7 *src, IDirectDrawSurface7 **dst) = 0;
    virtual HRESULT __stdcall EnumDisplayModes(DWORD flags, DDSURFACEDESC2 *desc, void *context,
                                               LPDDENUMMODESCALLBACK2 callback) = 0;
    virtual HRESULT __stdcall EnumSurfaces(DWORD flags, DDSURFACEDESC2 *desc, void *context,
                                           void *callback) = 0;
    virtual HRESULT __stdcall FlipToGDISurface() = 0;
    virtual HRESULT __stdcall GetCaps(DDCAPS *driverCaps, DDCAPS *helCaps) = 0;
    virtual HRESULT __stdcall GetDisplayMode(DDSURFACEDESC2 *desc) = 0;
    virtual HRESULT __stdcall GetFourCCCodes(DWORD *count, DWORD *codes) = 0;
    virtual HRESULT __stdcall GetGDISurface(IDirectDrawSurface7 **surface) = 0;
    virtual HRESULT __stdcall GetMonitorFrequency(DWORD *frequency) = 0;
    virtual HRESULT __stdcall GetScanLine(DWORD *line) = 0;
    virtual HRESULT __stdcall GetVerticalBlankStatus(BOOL *inVB) = 0;
    virtual HRESULT __stdcall Initialize(GUID *guid) = 0;
    virtual HRESULT __stdcall RestoreDisplayMode() = 0;
    virtual HRESULT __stdcall SetCooperativeLevel(HWND hWnd, DWORD flags) = 0;
    virtual HRESULT __stdcall SetDisplayMode(DWORD width, DWORD height, DWORD bpp, DWORD refreshRate,
                                             DWORD flags) = 0;
    virtual HRESULT __stdcall WaitForVerticalBlank(DWORD flags, void *event) = 0;
    virtual HRESULT __stdcall GetAvailableVidMem(DDSCAPS2 *caps, DWORD *total, DWORD *free) = 0;
    virtual HRESULT __stdcall GetSurfaceFromDC(void *hdc, IDirectDrawSurface7 **surface) = 0;
    virtual HRESULT __stdcall RestoreAllSurfaces() = 0;
};

struct IDirectDrawSurface : IUnknown {};

/* on every target: the records that keep one as bytes (SdwDdsd2Bytes, sdw_types.h) must have room for it */
typedef char DDSURFACEDESC2_fits_SdwDdsd2Bytes[(sizeof(DDSURFACEDESC2) == sizeof(SdwDdsd2Bytes)) ? 1 : -1];

extern "C" HRESULT __stdcall DirectDrawCreateEx(GUID *guid, void **dd, const GUID &iid, void *outer);
extern "C" HRESULT __stdcall DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA callback, void *context,
                                                    DWORD flags);
extern "C" const GUID IID_IDirectDraw7;

#endif
