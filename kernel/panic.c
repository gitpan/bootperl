#include <kernel/kernel.h>
#include <asm/lock.h>
#include <assert.h>
#include <kernel/task.h>
#include <kernel/bochs.h>

/* Unrecoverable kernel error */
asmlinkage void panic(const char *fmt, ...)
{
  va_list args;
  static char buf[1024];
  struct task_regs *regs = current->regs;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  lock_kernel();

  printk(KERN_EMERG "\n");
  printk(KERN_EMERG "*** Kernel panic: %s ***\n", buf);

  goto halt;

  printk(KERN_EMERG "\tregs = %08lx\n", (long)regs);
  if (regs == 0) goto skip_regs;
  printk(KERN_EMERG "\tcs:eip = %08lx:%08lx\n", regs->cs, regs->eip);
  if ((regs->cs & 7) == 0) {
    short ss;
    __asm__ ("movw %%ss, %0" : "=g" (ss));
    printk(KERN_EMERG "\tss:esp = %08hx:%08lx (no ring change)\n", ss,
	   regs->oldss);
  } else {
    printk(KERN_EMERG "\tss:esp = %08lx:%08lx\n", regs->oldss, regs->oldesp);
  }
  printk(KERN_EMERG "\tds = %08lx\tes = %08lx\n",
         regs->ds, regs->es);
  printk(KERN_EMERG "\teax = %08lx\tebx = %08lx\tecx = %08lx\tedx = %08lx\n",
	 regs->eax, regs->ebx, regs->ecx, regs->edx);
  printk(KERN_EMERG "\torig_eax = %08lx\n", regs->orig_eax);
  printk(KERN_EMERG "\tesi = %08lx\tedi = %08lx\tebp = %08lx\n",
	 regs->esi, regs->edi, regs->ebp);
 skip_regs:

  { /* do a stack frame trace */
    int i;
    unsigned long *frame = (unsigned long *)__builtin_frame_address(0);
    for (i = 0; i < 8; i++) {
      printk(KERN_EMERG "%i:\tframe = %08lx\treturn = %08lx\n",
	     i, (unsigned long)frame, frame[1]);
      frame = (unsigned long *)frame[0];
      if ((frame == 0) || ((unsigned long)frame > (32 * 1024 * 1024)))
	break;
    }
  }

  { /* print the last 8 dwords on the stack starting at the frame address */
    int i;
    unsigned long *frame = (unsigned long *)__builtin_frame_address(0);
    for (i = 0; i < 8; i++) {
      printk(KERN_EMERG "%i:\tstack + $0x%02x (%08lx) = %08lx%s\n",
	     i, i * 4, (long)(frame + i), frame[i],
	     (i == 0) ? "\t/* previous frame */" :
	     (i == 1) ? "\t/* return address */" : "");
    }
  }

halt:

  printk(KERN_EMERG "\n");
  printk(KERN_EMERG "*** System Halted ***\n");
  printk(KERN_EMERG "*** It is now safe to power off your system ***\n");

  bochs_debug(); /* if in bochs shell out */
  __asm__ ("1: hlt; jmp 1b");
}

void __assert_fail(__const char *__assertion,
                                __const char *__file,
                                unsigned int __line,
                                __const char *__function)
{
  printk(KERN_EMERG "ASSERTION FAILURE: the assertion `%s' failed.\n",
	 __assertion);
  printk(KERN_EMERG "                   at %s:%d in %s\n",
	 __file, __line, __function);

  panic("Assertion failure");
}
