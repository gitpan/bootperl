/* second.S  -  second stage boot loader for `os'
 * Code adapted from LILO's second.S and Linus' bootsect.S & second.S
 * (The code has been adapted quite a bit for the user input bits.) */
	
.code16

/* some defines */
BOOTSEG   = 0x07c0	/* abs */
FIRSTSEG  = 0x8a00	/* seg */
STACKSEG  = 0x8000	/* seg */
STACK     = 0xb000
SECONDSEG = 0x8b00	/* seg */
MAP       = 0x2000
SYSSEG    = 0x1000	/* seg */

.text
.globl	_start			/* entry point */
_start:
	jmp	start1		/* skip header NB second byte of this
				   instruction acts as a header version tag
				   because it is the length of the following
				   header. */
	nop
	nop			/* room for 2 bytes */
/* header & boot params */
header:
sig:	.ascii	"BOOT"
stage:	.word	2		/* second stage loader */
check:	.word	0x1234		/* check */
version:.word	0x0001		/* version */
flags:	.byte	0		/* flags */
	.byte	0		/* padding */
mapaddr1:
	.word	0
	.word	0
	.byte	0
mapaddr2:			/* unused !!! */
	.word	0
	.word	0
	.byte	0
	
start1:

drain:	/* drain the type-ahead buffer */
	movw	$32, %cx
drkbd:	movb	$1, %ah		/* is a key pressed ? */
	int	$0x16
	jz	comcom		/* no -> done */
	xorb	%ah, %ah	/* get the key */
	int	$0x16
	loop	drkbd
	
comcom:	movb	$'3', %al	/* display a '3' */
	call	prtchr

/* patch disk parameter table in DS:SI (don't worry DS:SI is loaded below) */
	xorw	%ax, %ax	/* get pointer to table in DS:SI */
	movw	%ax, %ds
	lds	(0x78), %si
	cmpb	$9, 4(%si)	/* okay? */
	ja	dskok		/* yes -> do not patch */
	movw	%cs, %ax	/* get pointer to new area in ES:DI */
	movw	%ax, %es
	movw	$dskprm, %di
	movw	$6, %cx		/* copy 12 bytes */
	rep
	movsw
	movb	$18, %es:-8(%di) /* patch number of sectors */
	cli			/* paranoia */
	xorw	%ax, %ax	/* store new pointer */
	movw	%ax, %ds
	movw	$dskprm, (0x78)
	movw	%es, (0x7a)
	sti
dskok:	movb	$0, %cs:break	/* clear the break flag (why?) */
	jmp	restrt		/* get going */
	
restrt:
	movw	%cs, %ax	/* adjust segment registers */
	movw	%ax, %ds
	movw	%ax, %es
	movw	$STACK, %sp

verify:	/* verify our header. */
	cmpl	$0x544f4f42, (sig) /* BOOT in reverse */
	jne	crshbrn
	cmpw	$2, (stage)
	jne	crshbrn
	cmpw	$0x1234, (check)
	jne	crshbrn
	cmpw	$0x0001, (version)
	jne	crshbrn

/* load up the map file with kernel addresses */
loadmap:	
	movw	$MAP, %bx	/* where */
	movw	mapaddr1 + 0, %cx
	movw	mapaddr1 + 2, %dx
	movb	mapaddr1 + 4, %al
	call	cread		/* ignore nuls */
	movw	$MAP+512, %bx	/* where */
	movw	mapaddr2 + 0, %cx
	movw	mapaddr2 + 2, %dx
	movb	mapaddr2 + 4, %al
	call	cread		/* ignore nuls */

	movb	$'4', %al	/* this is it we're all setup for the kernel */
	call	prtchr		/* load and boot (finally) */
	movb	$'\n', %al	/* newline */
	call	prtchr
	movb	$'\r', %al
	call	prtchr
	movw	$readingmsg, %si /* `Reading kernel...' message */
	call	prtstr		/* dots printed later (1 per sector) */

