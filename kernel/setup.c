#include <kernel/kernel.h>
#include <kernel/bitmap.h>
#include <asm/init.h>
#include <asm/system.h>
#include <asm/io.h>
#include <kernel/memory.h>
#include <kernel/multiboot.h>

extern char _end;
struct memparams {
  unsigned long memsize;
  unsigned short oldsize;
};
static const struct memparams * const memseg __initdata = (struct memparams *)0x8e020;

void __init kalloc_init(unsigned long *memory_start,
			 unsigned long *memory_end);

__initfunc(void setup_trad(unsigned long *memory_start,
			   unsigned long *memory_end))
{
  /* setup the end of real RAM */
  {
    unsigned long memory_alt_end;
    
    *memory_end = (1 << 20) + (memseg->oldsize << 10);
    memory_alt_end = (1 << 20) + (memseg->memsize << 10);
    if (memory_alt_end > *memory_end)
      *memory_end = memory_alt_end;
  }

  /* The BSS is aligned 1 page after the init stuff so the init stuff
     can be freed cleanly later. Before it wasn't and so it
     couldn't. The init stuff has whole pages dedicated to it. This
     alignment space isn't included in any size measurements of the
     sections */
  /* setup the start of RAM after the kernel (ie after the BSS) */
  /* page align the memory start */
  *memory_start = ((unsigned long)&_end + 4095) & ~4095;

  kalloc_init(memory_start, memory_end);
}

extern struct multiboot_info *mbi;

__initfunc(void setup_grub(unsigned long addr,
			   unsigned long *memory_start,
			   unsigned long *memory_end))
{
  mbi = (struct multiboot_info *)addr;
  *memory_start = ((unsigned long)&_end + 4095) & ~4095;
  *memory_end = mbi->mem_upper * 1024;
  kalloc_init(memory_start, memory_end);
  
  printk("Multiboot info:\n");
  printk("   flags: 0x%lx\n", mbi->flags);
  printk("   mem_lower: %ld\n", mbi->mem_lower);
  printk("   mem_upper: %ld\n", mbi->mem_upper);
  printk("   boot_device: 0x%lx\n", mbi->boot_device);
  /* parse boot_device */
  printk("     %lx:%lx:%lx:%lx\n", (mbi->boot_device >> 24) & 0xff,
	 (mbi->boot_device >> 16) & 0xff, (mbi->boot_device >> 8) & 0xff,
	 mbi->boot_device & 0xff);
  
  printk("   cmdline: 0x%lx\n", mbi->cmdline);
  printk("   mods_count: 0x%lx\n", mbi->mods_count);
  printk("   *cmdline: %s\n", (char *)mbi->cmdline);
  printk("   *boot_loader_name: %s\n", (char *)mbi->boot_loader_name);
}

/* IDT uses space past end of BSS */
segdesc_t *idt;

extern void unexpected_int(void);
extern void int3(void);

__initfunc(void setup_interrupts(void))
{
  register unsigned int i;
  /* Allocate memory for 256 idt entries */
  /* idt is now defined in BSS (I changed this back to dynamic) */
  /* idt = (segdesc_t *)*memory_start; */
/*   *memory_start += 256 * sizeof(segdesc_t); */
  idt = kalloc(256 * sizeof(segdesc_t));

  for (i = 0; i < 256; i++)
    set_intr_gate(i, &unexpected_int);

  set_intr_gate(3, int3);

  /* Load the address and limit of the idt into the idtr */
  load_idtr(idt, (256 * 8) - 1);
}

extern void __init init_8259A(void);
extern void __init init_irq_vectors(void);
extern void __init init_syscalls(void);
__initfunc(void setup_IRQs(void))
{
  init_8259A();
  init_irq_vectors();
  init_syscalls();
}
