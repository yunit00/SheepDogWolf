#!/usr/bin/env python3
"""Compile actual inventory functions in an isolated fixture (no renderer/world).
This checks collection lists and carousel state, not each item's level-specific action.
Run with Python 3 and a C++ compiler; no original game resources are needed.
"""
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]
source = (root / 'src/engine/progress_inventory.cpp').read_text()
names = ['Inventory_CountClass', 'Inventory_NextClass', 'Inventory_PrevClass',
         'Inventory_Add', 'Inventory_Remove', 'Inventory_GetSelectedObject',
         'Inventory_SelectClass', 'Inventory_SelectNext', 'Inventory_SelectPrev',
         'Inventory_ClearSelection']
functions = []
for name in names:
    import re
    match = re.search(r'^\w+[ *]+%s\([^;]*?\)\n\{' % name, source, re.M)
    if not match:
        raise RuntimeError('Missing production function: ' + name)
    start = match.start()
    pos = match.end()
    depth = 1
    while depth:
        depth += (source[pos] == '{') - (source[pos] == '}')
        pos += 1
    functions.append(source[start:pos])
fixture = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
using u16=uint16_t; using u32=uint32_t; using s32=int32_t;
constexpr u16 CLASSID_NONE=0xffff;
constexpr int SCN_CF_COMPOSITE_ITEM=1, MSG_ITEM_SPLIT_QUERY=1;
struct ScnObject {
    u16 id; bool kept=false;
    u16 GetClassId() { return id; }
    bool IsKept() { return kept; }
    void SetKept(int value) { kept=value; }
    int HandleMessage(int,int,ScnObject**) { return 0; }
};
struct ListNode { ListNode *next; void *data; };
ListNode *g_inventoryLists[200]{};
u16 g_invSelClass=CLASSID_NONE;
int g_inventoryAllCommit=1, g_invWheelTurn=0, g_inventoryWheelYaw=0xc00;
int Scenaric_ClassFlags(u16) { return 0; }
ListNode *List_First(ListNode **p) { return *p; }
u32 List_Count(ListNode **p) { u32 n=0; for(auto v=*p;v;v=v->next) ++n; return n; }
void List_AllocateNode(ListNode **p) { *p=new ListNode{}; }
void List_PushFront(ListNode **p,ListNode *n) { n->next=*p; *p=n; }
void List_Remove(ListNode **p,ListNode *n) { while(*p && *p!=n) p=&(*p)->next; if(*p) *p=n->next; }
void List_FreeNode(ListNode *p) { delete p; }
ScnObject *Inventory_GetSelectedObject();
'''
checks = r'''
int main() {
    assert(Inventory_GetSelectedObject()==nullptr);
    assert(Inventory_NextClass(CLASSID_NONE)==CLASSID_NONE);
    assert(Inventory_PrevClass(CLASSID_NONE)==CLASSID_NONE);
    assert(Inventory_SelectNext()==nullptr && Inventory_SelectPrev()==nullptr);
    ScnObject first{12}, second{12}, third{87}, last{199};
    Inventory_Add(&first); Inventory_Add(&second);
    assert(Inventory_CountClass(12)==2 && !g_inventoryAllCommit);
    Inventory_SelectClass(12);
    assert(Inventory_GetSelectedObject()==&second);
    Inventory_Remove(&second);
    assert(Inventory_CountClass(12)==1 && Inventory_GetSelectedObject()==&first);
    Inventory_ClearSelection();
    for(int i=0;i<2 && !Inventory_GetSelectedObject();i++) Inventory_SelectNext();
    assert(Inventory_GetSelectedObject()==&first);
    Inventory_SelectNext(); assert(Inventory_GetSelectedObject()==nullptr);
    for(int i=0;i<2 && !Inventory_GetSelectedObject();i++) Inventory_SelectPrev();
    assert(Inventory_GetSelectedObject()==&first);
    Inventory_Add(&third); Inventory_Add(&last); Inventory_ClearSelection();
    assert(Inventory_SelectNext()==&first);
    assert(Inventory_SelectNext()==&third);
    assert(Inventory_SelectNext()==&last);
    assert(Inventory_SelectNext()==nullptr);
    assert(Inventory_SelectPrev()==&last);
    assert(Inventory_SelectPrev()==&third);
    Inventory_Remove(&third);
    assert(Inventory_GetSelectedObject()==nullptr && Inventory_CountClass(87)==0);
    Inventory_SelectClass(87); assert(Inventory_GetSelectedObject()==nullptr);
    Inventory_Remove(&third); // removal twice must not alter other stacks
    assert(Inventory_CountClass(12)==1 && Inventory_CountClass(199)==1);
    Inventory_Remove(&first); Inventory_Remove(&last);
    assert(Inventory_SelectNext()==nullptr && Inventory_SelectPrev()==nullptr);
    puts("Inventory PASS: empty, pickup stacks, both directions, empty hands, consumed selection, repeated removal, absent class, final empty state.");
}
'''
with tempfile.TemporaryDirectory(prefix='sdw-inventory-') as temp:
    path = pathlib.Path(temp)
    (path/'test.cpp').write_text(fixture+'\n'+'\n'.join(functions)+'\n'+checks)
    subprocess.run(['c++','-std=c++17','-fsanitize=undefined','-fno-sanitize-recover=all',str(path/'test.cpp'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
