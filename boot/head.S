/*
  _           _    JIMP start module
   | . |   | |_|   * 11/12/2001  - realised with paging memory  
  \  | | | | |     * 26-aug-2002 - rewriten for GNU asm
                   * 19-nov-2002 - added receiving memory size from boot module 
                   * 24-nov-2003 - removed static memory for kernel page/tab dirs
*/

.text

/*  
 * Extern area  
*/
.globl	__mkernel_main	
.extern end, m_start, bios_m;

/* 
 * Global area
*/
.globl	start32

/*  Starting code here */
start32:
	movl	$0x10,%eax
	movw	%ax,%ss
	movw	%ax,%ds
	movw	%ax,%es	
	movw	%ax,%fs
	movw	%ax,%gs
	movl	$_kernel_stack_top,%esp
			
  /* Store memory size here */
    movl	%ecx, bios_m
  
  /* Store memory start here */
	movl	$end, %ecx
	movl	%ecx, m_start

   /* Go to main procedure */
	call	__mkernel_main

   /* Infinity loop here */	
L1:
	jmp		L1

/* Kernel stack */
.bss
_kernel_stack:			
	.fill	0x400,1,0
_kernel_stack_top:

