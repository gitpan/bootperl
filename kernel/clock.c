#include <kernel/kernel.h>
#include <asm/init.h>
#include <asm/io.h>
#include <kernel/irq.h>
#include <kernel/task.h>

#define CLOCK_TICK_RATE 1193180
#define LATCH  ((CLOCK_TICK_RATE + HZ/2) / HZ)  /* For divider (value
                                                   rounded up) */
volatile unsigned long jiffies = 0;
void do_timer_int(struct irq_handler *h)
{
  jiffies++;
/*   if ((jiffies % HZ) == 0) { */
/*     printk(KERN_DEBUG "Another second passed, jiffies = %ld\n", jiffies); */
/*   } */
  current->time++;
  printk(KERN_DEBUG "timer: current->timeslice is %li\n", current->timeslice);
  if (current->timeslice-- == 0) {
/*     printk(KERN_INFO "Process timeslice expired in %s\n", current->name); */
    current->need_resched = 1;
  }
}
static struct irq_handler timer_int = {
  0, &do_timer_int, "timer", 0, 0
};

/* void do_rtc_int(struct irq_handler *h) */
/* { */
/*   printk("RTC interrupt\n"); */
/* } */
/* static struct irq_handler rtc_int = { */
/*   8, &do_rtc_int, "rtc", 0, 0 */
/* }; */

__initfunc(void setup_clock(void))
{
  /* initialise the PIC */
  outb_p(0x34,0x43);              /* binary, mode 2, LSB/MSB, ch 0 */
  outb_p(LATCH & 0xff , 0x40);    /* LSB */
  outb(LATCH >> 8 , 0x40);        /* MSB */

  register_irq_handler(&timer_int); /* turn on timer ints */

  /* initialise the RTC, WARNING: THIS CODE IS INCOMPLETE */
/*   { */
/*     int i; */
/*     printk(KERN_DEBUG "Reading CMOS RAM\n"); */
/*     for (i = 0; i < 9; i++) { */
/*       unsigned char value; */
/*       outb(i, 0x70); */
/*       value = inb(0x71); */
/*       printk(KERN_DEBUG "  CMOS %02x: %02x\n", i, value); */
/*     } */
/*   } */

/*   register_irq_handler(&rtc_int); */ /* turn on rtc ints */
}
