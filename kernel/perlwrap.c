/* wrappers for lots of things perl things it needs */
#include <kernel/kernel.h>
#include <kernel/memory.h>

/* this breaks every rule in the book :-) */
#include <stdlib.h>
#include <stdio.h>
#include <linux/errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>

#define debugf(fmt, ...) printk("++ wrap: " fmt, ## __VA_ARGS__)
#define WRAP(a) a

int errno;

int *__errno_location(void)
{
  return &errno;
}

/* memory management */
void * WRAP(malloc) (size_t size)
{
  /* debugf("malloc(%d)\n", size); */
  return kalloc(size);
}

void WRAP(free) (void *ptr)
{
  /* debugf("free(%p)\n", ptr); */
  return kfree(ptr);
}

void * WRAP(calloc) (size_t nmemb, size_t size)
{
  void *mem;
  mem = kalloc(nmemb * size);
  return memset(mem, 0, nmemb * size);
}

void * WRAP(realloc) (void *ptr, size_t size)
{
  void *newmem = 0;
  if (size)
    newmem = kalloc(size);
  if (size && !newmem)
    return NULL;
  if (ptr && size)
    memcpy(newmem, ptr, size); /* cheat, this reads past the end of the old ptr */
  if (ptr)
    kfree(ptr);
  return newmem;
}

/* misc standard library functions */
int WRAP(chdir) (const char *path)
{
  debugf("chdir(%s)\n", path);
  errno = ENOSYS;
  return -1;
}
 
int WRAP(sleep) (unsigned int seconds)
{
  debugf("sleep(%d)\n", seconds);
  errno = ENOSYS;
  return -1;
}

int WRAP(chmod) (const char *path, mode_t mode)
{
  debugf("chmod(%s, %o)\n", path, mode);
  errno = ENOSYS;
  return -1;
}

pid_t WRAP(fork) (void)
{
  debugf("fork()\n");
  errno = ENOSYS;
  return -1;
}

time_t WRAP(time) (time_t *t)
{
  debugf("time(%p)\n", t);
  errno = ENOSYS;
  return -1;
}

struct tm * WRAP(localtime) (const time_t *timep)
{
  debugf("localtime(%p [%ld])\n", timep, *timep);
  errno = ENOSYS;
  return NULL;
}
                                                                                
struct tm * WRAP(gmtime) (const time_t *timep)
{
  debugf("gmtime(%p [%ld])\n", timep, *timep);
  errno = ENOSYS;
  return NULL;
}
                                                                                
int WRAP(execl) (const char *path, const char *arg, ...)
{
  debugf("execl(%s, %s, ...)\n", path, arg);
  errno = ENOSYS;
  return -1;
}

int WRAP(execv) (const char *path, char *const argv[])
{
  debugf("execv(%s, [", path);
  while (*argv != NULL) {
    printk("%s", *argv);
    argv++;
    if (*argv != NULL)
      printk(", ");
  }
  printk("]);\n");
  errno = ENOSYS;
  return -1;
}

int WRAP(execvp) (const char *path, char *const argv[])
{
  debugf("execvp(%s, [", path);
  while (*argv != NULL) {
    printk("%s", *argv);
    argv++;
    if (*argv != NULL)
      printk(", ");
  }
  printk("]);\n");
  errno = ENOSYS;
  return -1;
}

int WRAP(kill) (pid_t pid, int sig)
{
  debugf("kill(%d, %d)\n", pid, sig);
  errno = ENOSYS;
  return -1;
}

int WRAP(rand) (void)
{
  debugf("rand()\n");
  errno = ENOSYS;
  return 1;
}
                                                                                
void WRAP(srand) (unsigned int seed)
{
  debugf("srand(%u)\n", seed);
  errno = ENOSYS;
}
                                                                                
int WRAP(link) (const char *oldpath, const char *newpath)
{
  debugf("link(%s, %s)\n", oldpath, newpath);
  errno = ENOSYS;
  return -1;
}

int WRAP(unlink) (const char *pathname)
{
  debugf("unlink(%s)\n", pathname);
  errno = ENOSYS;
  return -1;
}
                                                                                
mode_t WRAP(umask) (mode_t mask)
{
  debugf("umask(%d)\n", mask);
  errno = ENOSYS;
  return 0;
}
                                                                                
