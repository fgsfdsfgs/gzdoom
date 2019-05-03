#if __arm__

#define f_QNAN  0xffffffff

#define d_QNAN0 0xffffffff
#define d_QNAN1 0xffffffff

#elif __mips__

#define f_QNAN  0x7fbfffff

#define d_QNAN0 0x7ff7ffff
#define d_QNAN1 0xffffffff

#else

#define f_QNAN  0xffc00000

#define d_QNAN0 0x00000000
#define d_QNAN1 0xfff80000

#endif

#if __LP64__
#define ld_QNAN0 0x7fff8000
#define ld_QNAN1 0x00000000
#define ld_QNAN2 0x00000000
#define ld_QNAN3 0x00000000
#endif
