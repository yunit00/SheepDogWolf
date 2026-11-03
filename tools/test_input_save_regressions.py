#!/usr/bin/env python3
"""Run production camera permission, cursor, tutorial tag and save code in isolated fixtures.
UBSan catch record/slot overflows; this is not a full campaign playthrough.
"""
import pathlib, re, subprocess, tempfile
root = pathlib.Path(__file__).resolve().parents[1]
def function(path, name):
    src = (root / path).read_text()
    match = re.search(r'^\w+[ *]+%s\([^;]*?\)\n\{' % re.escape(name), src, re.M)
    assert match, name
    end, depth = match.end(), 1
    while depth:
        depth += (src[end] == '{') - (src[end] == '}')
        end += 1
    return src[match.start():end]
pre = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include "sdw_enums.h"
using u32=uint32_t; using s32=int32_t; using u8=uint8_t; using s8=int8_t;
u32 g_gameFlags=0; bool mapOpen=false, g_camDebugMode=false;
bool Map_IsOpen() { return mapOpen; }
struct State { u32 flags=WSF_LOOK_MODE_OK; };
struct Bank { State states[2]; } g_wolfMoveBank0[1];
struct Wolf {
 u32 flags=0; int mode=0,state=0; s32 CanControl(); s32 CanLook();
 void HandleMessage(void*,u32 id,void* arg) {
  assert(id==MSG_WOLF_ALLOW_LOOK); if(arg) flags|=WOLF_FB_LOOK_ALLOWED; else flags&=~WOLF_FB_LOOK_ALLOWED;
 }
} wolf, *g_pWolf=&wolf;
struct DaffyTrainingLevel {
 char *currentText; int lockTag=0,viewTag=0,stopTag=0,confirmTag=0;
 s32 EatTextTag(u32); void ParseTextTags();
};
bool active=true, s_mouseCaptured=false, visible=true, s_menuPointerRequested=false;
void *s_window=nullptr; float s_cameraMouseX=0,s_cameraMouseY=0;
int Platform_IsActive(){ return active; }
void SDL_SetWindowRelativeMouseMode(void*,bool){}
bool SDL_CursorVisible(){return visible;}
void SDL_ShowCursor(){visible=true;} void SDL_HideCursor(){visible=false;}
'''
checks = r'''
int main(){
 assert(wolf.CanControl() && wolf.CanLook());
 wolf.flags=WOLF_FB_FROZEN; assert(!wolf.CanControl() && !wolf.CanLook());
 char text[]="$LOCK$$VIEW$$23$find me";
 DaffyTrainingLevel duck{text}; duck.ParseTextTags();
 assert(duck.lockTag && duck.viewTag && !wolf.CanControl() && wolf.CanLook());
 assert(!strcmp(duck.currentText,"$23$find me"));
 for(u32 denied:{u32(WOLF_FB_NO_LOOK),u32(WOLF_FB_DEAD),u32(WOLF_FB_WON)}) {
  wolf.flags=WOLF_FB_FROZEN|WOLF_FB_LOOK_ALLOWED|denied; assert(!wolf.CanLook());
 }
 wolf.flags=WOLF_FB_FROZEN|WOLF_FB_LOOK_ALLOWED;
 g_wolfMoveBank0[0].states[0].flags=0; assert(!wolf.CanLook());
 g_wolfMoveBank0[0].states[0].flags=WSF_LOOK_MODE_OK;
 float dx,dy;
 Platform_MouseCameraFrame(1,0,&dx,&dy); assert(s_mouseCaptured && !visible);
 s_cameraMouseX=12;s_cameraMouseY=-9;
 Platform_MouseCameraFrame(1,0,&dx,&dy);assert(dx==12&&dy==-9);
 Platform_MouseCameraFrame(0,0,&dx,&dy);assert(!s_mouseCaptured&&!visible&&dx==0); // cinematic
 Platform_MouseCameraFrame(0,1,&dx,&dy);assert(!s_mouseCaptured&&visible); // menu
 Platform_MouseCameraFrame(1,0,&dx,&dy);assert(s_mouseCaptured&&!visible&&dx==0);
 active=false;Platform_MouseCameraFrame(1,0,&dx,&dy);assert(!s_mouseCaptured&&visible);
 active=true;Platform_MouseCameraFrame(1,0,&dx,&dy);assert(s_mouseCaptured&&!visible);
 s_menuPointerRequested=true;Platform_MouseCameraFrame(1,0,&dx,&dy);assert(!s_mouseCaptured&&visible);
 Platform_MouseCameraFrame(1,0,&dx,&dy);assert(s_mouseCaptured&&!visible); // dialog closed
 puts("PASS: frozen VIEW lesson; normal/forbidden look; cinematic/menu/focus cursor transitions");
}
'''
savepre=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include "sdw_enums.h"
using s32=int32_t;using s8=int8_t;using u8=uint8_t;using u32=uint32_t;
int readResult=CARD_READ_OK, writes=0, copies=0;
int Card_Access(int slot){assert(slot==4);return readResult;}
int g_mcardImageCached; u8 g_cardBlocks[4][44]{}; char g_cardBlockState[4]={'E','E','E','E'};
u32 g_saveImageTimestamp=0,g_rawTime=123,g_saveImageStatus=0,g_saveImageCrc=0;
s8 g_saveLastSlot=0; u8 g_saveCardHeader[512]{};const char *g_mcardFileName="test";
struct Progress {void CopyRecord(void *p){memset(p,0x51,44);++copies;} void SetSaveTimestamp(u32){};} progress,*g_pProgress=&progress;
int CardStub_Status3(const char*){return CARD_PREWRITE_OK;}
u32 Crc32(void*,int){return 123;}
int Card_WriteFrames(const char*,int,void*){++writes;return CARD_WRITE_OK;}
'''
savechecks=r'''
int main(){
 assert(Card_CommitBlock(4,0)==CARD_WRITE_OK && copies==0 && writes==1);
 assert(Card_CommitBlock(-1,0)==CARD_CORRUPT && Card_CommitBlock(5,0)==CARD_CORRUPT);
 for(int i=0;i<4;++i)assert(Card_CommitBlock(i,0)==CARD_WRITE_OK);
 assert(copies==4 && writes==5);
 g_cardBlocks[0][0]=0x18;
 assert(Card_CommitBlock(0,0)==CARD_SLOT_FULL && g_cardBlocks[0][0]==0x18 && copies==4);
 assert(Card_CommitBlock(0,1)==CARD_WRITE_OK && copies==5);
 readResult=CARD_CORRUPT;assert(Card_CommitBlock(1,1)==CARD_CORRUPT && copies==5);
 puts("PASS: empty image; four slots; overwrite/refusal; corrupt image; invalid indices (UBSan)");
}
'''
parts=[function('src/game/wolf_misc.cpp',n) for n in ['Wolf::CanControl','Wolf::CanLook']]
parts += [function('src/objects/daffytraininglevel.cpp',n) for n in ['DaffyTrainingLevel::EatTextTag','DaffyTrainingLevel::ParseTextTags']]
parts += [function('src/platform/platform_sdl.cpp','Platform_MouseCameraFrame')]
# Match the allocated snapshot size to the actual serialization width.
pause=(root/'src/engine/pause_menu.cpp').read_text()
assert re.search(r'g_progressPauseCopy\[0x2c\]',pause)
with tempfile.TemporaryDirectory(prefix='sdw-regression-') as d:
 for name,source in [('input', '#include <initializer_list>\n'+pre+'\n'.join(parts)+checks),
                     ('save',savepre+function('src/objects/mcard.cpp','Card_CommitBlock')+savechecks)]:
  src=pathlib.Path(d)/f'{name}.cpp';exe=src.with_suffix('');src.write_text(source)
  subprocess.run(['clang++','-std=c++17','-fsanitize=undefined','-fno-sanitize-recover=all',
                  '-I'+str(root/'src/include'),str(src),'-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True,timeout=20)
