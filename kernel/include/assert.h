#ifndef __assert__
#define __assert__

#define assert(e) ((e) ? (void) 0: panic(#e))

#endif
