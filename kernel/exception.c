/* exception.c: handle processor exceptions and suprisingly NMIs too */
/* basically started as a hacked around version of irq.c */
/* then I deleted most of that code and instead used irq.c as a model */

#include <kernel/kernel.h>
#include <asm/init.h>
#include <asm/system.h>
#include <asm/lock.h>
#include <kernel/task.h>

#define __STR(x) #x
#define STR(x) __STR(x)

asmlinkage void common_exception(long exception);
asmlinkage void common_exception_ec(long exception, long errorcode);

#define EXCEPTION(nr) do_exception##nr

#define BUILD_EXCEPTION(nr) \
asmlinkage void EXCEPTION(nr)(void); \
__asm__(__ALIGN_STR "\n" \
        SYMBOL_NAME_STR(do_exception##nr) ":\n\t" \
        "movl $" STR(KERNEL_DATA) ", %eax\n\t" \
        "movl %eax, %ds\n\t" \
        "pushl $" #nr "\n\t" \
        "pushl $0\n\t" /* a dummy return address */ \
        "jmp common_exception");

#define BUILD_EXCEPTION_EC(nr) \
asmlinkage void EXCEPTION(nr)(void); \
__asm__(__ALIGN_STR "\n" \
        SYMBOL_NAME_STR(do_exception##nr) ":\n\t" \
        "movl $" STR(KERNEL_DATA) ", %eax\n\t" \
        "movl %eax, %ds\n\t" \
        "pushl $" #nr "\n\t" \
        "pushl $0\n\t" /* a dummy return address */ \
        "jmp common_exception_ec");

BUILD_EXCEPTION(0) BUILD_EXCEPTION(1) BUILD_EXCEPTION(2) BUILD_EXCEPTION(3)
BUILD_EXCEPTION(4) BUILD_EXCEPTION(5) BUILD_EXCEPTION(6) BUILD_EXCEPTION(7)
BUILD_EXCEPTION_EC(8) BUILD_EXCEPTION(9)
BUILD_EXCEPTION_EC(10) BUILD_EXCEPTION_EC(11)
BUILD_EXCEPTION_EC(12) BUILD_EXCEPTION_EC(13)
BUILD_EXCEPTION_EC(14) BUILD_EXCEPTION(15)
BUILD_EXCEPTION(16) BUILD_EXCEPTION_EC(17)
BUILD_EXCEPTION(18) BUILD_EXCEPTION(19)
BUILD_EXCEPTION(20) BUILD_EXCEPTION(21)
BUILD_EXCEPTION(22) BUILD_EXCEPTION(23)
BUILD_EXCEPTION(24) BUILD_EXCEPTION(25)
BUILD_EXCEPTION(26) BUILD_EXCEPTION(27)
BUILD_EXCEPTION(28) BUILD_EXCEPTION(29)
BUILD_EXCEPTION(30) BUILD_EXCEPTION(31)

const char *exception_names[32] = {
  "de", "db", "NMI", "bp", "of", "br", "ud", "nm",
  "df", "reserved", "ts", "np", "ss", "gp", "pf", "reserved",
  "mf", "ac", "mc", "xf", "reserved", "reserved", "reserved", "reserved",
  "reserved", "reserved", "reserved", "reserved",
  "reserved", "reserved", "reserved", "reserved"
};

void *exception_funcs[32] = {
  EXCEPTION(0), EXCEPTION(1), EXCEPTION(2), EXCEPTION(3), 
  EXCEPTION(4), EXCEPTION(5), EXCEPTION(6), EXCEPTION(7), 
  EXCEPTION(8), EXCEPTION(9), EXCEPTION(10), EXCEPTION(11), 
  EXCEPTION(12), EXCEPTION(13), EXCEPTION(14), EXCEPTION(15), 
  EXCEPTION(16), EXCEPTION(17), EXCEPTION(18), EXCEPTION(19), 
  EXCEPTION(20), EXCEPTION(21), EXCEPTION(22), EXCEPTION(23), 
  EXCEPTION(24), EXCEPTION(25), EXCEPTION(26), EXCEPTION(27), 
  EXCEPTION(28), EXCEPTION(29), EXCEPTION(30), EXCEPTION(31)
};

asmlinkage void common_exception(long exception)
{
  lock_kernel();

  printk(KERN_EMERG "exception: #%s", exception_names[exception]);
  panic("A processor exception occured");
}

asmlinkage void common_exception_ec(long exception, long errorcode)
{
  lock_kernel();

  printk("exception: #%s error code %08lx, %s%ssegment %04lx\n",
	 exception_names[exception], errorcode,
	 (errorcode & 0x1) ? "external, " : "",
	 (errorcode & 0x2) ? "LDT, " :
	   ((errorcode & 0x4) ? "LDT, " : "GDT, "),
	 errorcode >> 3);

  panic("A processor exception occured");
}

__initfunc(void setup_exceptions(void))
{
  int i;
  for (i = 0; i < 32; i++) {
    set_intr_gate(i, exception_funcs[i]);
  }
}
