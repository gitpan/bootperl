#if !defined(KERNEL_LINKAGE_H)
#define KERNEL_LINKAGE_H

#if defined(__i386__) && (__GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR > 7)
#define asmlinkage __attribute__((regparm(0)))
#else
#define asmlinkage
#endif

#define SYMBOL_NAME_STR(X) #X
#define SYMBOL_NAME(X) X
#ifdef __STDC__
#define SYMBOL_NAME_LABEL(X) X##:
#else
#define SYMBOL_NAME_LABEL(X) X/**/:
#endif

#if !defined(__i486__) && !defined(__i586__)
#define __ALIGN .align 4,0x90
#define __ALIGN_STR ".align 4,0x90"
#else
#define __ALIGN .align 16,0x90
#define __ALIGN_STR ".align 16,0x90"
#endif

#ifdef __ASSEMBLY__

#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR

#define ENTRY(name) \
  .globl SYMBOL_NAME(name); \
  ALIGN; \
  SYMBOL_NAME_LABEL(name)

#endif

#endif /* !defined(KERNEL_LINKAGE_H) */
