/*  _           _   
/*   | . |   | |_|  JIMP isr handlers
/*  \  | | | | |    09-jan-2005:	add LIRQ,HIRQ macroses and some redisign
/*  
/*/ 

.text
.globl	timer_handler, pagefault_handler, dispatch_irq

// Define for low IRQ
#define	LIRQ(x)     \
.align	4 ;         \
.globl	irq_##x ;   \
irq_##x: ;          \
push  $x ;          \
call  dispatch_irq ;\
movl  $0x20, %eax ; \
outb  %al, $0x20 ;  \
add   $4,%esp ;     \
iret
		
// Define for high IRQ
#define	HIRQ(x)     \
.align	4 ;         \
.globl	irq_##x ;   \
irq_##x: ;          \
push  $x ;          \
call  dispatch_irq ;\
movl  $0x20, %eax ; \
outb  %al, $0xA0 ;  \
outb  %al, $0x20 ;  \
add   $4,%esp ;     \
iret

// Define all default irq handlers here
LIRQ(0x01)
LIRQ(0x02)
LIRQ(0x03)
LIRQ(0x04)
LIRQ(0x05)
LIRQ(0x06)
LIRQ(0x07)
HIRQ(0x08)
HIRQ(0x09)
HIRQ(0x0a)
HIRQ(0x0b)
HIRQ(0x0c)
HIRQ(0x0d)
HIRQ(0x0e)
HIRQ(0x0f)

		
/** 
/* Define empty ISR here 
/*/
.align	4
.globl	empty_isr
empty_isr:
		iret

		
/**
/* Timer interrupt handler
/*/
.globl	timer_irq
.align	4
timer_irq:
		cli
		pusha

		movl    $0x10, %eax
		movw    %ax, %ds
		movw    %ax, %es
		movw    %ax, %gs

		call	timer_handler

/ 		movl    $0x20, %eax
/ 		outb    %al, $0x20

		popa
		sti
		iret


/** 
/* Pagefault interrupt trap
/*/ 
.globl	pagefault0E
.align	4
pagefault0E:
		cli
 		xchgl	%eax,(%esp)		/*  push eax
									eax <-- err code */

		pusha
		
	 /* Error code push onto stack */
		pushl   %eax
		movl    $0x10, %eax
		movw    %ax, %ds
		movw    %ax, %es
		movw    %ax, %gs
		

	  /* Call page fault handler:  pagefault_handler(uint32_t err); */
		call	pagefault_handler
		popl    %eax		

		movl    $0x20, %eax
		outb    %al, $0x20
		popa
	
		popl	%eax		 /* restore eax here */

		sti
		iret

		
/** 
/* Thread bootstrap function
/*/
.globl  thread_bootstrap
thread_bootstrap:
		iret                            /* here we go to 'task_code' */

		
/**
/* Context switch function
/*/
.globl  context_switch
context_switch:
		pushf
        pushl   %ebp
        pushl   %esi
        pushl   %edi
		pushl	%ebx

        movl    0x18(%esp,1), %eax      /* Where to save stack pointer */
        movl    %esp, (%eax)            /* Save old stack */
        movl    0x20(%esp,1), %eax      /* Get new PDBR */
        movl    0x1c(%esp,1), %ebx      /* Get new stack loc */
        cmpl    $0, %eax                /* Need to change addr. space? */
        je      changed_pdbr            /* If parameter was zero, no */
        movl    %eax, %cr3              /* Change address space */

changed_pdbr:   
		movl    %ebx, %esp              /* switch to new stack */

		popl	%ebx			        /* point 1 see 'kcreate_thread' function */
        popl    %edi                    /* point 2 */
        popl    %esi                    /* point 3 */
        popl    %ebp                    /* point 4 */
        popf                            /* point 5 */
			
		ret						        /* here we go to 'thread_bootstrap' */
