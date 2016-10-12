// x86 trap and interrupt constants.

// Processor-defined:
#define DIVIDE         0      // divide error
#define DEBUG          1      // debug exception
#define NMI            2      // non-maskable interrupt
#define BRKPT          3      // breakpoint
#define OFLOW          4      // overflow
#define BOUND          5      // bounds check
#define ILLOP          6      // illegal opcode
#define DEVICE         7      // device not available
#define DBLFLT         8      // double fault
// #define COPROC      9      // reserved (not used since 486)
#define TSS           10      // invalid task switch segment
#define SEGNP         11      // segment not present
#define STACK         12      // stack exception
#define GPFLT         13      // general protection fault
#define PGFLT         14      // page fault
// #define RES        15      // reserved
#define FPERR         16      // floating point error
#define ALIGN         17      // aligment check
#define MCHK          18      // machine check
#define SIMDERR       19      // SIMD floating point error

// These are arbitrarily chosen, but with care not to overlap
// processor defined exceptions or interrupt vectors.
#define SYSCALL       64      // system call
#define DEFAULT      500      // catchall

#define IRQ0          32      // IRQ 0 corresponds to int IRQ

#define IRQ_TIMER        0
#define IRQ_KBD          1
#define IRQ_COM1         4
#define IRQ_IDE         14
#define IRQ_ERROR       19
#define IRQ_SPURIOUS    0xDF

