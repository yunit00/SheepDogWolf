/* Small Win32 compatibility surface for code that is still shared with the Windows build. */
#include <SDL3/SDL.h>

#include "platform.h"
#include "../sdk/win32.h"
#include "../sdk/d3d7.h"
#include "../sdk/dinput.h"
#include "../sdk/dsound.h"
#include "../sdk/mmstream.h"

#include <string.h>

struct SdwPlatformHandle {
    enum Kind { Mutex, Event, Thread } kind;
    SDL_Mutex *mutex;
    SDL_Semaphore *semaphore;
    SDL_Thread *thread;
};

struct SdwThreadStart {
    LPTHREAD_START_ROUTINE proc;
    void *arg;
};

static int SdwThreadEntry(void *opaque)
{
    SdwThreadStart start = *(SdwThreadStart *)opaque;
    SDL_free(opaque);
    return (int)start.proc(start.arg);
}

extern "C" int MessageBoxA(HWND, const char *text, const char *caption, UINT type)
{
    if (type & MB_YESNO) {
        const SDL_MessageBoxButtonData buttons[] = {
            {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 7, "No"},
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, IDYES, "Yes"}};
        const SDL_MessageBoxData data = {SDL_MESSAGEBOX_INFORMATION, 0, caption, text, 2, buttons, 0};
        int selected = 7;
        if (SDL_ShowMessageBox(&data, &selected) == 0)
            return selected;
        return 7;
    }
    Platform_ShowMessage(caption, text, (type & (MB_ICONHAND | MB_ICONEXCLAMATION)) != 0);
    return IDOK;
}

extern "C" void OutputDebugStringA(const char *text)
{
    SDL_Log("%s", text ? text : "");
}

extern "C" HRESULT CoInitialize(void *) { return S_OK; }
extern "C" void CoUninitialize(void) {}
extern "C" HRESULT CoCreateInstance(const GUID &, void *, DWORD, const GUID &, void **out)
{
    if (out)
        *out = 0;
    return E_NOTIMPL;
}

extern "C" BOOL QueryPerformanceCounter(s64 *count)
{
    *count = (s64)SDL_GetPerformanceCounter();
    return TRUE;
}

extern "C" BOOL QueryPerformanceFrequency(s64 *frequency)
{
    *frequency = (s64)SDL_GetPerformanceFrequency();
    return TRUE;
}

extern "C" HANDLE CreateMutexA(void *, BOOL initialOwner, const char *)
{
    SdwPlatformHandle *handle = (SdwPlatformHandle *)SDL_calloc(1, sizeof(*handle));
    if (!handle)
        return 0;
    handle->kind = SdwPlatformHandle::Mutex;
    handle->mutex = SDL_CreateMutex();
    if (!handle->mutex) {
        SDL_free(handle);
        return 0;
    }
    if (initialOwner)
        SDL_LockMutex(handle->mutex);
    return handle;
}

extern "C" HANDLE CreateEventA(void *, BOOL, BOOL initialState, const char *)
{
    SdwPlatformHandle *handle = (SdwPlatformHandle *)SDL_calloc(1, sizeof(*handle));
    if (!handle)
        return 0;
    handle->kind = SdwPlatformHandle::Event;
    handle->semaphore = SDL_CreateSemaphore(initialState ? 1 : 0);
    if (!handle->semaphore) {
        SDL_free(handle);
        return 0;
    }
    return handle;
}

extern "C" HANDLE CreateThread(void *, DWORD, LPTHREAD_START_ROUTINE proc, void *arg, DWORD, DWORD *threadId)
{
    SdwPlatformHandle *handle = (SdwPlatformHandle *)SDL_calloc(1, sizeof(*handle));
    SdwThreadStart *start = (SdwThreadStart *)SDL_malloc(sizeof(*start));
    if (!handle || !start) {
        SDL_free(handle);
        SDL_free(start);
        return 0;
    }
    start->proc = proc;
    start->arg = arg;
    handle->kind = SdwPlatformHandle::Thread;
    handle->thread = SDL_CreateThread(SdwThreadEntry, "SheepD3D loader", start);
    if (!handle->thread) {
        SDL_free(start);
        SDL_free(handle);
        return 0;
    }
    if (threadId)
        *threadId = (DWORD)SDL_GetThreadID(handle->thread);
    return handle;
}

