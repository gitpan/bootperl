/* main code for kernel */

#include <kernel/kernel.h>
#include <asm/init.h>
#include <asm/lock.h>
#include <kernel/console.h>
#include <assert.h>
#include <kernel/task.h>
#include <kernel/delay.h>
#include <kernel/multiboot.h>
#include <kernel/memory.h>

typedef unsigned int size_t;

void *memset(void* s, int c, size_t n);
void *memcpy(void* __dest, __const void* __src,
	      size_t __n);

extern char _text;
extern char _etext;
extern char _edata;
extern char __init_begin;
extern char __init_end;
extern char __bss_start;
extern char _end;

long _kernel_lock = 0;

struct memparams {
  unsigned long memsize;
  unsigned short oldsize;
};

unsigned long memory_start, memory_end;
struct multiboot_info *mbi = 0;

extern const char *kernel_banner;
extern int debug_phase;

extern void __init console_init(void);
extern void __init setup_exceptions(void);
extern void __init setup_keyboard(void);
extern void __init setup_serial(void);
extern void __init setup_telemetry(void);
extern void __init setup_processor(void);
extern void __init setup_trad(unsigned long *memory_start,
			      unsigned long *memory_end);
extern void __init setup_grub(unsigned long addr,
			      unsigned long *memory_start,
			      unsigned long *memory_end);
extern void __init setup_interrupts(void);
extern void __init setup_IRQs(void);
extern void __init setup_clock(void);
extern void __init bochs_init(void);

void __init kalloc_init(unsigned long *memory_start,
			 unsigned long *memory_end);

extern void enable_irq(unsigned short irq);

extern volatile unsigned long jiffies;
extern void __init calibrate_delay(void);

void process(void);
void keyprocess(void);

void restore(void);

void system_idle(void);
void kernel_init(void);

/* the linker looks for _start which I don't want to change */
/* the weak is so that I can easily override this */
/* the regparam make this asmlinkage */
void __init _start(void)
     __attribute__ ((regparm(0), weak, alias ("start_kernel")));

__initfunc(asmlinkage void start_kernel(unsigned long magic,
					unsigned long addr))
{
  /* First task is to clear the BSS segment. */
  {
    char *dst = &__bss_start;
    while (dst < &_end) {
      *dst++ = 0;
    }
  }

  /* Interrupts are turned off at this point */
  /* This syncs the kernel's internal counters of nested cli/sti ops */
  lock_kernel();

  printk(kernel_banner);

  /* Loads the size of memory, and other stuff from the boot loader an
     then initialises kalloc */
  if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
    setup_grub(addr, &memory_start, &memory_end);
  }
  else {
    setup_trad(&memory_start, &memory_end);
  }

  /* Initialise data about the processor */
  setup_processor();

  /* Initialise the parallel port telemetry */
  setup_telemetry();

  /* Enable the bochs debugger if present */
  bochs_init();

  /* Creates an idt and inserts the standard exception handlers */
  setup_interrupts();

  /* Initialise handling of exceptions */
  setup_exceptions();

  /* Initialise the i8259A, and load IRQ handlers */
  setup_IRQs();

  /* Initialise the PIC and RTC */
  setup_clock();

  /* Initialises the keyboard */
  setup_keyboard();

  /* Initialises the console structures for printk and loads
     the hack driver (note: driver should output a '\n' first) */
  console_init();

  /* We're setup at this point so we turn interrupts on */
  unlock_kernel();
  assert(_kernel_lock == 0); /* Guard against any locking mistakes in the
			       above code */
  /* At the moment we are conceptually executing in Process 0, the idle task */

  /* calibrate our delay loops */
  calibrate_delay();

  /* Shows memory available after the above have run */
  printk("Memory: %luk/%luk available "
	 "(%dk kernel code, %dk data, %dk init, %dk bss, %dk other)\n",
	 (memory_end - memory_start)/1024, memory_end/1024,
	 (&_etext - &_text)/1024, (&_edata - &_etext)/1024,
	 (&__init_end - &__init_begin)/1024, (&_end - &__bss_start)/1024,
	 (int)(memory_start - (long)&_end)/1024);

  /* create the init task */
  create_process(1, kernel_init, "init", 4092);

  /* We don't panic anymore. We gracefully enter a sort of idle loop */
  /* this is the same loop as used by the famous panic() */
  /* This should be replaced by a call to the real idle loop */
  printk("System Initialised.\nEntering idle loop\n");
  system_idle();
}

void kernel_init(void)
{
  struct queue block_queue = QUEUE_INIT;
  char c, getch(void);

  /* free the init seg (FIXME: whole pages) */
  printk("Freeing init memory\n");
  mem_free(&__init_begin, &__init_end - &__init_begin, 1);

  printk(KERN_DEBUG "creating perl process\n");
  extern void perl_main(void);
  create_process(2, perl_main, "perl main", 16200);

  /* we can't fall off the end here BUG/FIXME */
  block(&block_queue);
}

void system_idle(void)
{
/*   __asm__ ("1: hlt; jmp 1b"); */
  printk(KERN_DEBUG "idle: Idle loop started\n");
  current->timeslice = 0;
  while(1)
    /* do nothing */;
}

void *memset(void* s, int c, size_t n)
{
  int i;
  char *ss = (char*)s;

  for (i=0;i<n;i++) ss[i] = c;
  return s;
}

void *memcpy(void* __dest, __const void* __src,
		     size_t __n)
{
  int i;
  char *d = (char *)__dest, *s = (char *)__src;

  for (i=0;i<__n;i++) d[i] = s[i];
  return __dest;
}

asmlinkage void do_int3(void)
{
  printk(KERN_ERR "Int3 occurred\n");
}

asmlinkage void do_unexpected_int(void)
{
  panic("Unexpected interrupt");
}

/* this should be approx 2 Bo*oMips to start (note initial shift), and will
   still work even if initially too large, it will just take slightly longer */
unsigned long loops_per_jiffy = (1<<12);

/* This is the number of bits of precision for the loops_per_jiffy.  Each
   bit takes on average 1.5/HZ seconds.  This (like the original) is a little
   better than 1% */
#define LPS_PREC 8

void __init calibrate_delay(void)
{
  unsigned long ticks, loopbit;
  int lps_precision = LPS_PREC;

  loops_per_jiffy = 1 << 12;

  printk("Calibrating delay loop... ");
  while (loops_per_jiffy <<= 1) {
    /* wait for "start of" clock tick */
    ticks = jiffies;
    while (ticks == jiffies)
      /* nothing */;
    /* Go ... */
    ticks = jiffies;
    __delay(loops_per_jiffy);
    ticks = jiffies - ticks;
    if (ticks)
      break;
  }

  /* Do a binary approximation to get loops_per_jiffy set to equal one
     clock (up to lps_precision bits) */
  loops_per_jiffy >>= 1;
  loopbit = loops_per_jiffy;
  while (lps_precision-- && (loopbit >>= 1)) {
    loops_per_jiffy |= loopbit;
    ticks = jiffies;
    while (ticks == jiffies);
    ticks = jiffies;
    __delay(loops_per_jiffy);
    if (jiffies != ticks) /* longer than 1 tick */
      loops_per_jiffy &= ~loopbit;
  }

  /* Round the value and print it */
  printk("%lu loops per jiffy, thats %lu.%02lu BogoMIPS\n",
	 loops_per_jiffy,
	 loops_per_jiffy/(500000/HZ),
	 (loops_per_jiffy/(5000/HZ)) % 100);
}
