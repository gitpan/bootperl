#if !defined(ASM_LOCK_H)
#define ASM_LOCK_H

#ifdef __KERNEL__
extern long _kernel_lock;

static void inline lock_kernel(void) { __asm__ ("cli"); _kernel_lock++; }
static void inline unlock_kernel(void) { if (--_kernel_lock == 0) __asm__ ("sti"); }

static void inline _save_flags(long *flags)
{
  __asm__ __volatile__ ("pushfl ; popl %0" : "=g" (*flags) : : "memory");
}
static void inline _restore_flags(long flags)
{
  __asm__ __volatile__ ("pushl %0 ; popfl" : : "g" (flags) : "memory");
}

#define lock_flags(x) do { _save_flags(&x); lock_kernel(); } while(0)
#define unlock_flags(x) do { unlock_kernel(); _restore_flags(x); } while(0)
#endif /* __KERNEL__ */

#endif /* !defined(ASM_LOCK_H) */
