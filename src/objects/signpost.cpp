
#define SDW_MEMBERS_ScnObject                    \
    static void *operator new(uptr size);         \
    void SetTint(u32 color, s16 amount, s32 on); \
    void StartCamera(u16 rx, u16 ry, u16 rz, Vec3s *at, u16 focal);


#include "sdw_types.h"
#include "sdw_enums.h"
#include "scenaric_props.h"
class Instance;
struct Animator;
u32 Anim_Start(Instance *inst, Animator *animator, u16 id, u32 opts);
#define SDW_MEMBERS_RenderPoly RenderPoly();
#define SDW_MEMBERS_PolyBatcher void SubmitPoly(RenderPoly *poly);
#include "sdw_classes.h"
#define SDW_INLINE_INSTANCE_INST 1
#include "instance_inlines.h"
#undef SDW_INLINE_INSTANCE_INST
#define SDW_INLINE_SCNOBJECT_GETCLASSID 1
#define SDW_INLINE_SCNOBJECT_ISINWORLD 1
#define SDW_INLINE_SCNOBJECT_SETVISIBLE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_GETCLASSID
#undef SDW_INLINE_SCNOBJECT_ISINWORLD
#undef SDW_INLINE_SCNOBJECT_SETVISIBLE_S32
#define SDW_INLINE_SCNOBJECT_SETCOLLIDABLE_S32 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETCOLLIDABLE_S32
#define SDW_INLINE_SCNBODY_ANIMFLAGS_U16 1
#define SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32 1
#include "../engine/scn_body_inlines.h"
#undef SDW_INLINE_SCNBODY_ANIMFLAGS_U16
#undef SDW_INLINE_SCNBODY_PLAYANIM_U16_S32_S32

extern Wolf *g_pWolf;
#include "../app/app_main.h"
#include "../engine/scenaric.h"
#include "../engine/input.h"
#include "../engine/scn_tools.h"
#include "../engine/id_list.h"
#include "../engine/interface.h"
#include "camera.h"
#include "../engine/langpack.h"
#include "../engine/polybatcher.h"
#include "../engine/draw2d.h"
#include "../engine/screen.h"
#include "../engine/progress.h"
#include "../platform/platform.h"
u8 g_signPostReadCount;
void Ui_DrawTextBox(TextBox *box, u16 lineCount);
void Camera_StartScripted(ScnObject *owner, Camera *cam, u16 rx, u16 ry, u16 rz, Vec3s *at, u16 focal, u32 flags,
                          s32 time);

#define SDW_INLINE_FREE_SCN_GETPROPU32_VOID_U32 1
#include "../engine/scn_tools_inlines.h"
#undef SDW_INLINE_FREE_SCN_GETPROPU32_VOID_U32
#define SDW_INLINE_FREE_SCREENWIDTHS16 1
#include "../engine/screen_inlines.h"
#undef SDW_INLINE_FREE_SCREENWIDTHS16
#define SDW_INLINE_FREE_INSTFLAGS_SET_U16_U16 1
#include "instance_inlines.h"
#undef SDW_INLINE_FREE_INSTFLAGS_SET_U16_U16
#define SDW_INLINE_FREE_INSTFLAGS_CLEAR_U16_U16 1
#include "instance_inlines.h"
#undef SDW_INLINE_FREE_INSTFLAGS_CLEAR_U16_U16
#define SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32 1
#define SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_SCNOBJECT_SETTINT_U32_S16_S32
#undef SDW_INLINE_SCNOBJECT_STARTCAMERA_U16_U16_U16_VEC3S_U16
#define SDW_INLINE_FREE_ZONES_GET_U8 1
#include "../engine/scenaric_inlines.h"
#undef SDW_INLINE_FREE_ZONES_GET_U8
#define SDW_INLINE_ZONELIST_FINDCONTAINING_VEC3S 1
#include "../engine/zone_list_inlines.h"
#undef SDW_INLINE_ZONELIST_FINDCONTAINING_VEC3S

