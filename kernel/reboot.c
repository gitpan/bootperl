#include <asm/lock.h>
#include <asm/io.h>

inline void kb_wait(void)
{
  unsigned char val;

  do {
    val = inb(0x64);
  } while ((val & 0x2) != 0);
}

void reboot(void)
{
  lock_kernel();
  kb_wait();
  *((unsigned short *)0x472) = 0x1234;
  outb(0xfc, 0x64);

  __asm__ ("1: hlt; jmp 1b");
}
