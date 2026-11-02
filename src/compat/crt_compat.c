
#define time ucrt_time  /* keep the header's inline version out of the way of ours */
#include <time.h>
#undef time

long __cdecl time(long *t)
{
    return (long)_time32((__time32_t *)t);
}