/* The cool part! This loads the kernel high at 1 meg */
cool:	movw	$gdt_bios, %bx	/* load address of GDT */
	call	lfile		/* load the kernel (addresses from MAP) */
	call	kill_motor	/* turn off floppy motor */
	call	video		/* initialise our video structures */
	call	memsize		/* work out amount of RAM */
	jmp	launch		/* GO! GO! GO!!!! */
	
/* crash and burn */
crshbrn:
	movb	$'?', %al
	call	prtchr
	cli
die:	hlt
	jmp	die		/* try and use fewer cycles using hlt */

lfile:	call	load		/* trust me although this _does_ look */
	jmp	lfile		/* brain-damaged (NB copied from LILO */
				/* ie. don't blame me) */

moff:	.word	0
tempal:	.byte	0
load:	push	%es		/* save ES:BX */
	push	%bx
lfetch:	movw	moff, %si
	movw	MAP+0(%si), %cx	/* get address */
	movw	MAP+2(%si), %dx
	movb	MAP+4(%si), %al
	orw	%cx, %cx	/* at EOF ? */
	jnz	noteof		/* no -> go on */
	orw	%dx, %dx
	jnz	noteof
	popw	%bx		/* restore ES:BX */
	popw	%es
	popw	%ax		/* pop return address */
	ret			/* return to outer function */
noteof:	add	$5, %si		/* increment pointer */
	movw	%si, moff	/* and store */
	pushw	%ax
	pushw	%bx
	movb	$'.', %al
	call	prtchr
	popw	%bx
	popw	%ax
	/* XXXXXXXX intentional fall thru  XXXXXXXX*/
	
/* Load one sector */
doload:	popw	%bx		/* return ES:BX */
	popw	%es
/* Load a sequence of sectors and move them info "high memory" */
rdhigh:	pushw	%bx		/* okay - DS:BX points to GDT */
	movw	$SYSSEG, %bx	/* adjust ES:BX */
	movw	%bx, %es
	xorw	%bx, %bx
	call	sread		/* load the sector(s) */
	movb	%al, tempal
	popw	%bx		/* get pointer to GDT */
	pushw	%ax		/* just in case ... */
	pushw	%cx
	pushw	%si
	movw	%bx, %si	/* turn ES:SI into pointer to GDT */
	movw	%ds, %ax
	movw	%ax, %es
	xorw	%cx, %cx	/* number of words to move */
/**/	movb	tempal, %ch
	pushw	%bx		/* do the transfer (the save is paranoia) */
	pushw	%cx
	pushw	%si
	movb	$0x87, %ah
	int	$0x15
	popw	%si
	popw	%cx
	popw	%bx
	jc	badmov		/* failed ... */
/**/	addw	%cx, %es:0x1a(%si) /* modify the GDT */
	adcb	$0, %es:0x1c(%si)
	addw	%cx, %es:0x1a(%si)
	adcb	$0, %es:0x1c(%si)
	subw	%ax,%ax		/* put ES back to 0 */
	movw	%ax, %es
	popw	%si
	popw	%cx
	popw	%ax
	ret			/* done */

badmov:	pushw	%ax		/* save the error code */
	movw	$msg_bm, %si	/* tell the user ... */
	call	prtstr		/* (standard procedure) */
	jmp	crshbrn		/* die */

/* Load a sequence of sectors */
sread:	pushw	%bx		/* save registers */
	pushw	%cx
	pushw	%dx
	call	cread
	movw	%ax, %di	/* save AL return count */
	jc	rerror		/* error -> complain */
	popw	%dx		/* restore registers */
	popw	%cx
rokay:	popw	%bx
	shlw	$8, %ax		/* convert sectors to bytes */
	addb	%ah, %ah
	jc	dowrap		/* loaded an entire segment -> advance ES */
	addw	%ax, %bx	/* move BX */
	jnc	nowrap		/* same segment -> go on */
dowrap:	movw	%es, %ax	/* move ES */
	addw	$0x1000, %ax
	movw	%ax, %es
nowrap:	movw	%di, %ax	/* restore the block count in AL */
aret:	ret			/* done */

/* Read error - try a second time and give up if that fails too */
rerror:	pushw	%ax
	pushw	%si
	movw	$msg_re, %si	/* say something */
