#include <kernel/kernel.h>
#include <asm/lock.h>
#include <asm/init.h>
#include <asm/processor.h>

struct cpuinfo boot_cpu_info;
extern int x86_udelay_tsc;

__initfunc(void setup_processor(void))
{
  register long flags_test, flags;

  boot_cpu_info.cpuid_level = -1;
  boot_cpu_info.x86 = 3; /* at least a 386 */

  __asm__("pushfl\n\t"
	  "popl %0\n\t"
	  "movl %0, %1\n\t"
	  "xorl $0x40000, %0\n\t"
	  "pushl %0\n\t"
	  "popfl\n\t"
	  "pushfl\n\t"
	  "popl %0\n\t"
	  "xorl %1, %0\n\t"
	  : "=r" (flags_test), "=r" (flags)
	  : /* no inputs */);

  if ((flags_test & 0x40000) == 0) {
  }
  else {
    boot_cpu_info.x86 = 4;
    __asm__("movl %1, %0\n\t"
	    "xorl $0x200000, %0\n\t"
	    "pushl %0\n\t"
	    "popfl\n\t"
	    "pushfl\n\t"
	    "popl %0\n\t"
	    "xorl %1, %0\n\t"
	    "pushl %1\n\t"
	    "popfl\n\t"
	    : "=r" (flags_test)
	    : "r" (flags));
    if ((flags_test & 0x200000) == 0) {
    }
    else { /* we have cpuid */
      cpuid(0, &boot_cpu_info.cpuid_level,
	    (int *)(&boot_cpu_info.x86_vendor_id[0]),
	    (int *)(&boot_cpu_info.x86_vendor_id[8]),
	    (int *)(&boot_cpu_info.x86_vendor_id[4]));
      if (boot_cpu_info.cpuid_level == 0) {
      }
      else { /* >486 */
	__asm__("movl $1, %%eax\n\t"
		"cpuid\n\t"
		"movb %%al, %%cl\n\t"
		"andb $0xf, %%ah\n\t"
		"movb %%ah, %1\n\t"
		"andb $0xf0, %%al\n\t"
		"shrb $4, %%al\n\t"
		"movb %%al, %2\n\t"
		"andb $0x0f, %%cl\n\t"
		"movb %%cl, %3\n\t"
		: "=d" (boot_cpu_info.x86_capability[0]),
		"=m" (boot_cpu_info.x86),
		"=m" (boot_cpu_info.x86_model),
		"=m" (boot_cpu_info.x86_mask)
		: /* no inputs */
		: "eax", "ebx", "ecx");
      }
    }
  }

  if (boot_cpu_info.x86_capability[0] & 0x10)
    x86_udelay_tsc = 1;

  printk("processor: %12.12s %d86 model%d mask%d cap%08x stepping%d tsc%s\n",
	 boot_cpu_info.x86_vendor_id,
	 boot_cpu_info.x86,
	 boot_cpu_info.x86_model,
	 boot_cpu_info.x86_mask,
	 boot_cpu_info.x86_capability[0],
	 boot_cpu_info.cpuid_level,
	 x86_udelay_tsc ? "yes" : "no");
}
