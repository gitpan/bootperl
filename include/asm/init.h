#if !defined(ASM_INIT_H)
#define ASM_INIT_H

/* taken from linux */

/* for c routines */
#define __init __attribute__ ((__section__ (".text.init")))
#define __initdata __attribute__ ((__section__ (".data.init")))
#define __initfunc(__arginit) \
        __arginit __init; \
        __arginit

/* for assembly routines */
#if defined(__ASSEMBLY__)
#define __INIT          .section        ".text.init",#alloc,#execinstr
#define __FINIT .previous
#define __INITDATA      .section        ".data.init",#alloc,#write
#endif /* defined(__ASSEMBLY__) */

#endif /* !defined(ASM_INIT_H) */