reset:	call	prtstr
	popw	%ax		/* display error code */
	pushw	%ax
	movb	%ah, %al
	call	bout
	popw	%ax
	call	bout
	jmp	crshbrn

bout:	pushw	%ax		/* save byte */
	shrb	$4, %al		/* display upper nibble */
	call	nout
	popw	%ax
nout:	andb	$15, %al
	addb	$48, %al
	cmpb	$58, %al
	jb	nokay
	addb	$7, %al
nokay:	jmp	prtchr

/* Start the kernel */
launch:	pushw	%es		/* save ES:BX (why ???) */
	pushw	%bx
	movb	$'\n', %al	/* display a CRLF */
	call	prtchr
	movb	$'\r', %al
	call	prtchr
	movw	$0x3f2, %dx	/* stop the floppy motor */
	xorb	%al, %al
	outb	%al, %dx
	xorw	%ax, %ax	/* reset the FDC */
	movb	%al, %dl
	int	$0x13
/* the following code readies us for protected mode */
ready_protected:
	cli			/* turn off ints (try not to use stack) */
	lidt	idt_48		/* load idt with 0,0 */
	lgdt	gdt_48		/* load gdt */

/* next evil thing is to enable a20 */
enable_a20:	
	call	empty_8042
	movb	$0xd1, %al	/* command write */
	outb	%al, $0x64
	call	empty_8042
	movb	$0xdf, %al	/* a20 on */
	outb	%al, $0x60
	call	empty_8042
	/* the other bits must be preserved here */
	inb	$0x92, %al
	orb	$02, %al	/* "fast a20" version */
	outb	%al, $0x92	/* some chips have only this */
	/* wait until a20 really *is* enabled. This uses mem at 0x200
	 * (int 0x80 vector */
	xorw	%ax, %ax	/* segment 0x0000 */
	movw	%ax, %fs
	decw	%ax
	movw	%ax, %gs	/* segment 0xffff (HMA) */
a20_wait:
	incw	%ax		/* unused memory location <0xfff0 */
	movw    %ax, %fs:(0x200) /* we use the "int 0x80" vector */
	cmpw    %gs:(0x210), %ax /* and its corresponding HMA addr */
	je      a20_wait	/* loop until no longer aliased */

/* make sure any possible coprocessor is properly reset... */
coproc:	xorw	%ax, %ax
	outb	%al, $0xf0
	call	delay
	outb	%al, $0xf1
	call	delay

/* now mask all interrupts - the rest is done later */
mask_irq:
	movb	$0xff, %al	/* mask all interrupts for now */
	outb	%al, $0xa1
	call	delay
	movb	$0xfb, %al	/* mask all irqs but irq2 which is cascaded */
	outb	%al, $0x21

/* This is it. Enable the PE flag */
go_protected:
	movl	%cr0, %eax
	orl	$0x1, %eax	/* add in the PE flag */
	movl	%eax, %cr0	/* This is it!!! */
	/* Long jump to the next insruction */
	.byte	0x66, 0xea	/* this uses an unsupported prefix */
	.long	flush_instr + 0x8b000	/* addr */
	.word	0x10		/* seg */
flush_instr:			/* normalisation follows */
.code32 /* this bit of code is 32bit */
	movl	$0x18, %eax	/* load our new data seg (all segs) */
	movl	%eax, %ds
	movl	%eax, %es	/* this is used by some string ops */
	movl	%eax, %ss
	movl	%eax, %fs
	movl	%eax, %gs
	movl	$STACKSEG * 0x10 + STACK, %esp /* load stack pointer */
	/* TODO: verify ELF header */
	/* long/far call not req. because already on right seg see above */
	/* this is a call near, absolute indirect */
	call	*(0x100018)	/* this jumps to the start of the code */
die32:	hlt
	jmp	die32
		
.code16	/* this is all 16 bit */

/* read code */

/* Sector read */
.globl	cread
cread:
	orw	%cx, %cx
	jz	cread_done
	pushw	%ax		/* save the count */
	movb	$2, %ah		/* read command */
	call	dsk_do_rw	/* int 0x13 with retries */
	pop	%cx		/* carry set means error on read */
	mov	%cl, %al	/* count in AL, error code in AH */