pid_t WRAP(wait) (int *status)
{
  debugf("wait(%p)\n", status);
  errno = ENOSYS;
  return -1;
}

FILE * WRAP(tmpfile) (void)
{
  debugf("tmpfile()\n");
  errno = ENOSYS;
  return NULL;
}

char * WRAP(getenv) (const char *name)
{
  debugf("getenv(%s)\n", name);
  errno = ENOSYS;
  return NULL;
}

int WRAP(putenv) (char *string)
{
  debugf("putenv(%s)\n", string);
  errno = ENOSYS;
  return -1;
}

pid_t WRAP(getpid) (void)
{
  debugf("getpid()\n");
  errno = ENOSYS;
  return -1;
}
 
uid_t WRAP(getuid) (void)
{
  debugf("getuid()\n");
  errno = ENOSYS;
  return -1;
}

uid_t WRAP(geteuid) (void)
{
  debugf("geteuid()\n");
  errno = ENOSYS;
  return -1;
}

int WRAP(setgid) (gid_t gid)
{
  debugf("setgid(%d)\n", gid);
  errno = ENOSYS;
  return -1;
}

int WRAP(setuid) (uid_t uid)
{
  debugf("setuid(%d)\n", uid);
  errno = ENOSYS;
  return -1;
}

gid_t WRAP(getgid) (void)
{
  debugf("getgid()\n");
  errno = ENOSYS;
  return -1;
}

gid_t WRAP(getegid) (void)
{
  debugf("getegid()\n");
  errno = ENOSYS;
  return -1;
}

typedef void (*sighandler_t)(int);
                                                                                
sighandler_t WRAP(signal) (int signum, sighandler_t handler)
{
  debugf("signal(%d, %p)\n", signum, handler);
  errno = ENOSYS;
  return SIG_ERR;
}

int WRAP(isatty) (int desc)
{
  debugf("isatty(%d)\n", desc);
  return 1;
}

int rename(const char *oldpath, const char *newpath)
{
  debugf("rename(%s, %s)\n", oldpath, newpath);
  errno = ENOSYS;
  return -1;
}

void WRAP(exit) (int status)
{
  debugf("exit(%d)\n", status);
  errno = ENOSYS;
  /* hmm, nothing to do now */
  /* this produces a warning, please ignore it */
  panic("program tried to exit: %d", status);
}

void WRAP(_exit) (int status)
{
  debugf("_exit(%d)\n", status);
  errno = ENOSYS;
  /* hmm, nothing to do now */
  /* this produces a warning, please ignore it */
  panic("program tried to _exit: %d", status);
}

char * WRAP(strerror) (int errnum)
{
  debugf("strerror(%d)\n", errnum);
  errno = ENOSYS;
  return "Unknown error";
}

/* io */
#ifndef USE_REAL_STDOUT
FILE *stdin = (FILE *)0x01;
FILE *stdout = (FILE *)0x02;
FILE *stderr = (FILE *)0x03;
#endif
FILE *devnull = (FILE *)0x04;

int WRAP(open) (const char *name, int flags, mode_t mode)
{
  debugf("open(%s, %x, %o)\n", name, flags, mode);
  errno = ENOSYS;
  return -1;
}

int WRAP(close) (int fd)
{
  debugf("close(%d)\n", fd);
  errno = ENOSYS;
  return -1;
}

int WRAP(dup) (int oldfd)
{
  debugf("dup(%d)\n", oldfd);
  errno = ENOSYS;
  return -1;
}

FILE * WRAP(fopen) (const char *path, const char *mode)
{
  debugf("fopen(%s, %s)\n", path, mode);
  if (strcmp(path, "/dev/null") == 0) {
    debugf("  opening devnull\n");
    return devnull;
  }
  errno = ENOSYS;
  return NULL;
}

int WRAP(fclose) (FILE *stream)
{
  debugf("fclose(%p)\n", stream);
  errno = ENOSYS;
  return EOF;
}

int WRAP(stat) (const char *file_name, struct stat *buf)
{
  debugf("stat(%s, %p)\n", file_name, buf);
  errno = ENOSYS;
  return -1;
}

int __xstat (int __ver, const char *__filename, struct stat *__stat_buf)
{
  debugf("__xstat(%d, %s, %p)\n", __ver, __filename, __stat_buf);
  errno = ENOSYS;
  return -1;
}
                                                                                
