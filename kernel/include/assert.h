#ifndef __assert__
#define __assert__

#define assert(e) ((e) ? (void) 0 : printf("ASSERT FAILED: %s %s:%d:%s\n", #e, __FILE__, __LINE__, __func__))

#endif
