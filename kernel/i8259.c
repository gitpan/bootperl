#include <asm/init.h>
#include <asm/lock.h>
#include <asm/io.h>
#include <kernel/kernel.h>
#include <kernel/irq.h>

static unsigned short cached_irq_mask = 0xffff;
#define __byte(x,y) (((unsigned char *)&(y))[x])

void disable_irq(unsigned char irq)
{
  unsigned short mask = 1 << irq;
  long flags;

  lock_flags(flags);
  cached_irq_mask |= mask;
  if (irq & 8)
    outb(__byte(1, cached_irq_mask), 0xA1);
  else
    outb(__byte(0, cached_irq_mask), 0x21);
  unlock_flags(flags);
}

void enable_irq(unsigned char irq)
{
  unsigned short mask = ~(1 << irq);
  long flags;

  lock_flags(flags);
  cached_irq_mask &= mask;
  if (irq & 8)
    outb(__byte(1, cached_irq_mask), 0xA1);
  else
    outb(__byte(0, cached_irq_mask), 0x21);
  unlock_flags(flags);
}

int pending_irq(unsigned char irq)
{
  unsigned short mask = 1 << irq;
  long flags;
  int ret;

  lock_flags(flags);
  if (irq < 8)
    ret = inb(0x20) & mask;
  else
    ret = inb(0xA0) & (mask >> 8);
  unlock_flags(flags);

  return ret;
}

/* Despite the name this also masks the irq so remember to call enable_irq */
void ack_irq(unsigned char irq)
{
  unsigned short mask = 1 << irq;
  long flags;

  lock_flags(flags);
  /* Quick and easy spurious irq testing */
  if (cached_irq_mask & mask) {
    static int spurious_mask; /* so messages only appear once */
    if (!(spurious_mask & mask)) {
      printk("spurious i8259A interrupt: IRQ%d.\n", irq);
      spurious_mask |= mask;
    }
    /* handle and mask it anyway */
  }
  /* Mask irq only if its not spurious */
  cached_irq_mask |= mask;
  if (irq & 8) { /* This is on the slave; more work :-( */
    inb(0xa1); /* Is this required?? Linux seems to think so */
    outb(__byte(1, cached_irq_mask), 0xa1); /* mask it on the secondary */
    outb(0x60 + (irq & 7), 0xa0); /* specific EOI for slave */
    outb(0x62, 0x20); /* specific EOI to mask for IRQ2 */
  }
  else { /* nice easy one on the master */
    inb(0x21); /* See comment above. Blame linux */
    outb(__byte(0, cached_irq_mask), 0x21); /* mask irq */
    outb(0x60 + irq, 0x20); /* specific EOI for master */
  }
  unlock_flags(flags);
}

__initfunc(void init_8259A(void))
{
  long flags;

  lock_flags(flags);

  /* Mask all the IRQs on both PICs */
  outb(0xff, 0x21); /* Mask all on master */
  outb(0xff, 0xa1); /* Mask all on slave */

  /* Initialise the master */
  outb_p(0x11, 0x20); /* ICW1 -> 20h: begin init of master */
  outb_p(0x20, 0x21); /* ICW2 -> 21h: change where ints are mapped to
			 (start at int 20h) */
  outb_p(0x04, 0x21); /* ICW3 -> 21h: spectifies which IRQs are slaved */
  outb_p(0x01, 0x21); /* ICW4 -> 21h: set for normal EOI */

  /* Initialise the slave */
  outb_p(0x11, 0xa0); /* ICW1 -> A0h: begin init of slaves */
  outb_p(0x28, 0xa1); /* ICW2 -> A1h: change where ints are mapped to
			 (start at int 28h, leaves space for master IRQs) */
  outb_p(0x02, 0xa1); /* ICW3 -> A1h: spectifies which IRQ we are slaved on */
  outb_p(0x01, 0xa1); /* ICW4 -> A1h: set for normal EOI */

  outb(__byte(0, cached_irq_mask), 0x21);
  outb(__byte(1, cached_irq_mask), 0xa1);

  unlock_flags(flags);
}
