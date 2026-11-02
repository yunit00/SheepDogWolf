/*
 * This object is BsPolyTexGouraud's constructor and destructor. Its ??_G is dropped: the vtable's ??_E slot is a weak
 * external that binds to the real ??_E BsPolyTexGouraud_VectorDeletingDtor. BsPolyTexGouraud derives from PolyTri: its
 * constructor calls PolyTri's and its destructor ~PolyTri, so PolyTri's constructor is declared here.
 */
#include "sdw_types.h"

#define SDW_MEMBERS_PolyTri PolyTri();

#define SDW_MEMBERS_BsPolyTexGouraud BsPolyTexGouraud();
#include "sdw_classes.h"
#define SDW_INLINE_POLYTRI_OPERATOR_CONST_POLYTRI 1
#include "poly_tri_inlines.h"
#undef SDW_INLINE_POLYTRI_OPERATOR_CONST_POLYTRI

BsPolyTexGouraud::BsPolyTexGouraud() {}

BsPolyTexGouraud::~BsPolyTexGouraud() {}
