/* Only what src/ uses is here. */
#ifndef SDW_SDK_WINDEF_H
#define SDW_SDK_WINDEF_H

#include "sdw_types.h"

/* Windows' long-based SDK types stay 32-bit on 64-bit Windows. */
typedef u32 DWORD;
typedef u32 ULONG;
typedef s32 LONG;
typedef int BOOL;
typedef unsigned int UINT;
typedef unsigned short WORD;
typedef unsigned short WCHAR;
typedef unsigned short ATOM;
/* Pointer-sized integers, as the SDK defines them (basetsd.h) */
typedef sptr INT_PTR;
typedef uptr UINT_PTR;
typedef sptr LONG_PTR;
typedef uptr ULONG_PTR;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef s32 HRESULT;
typedef void *HANDLE;
typedef void *HGDIOBJ;
typedef unsigned char BYTE;

/* DECLARE_HANDLE under STRICT: each handle is a pointer to its own incomplete struct */
struct HWND__;
typedef HWND__ *HWND;
struct HINSTANCE__;
typedef HINSTANCE__ *HINSTANCE;
struct HDC__;
typedef HDC__ *HDC;
struct HBITMAP__;
typedef HBITMAP__ *HBITMAP;
struct HBRUSH__;
typedef HBRUSH__ *HBRUSH;
struct HICON__;
typedef HICON__ *HICON;
typedef HICON HCURSOR;
struct HFONT__;
typedef HFONT__ *HFONT;
struct HMENU__;
typedef HMENU__ *HMENU;
struct HKEY__;
typedef HKEY__ *HKEY;

struct GUID {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
};

struct RECT {
    LONG left, top, right, bottom;
};

struct POINT {
    LONG x, y;
};

struct IUnknown {
    virtual HRESULT __stdcall QueryInterface(const GUID &riid, void **ppv) = 0;
    virtual ULONG __stdcall AddRef() = 0;
    virtual ULONG __stdcall Release() = 0;
};

struct IPersist : IUnknown {
    virtual HRESULT __stdcall GetClassID(GUID *) = 0;
};


#endif