int WRAP(fstat) (int filedes, struct stat *buf)
{
  debugf("fstat(%d, %p)\n", filedes, buf);
  errno = ENOSYS;
  return -1;
}

int __fxstat (int __ver, int __fildes, struct stat *__stat_buf)
{
  debugf("__fxstat(%d, %d, %p)\n", __ver, __fildes, __stat_buf);
  errno = ENOSYS;
  return -1;
}
                                                                                
long WRAP(ftell) (FILE *stream)
{
  debugf("ftell(%p)\n", stream);
  errno = ENOSYS;
  return -1;
}

void WRAP(rewind) (FILE *stream)
{
  debugf("rewind(%p)\n", stream);
  errno = ENOSYS;
}
                                                                                
int WRAP(fseek) (FILE *stream, long offset, int whence)
{
  debugf("fseek(%p, %ld, %d)\n", stream, offset, whence);
  errno = ENOSYS;
  return -1;
}
                                                                                
int WRAP(fileno) (FILE *stream)
{
  debugf("fileno(%p)\n", stream);
  errno = ENOSYS;
  return -1;
}

void WRAP(clearerr) (FILE *stream)
{
  debugf("clearerr(%p)\n", stream);
  errno = ENOSYS;
}
                                                                                
int WRAP(ferror) (FILE *stream)
{
  debugf("ferror(%p)\n", stream);
  errno = ENOSYS;
  return -1;
}
                                                                                
int WRAP(feof) (FILE *stream)
{
  debugf("feof(%p)\n", stream);
  errno = ENOSYS;
  return -1;
}

DIR * WRAP(opendir) (const char *name)
{
  debugf("opendir(%s)\n", name);
  errno = ENOSYS;
  return NULL;
}

struct dirent * WRAP(readdir) (DIR *dir)
{
  debugf("readdir(%p)\n", dir);
  errno = ENOSYS;
  return NULL;
}

int WRAP(closedir) (DIR *dir)
{
  debugf("closedir(%p)\n", dir);
  errno = ENOSYS;
  return -1;
}
                                                                                
FILE * WRAP(fdopen) (int fildes, const char *mode)
{
  debugf("fdopen(%d, %s)\n", fildes, mode);
  errno = ENOSYS;
  return NULL;
}

FILE * WRAP(freopen) (const char *path, const char *mode, FILE *stream)
{
  debugf("freopen(%s, %s, %p)\n", path, mode, stream);
  errno = ENOSYS;
  return NULL;
}
                                                                                
size_t WRAP(fread) (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  debugf("fread(%p, %d, %d, %p)\n", ptr, size, nmemb, stream);
  errno = ENOSYS;
  return 0;
}

int WRAP(fflush) (FILE *stream)
{
  debugf("fflush(%p)\n", stream);
  errno = ENOSYS;
  return EOF;
}

ssize_t WRAP(read) (int fd, void *buf, size_t count)
{
  debugf("read(%d, %p, %d)\n", fd, buf, count);
  errno = ENOSYS;
  return -1;
}
 
ssize_t WRAP(write) (int fd, const void *buf, size_t count)
{
  debugf("write(%d, %p, %d)\n", fd, buf, count);
  errno = ENOSYS;
  return -1;
}

off_t WRAP(lseek) (int filedes, off_t offset, int whence)
{
  debugf("lseek(%d, %ld, %d)\n", filedes, offset, whence);
  errno = ENOSYS;
  return (off_t)-1;
}

int WRAP(pipe) (int filedes[2])
{
  debugf("pipe([%d, %d])\n", filedes[0], filedes[1]);
  errno = ENOSYS;
  return -1;
}

int WRAP(setvbuf) (FILE *stream, char *buf, int mode, size_t size)
{
  debugf("setvbuf(%p, %s, %d, %d)\n", stream, buf, mode, size);
  errno = ENOSYS;
  return -1;
}

/* These are floating point functions.  We can get away with using floating
 * point in perl because it's the only process doing it. */

double WRAP(exp) (double x)
{
  debugf("exp(%f)\n", x);
  return 0.0;
}
                                                                                
double WRAP(log) (double x)
{
  debugf("log(%f)\n", x);
  return 0.0;
}
                                                                                
