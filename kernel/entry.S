/* These are the low-level assembly stuff for interrupts */

.text
	
#include <kernel/linkage.h>

#define SAVE_ALL			\
	cld;				\
	pushl %es;			\
	pushl %ds;			\
	pushl %eax;			\
	pushl %ebp;			\
	pushl %edi;			\
	pushl %esi;			\
	pushl %edx;			\
	pushl %ecx;			\
	pushl %ebx;			\
	movl $0x18,%edx;	\
	movl %edx,%ds;			\
	movl %edx,%es;

#define RESTORE_ALL	\
	popl %ebx;	\
	popl %ecx;	\
	popl %edx;	\
	popl %esi;	\
	popl %edi;	\
	popl %ebp;	\
	popl %eax;	\
	popl %ds;	\
	popl %es;
	
.globl	do_unexpected_int, do_int3, current, schedule, do_block, _kernel_lock
.globl	handle_syscall

#define LOCK_KERNEL	\
	cli;		\
	incl	_kernel_lock

/* this is a fix to stop %eax from being clobbered. */
#define UNLOCK_KERNEL	\
	decl	_kernel_lock;	\
	jne	1f;	/* do we need to testl? */ \
	sti;		\
	orl	$0x20, 0x08(%esp); /* this is a big fat ugly hack (works!) */ \
	/* it removes the interrupt bit from the stored flags */ \
	1:

#define SAVE_REGS	\
	movl	(current), %ebp;	\
	pushl	0x18(%ebp);	\
	movl	%esp, 0x18(%ebp);	\
	addl    $0x4, 0x18(%ebp)

ENTRY(unexpected_int)
	pushl	$-1
	SAVE_ALL
	LOCK_KERNEL
	SAVE_REGS
	call	SYMBOL_NAME(do_unexpected_int)
	jmp	ret_from_intr

ENTRY(int3)
	pushl	$0			/* orig_eax */
	SAVE_ALL
	LOCK_KERNEL
	SAVE_REGS
	call	SYMBOL_NAME(do_int3)
	jmp	ret_from_intr

ENTRY(syscall)
	pushl	%eax
	SAVE_ALL
	LOCK_KERNEL
	SAVE_REGS
	call	SYMBOL_NAME(handle_syscall)
	jmp	ret_from_intr

ENTRY(block) /* a pretend int */
	xchg	%eax, 0x04(%esp) /* swap the addr into %eax */
	pushf	/* push the flags */
	push	%cs /* push the cs */
	call	block_lowlevel
	/* flags and cs popped by reti */
	xchg	%eax, 0x04(%esp)
	ret

ENTRY(block_lowlevel)
	pushl	%eax /* orig_eax */	
	SAVE_ALL
	LOCK_KERNEL
	pushl	%eax
	call	SYMBOL_NAME(do_block)
	addl	$4, %esp
	call	SYMBOL_NAME(schedule)
	RESTORE_ALL
	addl	$4, %esp		/* skip over orig_eax */
	UNLOCK_KERNEL
	iret

ENTRY(ret_from_intr)
	movl	(current), %ebp		/* store the old current in ebp */
	cmpl	$0, %ebp		/* if null never schedule */
	je	restore
	cmpl	$0, 0x20(%ebp)		/* check need_resched */
	je	restore
	call	SYMBOL_NAME(schedule)	/* schedule */
ENTRY(restore)
	popl	0x18(%ebp)
	RESTORE_ALL
	addl	$4, %esp		/* skip over orig_eax */
	UNLOCK_KERNEL
	iret

ENTRY(switch_to)
	xchg	%ebp, 0x04(%esp) /**/
	xchg	%ebx, 0x08(%esp) /**/
	cmpl	%ebp, %ebx		/* if same skip switch */
	je	1f /* we want to unswap %ebp, and %ebx right? */
	SAVE_ALL
	pushl	$1			/* flags for switch_to restore */
	movl	%esp, 0x10(%ebx)	/* save */
	movl	%ss, 0x14(%ebx)
	movl	0x10(%ebp), %esp	/* recall */
	movl	0x14(%ebp), %ss
	popl	%eax
	cmpl	$0, %eax
	jz	1f
	RESTORE_ALL
1:	xchg	%ebp, 0x04(%esp) /**/
	xchg	%ebx, 0x08(%esp) /**/
2:	ret

ENTRY(common_interrupt)
	SAVE_ALL
	LOCK_KERNEL
	SAVE_REGS
	call handle_irq
	jmp ret_from_intr

