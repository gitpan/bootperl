#if !defined(KERNEL_CONSOLE_H)
#define KERNEL_CONSOLE_H

/* parameter struct passed from the startup code to the kernel */
struct vidparams {
  unsigned char orig_x;
  unsigned char orig_y;
  unsigned short page;
  unsigned char mode;
  unsigned char cols;
  unsigned char lines;
  unsigned char pad1;
  unsigned short font_points;
  unsigned short magic;
};

/* struct representing a console driver */
struct console {
  void (*write)(struct console *, const char *, unsigned int);
  void *data;
  unsigned int loglevel;
  struct console *next;
};

#define LOGLEVEL_MAX 7
#define LOGLEVEL_MIN 0
#define LOGLEVEL_DEFAULT 4

void register_console(struct console *);
void puts(char *c);

#endif /* KERNEL_CONSOLE_H */