void SignPostAnimated::PostLoadInit()
{
    void *rec = record;
    textIndex = Scn_GetPropU32(rec, 8);
    hidden = Scn_GetPropU32(rec, 4);
    text = Text_GetClassString((u8)textIndex);
    distance = (u16)Scn_GetPropU32(rec, 0);
    state = SIGNANIM_ST_IDLE;
    if (hidden) {
        SetVisible(0);
        SetCollidable(0);
    }
    PlayAnim(SIGNANIM_ANIM_IDLE, 1, 0);
}

void SignPostSimple::PostLoadInit()
{
    wolfFrozen = 0;
    void *rec = record;
    textIndex = Scn_GetPropU32(rec, 8);
    text = Text_GetClassString((u8)textIndex);
    distance = (u16)Scn_GetPropU32(rec, 0);
    state = SIGN_ST_IDLE;
    if (Zones_Get(ZONE_SHADOW)->FindContaining(&pos))
        SetTint(0, 0x600, 1);
    savedPos.x = pos.x;
    savedPos.y = pos.y;
    savedPos.z = pos.z;
}

void SignPostSimple::Reset()
{
    if (IsInWorld()) {
        SetPosition(&savedPos);
        SnapToGround(0);
    }
}

void SignPost::PostLoadInit()
{
    void *rec = record;
    u32 anvilId;
    textNum = Scn_GetPropU32(rec, 12);
    text = Text_GetClassString((u8)textNum);
    g_signPostReadCount = 0;
    textVariant = -1;
    {
        u32 resource = Scn_GetPropU32(rec, 8);
        u16 count;
        uptr *cameraList = Scn_FindIdList((u16)resource, &count);
        if (cameraList)
            camera = (CamSetup *)*cameraList;
        else
            camera = 0;
    }
    anvilId = Scn_GetPropU32(rec, 4);
    if (anvilId == 0)
        anvil = 0;
    else
        anvil = Scenaric_FindByIdList((u16)anvilId);
    distance = (u16)Scn_GetPropU32(rec, 0);
    state = SIGNPOST_ST_IDLE;
    wolfFrozen = 0;
    if (Zones_Get(ZONE_SHADOW)->FindContaining(&pos))
        SetTint(0, 0x600, 1);
    savedPos.x = pos.x;
    savedPos.y = pos.y;
    savedPos.z = pos.z;
}

void SignPost::Reset()
{
    if (IsInWorld()) {
        SetPosition(&savedPos);
        SnapToGround(0);
    }
}

void SignPostAnimated::Update()
{
    s16 rect[4];
    rect[0] = 50;
    rect[1] = 50;
    rect[2] = ScreenWidthS16() - 100;
    rect[3] = 140;
    switch (state) {
        case SIGNANIM_ST_OPENING:
            if (!Pad_MenuPressed((u16)~PAD_CROSS))
                state = SIGNANIM_ST_SHOWING;
            break;
        case SIGNANIM_ST_SHOWING:
            Ui_DrawSubtitleBox(text, rect, DAV_IDI_IGLCADP_);
            if (Pad_MenuPressed((u16)~PAD_CROSS)) {
                state = SIGNANIM_ST_CLOSING;
                break;
            }
            break;
        case SIGNANIM_ST_CLOSING:
            if (!Pad_MenuPressed((u16)~PAD_CROSS)) {
                if (wolfFrozen)
                    g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
                state = SIGNANIM_ST_IDLE;
            }
            break;
        case SIGNANIM_ST_REVEALING:
            if (AnimFlags(ANIM_F_FINISHED)) {
                state = SIGNANIM_ST_IDLE;
                SetCollidable(1);
                PlayAnim(SIGNANIM_ANIM_IDLE, 1, 0);
            }
            break;
    }
    AdvanceAnim();
}

