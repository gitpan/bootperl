#include <kernel/kernel.h>
#include <kernel/irq.h>
#include <asm/init.h>
#include <asm/system.h>
#include <asm/lock.h>
#include <kernel/task.h>
#include <assert.h>

#define __STR(x) #x
#define STR(x) __STR(x)

__asm__(".globl common_interrupt");

#define IRQ(nr) IRQ##nr##_interrupt

/* this pushes the interrupt number as orig_eax */
#define BUILD_IRQ(nr) \
asmlinkage void IRQ(nr)(void); \
__asm__(__ALIGN_STR "\n" \
	SYMBOL_NAME_STR(IRQ) #nr "_interrupt:\n\t" \
	"pushl $" #nr "\n\t" \
	"jmp common_interrupt")

BUILD_IRQ(0); BUILD_IRQ(1); BUILD_IRQ(2); BUILD_IRQ(3);
BUILD_IRQ(4); BUILD_IRQ(5); BUILD_IRQ(6); BUILD_IRQ(7);
BUILD_IRQ(8); BUILD_IRQ(9); BUILD_IRQ(10); BUILD_IRQ(11);
BUILD_IRQ(12); BUILD_IRQ(13); BUILD_IRQ(14); BUILD_IRQ(15);

void (*interrupts[16])(void) = {
  IRQ(0), IRQ(1), IRQ(2), IRQ(3),
  IRQ(4), IRQ(5), IRQ(6), IRQ(7),
  IRQ(8), IRQ(9), IRQ(10), IRQ(11),
  IRQ(12), IRQ(13), IRQ(14), IRQ(15)
};

struct irq_handler *irq_handlers[16];

#define FIRST_VECTOR 0x20

__initfunc(void init_irq_vectors(void))
{
  int i;
  /* IRQs are not enabled until they are registered so we know about
     spurious ones */
  for (i = 0; i < 16; i++) {
    set_intr_gate(FIRST_VECTOR + i, interrupts[i]);
    irq_handlers[i] = 0;
  }
}

void register_irq_handler(struct irq_handler *handler)
{
  lock_kernel();
  /* This just adds the handler to the top of the linked list */
  handler->next = irq_handlers[handler->irq];
  handler->prev = 0;
  if (handler->next)
    handler->next->prev = handler;
  irq_handlers[handler->irq] = handler;
  enable_irq(handler->irq);
  unlock_kernel();
}

asmlinkage void handle_irq()
{
  struct irq_handler *cur;
  unsigned char irq = current->regs->orig_eax;

  ack_irq(irq);

  cur = irq_handlers[irq];
  if (!cur) {
    printk("handle_IRQ: no registered interrupt handler, IRQ%d\n", irq);
  }
  printk(KERN_DEBUG "Interrupt %i occured\n", irq);
  while (cur) {
    if (cur->handler)
      cur->handler(cur);
    cur = cur->next;
  }
  enable_irq(irq);
}
