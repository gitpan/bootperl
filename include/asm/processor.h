/* include/asm/processor.h
 * This is based upon the include/asm-i386/processor.h in the linux kernel
 * written by Linux Torvalds and Copyright (C) 1994 Linus Torvalds
 * The changes made in this version are Copyright (C) 2002 Benjamin Smith.
 */


#if !defined(ASM_PROCESSOR_H)
#define ASM_PROCESSOR_H

typedef unsigned char uint8_t;

struct cpuinfo {
  uint8_t x86; /* family */
  uint8_t x86_vendor; /* vendor */
  uint8_t x86_model; /* model */
  uint8_t x86_mask; /* mask */
  char wp_works_ok; /* doesn't on 386 */
  char hlt_works_ok; /* problems on some 486dx4 and old 386 */
  char hard_math;
  char rfu;
  int cpuid_level; /* max cpuid level, -1 = no CPUID */
  unsigned int x86_capability[4];
  char x86_vendor_id[16];
  char x86_model_id[64];
  int x86_cache_size; /* in kb, if supported */
  int fdiv_bug;
  int f00f_bug;
  int coma_bug;
};

#define X86_VENDOR_INTEL 0
#define X86_VENDOR_CYRIX 1
#define X86_VENDOR_AMD 2
#define X86_VENDOR_UMC 3
#define X86_VENDOR_NEXGEN 4
#define X86_VENDOR_CENTAUR 5
#define X86_VENDOR_RISE 6
#define X86_VENDOR_TRANSMETA 7
#define X86_VENDOR_UNKNOWN 0xff

/*
 * EFLAGS43
 bits
 */
#define X86_EFLAGS_CF   0x00000001 /* Carry Flag */
#define X86_EFLAGS_PF   0x00000004 /* Parity Flag */
#define X86_EFLAGS_AF   0x00000010 /* Auxillary carry Flag */
#define X86_EFLAGS_ZF   0x00000040 /* Zero Flag */
#define X86_EFLAGS_SF   0x00000080 /* Sign Flag */
#define X86_EFLAGS_TF   0x00000100 /* Trap Flag */
#define X86_EFLAGS_IF   0x00000200 /* Interrupt Flag */
#define X86_EFLAGS_DF   0x00000400 /* Direction Flag */
#define X86_EFLAGS_OF   0x00000800 /* Overflow Flag */
#define X86_EFLAGS_IOPL 0x00003000 /* IOPL mask */
#define X86_EFLAGS_NT   0x00004000 /* Nested Task */
#define X86_EFLAGS_RF   0x00010000 /* Resume Flag */
#define X86_EFLAGS_VM   0x00020000 /* Virtual Mode */
#define X86_EFLAGS_AC   0x00040000 /* Alignment Check */
#define X86_EFLAGS_VIF  0x00080000 /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIP  0x00100000 /* Virtual Interrupt Pending */
#define X86_EFLAGS_ID   0x00200000 /* CPUID detection flag */

/*
 * Generic CPUID function
 */
static inline void cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
        __asm__("cpuid"
                : "=a" (*eax),
                  "=b" (*ebx),
                  "=c" (*ecx),
                  "=d" (*edx)
                : "0" (op));
}

/*
 * CPUID functions returning a single datum
 */
static inline unsigned int cpuid_eax(unsigned int op)
{
        unsigned int eax;

        __asm__("cpuid"
                : "=a" (eax)
                : "0" (op)
                : "bx", "cx", "dx");
        return eax;
}
static inline unsigned int cpuid_ebx(unsigned int op)
{
        unsigned int eax, ebx;

        __asm__("cpuid"
                : "=a" (eax), "=b" (ebx)
                : "0" (op)
                : "cx", "dx" );
        return ebx;
}
static inline unsigned int cpuid_ecx(unsigned int op)
{
        unsigned int eax, ecx;

        __asm__("cpuid"
                : "=a" (eax), "=c" (ecx)
                : "0" (op)
                : "bx", "dx" );
        return ecx;
}
static inline unsigned int cpuid_edx(unsigned int op)
{
        unsigned int eax, edx;

        __asm__("cpuid"
                : "=a" (eax), "=d" (edx)
                : "0" (op)
                : "bx", "cx");
        return edx;
}

/*
 * Intel CPU features in CR4
 */
#define X86_CR4_VME             0x0001  /* enable vm86 extensions */
#define X86_CR4_PVI             0x0002  /* virtual interrupts flag enable */
#define X86_CR4_TSD             0x0004  /* disable time stamp at ipl 3 */
#define X86_CR4_DE              0x0008  /* enable debugging extensions */
#define X86_CR4_PSE             0x0010  /* enable page size extensions */
#define X86_CR4_PAE             0x0020  /* enable physical address extensions */
#define X86_CR4_MCE             0x0040  /* Machine check enable */
#define X86_CR4_PGE             0x0080  /* enable global pages */
#define X86_CR4_PCE             0x0100  /* enable performance counters at ipl 3 */
#define X86_CR4_OSFXSR          0x0200  /* enable fast FPU save and restore */
#define X86_CR4_OSXMMEXCPT      0x0400  /* enable unmasked SSE exceptions */

#endif /* !defined(ASM_PROCESSOR_H) */
