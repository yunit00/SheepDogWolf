/*
 * Heap - the engine's block allocator.
 *
 * A heap manages one caller-supplied arena. A block is {u32 sizeFlags; HeapBlock *next; HeapBlock *prev; ...}: the size
 * (header included) in bits 2..27, bit 1 = this block is free, bit 0 = the block before it is free (its size is then in
 * the u32 just below this header, its footer), bits 28..31 = log2(align)-2 for Heap_AllocAligned blocks. Free blocks
 * form a doubly linked list headed by freeHead, whose prev points to itself; a permanent 0x10-byte end sentinel ends
 * the arena. An allocated block keeps the magic 0x98765432 in its second word, which nothing checks. Heap_Alloc is a
 * bounded best fit (see there). A second allocator, bitmap pools for small sizes, is set up only for arenas above
 * 1 MB; neither shipped heap (the 512 KB level heap g_scenaricHeap, the 32 KB list-node heap g_listNodeHeap
 *) is that large, so it is dead code in the shipped game.
 *
 * Casts: the heap computes block addresses from raw byte offsets, so its pointer / integer casts are kept; each
 * function that has them says so.
 */
#define SDW_MEMBERS_Heap void SetFreeHead(HeapBlock *b);
#include "sdw_classes.h"

u32 Heap_CeilPow2(u32 x);
u8 Heap_Log2Floor(u32 x);

#define HEAP_SIZE_MASK 0x0ffffffc /* bits 2..27 of sizeFlags: the block size */
#define HEAP_PREV_FREE 1
#define HEAP_FREE 2
#define HEAP_MAGIC 0x98765432 /* written into an allocated block's second word, never checked */

/* ---- inline helpers ---- */

/* Makes b a free block of `size` bytes linked between prev and next, and writes its footer (the size, in the last u32
 * of the block) and the successor's "previous block is free" bit. */
inline void HeapBlock_SetFree(HeapBlock *b, HeapBlock *prev, HeapBlock *next, u32 size)
{
    u32 *footer;
    b->next = next;
    b->prev = prev;
    b->sizeFlags = (size & 0x0fffffff) | HEAP_FREE;
    footer = (u32 *)((u8 *)b + size - 4);
    *footer = size;
    footer++;
    *footer |= HEAP_PREV_FREE;
}

/* b becomes the head of the free list (the head's prev points to itself). */
inline void Heap::SetFreeHead(HeapBlock *b)
{
    freeHead = b;
    freeHead->prev = freeHead;
}

/* Links b after a on the free list. */
inline void HeapBlock_Link(HeapBlock *a, HeapBlock *b)
{
    a->next = b;
    a->next->prev = a;
}

/* Size of the block before b if that block is free (its footer, the u32 just below b), else 0. */
inline u32 HeapBlock_PrevFreeSize(HeapBlock *b)
{
    u32 *footer;
    if (b->sizeFlags & HEAP_PREV_FREE) {
        footer = (u32 *)b - 1;
        return *footer;
    }
    return 0;
}

/* Block size in bytes, header included. */
inline u32 HeapBlock_Size(HeapBlock *b)
{
    return b->sizeFlags & HEAP_SIZE_MASK;
}

/* Records log2(align)-2 in bits 28..31 of the header. */
inline void HeapBlock_SetAlignShift(HeapBlock *b, u8 shift)
{
    b->sizeFlags |= shift << 28;
}

/* x rounded up to a multiple of align (a power of 2). */
inline u32 Heap_AlignUp(u32 x, u32 align)
{
    align--;
    return (x + align) & ~align;
}

/* ---- the heap ----
 * The original's block format ({u32 sizeFlags; HeapBlock *next; HeapBlock *prev}, an 8-byte allocated header, 4-byte
 * alignment) assumes 4-byte pointers. This build keeps what callers can observe - blocks come from the caller's arena
 * (so a zero-filled arena gives zero-filled first allocations), Alloc returns 0 when the arena is full, and Init
 * forgets every block - with a plain first-fit free list that works at any pointer size. Only Init, Term, Alloc and
 * Free are used outside this file. The Heap fields are reused: firstBlock = the arena start, freeHead = the free list
 * (sorted by address), endSentinel = the arena end. */
struct ModernHeapBlock {
    uptr size;
    ModernHeapBlock *next; /* free blocks only: the next free block, by address */
};
#define MODERN_HEAP_ALIGN 16
#define MODERN_HEAP_HEADER 16 /* >= sizeof(ModernHeapBlock), and keeps user pointers 16-aligned */

static uptr ModernHeap_Round(uptr x)
{
    return (x + MODERN_HEAP_ALIGN - 1) & ~(uptr)(MODERN_HEAP_ALIGN - 1);
}

void Heap::Init(u8 *block, u32 size)
{
    uptr start = ModernHeap_Round((uptr)block);
    uptr end = ((uptr)block + size) & ~(uptr)(MODERN_HEAP_ALIGN - 1);
    ModernHeapBlock *all = (ModernHeapBlock *)start;
    all->size = end - start;
    all->next = 0;
    firstBlock = (HeapBlock *)start;
    freeHead = (HeapBlock *)all;
    endSentinel = (HeapBlock *)end;
}

void Heap::Term()
{
    firstBlock = 0;
    freeHead = 0;
    endSentinel = 0;
}

void *Heap::Alloc(u32 size)
{
    uptr need = ModernHeap_Round((uptr)size + MODERN_HEAP_HEADER);
    ModernHeapBlock *prev = 0;
    ModernHeapBlock *b = (ModernHeapBlock *)freeHead;
    while (b && b->size < need) {
        prev = b;
        b = b->next;
    }
    if (!b)
        return 0;
    ModernHeapBlock *rest = b->next;
    if (b->size - need >= MODERN_HEAP_HEADER + MODERN_HEAP_ALIGN) { /* split: the tail stays free */
        rest = (ModernHeapBlock *)((u8 *)b + need);
        rest->size = b->size - need;
        rest->next = b->next;
        b->size = need;
    }
    if (prev)
        prev->next = rest;
    else
        freeHead = (HeapBlock *)rest;
    return (u8 *)b + MODERN_HEAP_HEADER;
}

void Heap::Free(void *ptr)
{
    if (!ptr)
        return;
    ModernHeapBlock *b = (ModernHeapBlock *)((u8 *)ptr - MODERN_HEAP_HEADER);
    ModernHeapBlock *prev = 0;
    ModernHeapBlock *cur = (ModernHeapBlock *)freeHead;
    while (cur && cur < b) {
        prev = cur;
        cur = cur->next;
    }
    b->next = cur;
    if (prev)
        prev->next = b;
    else
        freeHead = (HeapBlock *)b;
    if (cur && (u8 *)b + b->size == (u8 *)cur) { /* merge with the free block after it */
        b->size += cur->size;
        b->next = cur->next;
    }
    if (prev && (u8 *)prev + prev->size == (u8 *)b) { /* and with the one before it */
        prev->size += b->size;
        prev->next = b->next;
    }
}
