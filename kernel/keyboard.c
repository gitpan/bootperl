#include <kernel/kernel.h>
#include <asm/init.h>
#include <kernel/irq.h>
#include <asm/io.h>
#include <kernel/task.h>

static const long sh_shift       = 0x0001;
static const long sh_ctrl        = 0x0004;
static const long sh_lalt        = 0x0010;
static const long sh_ralt        = 0x0020;
static const long sh_numlock     = 0x0040;
static const long sh_capslock    = 0x0080;
static const long sh_scroll_lock = 0x0100;
long shifts = 0;
long e0 = 0;

typedef void (*pfunc_key)(unsigned char code);
typedef void (func_key)(unsigned char code); /* neat trick!!!! (saw it
						in the kernel) */
func_key key_none, key_letter, key_shift, key_ctrl, key_alt;
func_key key_num, key_caps, key_scroll, key_sysreq, key_func, key_keypad;
func_key key_win, key_menu;

pfunc_key key_table[256] = {
  key_none,   key_letter, key_letter, key_letter, /* 00-03 s0 esc 1 2 */
  key_letter, key_letter, key_letter, key_letter, /* 04-07 3 4 5 6 */
  key_letter, key_letter, key_letter, key_letter, /* 08-0b 7 8 9 0 */
  key_letter, key_letter, key_letter, key_letter, /* 0c-0f - = bs tab */
  key_letter, key_letter, key_letter, key_letter, /* 10-13 q w e r */
  key_letter, key_letter, key_letter, key_letter, /* 14-17 t y u i */
  key_letter, key_letter, key_letter, key_letter, /* 18-1b o p [ ] */
  key_letter, key_ctrl,   key_letter, key_letter, /* 1c-1f enter lctrl a s */
  key_letter, key_letter, key_letter, key_letter, /* 20-23 d f g h */
  key_letter, key_letter, key_letter, key_letter, /* 24-27 j k l ;*/
  key_letter, key_letter, key_shift,  key_letter, /* 28-2b ' ` lshift # */
  key_letter, key_letter, key_letter, key_letter, /* 2c-2f z x c v */
  key_letter, key_letter, key_letter, key_letter, /* 30-33 b n m , */
  key_letter, key_letter, key_shift,  key_keypad, /* 34-37 . / rshift kp-* */
  key_alt,    key_letter, key_caps,   key_func,   /* 38-3b lalt sp caps f1 */
  key_func,   key_func,   key_func,   key_func,   /* 3c-3f f2 f3 f4 f5 */
  key_func,   key_func,   key_func,   key_func,   /* 40-43 f6 f7 f8 f9 */
  key_func,   key_num,    key_scroll, key_keypad, /* 44-47 f10 num scr kp-7 */
  key_keypad, key_keypad, key_keypad, key_keypad, /* 48-4b kp: 8 9 - 4 */
  key_keypad, key_keypad, key_keypad, key_keypad, /* 4c-4f kp: 5 6 + 1 */
  key_keypad, key_keypad, key_keypad, key_keypad, /* 50-53 kp: 2 3 0 - */
  key_sysreq, key_none,   key_letter, key_func,   /* 54-57 sysreq ? \ f11 */
  key_func,   key_none,   key_none,   key_win,    /* 58-5b f12 ? ? lwin */
  key_win,    key_menu,   key_none,   key_none,   /* 5c-5f rwin menu ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 60-63 ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 64-67 ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 68-6b ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 6c-6f ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 70-73 ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 74-77 ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 78-7b ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 7c-7f ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* 80-83 ? br br br */
  key_none,   key_none,   key_none,   key_none,   /* 84-87 br br br br */
  key_none,   key_none,   key_none,   key_none,   /* 88-8b br br br br */
  key_none,   key_none,   key_none,   key_none,   /* 8c-8f br br br br */
  key_none,   key_none,   key_none,   key_none,   /* 90-93 br br br br */
  key_none,   key_none,   key_none,   key_none,   /* 94-97 br br br br */
  key_none,   key_none,   key_none,   key_none,   /* 98-9b br br br br */
  key_none,   key_ctrl,   key_none,   key_none,   /* 9c-9f br lctrl br br */
  key_none,   key_none,   key_none,   key_none,   /* a0-a3 br br br br */
  key_none,   key_none,   key_none,   key_none,   /* a4-a7 br br br br */
  key_none,   key_none,   key_shift,  key_none,   /* a8-ab br br lshift br */
  key_none,   key_none,   key_none,   key_none,   /* ac-af br br br br */
  key_none,   key_none,   key_none,   key_none,   /* b0-b3 br br br br */
  key_none,   key_none,   key_shift,  key_none,   /* b4-b7 br br rshift br */
  key_alt,    key_none,   key_none,   key_none,   /* b8-bb lalt br br br */
  key_none,   key_none,   key_none,   key_none,   /* bc-bf br br br br */
  key_none,   key_none,   key_none,   key_none,   /* c0-c3 br br br br */
  key_none,   key_none,   key_none,   key_none,   /* c4-c7 br br br br */
  key_none,   key_none,   key_none,   key_none,   /* c8-cb br br br br */
  key_none,   key_none,   key_none,   key_none,   /* cc-cf br br br br */
  key_none,   key_none,   key_none,   key_none,   /* d0-d3 br br br br */
  key_none,   key_none,   key_none,   key_none,   /* d4-d7 br br br br */
  key_none,   key_none,   key_none,   key_none,   /* d8-db br br br br */
  key_none,   key_none,   key_none,   key_none,   /* dc-df br br ? ? */
  key_none,   key_none,   key_none,   key_none,   /* e0-e3 e0 e1 ? ? */
  key_none,   key_none,   key_none,   key_none,   /* e4-e7 ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* e8-eb ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* ec-ef ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* f0-f3 ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* f4-f7 ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* f8-fb ? ? ? ? */
  key_none,   key_none,   key_none,   key_none,   /* fc-ff ? ? ? ? */
};

