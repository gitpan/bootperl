#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <setjmp.h>
#include <math.h>

/* proceed to be amazed by this magic from mooix */
#ifdef __GNUC__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif
#endif
#include <dlfcn.h>

#define WRAP(a) a

#define REAL_DECL(func, ret, args) \
  typedef ret (*func##_ftype)args; \
  static func##_ftype func##_real

#define REAL_LOAD(func) \
  if (!func##_real) { func##_real = dlsym(RTLD_NEXT, #func); }

int debugf (const char *format, ...)
{
  va_list ap;
  int n;
  REAL_DECL(fflush, int, (FILE *stream));
  REAL_DECL(fputs, int, (const char *s, FILE *stream));
  
  REAL_LOAD(fflush);
  REAL_LOAD(fputs);
  
  fputs_real("wrap: ", stderr);
  va_start(ap, format);
  n = vfprintf(stderr, format, ap);
  va_end(ap);
  fflush_real(stderr);

  return n;
}

extern int errno;

int WRAP(chmod) (const char *path, mode_t mode)
{
  debugf("chmod(%s, %o)\n", path, mode);
  errno = ENOSYS;
  return -1;
}

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

int WRAP(open) (const char *name, int flags, mode_t mode)
{
/*  typedef int (*open_ftype)(const char *name, int flags, mode_t mode);
  static open_ftype f;
  if (!f) {
    f = dlsym(RTLD_NEXT, "open");
    if (!f)
      return -1;
  } */

  debugf("open(%s, %x, %o)\n", name, flags, mode);
  errno = ENOSYS;
  return -1;
  /*return f(name, flags, mode);*/
}

/* disabled since this is important */
FILE * WRAP(fopen) (const char *path, const char *mode)
{
  debugf("fopen(%s, %s)\n", path, mode);
  errno = ENOSYS;
  return NULL;
}

int WRAP(dup) (int oldfd)
{
  debugf("dup(%d)\n", oldfd);
  errno = ENOSYS;
  return -1;
}

void WRAP(exit) (int status)
{
  typedef int (*exit_ftype)(int status);
  static exit_ftype f;
  if (!f) {
    f = dlsym(RTLD_NEXT, "exit");
    /* if f is null, just seg fault: it's the best way to exit :-P */
  }
  debugf("exit(%d)\n", status);
  f(status);
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

int WRAP(stat) (const char *file_name, struct stat *buf)
{
  debugf("stat(%s, %x)\n", file_name, buf);
  errno = ENOSYS;
  return -1;
}

int WRAP(fstat) (int filedes, struct stat *buf)
{
  debugf("fstat(%d, %x)\n", filedes, buf);
  errno = ENOSYS;
  return -1;
}

long WRAP(ftell) (FILE *stream)
{
  debugf("ftell(%x)\n", stream);
  errno = ENOSYS;
  return -1;
}

void WRAP(rewind) (FILE *stream)
{
  debugf("rewind(%x)\n", stream);
  errno = ENOSYS;
}

int WRAP(fseek) (FILE *stream, long offset, int whence)
{
  debugf("fseek(%x, %d, %d)\n", stream, offset, whence);
  errno = ENOSYS;
  return -1;
}

int WRAP(fileno) (FILE *stream)
{
  debugf("fileno(%x)\n", stream);
  errno = ENOSYS;
  return -1;
}

void WRAP(clearerr) (FILE *stream)
{
  debugf("clearerr(%x)\n", stream);
  errno = ENOSYS;
}

int WRAP(ferror) (FILE *stream)
{
  debugf("ferror(%x)\n", stream);
  errno = ENOSYS;
  return -1;
}

int WRAP(feof) (FILE *stream)
{
  debugf("feof(%x)\n", stream);
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
  debugf("time(%x)\n", t);
  errno = ENOSYS;
  return (time_t)-1;
}

struct tm * WRAP(localtime) (const time_t *timep)
{
  debugf("localtime(%x [%d])\n", timep, *timep);
  errno = ENOSYS;
  return NULL;
}

struct tm * WRAP(gmtime) (const time_t *timep)
{
  debugf("gmtime(%x [%d])\n", timep, *timep);
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
    fprintf(stderr, "%s", *argv);
    argv++;
    if (*argv != NULL)
      fprintf(stderr, ", ");
  }
  fprintf(stderr, "]);\n");
  fflush(stderr);
  errno = ENOSYS;
  return -1;
}

int WRAP(execvp) (const char *path, char *const argv[])
{
  debugf("execvp(%s, [", path);
  while (*argv != NULL) {
    fprintf(stderr, "%s", *argv);
    argv++;
    if (*argv != NULL)
      fprintf(stderr, ", ");
  }
  fprintf(stderr, "]);\n");
  fflush(stderr);
  errno = ENOSYS;
  return -1;
}

int WRAP(kill) (pid_t pid, int sig)
{
  debugf("kill(%d, %d)\n", pid, sig);
  errno = ENOSYS;
  return -1;
}

char * WRAP(getenv) (const char *name)
{
  debugf("getenv(%s)\n", name);
  errno = ENOSYS;
  return NULL;
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
  debugf("umask(%s)\n", mask);
  errno = ENOSYS;
  return 0;
}

pid_t WRAP(wait) (int *status)
{
  debugf("wait(%x)\n", status);
  errno = ENOSYS;
  return -1;
}

FILE * WRAP(tmpfile) (void)
{
  debugf("tmpfile()\n");
  errno = ENOSYS;
  return NULL;
}

int WRAP(closedir) (DIR *dir)
{
  debugf("closedir(%x)\n", dir);
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
  debugf("freopen(%s, %s, %x)\n", path, mode, stream);
  errno = ENOSYS;
  return NULL;
}

size_t WRAP(fread) (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  debugf("fread(%x, %d, %d, %x)\n", ptr, size, nmemb, stream);
  errno = ENOSYS;
  return 0;
}

/* used for print */
/*size_t WRAP(fwrite) (const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  debugf("fwrite(%x, %d, %d, %x)\n", ptr, size, nmemb, stream);
  errno = ENOSYS;
  return 0;
}*/

int WRAP(fputc) (int c, FILE *stream)
{
  debugf("fputc(%c, %x)\n", c, stream);
  errno = ENOSYS;
  return EOF;
}

int WRAP(fputs) (const char *s, FILE *stream)
{
  debugf("fputs(%s, %x)\n", s, stream);
  errno = ENOSYS;
  return EOF;
}

typedef void (*sighandler_t)(int);

sighandler_t WRAP(signal) (int signum, sighandler_t handler)
{
  debugf("signal(%d, %x)\n", signum, handler);
  errno = ENOSYS;
  return SIG_ERR;
}

int WRAP(fflush) (FILE *stream)
{
  debugf("fflush(%x)\n", stream);
  errno = ENOSYS;
  return EOF;
}

/* required for io */
/* int WRAP(fgetc) (FILE *stream)
{
  debugf("fgetc(%x)\n", stream);
  errno = ENOSYS;
  return EOF;
} */

ssize_t WRAP(read) (int fd, void *buf, size_t count)
{
  debugf("read(%d, %x, %d)\n", fd, buf, count);
  errno = ENOSYS;
  return -1;
}

ssize_t WRAP(write) (int fd, const void *buf, size_t count)
{
  debugf("write(%d, %x, %d)\n", fd, buf, count);
  errno = ENOSYS;
  return -1;
}

/* required for some kind of exception handling */
/*void WRAP(longjmp) (jmp_buf env, int val)
{
  debugf("longjmp(env, %d)\n", val);
  errno = ENOSYS;
} */

int WRAP(isatty) (int desc)
{
  debugf("isatty(%d)\n", desc);
  return 1;
}

off_t WRAP(lseek) (int filedes, off_t offset, int whence)
{
  debugf("lseek(%d, %d, %d)\n", filedes, offset, whence);
  errno = ENOSYS;
  return (off_t)-1;
}

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
  debugf("sqrt(%x)\n", x);
  return 0.0;
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
