/* Use the actual wrappers from the kernel */

#include <stdint.h>

#define __KERNEL__
#include <kernel/kernel.h>

#define USE_REAL_STDOUT
#include "../kernel/perlwrap.c"

#ifdef __GNUC__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif
#endif
#include <dlfcn.h>

#define REAL_DECL(func, ret, args) \
  typedef ret (*func##_ftype)args; \
  static func##_ftype func##_real

#define REAL_LOAD(func) \
  if (!func##_real) { func##_real = dlsym(RTLD_NEXT, #func); }

void *kalloc(unsigned int n)
{
  REAL_DECL(malloc, void *, (size_t size));
  REAL_LOAD(malloc);

  return malloc_real(n);
}

void kfree(void *mem)
{
  REAL_DECL(free, void, (void *ptr));
  REAL_LOAD(free);

  free_real(mem);
}

int printk(const char *fmt, ...)
{
  REAL_DECL(vprintf, int, (const char *format, va_list ap));
  va_list ap;
  int n;
  
  REAL_LOAD(vprintf);

  va_start(ap, fmt);
  n = vprintf_real(fmt, ap);
  va_end(ap);

  return n;
}

void panic(const char *fmt, ...)
{
  REAL_DECL(vprintf, int, (const char *format, va_list ap));
  REAL_DECL(exit, void, (int));
  va_list ap;
  int n;
  
  REAL_LOAD(vprintf);
  REAL_LOAD(exit);

  va_start(ap, fmt);
  n = vprintf_real(fmt, ap);
  va_end(ap);

  exit_real(1);
}