struct queue keyboard_queue = QUEUE_INIT;

void do_keyboard_int(struct irq_handler *h)
{
  unsigned char code, val;

  code = inb(0x60);
  val = inb(0x61);
  outb(val | 0x80, 0x61);
  outb(val, 0x61);

  printk(KERN_DEBUG "keyboard: keypress_int e0=%ld code=%02x\n", e0, code);

  if (code == 0xe0)
    e0 = 1;
  else if (code == 0xe1)
    e0 = 2;
  else {
    key_table[code](code);
    e0 = 0;
  }

  /* wake up sleeping processes */
  wake_up_queue(&keyboard_queue);
}
static struct irq_handler keyboard_int = {
  1, &do_keyboard_int, "keyboard", 0, 0
};

#define BUFFER_SIZE (16384)
#define BUFFER_MASK (BUFFER_SIZE - 1)
static char buffer[BUFFER_SIZE + 1];
static unsigned long buffer_start = 0;
static unsigned long entered_chars = 0;
static unsigned long buffer_size = 0;

void add_buffer(char c)
{
  char out_buffer[2] = {c, '\0'};
  void puts(char *);

  buffer[(buffer_start + buffer_size) & BUFFER_MASK] = c;
  if (buffer_size < BUFFER_SIZE)
    buffer_size++;
  else
    buffer_start++;
  entered_chars++;

  puts(out_buffer);
}

char get_buffer(void)
{
  char c;

  if (buffer_size == 0)
    return 0;

  c = buffer[buffer_start & BUFFER_MASK];
  buffer_start++;
  buffer_size--;

  return c;
}

char getch(void)
{
  char c;

  printk(KERN_DEBUG "getch: abc\n");
  while ((c = get_buffer()) == 0) {
    block(&keyboard_queue);
  }

  return c;
}

__initfunc(void setup_keyboard(void))
{
  register_irq_handler(&keyboard_int);
}

/* handlers for the keys */
void key_none(unsigned char code)
{
  /* do absolutely nothing */
}

