/* The PlayStation version's .DAV (texture pack), converted on load into the PC's VDX7 layout, so that the PC readers
 * (Load_DAV, PolyBatcher::LoadTexturePages, Vdx7) take it unchanged. See psx_dav.cpp for the format. */
#ifndef SDW_PSX_DAV_H
#define SDW_PSX_DAV_H

#include "sdw_types.h"

/* the u32 at offset 4 of a converted image ("PSXC"; a PC .DAV has "CHEK" there): its pages are not the PC's, so its
 * page overrides are in retexture-psx/, not retexture/ */
#define PSXDAV_TAG 0x43585350u

/* whether data (size bytes: the whole file, or at least its first 0x44) is a PlayStation .DAV */
int PsxDav_Is(const void *data, u32 size);

/* the VDX7 image of a PlayStation .DAV, in a new[]'d buffer of *outSize bytes; 0 (and *outSize 0) when the file is
 * malformed. The last conversion is cached: a level's three readers convert it once. */
char *PsxDav_ToVdx7(const void *data, u32 size, u32 *outSize);

#endif
