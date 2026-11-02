#ifndef SDW_ENGINE_DRAW2D_H
#define SDW_ENGINE_DRAW2D_H

/* The functions and globals draw2d.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

class D3DApp;
class Frustrum;
class InputMgr;
class Mat44;
class PolyBatcher;
class SoundDevice;
class TextResBank;

extern char g_dirBonusGame[];       /* exeDir + ".\Bonus\" */
extern char g_dirReference[];
extern InputMgr g_inputMgr;
extern char g_introDir[];
extern char g_levelPathFmt[];       /* exeDir + ".\Levels\Lvl-%02d\Lvl-%02d" */
extern Mat44 g_matUnk6d5428;
extern u32 g_maxImmediateTriangles;
extern char g_musicsPath[];
extern D3DApp *g_pD3DAppMain;
extern PolyBatcher *g_pPolyBin;
extern SoundDevice *g_pSoundSystem;
extern Frustrum *g_pViewFrustum;
extern char g_pathDemoDir[];
extern char g_pathEnding[];
extern char g_pathFendDir[];
extern char g_pathScene[];
extern char g_pathWheelDir[];
extern Mat44 g_projMatrix;          /* the projection matrix */
extern TextResBank g_textCatalog;
extern char g_voiceDir[];
void Draw2D_FlatRect(float z, float x0, float y0, float x1, float y1, u32 color, u8 blendMode);
void Draw2D_FlatRect_Immediate(float z, float x0, float y0, float x1, float y1, u32 renderFlags,
                               u32 color);
void Draw2D_FlatTri(float z, float x0, float y0, float x1, float y1, float x2, float y2, u32 color,
                    u8 blendMode);
void Draw2D_GouraudRect(float z, float x0, float y0, float x1, float y1, u32 cTL, u32 cBL, u32 cTR, u32 cBR,
                        u8 blendMode);

#endif
