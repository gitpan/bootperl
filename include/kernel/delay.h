#if !defined(KERNEL_DELAY_H)
#define KERNEL_DELAY_H

/*
 * Copyright (C) 1993 Linus Torvalds
 *
 * Delay routines calling functions in arch/i386/lib/delay.c
 * adapted for use in `os' by bsmith august 2001
 */
 
extern void __bad_udelay(void);

extern void __udelay(unsigned long usecs);
extern void __const_udelay(unsigned long usecs);
extern void __delay(unsigned long loops);

#define udelay(n) (__builtin_constant_p(n) ? \
        ((n) > 20000 ? __bad_udelay() : __const_udelay((n) * 0x10c6ul)) : \
        __udelay(n))

#endif /* KERNEL_DELAY_H */
