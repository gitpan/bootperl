#if !defined(KERNEL_GDT_H)
#define KERNEL_GDT_H

/* Data segment types */
#define SEG_DATA_RO        0x0
#define SEG_DATA_RO_A      0x1
#define SEG_DATA_RW        0x2
#define SEG_DATA_RW_A      0x3
#define SEG_DATA_DOWN_RO   0x4
#define SEG_DATA_DOWN_RO_A 0x5
#define SEG_DATA_DOWN_RW   0x6
#define SEG_DATA_DOWN_RW_A 0x7

/* Code segment types */
#define SEG_CODE_EO           0x8
#define SEG_CODE_EO_A         0x9
#define SEG_CODE_ER           0xa
#define SEG_CODE_ER_A         0xb
#define SEG_CODE_CONFORM_EO   0xc
#define SEG_CODE_CONFORM_EO_A 0xd
#define SEG_CODE_CONFORM_ER   0xe
#define SEG_CODE_CONFORM_ER_A 0xf

/* System segment types */
#define SEG_SYS_TSS16      0x1
#define SEG_SYS_LDT        0x2
#define SEG_SYS_TSS16_BUSY 0x3
#define SEG_SYS_CALL16     0x4
#define SEG_SYS_TASK       0x5
#define SEG_SYS_INT16      0x6
#define SEG_SYS_TRAP16     0x7
#define SEG_SYS_TSS32      0x9
#define SEG_SYS_TSS32_BUSY 0xb
#define SEG_SYS_CALL32     0xc
#define SEG_SYS_INT32      0xe
#define SEG_SYS_TRAP32     0xf

/* Descriptor types */
#define DESC_SYSTEM   0x0
#define DESC_CODEDATA 0x1

struct gdt_entry {
  unsigned short limit_low : 16;  /* Low-order bits of limit */
  unsigned short base_low : 16;   /* Low-order bits of base */
  unsigned short base_middle : 8; /* Middle-order bits of base */
  unsigned short seg_type : 4;    /* Segment type */
  unsigned short desc_type : 1;   /* System or Code/Data */
  unsigned short dpl : 2;         /* Descriptor Privilege Level */
  unsigned short present : 1;     /* Present? */
  unsigned short limit_high : 4;  /* High-order bits of limit */
  unsigned short avail : 1;       /* Availble for use */
  unsigned short reserved : 1;    /* Reserved: 0 */
  unsigned short op_size : 1;     /* Operation Size */
  unsigned short granularity : 1; /* Byte or Page? */
  unsigned short base_high : 8;   /* High-order bits of base */
};

struct np_entry {
  unsigned long available1 : 32;
  unsigned short available2 : 8;
  unsigned short seg_type : 4;
  unsigned short desc_type : 1;
  unsigned short dpl : 2;
  unsigned short present : 1;
  unsigned short available3 : 16;
};

union entry {
  struct gdt_entry gdt;
  struct np_entry np;
};

extern struct gdt_entry gdt[];

static const inline unsigned long gdt_get_limit(const struct gdt_entry gdt)
{
  return (unsigned long)gdt.limit_low | (gdt.limit_high << 16);
}

static inline void gdt_set_limit(struct gdt_entry *gdt,
				 unsigned long limit)
{
  gdt->limit_low = (limit >> 0) & 0xffff;
  gdt->limit_high = (limit >> 16) & 0xf;
}

static const inline unsigned long gdt_get_base(const struct gdt_entry gdt)
{
  return (unsigned long)gdt.base_low | (gdt.base_middle << 16) |
    (gdt.base_high << 24);
}

static inline void gdt_set_base(struct gdt_entry *gdt,
				unsigned long base)
{
  gdt->base_low = (base >> 0) & 0xffff;
  gdt->base_middle = (base >> 16) & 0xff;
  gdt->base_high = (base >> 24) & 0xff;
}

#endif
