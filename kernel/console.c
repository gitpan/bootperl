#include <kernel/kernel.h>
#include <asm/init.h>
#include <asm/io.h>
#include <kernel/console.h>
#include <kernel/memory.h>
#include <asm/lock.h>
#include <memory.h>

#define CHAR_ATTR 0x0700

static volatile unsigned short *vidmem = (unsigned short *)0xb8000;
static const int vidport = 0x3d0;
static struct vidparams *params = (void *)0x8e000;

static void scroll(struct vidparams *vidseg)
{
  int i;

  memcpy((void *)vidmem, (void *)vidmem + vidseg->cols * 2,
	 (vidseg->lines - 1) * vidseg->cols * 2);
  for (i = (vidseg->lines - 1) * vidseg->cols;
       i < vidseg->lines * vidseg->cols; i++)
    //vidmem[i] = ' ';
    vidmem[i] = vidmem[i] & 0xff + 0x2800;
}

void con_write(struct console *con, const char *s, unsigned int len)
{
  int x, y, pos;
  char c;
  int i;
  struct vidparams *vidseg = (struct vidparams *)con->data;

  lock_kernel();

  x = vidseg->orig_x;
  y = vidseg->orig_y;

  for (i = 0; i < len; i++) {
    c = s[i];
    if (c == '\t') {
      c = ' ';
    }
    if (c == '\n') {
      x = 0;
      if (++y >= vidseg->lines) {
	scroll(vidseg);
	y--;
      }
    }
    else if (c == '\r') {
      x = 0;
      y--;
    }
    else {
      vidmem[x + vidseg->cols * y] = c + CHAR_ATTR; 
      if ( ++x >= vidseg->cols ) {
	x = 0;
	if ( ++y >= vidseg->lines ) {
	  scroll(vidseg);
	  y--;
	}
      }
    }
  }

  vidseg->orig_x = x;
  vidseg->orig_y = y;

  pos = (x + vidseg->cols * y) * 2;       /* Update cursor position */
  outb_p(14, vidport+4);
  outb_p(0xff & (pos >> 9), vidport+5);
  outb_p(15, vidport+4);
  outb_p(0xff & (pos >> 1), vidport+5);
  unlock_kernel();
}

void puts(char *c)
{
  static struct console con;
  unsigned int len = 0;
  char *s;
  con.data = (void *)params;
  for(s = c; *s != '\0'; len++, s++)
    /* nothing */;
  con_write(&con, c, len);
}

extern struct multiboot_info *mbi;
int video_init = 0;

__initfunc(void console_init(void))
{
  struct console *con;

  if (mbi != 0) { /* oh no, we've got grub... */
    params = kalloc(sizeof(struct vidparams));
    params->orig_x = 0;
    params->orig_y = 23;
    params->lines = 25;
    params->cols = 80;
    params->magic = 0x4321;
  }

  scroll(params);
  params->lines--;

  if (params->orig_y >= params->lines) {
    scroll(params);
    params->orig_y--;
  }

  con = kalloc(sizeof(struct console));
  con->write = &con_write;
  con->data = (void *)params;
  con->loglevel = 5; /* NOTICE */
  con->write(con, "\n\n", 2);

  register_console(con);

  video_init = 1;
}
