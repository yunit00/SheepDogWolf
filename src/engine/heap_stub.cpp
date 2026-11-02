#include "sdw_types.h"

/* returns 0. No call to it; the only reference is the pointer below. */
s32 Stub_Ret0_54d500(void)
{
    return 0;
}

/* the address of the stub above; no instruction reads it. */
s32 (*g_pStubRet0_54d500)(void) = Stub_Ret0_54d500;
