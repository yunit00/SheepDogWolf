#ifndef SDW_ENGINE_LIST_H
#define SDW_ENGINE_LIST_H

/* The functions and globals list.cpp defines, declared once for every file that uses them. */
#include "sdw_types.h"

struct Dav;
struct ListNode;

void List_AllocateNode(ListNode **nodeOut);
u32 List_Count(ListNode **list);
void List_Create(ListNode ***listOut);
void List_FreeNode(void *node);
void List_PushFront(ListNode **list, ListNode *node);
void List_Remove(ListNode **list, ListNode *node);
u8 Load_DAVnWAR(const char *levelPath, Dav *dav);
void Load_FreeLevel();
void Scratch32k_Alloc();
void Scratch32k_Free();
void Scratch32k_Install();

#endif
