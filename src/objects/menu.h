#ifndef SDW_OBJECTS_MENU_H
#define SDW_OBJECTS_MENU_H

/* The functions and globals menu.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct Menu;
struct MenuPage;

/* Ends Menu_AddItems' list of handlers. It must be a pointer: a plain 0 is passed as a 4-byte int, and the list is read
 * back as 8-byte pointers, so on a 64-bit target the end would not be seen. */
#define MENU_END ((void *)0)

MenuPage *Menu_AddPage(MenuPage *parent, void *labelOrHandler, u8 hasHandler);
void Menu_Close();
void Menu_GoBack();
void Menu_Init(Menu *menu);
void Menu_ResetToRoot(Menu *menu, s16 cursor);
void Menu_SetCapture(u8 captureMode, int notifyOnRelease);
void Menu_SetCurrent(Menu *menu);
void Menu_SetDirty(u32 on);
u32 Menu_Update(s32 layout, u8 align);

#endif