cread_done:	
	ret

bsmith_counter:	.byte	194
bsmith_diemsg:	.string	" KERNEL BIGGER THAN THOUGHT"
bsmith_die:
	mov	$bsmith_diemsg, %si
	call	prtstr
	jmp die
bsmith_dsk_debug:
	pushf
	pushw	%ax
	pushw	%bx
	pushw	%cx
	pushw	%dx
	pushw	%bp
	pushw	%si
	pushw	%di
	movw	%sp, %bp

	movb	$'[', %al
	call	prtchr

	movw	12(%bp), %ax	/* ax */
	call	bsmith_prtax
	movw	10(%bp), %ax	/* bx */
	call	bsmith_prtax
	movw	8(%bp), %ax	/* cx */
	call	bsmith_prtax
	movw	6(%bp), %ax	/* dx */
	call	bsmith_prtax

	movb	$']', %al
	call	prtchr

	decb	bsmith_counter
	jz	bsmith_die
	
	popw	%di
	popw	%si
	popw	%bp
	popw	%dx
	popw	%cx
	popw	%bx
	popw	%ax
	popf
	ret

bsmith_prtax:
	pushw	%ax
	xchgb	%al, %ah
	call	bout
	popw	%ax
	call	bout
	ret

/* actual do read/write with retries: function passed in %ah */
dsk_do_rw:
	call	bsmith_dsk_debug
dsk_do_int13:
	pushw	%bp
	movw	$5, %bp		/* number of tries */
dsk_do_int13a:
	pushw	%ax
	int	$0x13
	jnc	dsk_io_exit
	decw	%bp		/* does not affect the carry */
	jz	dsk_io_exit
	xorw	%ax, %ax	/* reset disk controllers */
	int	$0x13
	popw	%ax
	decw	%bp
	jmp	dsk_do_int13a
dsk_io_exit:	
	movw	%sp, %bp	/* do not touch any flags */
	lea	6(%bp), %sp	/* an ADD would touch flags */
	popw	%bp		/* do not touch any flags */
	ret

VIDEOSEG        = 0x8e00
VID_CUR_POS     = 0x0
VID_PAGE        = 0x2
VID_MODE        = 0x4
VID_COLS        = 0x5
VID_LINES	= 0x6
VID_FONT_POINTS = 0x8
VID_MAGIC	= 0xA

/* video mode handling */
.globl	video
video:
	pushw	%fs		/* save fs & gs */
	pushw	%gs
	xorw	%ax, %ax	/* gs is at 0x0 */
	movw	%ax, %gs
	movw	$VIDEOSEG, %ax	/* fs is the data to pass to protected mode */
	movw	%ax, %fs
	
	movb	$0x03, %ah	/* read cursor pos */
	xorb	%bh, %bh
	int	$0x10
	movw	%dx, %fs:(VID_CUR_POS)

	movb	$0x0f, %ah	/* read page/mode/width */
	int	$0x10
	movw	%bx, %fs:(VID_PAGE)
	movw	%ax, %fs:(VID_MODE)

	movw	%gs:(0x485), %ax /* font size */
	movw	%ax, %fs:(VID_FONT_POINTS)

	movb	%gs:(0x484), %al /* get lines (EGA+ BIOS) */
	incb	%al
	movb	%al, %fs:(VID_LINES)

	movw	$0x1234, %fs:(VID_MAGIC)

	popw	%gs
	popw	%fs
	ret

MEMSEG      = 0x8e02
MEM_SIZE    = 0x0	/* The 32bit mem size */
MEM_SIZEOLD = 0x4	/* The 16bit mem size */
	
/* memory size calculation */
.globl	memsize
memsize:
	pushw	%fs		/* save fs */
	movw	$MEMSEG, %ax	/* fs is the data to pass to protected mode */
	movw	%ax, %fs

	xorl	%ebx, %ebx
	movl	%ebx, %fs:(MEM_SIZE)
	movw	$0xe801, %ax
	int	$0x15
	jc	oldstylemem

	andl	$0xffff, %ebx
	shll	$6, %ebx
	movl	%ebx, %fs:(MEM_SIZE)

	andl	$0xffff, %eax
	andl	%eax, %fs:(MEM_SIZE)

