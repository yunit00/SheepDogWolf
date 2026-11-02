/* Pointer fields inside the level files' records (.WAR.DAV).
 *
 * The files store these fields as 32-bit offsets from the start of the file, and the original loaders overwrite them in
 * place with addresses. An address does not fit in 32 bits on a 64-bit target, so the game leaves
 * the offsets in place and resolves them when read: SDW_WARPTR(T) / SDW_DAVPTR(T) is a 4-byte field that behaves like a
 * T * (->, and conversion to T *). The record layouts and addresses stay those of the file.
 *
 * The bases are the loaded blobs of the current level (set by Load_WAR / Load_DAV); an offset of 0 is NULL. */
#ifndef SDW_FILEPTR_H
#define SDW_FILEPTR_H

#include "sdw_types.h"

extern u8 *g_warFileBase; /* the WAR blob: offsets in .WAR records are from here */
extern u8 *g_davFileBase; /* the DAV blob: offsets in .DAV records are from here */

template <class T, u8 **Base> struct SdwFilePtr {
    u32 off; /* the file offset, as stored in the file; 0 = none */
    T *get() const { return off ? (T *)(*Base + off) : 0; }
    operator T *() const { return get(); }
    T *operator->() const { return get(); }
    template <class U> explicit operator U *() const { return (U *)get(); } /* (u8 *)ptr, as with a pointer */
    /* the game stores addresses inside the same file (CineRecord.keyCursor): kept as their offset */
    SdwFilePtr &operator=(T *p)
    {
        off = p ? (u32)((u8 *)p - *Base) : 0;
        return *this;
    }
};
#define SDW_WARPTR(T) SdwFilePtr<T, &g_warFileBase>
#define SDW_DAVPTR(T) SdwFilePtr<T, &g_davFileBase>

/* A 32-bit word of WAR / DAV data that holds a file offset, as an address (0 stays NULL) */
inline void *War_Resolve(uptr off) { return off ? g_warFileBase + off : 0; }
inline void *Dav_Resolve(uptr off) { return off ? g_davFileBase + off : 0; }

/* A 4-byte slot of file data that the game fills with a runtime object's address (CineTrack.obj): the game
 * stores a handle there, a number that stands for the address (0 = NULL), from one table for the whole session
 * (src/engine/load_war.cpp). SDW_OBJREF(T) behaves like a T *: ->, comparison, conversion, and assignment of a T *. */
u32 SdwObjRef_Handle(void *p); /* the handle for p, made once per address */
void *SdwObjRef_Address(u32 handle);

template <class T> struct SdwObjRef {
    u32 handle;
    T *get() const { return (T *)SdwObjRef_Address(handle); }
    operator T *() const { return get(); }
    T *operator->() const { return get(); }
    template <class U> explicit operator U *() const { return (U *)get(); } /* (Derived *)ref, as with a pointer */
    SdwObjRef &operator=(T *p)
    {
        handle = SdwObjRef_Handle(p);
        return *this;
    }
};
#define SDW_OBJREF(T) SdwObjRef<T>

#endif
