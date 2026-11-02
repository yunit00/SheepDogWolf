/*
 * The six DirectShow GUIDs are strmiids.lib data, outside this object.
 *
 * COM declarations (src/sdk/mmstream.h) are SDK interfaces, not replacement game classes. Shapes that fixed block
 * order: OpenFile / CreateStream / ReleaseFilter use `goto fail` chains (return 0 block laid out before return 1);
 * RunLoop's loop is one `while(a && b) ;`.
 */
#include "../sdk/mmstream.h"
#include "../sdk/mmstream.h"
#include "../sdk/win32.h"
#define SDW_MEMBERS_VideoPlayer VideoPlayer();
#define SDW_MEMBERS_Video \
    Video();              \
    VideoFrameProc SetFrameCallback(VideoFrameProc);
#include "sdw_classes.h"
#include "../sdk/crt.h"
#define RELEASE(p)      \
    if (p)              \
        (p)->Release(); \
    p = 0
s32 Video_NullFrameProc(IDirectDrawSurface *, RECT *);

/* Windows WCHAR is 16-bit; macOS wchar_t is 32-bit. Build the ASCII COM names
 * directly in the 16-bit buffers expected by the stand-in interfaces. */
static int Video_CopyAsciiToWchar16(unsigned short *out, const char *text)
{
    int i = 0;
    while (text[i]) {
        out[i] = (unsigned char)text[i];
        ++i;
    }
    out[i] = 0;
    return i;
}

static void Video_FormatAudioName(unsigned short *out, s32 audioIndex)
{
    unsigned short digits[10];
    int count = 0;
    int pos = Video_CopyAsciiToWchar16(out, "AudioRender ");
    u32 value;
    if (audioIndex < 0) {
        out[pos++] = '-';
        value = (u32)(-(s64)audioIndex);
    } else {
        value = (u32)audioIndex;
    }
    do {
        digits[count++] = (unsigned short)('0' + value % 10);
        value /= 10;
    } while (value);
    while (count)
        out[pos++] = digits[--count];
    out[pos] = 0;
}

Video::Video()
{
    frameCallback = Video_NullFrameProc;
    graph = 0;
    mediaStream = 0;
    unknownStream = 0;
    sample = 0;
    surface = 0;
    audioFilter = 0;
}
/* inlined in the deleting destructor */
inline Video::~Video()
{
    RELEASE(graph);
    RELEASE(surface);
    RELEASE(sample);
    RELEASE(unknownStream);
    RELEASE(mediaStream);
}
s32 Video::OpenFile(const char *path)
{
    IMediaStream *media = 0;
    IDirectDrawMediaStream *draw = 0;
    unsigned short widePath[260];
    if (!mediaStream)
        goto fail;
    MultiByteToWideChar(CP_ACP, 0, path, -1, widePath, 260);
    if (mediaStream->OpenFile(widePath, 0) < 0)
        goto fail;
    if (mediaStream->GetFilterGraph(&graph) < 0)
        goto fail;
    if (mediaStream->GetMediaStream(MSPID_PrimaryVideo, &media) < 0)
        goto fail;
    if (media->QueryInterface(IID_IDirectDrawMediaStream, (void **)&draw) < 0)
        goto fail;
    if (draw->CreateSample(0, 0, 0, &sample) < 0)
        goto fail;
    if (sample->GetSurface(&surface, &sourceRect) < 0)
        goto fail;
    RELEASE(draw);
    RELEASE(media);
    return 1;
fail:
    RELEASE(draw);
    RELEASE(media);
    return 0;
}
s32 Video::CreateStream(IDirectDraw7 *directDraw)
{
    if (CoCreateInstance(CLSID_AMMultiMediaStream, 0, CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream,
                         (void **)&mediaStream) < 0)
        goto fail;
    if (mediaStream->Initialize(STREAMTYPE_READ, 0, 0) < 0)
        goto fail;
    if (mediaStream->AddMediaStream(directDraw, &MSPID_PrimaryVideo, 0, 0) < 0)
        goto fail;
    return 1;
fail:
    return 0;
}
s32 Video::CloseFile()
{
    RELEASE(graph);
    RELEASE(surface);
    RELEASE(sample);
    RELEASE(unknownStream);
    return 1;
}
/* A movie whose video cannot be decoded never delivers a frame, and the original's synchronous
 * Update waits for one forever (a black screen). The movies are Indeo 5, whose codec current Windows ships unregistered.
 * Each frame is asked for asynchronously instead, and a frame that takes over 2 s ends the movie. */
#define SSUPDATE_ASYNC 1
#define COMPSTAT_WAIT 2
#define COMPSTAT_ABORT 4
#define MS_S_PENDING ((HRESULT)0x00040001L)
#define VIDEO_FRAME_TIMEOUT_MS 2000
s32 Video::RunLoop()
{
    HANDLE frameReady;
    HRESULT hr;
    s32 result = 0;
    if (mediaStream && sample && mediaStream->SetState(STREAMSTATE_RUN) >= 0) {
        frameReady = CreateEventA(0, 0, 0, 0);
        for (;;) {
            hr = sample->Update(SSUPDATE_ASYNC, frameReady, 0, 0);
            if (hr == MS_S_PENDING) {
                if (WaitForSingleObject(frameReady, VIDEO_FRAME_TIMEOUT_MS) != WAIT_OBJECT_0) {
                    sample->CompletionStatus(COMPSTAT_WAIT | COMPSTAT_ABORT, INFINITE);
                    break;
                }
                hr = sample->CompletionStatus(0, 0);
            }
            if (hr != S_OK || !frameCallback(surface, &sourceRect))
                break;
        }
        CloseHandle(frameReady);
        if (mediaStream->SetState(STREAMSTATE_STOP) >= 0)
            result = 1;
    }
    return result;
}
VideoFrameProc Video::SetFrameCallback(VideoFrameProc callback)
{
    VideoFrameProc old = frameCallback;
    frameCallback = callback;
    return old;
}
s32 Video::ConnectAudioStream(s32 audioIndex)
{
    unsigned short name[256];
    unsigned short inputPinName[64];
    unsigned short splitterName[32];
    IBaseFilter *splitter = 0, *existing = 0;
    IPin *input = 0, *output;
    IEnumPins *pins;
    u32 fetched;
    s32 result = 0;
    if (graph) {
        Video_FormatAudioName(name, audioIndex);
        Video_CopyAsciiToWchar16(inputPinName, "Audio Input pin (rendered)");
        Video_CopyAsciiToWchar16(splitterName, "AVI Splitter");
        if (graph->FindFilterByName(name, &existing) < 0 &&
            CoCreateInstance(CLSID_AudioRender, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&audioFilter) >= 0 &&
            graph->AddFilter(audioFilter, name) >= 0 &&
            audioFilter->FindPin(inputPinName, &input) >= 0 &&
            graph->FindFilterByName(splitterName, &splitter) >= 0) {
            u16 index = 0;
            pins = 0;
            splitter->EnumPins(&pins);
            pins->Reset();
            output = 0;
            while (pins->Next(1, &output, &fetched) == S_OK) {
                if (graph->Connect(output, input) < 0)
                    output->Release();
                else {
                    if (index == audioIndex)
                        break;
                    ++index;
                    graph->Disconnect(input);
                    graph->Disconnect(output);
                }
            }
            if (output)
                result = 1;
        }
    }
    RELEASE(pins);
    RELEASE(output);
    RELEASE(splitter);
    RELEASE(input);
    return result;
}
s32 Video::ReleaseFilter(s32 unused)
{
    long hr;
    if (!graph)
        goto fail;
    hr = graph->RemoveFilter(audioFilter);
    RELEASE(audioFilter);
    if (hr < 0)
        goto fail;
    return 1;
fail:
    return 0;
}
s32 Video_NullFrameProc(IDirectDrawSurface *, RECT *)
{
    return 0;
}
#undef RELEASE
