#include <kernel/kernel.h>
#include <asm/init.h>
#include <asm/io.h>
#include <kernel/console.h>
#include <kernel/memory.h>
#include <asm/lock.h>

void par_write(struct console *con, const char *s, unsigned int len)
{
  unsigned int i;

  for(i = 0; i < len; i++) {
    outb(0x08, 0x37a); /* Enable the printer in bochs */
    outb(s[i], 0x378);
    outb(0x01, 0x37a);
    outb(0x00, 0x37a);
  }
}

struct console con;

__initfunc(void setup_telemetry(void))
{
  con.write = &par_write;
  con.data = 0;
  con.loglevel = LOGLEVEL_MAX;

  register_console(&con);
}
