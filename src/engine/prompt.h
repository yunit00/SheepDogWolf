#ifndef SDW_ENGINE_PROMPT_H
#define SDW_ENGINE_PROMPT_H

/* The functions and globals prompt.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct DialogBox;
class ScnObject;

extern s32 g_promptActive;
extern u32 g_promptLineCount;
extern u32 g_promptMaxLineWidth;
extern s32 g_promptResult;
extern ScnObject *g_promptSender;
extern u16 g_uiSoundHandle;
u8 Dialog_Update(DialogBox *dlg);
u32 Dialog_UpdateAnswer(DialogBox *dlg, const u32 *answerVoices);
void Prompt_End();
void Ui_DrawMemCardBackdrop(u8);
void Ui_DrawScrollArrow(s16, s16, u16);
void Ui_DrawTextInRect(s16 *, u8, u32, char *);
void Ui_PlayCancelSound();
void Ui_PlayConfirmSound();
void Ui_PlayMoveSound();
u8 Ui_PromptConfirm(char *, char *);
s8 Ui_PromptYesNo(char *, char *, char *, u8 *);

/* The functions and globals prompt.cpp defines, declared once for every file that uses them. */

struct TextBox;

extern TextBox g_promptBox;

#endif
