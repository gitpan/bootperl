/* This is start.S. The ORIGINAL work of bsmith to add multiboot
	capability to xos on 22/9/2001. I wrote this myself!! (cynical
	mood here people) */

#include <kernel/linkage.h>
#include <kernel/multiboot.h>

/* I debated putting this code in the .init section but that would put
	the start up code too high up the image for my likeing. Plus I
	would have to separate the multiboot header from it to get it
	close to beginning. */

ENTRY(STACKTOP)
	.long	0x67FFF /* stack grows down from 416K-1 */
	
ENTRY(_start)
	/* a proper GDT is loaded later (Rule 1: minimise assembly) */
	
	/* load the GDT now since I can't be bother to hack some C to
	do it later */
	lgdt	gdt_48

        /* Long jump to the next insruction to load the code seg */
	ljmp	$0x10, $flush_instr
flush_instr:                    /* normalisation follows */
	movl    $0x18, %ecx     /* load our new data seg (all segs) */
	movl    %ecx, %ds	/* using ecx because eax & ebx already used */
	movl    %ecx, %es       /* this is used by some string ops */
	movl    %ecx, %ss
	movl    %ecx, %fs
	movl    %ecx, %gs
	movl	STACKTOP, %esp	/* load the esp. currently the stack
				   starts at the top of the lower 416K
				   of memory, the same as grub's
				   protected mode stack */

	pushl	$0			/* reset eflags */
	popf

	/* Push the pointer to the Multiboot information structure. */
        pushl   %ebx
        /* Push the magic value.  */
        pushl   %eax

	pushl	$0			/* if we try and return we auto-
					   matically panic. smart. */
	jmp	SYMBOL_NAME(start_kernel)
	/* there are no returns past here luckily; see above */
	
/* Documentation is taken from the Multiboot Specification */
ENTRY(multiboot_header)
		/* Offset  Type  Field Name    Note */
		/* 0       u32   magic         required */
	.long	MULTIBOOT_HEADER_MAGIC
		/* 4       u32   flags         required */
	.long	MULTIBOOT_HEADER_FLAGS
		/* 8       u32   checksum      required */
	.long	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
		/* 12      u32   header_addr   if flags[16] is set */
	.long	0 /* not needed we are elf */
		/* 16      u32   load_addr     if flags[16] is set */
	.long	0 /* not needed we are elf */
		/* 20      u32   load_end_addr if flags[16] is set */
	.long	0 /* not needed we are elf */
		/* 24      u32   bss_end_addr  if flags[16] is set */
	.long	0 /* not needed we are elf */
		/* 28      u32   entry_addr    if flags[16] is set */
	.long	0 /* not needed we are elf */
		/* 32      u32   mode_type     if flags[2] is set */
	.long	0
		/* 36      u32   width         if flags[2] is set */
	.long	0
		/* 40      u32   height        if flags[2] is set */
	.long	0
		/* 44      u32   depth         if flags[2] is set */
	.long	0

	/* real GDT for kernel startup */
ENTRY(gdt)
        /* space for BIOS */
        .space  0x10
        /* kernel code */
        .word   0xFFFF          /* 4Gb - (0x100000*0x1000 = 4Gb) */
        .word   0               /* base address = 0 */
        .word   0x9A00          /* code read/exec */
        .word   0x00CF          /* granularity = 4096, 386 */
                                /*  (+5th nibble of limit) */
        /* kernel data */
        .word   0xFFFF          /* 4Gb - (0x100000*0x1000 = 4Gb) */
        .word   0               /* base address = 0 */
        .word   0x9200          /* data read/write */
        .word   0x00CF          /* granularity = 4096, 386 */
                                /*  (+5th nibble of limit) */
        /* space for BIOS */

gdt_48: /* pointer thing to gdt */
        .word	64		/* gdt limit = 2048, 256 GDT entries */
        .long	SYMBOL_NAME(gdt)/* gdt base (to be filled in) */
	
