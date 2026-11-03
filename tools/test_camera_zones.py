#!/usr/bin/env python3
"""Exercise actual follow/directed camera functions with isolated world/renderer adapters.
Checks authored-zone locks, directed movement, collision distance and legacy fallback.
This is not an end-to-end test of every corridor in the original levels.
"""
from pathlib import Path
import re, subprocess, tempfile
root = Path(__file__).resolve().parents[1]
src = (root / 'src/objects/camera.cpp').read_text()
def function(name):
    m = re.search(r'^(?:static )?\w+[ *]+%s\([^;]*?\)\n\{' % name, src, re.M)
    assert m, name
    end, depth = m.end(), 1
    while depth:
        depth += (src[end]=='{')-(src[end]=='}'); end += 1
    return src[m.start():end]
fixture=r'''
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "sdw_enums.h"
using s16=short; using u16=unsigned short; using s32=int; using u32=unsigned; using u8=unsigned char;
#define ABS_VALUE(x) std::abs(x)
#define ANGLE_DELTA(a,b) ((((a)-(b)+0x800)&0xfff)-0x800)
#define DELTA_ANGLE(a,b) ANGLE_DELTA(a,b)
struct Vec3s {s16 x=0,y=0,z=0;}; struct Vec3i {s32 x=0,y=0,z=0;};
struct Camera {Vec3s pos,rot; s16 dist=600;};
struct Pad { struct Data {struct {int type=0;} typeLen; int rightX=0,rightY=0; u16 buttons=0xffff;} cur,prev;};
Pad g_pad; bool captured=true;
int Platform_MouseCameraCaptured(){return captured;}
struct CamModeParams {s32 pitchSpeed=10,yawSpeed=10,rollSpeed=10; s16 dist=600,focal=400;};
struct Trajectory{};
struct CamRestrict {
 struct {bool modeFollow=true,modeRobot=true,modeRun=true,forbidPad=false,mirrorYaw=false,snapYaw=false;} flags;
 Trajectory *trajectory=nullptr;
 s16 pitchMin=0,pitchMax=0,yawMin=0,yawMax=0,rollMin=0,rollMax=0,distMin=0,distMax=500,focal=450;
 s32 pitchSpeed=10,yawSpeed=10,rollSpeed=10; Vec3s aimOffset; s16 eyeVertMax=32000;
};
struct {bool useYaw=false,usePitch=false,snapYaw=false,restriction=false,padRotate=false;} g_camReqFlags;
struct {bool smooth=false,snap=false,manual=false;} g_camFlags;
struct {struct {bool cut=false;}bits;} g_camStateFlags;
struct {int state=0;}g_camPitchAdjState;
CamModeParams g_camModeParams[20]; CamRestrict *g_camRestriction=nullptr;
Vec3s g_camReqAimOffset,g_camReqRot,g_camReqVel,g_camReqTarget,g_camAimOffset,g_camLastHitTarget,g_camBlockPosNeg,g_camBlockPosPos;
u8 g_camMode=CAM_FOLLOW; int g_camReqFocal=0,g_camReqDist=0,g_camReqSpeedFactor=4096;
int g_camHitFrames=0,g_camZoomOut=0,g_camAutoYawRate=0,g_camAutoPitchRate=0,g_camMinPitch=0;
int g_camAutoYawGain=0,g_camYawVel=0,g_camPitchVel=0,g_camLedgeLookState=LEDGE_NONE,g_camTrajSegment=0;
int g_camSettings=CAMSET_COLLISION|CAMSET_LEDGE_LOOK,g_dt=4096;
Trajectory *g_camTrajectory=nullptr; u16 g_padMasks[16]{};
bool s_mouseOrbitActive=true; s16 s_mouseOrbitPitch=100,s_mouseOrbitYaw=800; u8 s_camManualPitch=0,s_camManualYaw=0;
struct Wolf {bool controls=true,look=false; int state=0; bool CanControl(){return controls;}bool CanLook(){return look;}} wolf,*g_pWolf=&wolf;
struct {bool active=false;bool IsActive(){return active;}}g_cinePlayer;
int trajectoryCalls=0,ledgeCalls=0,collisionCalls=0,legacySearch=-1,restrictionFocal=0;
s16 clearDistance=120;
void Pad_StickToDeadzonedAxes(int,int,s32*x,s32*y){*x=*y=0;}
int Vec3s_ManhattanDist(Vec3s*,Vec3s*){return 100;}
void Camera_OnManualRotate(s32){}
s32 Camera_ManualTurnStep(s32 input,s32,s32){return input;}
void Camera_FollowTrajectory(Trajectory*,Vec3s*a,s16*,s32*,s32){++trajectoryCalls;a->x=512;a->y=1024;}
void Camera_SmoothToDesired(Camera*,Vec3s*,const s32*,s16*,s16 focal,const Vec3s*aim,s32){g_camAimOffset=*aim;restrictionFocal=focal;}
s16 Camera_OrbitClearDistance(const Vec3s*,const Vec3s*,s16 dist){++collisionCalls;return dist<clearDistance?dist:clearDistance;}
void Vec3s_OffsetAlongAngles(Vec3s*eye,const Vec3s*,s16 dist,const Vec3s*target){*eye=*target;eye->z+=dist;}
void Camera_SolveCollision(Camera*c,Vec3s*a,s16 dist,s32 search,s16){c->rot=*a;c->dist=dist;legacySearch=search;}
void Camera_UpdateLedgeLook(Camera*){++ledgeCalls;}
void Platform_Log(const char*,...){}
'''
checks=r'''
int main(){
 Pad pad; Camera camera; CamRestrict zone; Trajectory track;
 g_camRestriction=&zone; zone.aimOffset={5,7,9};
 for(int mode:{CAM_FOLLOW,CAM_FOLLOW_RUN,CAM_ROBOT,CAM_DIRECTED}) {
  g_camMode=mode;
  for(int mask=0;mask<32;++mask) {
   zone.flags.forbidPad=mask&1;zone.trajectory=(mask&2)?&track:nullptr;
   g_camReqFlags.usePitch=mask&4;g_camReqFlags.useYaw=mask&8;zone.flags.snapYaw=mask&16;
   g_camReqFlags.restriction=true;g_camLedgeLookState=LEDGE_LOOKING;
   camera.rot={512,1024,0}; camera.dist=600;g_camPitchVel=99;g_camYawVel=99;
   if(mode==CAM_DIRECTED)Camera_UpdateDirected(&camera,&pad);else Camera_UpdateFollow(&camera,&pad);
   assert(camera.rot.x==100 && camera.rot.y==800);
   assert(camera.dist==120 && legacySearch==0 && restrictionFocal==450);
   assert(g_camAimOffset.x==5 && g_camAimOffset.y==7 && g_camAimOffset.z==9);
   assert(g_camPitchVel==0 && g_camYawVel==0 && g_camLedgeLookState==LEDGE_NONE);
  }
 }
 assert(collisionCalls==128 && trajectoryCalls==0 && ledgeCalls==0);
 // Moving the mouse again changes both angles, with the same collision protection.
 s_mouseOrbitPitch=3900;s_mouseOrbitYaw=2400;Camera_UpdateDirected(&camera,&pad);
 assert(camera.rot.x==3900 && camera.rot.y==2400 && camera.dist==120);
 // Cinematics, frozen actions, death (CanControl/CanLook=false), and special
 // camera modes must not be replaced by the mouse orbit.
 g_cinePlayer.active=true;assert(!Camera_PlayerOwnsOrbit());g_cinePlayer.active=false;
 wolf.controls=false;assert(!Camera_PlayerOwnsOrbit());wolf.look=true;assert(Camera_PlayerOwnsOrbit());
 wolf.look=false;wolf.controls=true;
 for(int mode:{CAM_SCRIPTED,CAM_SCRIPT_BLEND_IN,CAM_SCRIPT_RETURN,CAM_FIXED_LOOKAT,CAM_ROCKET,CAM_SAM_CHASE}) {
  g_camMode=mode;assert(!Camera_PlayerOwnsOrbit());
 }
 g_camMode=CAM_FOLLOW;s_mouseOrbitActive=false;Camera_UpdateFollow(&camera,&pad);
 assert(trajectoryCalls==1 && camera.rot.x==512 && camera.rot.y==1024);
 assert(Camera_UseManualZoneView()); // keyboard can recover an already fixed zone shot
 captured=false;assert(!Camera_UseManualZoneView());captured=true;
 g_pad.cur.typeLen.type=PADTYPE_ANALOG;assert(!Camera_UseManualZoneView());
 s_mouseOrbitActive=true;assert(Camera_UseManualZoneView());
 g_cinePlayer.active=true;assert(!Camera_UseManualZoneView());g_cinePlayer.active=false;
 wolf.controls=false;wolf.look=true;assert(!Camera_UseManualZoneView()); // frozen NPC lesson
 g_pWolf=nullptr;assert(!Camera_UseManualZoneView());
 puts("PASS: 128 zone/mode combinations; area-shot eligibility; manual angles and collision preserved; directed states; cinematic/special/legacy fallback");
}
'''
with tempfile.TemporaryDirectory(prefix='sdw-camera-zones-') as d:
 p=Path(d)/'zones.cpp';exe=p.with_suffix('')
 names=['Camera_IsMouseOrbitMode','Camera_PlayerOwnsOrbit','Camera_UseManualZoneView','Camera_ClampAngleToArc','Camera_ApplyRestriction','Camera_UpdateFollow','Camera_UpdateDirected']
 p.write_text('#include <initializer_list>\n'+fixture+'\n'.join(map(function,names))+checks)
 subprocess.run(['clang++','-std=c++17','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(root/'src/include'),str(p),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True,timeout=20)