oldstylemem:
	movb	$0x88, %ah
	int	$0x15
	movw	%ax, %fs:(MEM_SIZEOLD)

	popw	%fs
	ret

/* This turns off the floppy motor so that the next stage is entered in
 * a known state */
kill_motor:
	movw    $0x3f2, %dx
	xorb    %al, %al
	outb    %al, %dx
	ret
	
/* library code */

/* Routine to print asciiz string at ds:si */
.globl	prtstr
prtstr:	
	lodsb
	andb	%al, %al
	jz	fin
	cmpb	$10, %al
	jne	nonl
	movb	$13, %al
	call	prtchr
	movb	$10, %al
nonl:	cmpb	$12, %al
	jne	nocls
	movb	$0x0f, %ah
	int	$0x10
	xorb	%ah, %ah
	int	$0x10
	jmp	prtstr
nocls:	call	prtchr
	jmp	prtstr
fin:	ret
	
/* Part of prtspc, this just prints ascii al */
.globl	prtchr
prtchr:	xorb	%bh, %bh
	movb	$0x0e,  %ah
	int	$0x10
	ret

/* this checks that the keyboard command queue is empty */
empty_8042:
	pushl	%ecx
	movl	$100000, %ecx

empty_8042_loop:
	decl    %ecx
	jz      empty_8042_end_loop

	call    delay

	inb     $0x64, %al	/* 8042 status port */
	testb   $1, %al		/* output buffer? */
	jz      no_output

	call    delay
	inb     $0x60, %al	/* read it */
	jmp     empty_8042_loop

no_output:
	testb   $2, %al		/* is input buffer full? */
	jnz     empty_8042_loop	/* yes - loop */
empty_8042_end_loop:
	popl    %ecx
	ret

/* delay is needed after doing I/O */
.globl	delay
delay:	outb	%al, $0x80
	ret

end_of_code:

data:

break:	.byte	0
dskprm:	.word	0,0,0,0,0,0

/* strings */
readingmsg:
	.string	"Reading kernel"
msg_bm:	.string	"\nBlock move error 0x"
msg_re:	.string	"\nError 0x"

/* GDT for "high" loading */
gdt_bios:
	/* space for BIOS */
	.space	0x10
	/* source */
	.word	0xffff		/* no limits */
	.word	0x0000		/* start: 0x10000 */
	.byte	0x01
	.byte	0x93		/* permissions */
	.word	0		/* padding for 80286 mode :-( */
	/* destination (might also be used for starting kernel) */
	.word	0xffff		/* no limits */
	.word	0x0000		/* start: 0x100000 (1-MByte) */
	.byte	0x10
	.byte	0x93		/* permissions */
	.word	0		/* padding for 80286 mode :-( */
	/* space for BIOS */
	.space	0x10
/* real GDT for kernel startup */
gdt:
	/* space for BIOS */
	.space	0x10
	/* kernel code */
	.word   0xFFFF		/* 4Gb - (0x100000*0x1000 = 4Gb) */
	.word   0		/* base address = 0 */
	.word   0x9A00		/* code read/exec */
	.word   0x00CF		/* granularity = 4096, 386 */
	                        /*  (+5th nibble of limit) */
	/* kernel data */
	.word   0xFFFF		/* 4Gb - (0x100000*0x1000 = 4Gb) */
	.word   0		/* base address = 0 */
	.word   0x9200		/* data read/write */
	.word   0x00CF		/* granularity = 4096, 386 */
                                /*  (+5th nibble of limit) */
	/* space for BIOS */
	.space	0x10
idt_48:	/* pointer thing to idt */
	.word	0		/* idt limit = 0 */
	.long	0		/* idt  base = 0L */
gdt_48:	/* pointer thing to gdt */
	.word	64		/* gdt limit = 2048, 256 GDT entries */
	.long	gdt + 0x8b000	/* gdt base (to be filled in) */
	
end_of_data:
