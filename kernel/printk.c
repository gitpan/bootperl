#include <kernel/kernel.h>
#include <asm/init.h>
#include <asm/lock.h>
#include <asm/io.h>
#include <kernel/console.h>

struct console *consoles = 0;
#define LOG_BUF_SIZE (16384)
#define LOG_BUF_MASK (LOG_BUF_SIZE - 1)
static char log_buf[LOG_BUF_SIZE + 1];
static unsigned long log_start = 0;
unsigned long logged_chars = 0;
unsigned long log_size = 0;

static char buf[1024];

asmlinkage int printk(const char *fmt, ...)
{
  va_list args;
  long flags;
  int i, line_feed;
  static signed char msg_level = -1;
  char *msg, *p, *buf_end;

  lock_flags(flags);

  va_start(args, fmt);
  /* add call vsprintf */
  i = vsprintf(buf + 3, fmt, args);
  buf_end = buf + 3 + i;
  va_end(args);

  for (p = buf + 3; p < buf_end; p++) {
    msg = p;
    if (msg_level < 0) { /* If there is no current log_level */
      if ((p[0] != '<') || /* If the start isn't a log level */
	  (p[1] < '0') ||
	  (p[1] > '7') ||
	  (p[2] != '>')) {
	p -= 3; /* move into the extra space at the start and insert a level */
	p[0] = '<';
	p[1] = LOGLEVEL_DEFAULT + '0';
	p[2] = '>';
      }
      else { /* there is a log level ...*/
	/* leve p pointing to log level and msg at the message */
	msg += 3;
      }
      msg_level = p[1] - '0'; /* retreive log level; */
    }
    line_feed = 0; /* zero flag */
    for (; p < buf_end; p++) { /* loop over the msg string looking for nl */
      /* add it to the circular buffer */
      log_buf[(log_start+log_size) & LOG_BUF_MASK] = *p;
      if (log_size < LOG_BUF_SIZE)
	log_size++;
      else
	log_start++;

      logged_chars++;
      if ((*p == '\n') || (*p == '\r')) { /* do we have a nl? */
	line_feed = 1;
	break;
      }
    }

    /* output to the attached consoles */
    if (consoles) { /* check head of linked list */
      struct console *c = consoles;
      while (c) {
	if ((msg_level <= c->loglevel) && c->write)
	  c->write(c, msg, p - msg + line_feed);
	c = c->next;
      }
    }
    if (line_feed)
      msg_level = -1;
  }

  unlock_flags(flags);
  /* wait up blocked processes */
  return i;
}

void register_console(struct console *con)
{
  signed char msg_level = -1;
  long flags;
  int p;
  int i, j, len;
  char buf[16];
  char *q;

  lock_flags(flags);

  /* Add console to linked list at head */
  con->next = consoles;
  consoles = con;

  /* print any buffered output */
  p = log_start & LOG_BUF_MASK;

  for (i = 0, j = 0; i < log_size; i++) {
    buf[j++] = log_buf[p];
    p = (p + 1) & LOG_BUF_MASK;
    if ((buf[j - 1] != '\n') && (i < (log_size - 1)) &&
	(j < (sizeof(buf) - 1)))
	continue;
    buf[j] = 0;
    q = buf;
    len = j;
    if (msg_level < 0) { /* no current log level */
      if ((buf[0] == '<') && /* log level string at start of buf */
	  (buf[1] >= '0') &&
	  (buf[1] <= '7') &&
	  (buf[2] == '>')) {
	msg_level = buf[1] - '0'; /* get level */
	q = buf + 3; /* move past it */
	len -= 3; /* shorten length */
      }
      else {
	msg_level = LOGLEVEL_DEFAULT;
      }
    }
    if (msg_level <= con->loglevel)
      con->write(con, q, len);
    if (buf[j - 1] == '\n')
      msg_level = -1;
    j = 0;
  }

  unlock_flags(flags);
}
