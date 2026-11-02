/*
 * This object is the VideoPlayer part of the video code; the Video object is src/objects/video.cpp, and Init calls
 * Video::Video, CreateStream and SetFrameCallback out of line across that boundary. COM declarations
 * (src/sdk/mmstream.h) are SDK interfaces, not replacement game classes.
 * */
#include "../sdk/mmstream.h"
#include "../sdk/ddraw.h"
#define SDW_MEMBERS_VideoPlayer VideoPlayer();
#define SDW_MEMBERS_Video \
    Video();              \
    VideoFrameProc SetFrameCallback(VideoFrameProc);
#include "sdw_classes.h"

u8 g_videoStretchFlag;                       /* PlayFile's stretch argument, read by VideoPlayer_BlitFrame */
RECT g_videoDestRect;                        /* the D3DApp client rect, the stretched blit's destination */
IDirectDrawSurface7 *g_pVideoPrimarySurface; /* the primary surface the frames are blitted to */

s32 VideoPlayer_BlitFrame(IDirectDrawSurface *, RECT *);

VideoPlayer::VideoPlayer()
{
    ready = 0;
    app = 0;
    stream = 0;
    g_pVideoPrimarySurface = 0;
}
/* (inlined in the deleting destructor) */
VideoPlayer::~VideoPlayer()
{
    if (stream)
        delete stream;
    app = 0;
}
u8 VideoPlayer::Init(D3DApp *device)
{
    ready = 0;
    if (device->deviceReady == 1) {
        app = device;
        g_pVideoPrimarySurface = app->pPrimary;
        g_videoDestRect = app->clientRect;
        if (g_pVideoPrimarySurface) {
            if (stream)
                delete stream;
            stream = new Video;
            IDirectDraw7 *dd = app->pDD;
            ready = stream->CreateStream(dd) != 0;
            stream->SetFrameCallback(VideoPlayer_BlitFrame);
        }
    }
    return ready;
}
u8 VideoPlayer::PlayFile(const char *path, u8 stretch)
{
    u8 result = 0;
    if (stream->OpenFile(path) == 1) {
        s32 played;
        g_videoStretchFlag = stretch;
        stream->ConnectAudioStream(0);
        played = stream->RunLoop();
        stream->ReleaseFilter(0);
        if (played == 1 && stream->CloseFile() == 1)
            result = 1;
    }
    return result;
}
s32 VideoPlayer_BlitFrame(IDirectDrawSurface *source, RECT *sourceRect)
{
    if (g_videoStretchFlag == 1)
        g_pVideoPrimarySurface->Blt(&g_videoDestRect, (IDirectDrawSurface7 *)source, sourceRect, DDBLT_WAIT, 0);
    else
        g_pVideoPrimarySurface->Blt(sourceRect, (IDirectDrawSurface7 *)source, sourceRect, DDBLT_WAIT, 0);
    return 1;
}
