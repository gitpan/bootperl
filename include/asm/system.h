#if !defined(ASM_SYSTEM_H)
#define ASM_SYSTEM_H

/* Segment descriptors */
#define KERNEL_CODE 0x10
#define KERNEL_DATA 0x18

typedef unsigned long long segdesc_t;

extern segdesc_t *idt;

static void inline set_gate(segdesc_t *desc, unsigned char type,
			    unsigned char dpl, void *addr,
			    unsigned short segment)
{
  *(0 + (unsigned long *)desc) = (segment << 16) + ((long)addr & 0xffff);
  *(1 + (unsigned long *)desc) = ((long)addr & 0xffff0000) +
    (1 << 15) + (dpl << 13) + (type << 8);
}

static void inline set_intr_gate(unsigned short n, void *addr)
{
  set_gate(&idt[n], 14, 0, addr, KERNEL_CODE);
}

static void inline set_trap_gate(unsigned short n, void *addr)
{
  set_gate(&idt[n], 15, 0, addr, KERNEL_CODE);
}

static void inline load_idtr(segdesc_t *idt, short limit)
{
  asm("jmp 2f\n"
      "1:\t.word 0\n\t"
      ".long 0\n"
      "2:\tmovl $1b, %%eax\n\t"
      "movw %0, 0(%%eax)\n\t"
      "movl %1, 2(%%eax)\n\t"
      "lidt 1b"
      : : "r" (limit), "r" (idt) : "eax");
}

#endif /* !defined(ASM_SYSTEM_H) */
