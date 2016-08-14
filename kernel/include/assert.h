#ifndef __assert__
#define __assert__

#define assert(e) ((e) ? (void) 0: print_assert(#e, __FILE__, __LINE__))

#endif