char letter_map[] = {
  0, 27, '1', '2', '3', '4', '5', '6',      /* 00-07 */
  '7', '8', '9', '0', '-', '=', '\b', '\t', /* 08-0f */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',   /* 10-17 */
  'o', 'p', '[', ']', '\n', 0, 'a', 's',    /* 18-1f */
  'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 20-27 */
  '\'', '`', 0, '#', 'z', 'x', 'c', 'v',    /* 28-2f */
  'b', 'n', 'm', ',', '.', '/', 0, 0,       /* 30-37 */
  0, ' ', 0, 0, 0, 0, 0, 0,                 /* 38-39 */
  0, 0, 0, 0, 0, 0, 0, 0,                   /* 40-47 */
  0, 0, 0, 0, 0, 0, 0, 0,                   /* 48-49 */
  0, 0, 0, 0, 0, 0, '\\', 0                 /* 50-57 */
};
char letter_shift_map[] = {
  0, 27, '!', '"', '£', '$', '%', '^',      /* 00-07 */
  '&', '*', '(', ')', '_', '+', '\b', '\t', /* 08-0f */
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',   /* 10-17 */
  'O', 'P', '{', '}', '\n', 0, 'A', 'S',    /* 18-1f */
  'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',   /* 20-27 */
  '@', '¬', 0, '~', 'Z', 'X', 'C', 'V',     /* 28-2f */
  'B', 'N', 'M', '<', '>', '?', 0, 0,       /* 30-37 */
  0, ' ', 0, 0, 0, 0, 0, 0,                 /* 38-39 */
  0, 0, 0, 0, 0, 0, 0, 0,                   /* 40-47 */
  0, 0, 0, 0, 0, 0, 0, 0,                   /* 48-49 */
  0, 0, 0, 0, 0, 0, '|', 0                  /* 50-57 */
};

void key_letter(unsigned char code)
{
  char c;

  if ((shifts & sh_shift) && !(shifts & sh_capslock)) {
    c = letter_shift_map[code];
  }
  else if ((shifts & sh_capslock) && !(shifts & sh_shift)) {
    c = letter_shift_map[code];
  } else {
    c = letter_map[code];
  }

  if (shifts & sh_ctrl) {
    /* create some sort of proper hooks system for this */
    void show_memory(void);
    void show_tasks(void);
    extern const char *kernel_banner;

    if ((c == 'm') || (c == 'M'))
      show_memory();
    else if ((c == 'p') || (c == 'P'))
      show_tasks();
    else if ((c == 'v') || (c == 'V'))
      printk("version: %s", kernel_banner);

    return;
  }

  add_buffer(c);
}

void key_shift(unsigned char code)
{
  if (code & 0x80) {
    shifts &= ~sh_shift;
  }
  else {
    shifts |= sh_shift;
  }
}

void key_ctrl(unsigned char code)
{
  if (code & 0x80) {
    shifts &= ~sh_ctrl;
  }
  else {
    shifts |= sh_ctrl;
  }
}

void key_alt(unsigned char code)
{
  if (e0 == 1) {
    if (code & 0x80) {
      shifts &= ~sh_ralt;
    }
    else {
      shifts |= sh_ralt;
    }
  }
  else {
    if (code & 0x80) {
      shifts &= ~sh_lalt;
    }
    else {
      shifts |= sh_lalt;
    }
  }
}

void key_num(unsigned char code)
{
  if (shifts & sh_numlock) {
    shifts &= ~sh_numlock;
  }
  else {
    shifts |= sh_numlock;
  }
}

void key_caps(unsigned char code)
{
  if (shifts & sh_capslock) {
    shifts &= ~sh_capslock;
  }
  else {
    shifts |= sh_capslock;
  }
}

void key_scroll(unsigned char code)
{
  printk("You pressed scroll\n");
}

void key_sysreq(unsigned char code)
{
  printk("You pressed sysreq\n");
}

void key_func(unsigned char code)
{
  printk("You pressed func\n");
}

void key_keypad(unsigned char code)
{
  if ((code == 0x53) &&
      (shifts & sh_ctrl) &&
      ((shifts & sh_lalt) ||
       (shifts & sh_ralt))) {
    extern void reboot(void);
    printk("keyboard: c-m-del\n");
    reboot();
  }
  else {
    printk("You pressed keypad\n");
  }
}

void key_win(unsigned char code)
{
  printk("You pressed win\n");
}

void key_menu(unsigned char code)
{
  printk("You pressed menu\n");
};