void SignPostSimple::Update()
{
    s16 rect[4];
    rect[0] = 50;
    rect[1] = 50;
    rect[2] = 412;
    rect[3] = 140;
    switch (state) {
        case SIGN_ST_OPENING:
            if (!Pad_MenuPressed((u16)~PAD_CROSS))
                state = SIGN_ST_SHOWING;
            break;
        case SIGN_ST_SHOWING:
            Ui_DrawSubtitleBox(text, rect, DAV_IDI_IGLCADP_);
            if (Pad_MenuPressed((u16)~PAD_CROSS)) {
                state = SIGN_ST_CLOSING;
                break;
            }
            break;
        case SIGN_ST_CLOSING:
            if (!Pad_MenuPressed((u16)~PAD_CROSS)) {
                if (wolfFrozen)
                    g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
                state = SIGN_ST_IDLE;
            }
            break;
    }
}

void SignPost::Update()
{
    TextBox box;
    s16 rects[4];
    rects[0] = 50;
    rects[1] = 50;
    rects[2] = 412;
    rects[3] = 140;
    box.text = text;
    ((u32 *)box.rect)[0] = ((u32 *)rects)[0];
    ((u32 *)box.rect)[1] = ((u32 *)rects)[1];
    box.bgColor = 0xa0a0a;
    box.fits = 0;
    switch (state) {
        case SIGNPOST_ST_OPENING:
            if (!Pad_MenuPressed((u16)~PAD_CROSS))
                state = SIGNPOST_ST_SHOWING;
            break;
        case SIGNPOST_ST_SHOWING:
            if (textVariant == 1)
                Ui_DrawTextBox(&box, 0xffff);
            else
                Ui_DrawSubtitleBox(text, rects, DAV_IDI_IGLCADP_);
            if (Pad_MenuPressed((u16)~PAD_CROSS)) {
                if (box.fits && (s8)box.confirmChoice == 0 && anvil) {
                    anvil->HandleMessage(this, MSG_ANVIL_DROP, 0);
                    g_pWolf->HandleMessage(this, MSG_WOLF_DAZE, 0);
                    state = SIGNPOST_ST_ANVIL;
                    if (camera)
                        StartCamera(camera->rot[0], camera->rot[1], camera->rot[2], &camera->eye, camera->focal);
                    break;
                } else {
                    state = SIGNPOST_ST_CLOSING;
                    break;
                }
            }
            break;
        case SIGNPOST_ST_CLOSING:
            if (!Pad_MenuPressed((u16)~PAD_CROSS)) {
                if (wolfFrozen)
                    g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
                state = SIGNPOST_ST_IDLE;
            }
            break;
        case SIGNPOST_ST_ANVIL:
            if (((Anvil *)anvil)->dropping == 0) {
                Camera_ReleaseAny();
                state = SIGNPOST_ST_SHOWING;
            }
            break;
    }
}

sptr SignPostAnimated::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    switch (msgId) {
        case MSG_USE:
            if (state == SIGNANIM_ST_IDLE && !hidden) {
                wolfFrozen = g_pWolf->HandleMessage(this, MSG_FREEZE, 0);
                state = SIGNANIM_ST_OPENING;
                return 1;
            }
            break;
        case MSG_QUERY_ACTION:
            if (state == SIGNANIM_ST_IDLE && !hidden && sender->GetClassId() == CLASSID_WOLF &&
                Vec3s_ManhattanDistXZ(&pos, &g_pWolf->pos) < distance)
                return CTX_READ;
            break;
        case MSG_FREEZE:
            wolfFrozen = 0;
            state = SIGNANIM_ST_IDLE;
            return 1;
        case MSG_SIGN_REVEAL:
            hidden = 0;
            SetVisible(1);
            PlayAnim(SIGNANIM_ANIM_HIT, 0, 0);
            state = SIGNANIM_ST_REVEALING;
            break;
    }
    return 0;
}