extern "C" BOOL ReleaseMutex(HANDLE opaque)
{
    SdwPlatformHandle *handle = (SdwPlatformHandle *)opaque;
    if (!handle || handle->kind != SdwPlatformHandle::Mutex)
        return FALSE;
    SDL_UnlockMutex(handle->mutex);
    return TRUE;
}

extern "C" DWORD WaitForSingleObject(HANDLE opaque, DWORD milliseconds)
{
    SdwPlatformHandle *handle = (SdwPlatformHandle *)opaque;
    if (!handle)
        return (DWORD)-1;
    if (handle->kind == SdwPlatformHandle::Mutex) {
        if (milliseconds == INFINITE)
        {
            SDL_LockMutex(handle->mutex);
            return WAIT_OBJECT_0;
        }
        if (milliseconds == 0)
            return SDL_TryLockMutex(handle->mutex) ? WAIT_OBJECT_0 : (DWORD)0x102;
        Uint64 until = SDL_GetTicks() + milliseconds;
        do {
            if (SDL_TryLockMutex(handle->mutex))
                return WAIT_OBJECT_0;
            SDL_Delay(1);
        } while (SDL_GetTicks() < until);
        return (DWORD)0x102;
    }
    if (handle->kind == SdwPlatformHandle::Event)
        return SDL_WaitSemaphoreTimeout(handle->semaphore, milliseconds == INFINITE ? -1 : milliseconds)
                   ? WAIT_OBJECT_0
                   : (DWORD)0x102;
    if (handle->thread) {
        int status = 0;
        SDL_WaitThread(handle->thread, &status);
        handle->thread = 0;
    }
    return WAIT_OBJECT_0;
}

extern "C" BOOL CloseHandle(HANDLE opaque)
{
    SdwPlatformHandle *handle = (SdwPlatformHandle *)opaque;
    if (!handle)
        return FALSE;
    if (handle->kind == SdwPlatformHandle::Mutex)
        SDL_DestroyMutex(handle->mutex);
    else if (handle->kind == SdwPlatformHandle::Event)
        SDL_DestroySemaphore(handle->semaphore);
    else if (handle->thread)
        SDL_DetachThread(handle->thread);
    SDL_free(handle);
    return TRUE;
}

extern "C" BOOL SetRect(RECT *rect, int left, int top, int right, int bottom)
{
    if (!rect)
        return FALSE;
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
    return TRUE;
}

extern "C" int ShowCursor(BOOL show)
{
    static int s_displayCount; /* Win32's: the cursor shows while it is >= 0 */
    s_displayCount += show ? 1 : -1;
    if (s_displayCount >= 0)
        SDL_ShowCursor();
    else
        SDL_HideCursor();
    return s_displayCount;
}

extern "C" void Sleep(DWORD ms) { SDL_Delay(ms); }

extern "C" BOOL SetPriorityClass(HANDLE, DWORD) { return TRUE; }
extern "C" BOOL SetThreadPriority(HANDLE, int) { return TRUE; }

extern "C" const GUID CLSID_AMMultiMediaStream = {0x49c47ce5, 0x9ba4, 0x11d0, {0x82, 0x12, 0x00, 0xc0, 0x4f, 0xc3, 0x2c, 0x45}};
extern "C" const GUID CLSID_AudioRender = {0xe30629d1, 0x27e5, 0x11ce, {0x87, 0x5d, 0x00, 0x60, 0x8c, 0xb7, 0x80, 0x66}};
extern "C" const GUID IID_IAMMultiMediaStream = {0xbebe595c, 0x9a6f, 0x11d0, {0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d}};
extern "C" const GUID IID_IBaseFilter = {0x56a86895, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0x77, 0x0}};
extern "C" const GUID IID_IDirectDrawMediaStream = {0xf4104fce, 0x9a70, 0x11d0, {0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d}};
extern "C" const GUID MSPID_PrimaryVideo = {0xa35ff56a, 0x9fda, 0x11d0, {0x8f, 0xdf, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d}};
extern "C" const GUID IID_IDirect3DTnLHalDevice = {0xf5049e77, 0x4861, 0x11d2, {0xa4, 0x07, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0xa8}};
extern "C" const GUID IID_IDirectSoundNotify = {0xb0210783, 0x89cd, 0x11d0, {0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16}};
