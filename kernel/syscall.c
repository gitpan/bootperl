#include <kernel/kernel.h>
#include <asm/init.h>
#include <asm/lock.h>
#include <assert.h>
#include <kernel/memory.h>
#include <asm/system.h>
#include <kernel/task.h>
#include <kernel/console.h>
#include <kernel/bochs.h>

asmlinkage void syscall(void);
char getch(void);

asmlinkage void handle_syscall(void)
{
  unsigned long retval = (unsigned long)-1;
  long flags;
  struct task_regs *saved_regs;

  _save_flags(&flags);
  printk(KERN_DEBUG "syscall: n=%ld _kernel_lock=%ld %01ld\n",
	 current->regs->orig_eax, _kernel_lock, flags & (1 << 9));
  switch (current->regs->orig_eax) {
  case 12:
  case 1:
    unlock_kernel();
    saved_regs = current->regs;
    retval = (unsigned long)getch();
    printk(KERN_DEBUG "call getch = %08lx\n", retval);
    printk(KERN_DEBUG "%08lx -> %08lx\n", saved_regs, current->regs);
    lock_kernel();
    break;
  case 2:
    printk(KERN_DEBUG "calling puts\n");
    puts((char *)current->regs->ebx);
    retval = 0;
    break;
  case 3:
    printk("Syscall Checkpoint\n");
    retval = 0xdeadbeef;
    break;
  case 0:
  default:
    printk("Unknown syscall\n");
    break;
  }
  current->regs->eax = retval;
  current->regs->orig_eax = retval;
}

__initfunc(void init_syscalls(void))
{
  unsigned long retval;
  set_intr_gate(0x40, syscall);
  printk("syscall tests:\n");
  __asm__("int $0x40" : "=a" (retval) : "a" (22));
  printk("retval = %08lx\n", retval);
  __asm__("int $0x40" : "=a" (retval) : "a" (3));
  printk("retval = %08lx\n", retval);
}