double WRAP(pow) (double x, double y)
{
  debugf("pow(%f, %f)\n", x, y);
  return 0.0;
}
                                                                                
double WRAP(floor) (double x)
{
  debugf("floor(%f)\n", x);
  return 0.0;
}

double WRAP(fmod) (double x, double y)
{
  debugf("fmod(%f, %f)\n", x, y);
  return 0.0;
}
 
double WRAP(atan2) (double y, double x)
{
  debugf("atan2(%f, %f)\n", y, x);
  return 0.0;
}
 
double WRAP(sin) (double x)
{
  debugf("sin(%f)\n", x);
  return 0.0;
}

double WRAP(cos) (double x)
{
  debugf("cos(%f)\n", x);
  return 0.0;
}
                                                                                
double WRAP(sqrt) (double x)
{
  debugf("sqrt(%f)\n", x);
  return 0.0;
}

double WRAP(ceil) (double x)
{
  debugf("ceil(%f)\n", x);
  return 0.0;
}

double WRAP(modf) (double x, double *iptr)
{
  debugf("modf(%f, %p)\n", x, iptr);
  return 0.0;
}

double WRAP(frexp) (double x, int *exp)
{
  debugf("frexp(%f, %p)\n", x, exp);
  return 0.0;
}

/* broken yet subtly working functions */
void WRAP(qsort) (void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *))
{
  void *temp;
  /* a bubble sort! */
  /* base is a array of nmemb elements, each size bytes big */
  /* this isn't what most people would call a bubble sort: it doesn't do
   * anything */

  debugf("qsort(%p, %d, %d, %p)\n", base, nmemb, size, compar);

  temp = kalloc(size);

  printk("BUBBLE!\n");

  kfree(temp);
}

/* we pray that longjmps aren't important to perl */
int WRAP(_setjmp) (jmp_buf env)
{
  /* do nothing except pray */
  debugf("_setjmp(%p)\n", env);
  return 0;
}

void WRAP(longjmp) (jmp_buf env, int val)
{
  /* do nothing except pray */
  debugf("longjmp(%p, %d)\n", env, val);
}

/* some functions which actually work */
void * WRAP(memchr) (const void *s, int c, size_t n)
{
  int i;
  char *ss = (char *)s;

  for (i = 0; i < n; i++)
    if (ss[i] == c)
      return ss + i;
  return NULL;
}

long int WRAP(__sysconf) (int k)
{
  /* CLK_TCK calls __sysconf(2) */
  debugf("__sysconf(%d)\n", k);
  return HZ;
}

char * WRAP(strcat) (char *dest, const char *src)
{
  char *start = dest;

  while (*dest != '\0')
    dest++;
  while (*src != '\0')
    *dest++ = *src++;
  *dest = '\0';

  return start;
}

#undef strchr
char * WRAP(strchr) (const char *s, int c)
{
  while (*s != '\0') {
    if (*s == c)
      return (char *)s;
    s++;
  }
  return NULL;
}

char * WRAP(strrchr) (const char *s, int c)
{
  char *match = NULL;
  while (*s != '\0') {
    if (*s == c)
      match = (char *)s;
    s++;
  }
  return match;
}

char * WRAP(strcpy) (char *dest, const char *src)
{
  char *start = dest;

  while (*src != '\0')
    *dest++ = *src++;
  *dest = '\0';
 
  return start;
}

#undef strcmp
int WRAP(strcmp) (const char *s1, const char *s2)
{
  while (*s1 != '\0' && *s2 != '\0') {
    if (*s1 < *s2)
      return -1;
    if (*s1 > *s2)
      return 1;
  }

  if (*s1 < *s2)
    return -1;
  if (*s1 > *s2)
    return 1;
  return 0;
}

#undef strncmp
int WRAP(strncmp) (const char *s1, const char *s2, size_t n)
{
  int i;

  for (i = 0; i < n; i++) {
    if (s1[i] < s2[i])
      return -1;
    if (s1[i] > s2[i])
      return 1;
    if (s1[i] == '\0' || s2[i] == '\0')
      return 0;
  }

  return 0;
}

