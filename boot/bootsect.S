/* bootsect.S  -  first stage boot loader for `os'
 * Code adapted from LILO's first.S and Linus' bootsect.S
 * adapted = copied (almost verbatim, I removed LBA support from the
 * LILO code) */
	
.code16

/* some defines */
BOOTSEG   = 0x07c0	/* abs */
FIRSTSEG  = 0x8a00	/* seg */
STACKSEG  = 0x8000	/* seg */
STACK     = 0xb000
SECOND    = 0x1000	/* load second 1000h bytes above first seg (8b000h) */
SECONDSEG = 0x8b00	/* seg */
MAXADDR   = 10
HALT      = 1

.text
.globl	_start			/* entry point */
_start:
	cli			/* put interrupts in a known state */
	cld			/* clear direction (upwards). this uses 1
				   byte as padding */
	jmp	start1		/* skip header NB second byte of this
				   instruction acts as a header version tag
				   because it is the length of the following
				   header. */

/* header & boot params */
header:	.ascii	"BOOT"
	.word	1		/* first stage loader */
	.word	0x1234		/* check */
	.word	0x0001		/* version */
flags:	.byte	HALT		/* flags */
	.byte	0		/* padding */
d_addr:	.fill	MAXADDR, 5	/* these are the addresses of the secondary */
	.long	0		/* end of array */
	
start1:	/* task 1: move us to safety */
	movw	$BOOTSEG, %ax	/* set DS to this code seg */
	movw	%ax, %ds
	movw	$FIRSTSEG, %ax	/* where to copy to (in ES) */
	movw	%ax, %es	/* NB. load a lower address for a large EBDA */
	movw	$256, %cx	/* copy 256 words (512B) */
	subw	%si, %si	/* clear si & di */
	subw	%di, %di
	/* cld performed in header */
	rep
	movsw			/* move string from ds:si to es:di in words */
	ljmp	$FIRSTSEG, $start2	/* jump to our new code */

start2:				/* cli still in effect */
	movw	%ax, %ds	/* setup new ds and ss (es?) (ax still set) */
	movw	$STACKSEG, %ax
	movw	%ax, %ss
	movw	$STACK, %sp
	sti			/* safe now */

	movb	$0x0d, %al	/* gimme a CR ... */
	call	prtchr
	movb	$0x0a, %al	/* ... an LF ... */
	call	prtchr
	movb	$'1', %al	/* ... a '1' ... */
	call	prtchr

	/* next task is to load the second stage loader */
lagain:	movw	$d_addr, %si	/* load si with d_addr location */
	movw	$SECOND, %bx	/* how much above us to load second stage */
				/* cld still in effect */

sload:	lodsw			/* get CX (gets word into ax & swaps) */
	xchgw	%ax, %cx
	lodsw			/* get DX (gets word into ax & movs) */
	movw	%ax, %dx
	orw	%cx, %ax	/* EOF? */
	jz	done		/* yes -> start it */
				/* in LILO first.S it does some LBA stuff
				   I don't do this */
	lodsb			/* load final value for al */
	movb	$2, %ah		/* read command */
	int	$0x13		/* BIOS read */
	jc	error		/* error -> start over again */
	addb	$2, %bh		/* next sector */
	jmp	sload

done:	movb	$'2', %al	/* display a '2' */
	call	prtchr

/* This turns off the floppy motor so that the next stage is entered in
 * a known state (wrong!!!!) */
/*kill_motor:
        movw    $0x3f2, %dx
        xorb    %al, %al
        outb    %al, %dx */

checkflags:
	movb	(flags), %al
	testb	$HALT, %al
	jnz	halt
	ljmp	$SECONDSEG, $0	/* the jump */

halt:
	movb	$'h', %al
	call	prtchr
	cli
die:	hlt
	jmp	die

/* handle error in loading */
error:	movb	$32, %al	/* display a space */
	call	prtchr
	call	bout		/* display error code in AH */
	xorw	%ax, %ax	/* reset FDC */
	int	$0x13
	jmp	lagain		/* redo from start */

/* library code */

/* Routine to print asciiz string at ds:si */
/*.globl	prtstr
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
fin:	ret*/

/* Part of prtspc, this just prints ascii al */
.globl	prtchr
prtchr:	xorb	%bh, %bh
	movb	$0x0e,  %ah
	int	$0x10
	ret

/* display the error code in ah */
.globl	bout
bout:	rolw	$4, %ax		/* bring hi-nibble to position */
	call	nout
	rolw	$4, %ax		/* bring lo-nibble to position */
nout:	andb	$15, %al	/* display one nibble */
	addb	$48, %al
	cmpb	$58, %al
	jb	nokay
	addb	$7, %al
nokay:	pushw	%ax		/* does not affect AX */
	xorb	%bh, %bh	/* display on screen */
	movb	$14, %ah
	int	$0x10
	popw	%ax
	ret

end_of_code:

.org 510
	.word	0xAA55		/* boot signature (read the group) */