sptr SignPostSimple::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    switch (msgId) {
        case MSG_USE:
            if (state == SIGN_ST_IDLE) {
                wolfFrozen = g_pWolf->HandleMessage(this, MSG_FREEZE, 0);
                state = SIGN_ST_OPENING;
                return 1;
            }
            break;
        case MSG_QUERY_ACTION:
            if (state == SIGN_ST_IDLE && sender->GetClassId() == CLASSID_WOLF &&
                Vec3s_ManhattanDistXZ(&pos, &g_pWolf->pos) < distance)
                return CTX_READ;
            break;
        case MSG_FREEZE:
            state = SIGN_ST_IDLE;
            return 1;
        case MSG_CINE_END:
            savedPos.x = pos.x;
            savedPos.y = pos.y;
            savedPos.z = pos.z;
            break;
        case MSG_CONTAINER_STATE:
            if ((s32)(sptr)arg == CONTAINER_RELEASED) {
                savedPos.x = pos.x;
                savedPos.y = pos.y;
                savedPos.z = pos.z;
            }
            break;
        case MSG_TELEPORTED:
            return 1;
    }
    return 0;
}

sptr SignPost::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    switch (msgId) {
        case MSG_USE:
            if (state == SIGNPOST_ST_IDLE) {
                if (textVariant == -1) {
                    text = Text_GetClassString((u8)(g_signPostReadCount + textNum));
                    textVariant = g_signPostReadCount;
                    g_signPostReadCount++;
                } else {
                    text = Text_GetClassString((u8)(textVariant + textNum));
                }
                wolfFrozen = g_pWolf->HandleMessage(this, MSG_FREEZE, 0);
                state = SIGNPOST_ST_OPENING;
                return 1;
            }
            break;
        case MSG_QUERY_ACTION:
            if (state == SIGNPOST_ST_IDLE && sender->GetClassId() == CLASSID_WOLF &&
                Vec3s_ManhattanDistXZ(&pos, &g_pWolf->pos) < distance)
                return CTX_READ;
            break;
        case MSG_FREEZE:
            wolfFrozen = 0;
            state = SIGNPOST_ST_IDLE;
            return 1;
        case MSG_TELEPORTED:
            return 1;
    }
    return 0;
}

ScnObject *SignPostSimple_Create(void *record)
{
    ScnLogic *obj = new SignPostSimple;
    obj = (ScnLogic *)obj->Init(record);
    return obj;
}

ScnObject *SignPostAnimated_Create(void *record)
{
    ScnBody *obj = new SignPostAnimated;
    obj = obj->Init(record, 0);
    return obj;
}

ScnObject *SignPost_Create(void *record)
{
    ScnLogic *obj = new SignPost;
    obj = (ScnLogic *)obj->Init(record);
    return obj;
}

/* An additional, opt-in language-pack sign. It never replaces a level hint or
 * edits the original WAR. The per-level heap owns the object and its record is
 * an immutable reference to the existing sign's model record. */
void Draw2D_TexRect(float z, float x0, float y0, float x1, float y1, s32 page,
                   float u0, float v0, u32 c0, float u1, float v1, u32 c1,
                   float u2, float v2, u32 c2, float u3, float v3, u32 c3);

extern "C" double sqrt(double);
namespace {
#include "channel_sign_outline.h"
class ChannelSign : public SignPostSimple {
public:
    s32 imagePage;
    float axisX = 1, axisZ = 0;
    void PostLoadInit() override {
        state = SIGN_ST_IDLE;
        wolfFrozen = 0;
        distance = 240;
        savedPos = pos;
        text = (char *)LangPack_MenuString(0x41, 0);
        float dx = pos.x - g_pWolf->pos.x, dz = pos.z - g_pWolf->pos.z;
        float length = (float)sqrt(dx*dx + dz*dz);
        if (length > 1) { axisX = dz/length; axisZ = -dx/length; }
    }
    void Update() override {
        if (state == SIGN_ST_OPENING && !Pad_MenuHeld((u16)~PAD_CROSS)) {
            Menu_BuildConfirmMenu();
            state = SIGN_ST_SHOWING;
        } else if (state == SIGN_ST_SHOWING) {
            TextBox box = {};
            box.text = text;
            box.rect[0] = 64; box.rect[1] = 65;
            box.rect[2] = 384; box.rect[3] = 125;
            Ui_DrawTextBox(&box, 0xffff);
            if (Pad_MenuPressed((u16)~PAD_CROSS) && box.fits) {
                // Ui_DrawTextBox: first row (Yes) is 1, second row (No) is 0.
                state = SIGN_ST_CLOSING;
                if (box.confirmChoice == 1 && !Platform_OpenChannel())
                    Platform_Log("Could not open channel URL");
            } else if (Pad_MenuPressed((u16)~PAD_TRIANGLE)) {
                state = SIGN_ST_CLOSING;
            }
        } else if (state == SIGN_ST_CLOSING && !Pad_MenuHeld((u16)~PAD_CROSS)) {
            if (wolfFrozen) g_pWolf->HandleMessage(this, MSG_UNFREEZE, 0);
            wolfFrozen = 0;
            state = SIGN_ST_IDLE;
        }
    }
    // Local coordinates: horizontal, height above ground, depth toward the back.
    void Face(Camera *view, const float corners[4][3], const float normal[3], bool printed, u32 shade, bool continuousWood = false) {
        Vec3f centre = {(float)pos.x, (float)pos.y, (float)pos.z};
        for (int i=0; i<4; ++i) {
            centre.x += (corners[i][0]*axisX - corners[i][2]*axisZ)*0.25f;
            centre.y -= corners[i][1]*0.25f;
            centre.z += (corners[i][0]*axisZ + corners[i][2]*axisX)*0.25f;
        }
        Vec3f n = {normal[0]*axisX-normal[2]*axisZ, -normal[1],
                   normal[0]*axisZ+normal[2]*axisX};
        if ((view->pos.x-centre.x)*n.x + (view->pos.y-centre.y)*n.y +
            (view->pos.z-centre.z)*n.z <= 0) return;
        struct Vertex { float x,y,z,rhw; u32 color,specular; float u,v; } v[4];
        for (int i=0; i<4; ++i) {
            Vec3f world = {(float)pos.x + corners[i][0]*axisX - corners[i][2]*axisZ,
                          (float)pos.y - corners[i][1],
                          (float)pos.z + corners[i][0]*axisZ + corners[i][2]*axisX}, eye;
            view->viewMatCopy.TransformPoint(&world, &eye);
            if (eye.z <= 20.0f) return;
            float iw = 1.0f/eye.z;
            v[i].x = (1 + eye.x*g_projMatrix.m[0][0]*iw)*g_screen.viewportWidth*0.5f;
            v[i].y = (1 - eye.y*g_projMatrix.m[1][1]*iw)*g_screen.viewportHeight*0.5f;
            v[i].z = g_projMatrix.m[2][2] + g_projMatrix.m[3][2]*iw;
            if (v[i].z <= 0 || v[i].z >= 1) return;
            v[i].rhw = iw; v[i].color = shade; v[i].specular = 0xff000000;
            float u = (i==1 || i==2) ? 1.0f : 0.0f, t = i>=2 ? 1.0f : 0.0f;
            // A clean wood-grain patch in the same image; no lettering or logo on the rear/edges.
            v[i].u = printed ? (corners[i][0]+110)/220 : 0.12f + (continuousWood ? (corners[i][0]+110)/220 : u)*0.24f;
            v[i].v = printed ? (220-corners[i][1])/147 : 0.10f + (continuousWood ? (220-corners[i][1])/147 : t)*0.20f;
        }
        const int indices[2][3] = {{0,1,2},{0,2,3}};
        RenderPoly poly;
        poly.type = imagePage + RPOLY_TEXTURED_BASE;
        for (int t=0; t<2; ++t) {
            Vertex *out = (Vertex *)poly.verts;
            for (int i=0;i<3;++i) out[i] = v[indices[t][i]];
            poly.sortZ = out[0].z + out[1].z + out[2].z;
            g_pPolyBin->SubmitPoly(&poly);
        }
    }
    void Timber(Camera *view, float left, float right, float top, float bottom,
                float front, float back, bool printed) {
        const float faces[6][4][3] = {
            {{left,top,front},{right,top,front},{right,bottom,front},{left,bottom,front}},
            {{right,top,back},{left,top,back},{left,bottom,back},{right,bottom,back}},
            {{left,top,back},{left,top,front},{left,bottom,front},{left,bottom,back}},
            {{right,top,front},{right,top,back},{right,bottom,back},{right,bottom,front}},
            {{left,top,back},{right,top,back},{right,top,front},{left,top,front}},
            {{left,bottom,front},{right,bottom,front},{right,bottom,back},{left,bottom,back}}
        };
        const float normals[6][3] = {{0,0,-1},{0,0,1},{-1,0,0},{1,0,0},{0,1,0},{0,-1,0}};
        const u32 shades[6] = {0x808080,0x606060,0x484848,0x585858,0x888888,0x383838};
        for (int i=0; i<6; ++i) Face(view, faces[i], normals[i], printed && i==0, shades[i]);
    }
    void Render(Camera *view) override {
        Timber(view, -9, 9, 160, 0, 6, 24, false);
        const int count = sizeof(kChannelOutline)/sizeof(kChannelOutline[0]);
        for (int i=0; i<count; ++i) {
            const float *a = kChannelOutline[i], *b = kChannelOutline[(i+1)%count];
            float front[4][3] = {{0,146.5f,-8},{a[0],a[1],-8},{b[0],b[1],-8},{b[0],b[1],-8}};
            float back[4][3] = {{0,146.5f,8},{b[0],b[1],8},{a[0],a[1],8},{a[0],a[1],8}};
            float edge[4][3] = {{a[0],a[1],-8},{a[0],a[1],8},{b[0],b[1],8},{b[0],b[1],-8}};
            const float frontN[3] = {0,0,-1}, backN[3] = {0,0,1};
            float edgeN[3] = {a[1]-b[1], b[0]-a[0], 0};
            Face(view, front, frontN, true, 0x808080);
            // Use position-based wood UVs on the rear for a continuous grain.
            Face(view, back, backN, false, 0x606060, true);
            Face(view, edge, edgeN, false, edgeN[1] > 0 ? 0x707070 : 0x484848);
        }
    }
};
}

void ChannelSign_AddToTutorial()
{
    if (!g_pWolf || !g_pProgress || g_pProgress->currentLevel != 0) return;
    const char *picture = LangPack_ChannelPicture();
    if (!picture || !picture[0]) return;
    ScnObject *source = 0;
    s32 nearest = 0x7fffffff;
    for (u16 i = 0; i < g_scnObjectCount; ++i) {
        ScnObject *object = g_scnObjects[i];
        if (!object || (object->classId != CLASSID_SIGNPOST && object->classId != CLASSID_SIGNPOSTSIMPLE)) continue;
        s32 d = Vec3s_ManhattanDistXZ(&object->pos, &g_pWolf->pos);
        if (d < nearest) { nearest = d; source = object; }
    }
    bool freeSlot = false;
    for (u16 i=g_scnObjectCount; i<g_scnActiveCapacity; ++i) if (!g_scnActive[i]) { freeSlot = true; break; }
    if (!freeSlot) { Platform_Log("Channel sign: no free scene slot"); return; }
    if (!source) { Platform_Log("Channel sign: no rigid tutorial sign available"); return; }
    s32 page = PolyBatcher_AddImagePage(picture, 0);
    if (page < 0) return;
    // Inherit the world's depth test, like every other 3D texture page.
    // Per-page RSF_ZTEST would be cleared after this material and disable
    // depth testing for the remaining transparent plants in the same frame.
    ChannelSign *sign = new ChannelSign;
    sign->Init(source->record);
    sign->imagePage = page;
    Vec3s position = g_pWolf->pos;
    position.x -= 260;
    position.z -= 480;
    sign->AddToWorld(&position);
    sign->SnapToGround(0);
    sign->PostLoadInit();
    sign->boundRadius = 260;
    Platform_Log("Channel sign: added beside hint (%d,%d,%d), wolf (%d,%d,%d)",
        sign->pos.x, sign->pos.y, sign->pos.z, g_pWolf->pos.x, g_pWolf->pos.y, g_pWolf->pos.z);
}