/* this one is copied from glibc-2.3.2 */
unsigned long int WRAP(strtoul) (const char *nptr, char **endptr, int base)
{
  int negative;
  register unsigned long int cutoff;
  register unsigned int cutlim;
  register unsigned long int i;
  register const char *s;
  register unsigned char c;
  const char *save, *end;
  int overflow;
                                                                                
  if (base < 0 || base == 1 || base > 36) {
    errno = EINVAL;
    return 0;
  }
                                                                                
  save = s = nptr;
                                                                                
  /* Skip white space.  */
  while (isspace (*s))
    ++s;
  if (*s == '\0')
    goto noconv;
                                                                                
  /* Check for a sign.  */
  negative = 0;
  if (*s == '-') {
    negative = 1;
    ++s;
  }
  else if (*s == '+')
    ++s;
                                                                                
  /* Recognize number prefix and if BASE is zero, figure it out ourselves.  */
  if (*s == '0') {
    if ((base == 0 || base == 16) && toupper(s[1]) == 'X') {
      s += 2;
      base = 16;
    }
    else if (base == 0)
      base = 8;
  }
  else if (base == 0)
    base = 10;
                                                                                
  /* Save the pointer so we can check later if anything happened.  */
  save = s;
                                                                                
  end = NULL;
                                                                                
  cutoff = ULONG_MAX / (unsigned long int) base;
  cutlim = ULONG_MAX % (unsigned long int) base;
                                                                                
  overflow = 0;
  i = 0;
  c = *s;
  for (; c != '\0'; c = *++s) {
    if (s == end)
      break;
    if (c >= '0' && c <= '9')
      c -= '0';
    else if (isalpha (c))
      c = toupper (c) - 'A' + 10;
    else
      break;
    if ((int) c >= base)
      break;
    /* Check for overflow.  */
    if (i > cutoff || (i == cutoff && c > cutlim))
      overflow = 1;
    else {
      i *= (unsigned long int) base;
      i += c;
    }
  }
                                                                                
  /* Check if anything actually happened.  */
  if (s == save)
    goto noconv;
                                                                                
  /* Store in ENDPTR the address of one character
     past the last character we converted.  */
  if (endptr != NULL)
    *endptr = (char *) s;
                                                                                
  if (overflow) {
    errno = ERANGE;
    return ULONG_MAX;
  }
                                                                                
  /* Return the result of the appropriate sign.  */
  return negative ? -i : i;
                                                                                
noconv:
  /* We must handle a special case here: the base is 0 or 16 and the
     first two characters are '0' and 'x', but the rest are no
     hexadecimal digits.  This is no error case.  We return 0 and
     ENDPTR points to the `x`.  */
  if (endptr != NULL) {
    if (save - nptr >= 2 && toupper (save[-1]) == 'X'
      && save[-2] == '0')
      *endptr = (char *) &save[-1];
    else
      /*  There was no number to convert.  */
      *endptr = (char *) nptr;
  }
                                                                                
  return 0L;
}

int WRAP(atoi) (const char *nptr)
{
  /* notice the bad cast */
  return (int) strtoul(nptr, (char **)NULL, 10);
}

/* TODO: these functions will in the future be unstubbed */

int WRAP(vfprintf) (FILE *stream, const char *format, va_list ap)
{
  debugf("vfprintf(%p, %s, ...)\n", stream, format);
  errno = ENOSYS;
  return -1;
}

int WRAP(ungetc) (int c, FILE *stream)
{
  debugf("ungetc(%c, %p)\n", c, stream);
  errno = ENOSYS;
  return EOF;
}

size_t WRAP(fwrite) (const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  debugf("fwrite(%p, %d, %d, %p)\n", ptr, size, nmemb, stream);
  debugf("  %d bytes: %s\n", size * nmemb, ptr);
  errno = ENOSYS;
  return EOF;
}

int WRAP(fputc) (int c, FILE *stream)
{
  debugf("fputc(%c, %p)\n", c, stream);
  errno = ENOSYS;
  return EOF;
}

int WRAP(fputs) (const char *s, FILE *stream)
{
  debugf("fputs(%s, %p)\n", s, stream);
  errno = ENOSYS;
  return EOF;
}

int WRAP(fprintf) (FILE *stream, const char *format, ...)
{
  debugf("vfprintf(%p, %s, ...)\n", stream, format);
  errno = ENOSYS;
  return -1;
}

int WRAP(fgetc) (FILE *stream)
{
  debugf("fgetc(%p)\n", stream);
  errno = ENOSYS;
  return -1;
}
