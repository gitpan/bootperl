#include <kernel/kernel.h>
#include <asm/io.h>
#include <asm/init.h>
#include <asm/lock.h>
#include <kernel/console.h>
#include <assert.h>
#include <kernel/memory.h>
#include <kernel/bochs.h>

int bochs_debug_enabled = 0;

void bochs_debug(void)
{
  if (bochs_debug_enabled) {
    printk(KERN_EMERG "bochs: returning to debugger\n");
    outw(0x8AE0, 0x8A00);
  }
}

__initfunc(void bochs_init(void))
{
  outw(0x8a00, 0x8a00);
  if (inw(0x8a00) == 0x8a00) {
    printk("Bochs iodebug capability found and enabled\n");
    bochs_debug_enabled = 1;
  }
}
