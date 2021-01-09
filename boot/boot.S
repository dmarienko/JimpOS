/
/  _           _   JIMP boot loader 
/   | . |   | |_|  * 27/08/2001 - started
/  \  | | | | |    * 29/09/2001 - fixed read_sect bug
/                  * 05-nov-2002 - simplify in the GDT setuping part
/                  * 19-nov-2002 - added get_all_mem function for obtaining memory size (int15)
/                  * 25-nov-2002 - kernel replaced to 0x10000 in the RAM ( was at 0x0 )
/                                  previouse release: cvs co -r jimp-0x0000 JimpOS_NTO
/                  * 25-nov-2002 - kernel replaced to 1 MB in the RAM ( was at 65536 ;)
/                  * 23-dec-2002 - rewriten on GNU asm

.code16
.text

JIMP_SIZE		= 4*4096
BOOTSEG			= 0x07c0
INITSEG			= 0x9000
SYSSEG			= 0x1000
ENDSEG			= SYSSEG+JIMP_SIZE
SECTORS			= 18 
KERNEL_START	= 0x100000

/ - - - - - - - - - - -
/ move code to INITSEG
	.global _start
_start:
	movw	$BOOTSEG,%ax
	movw	%ax,%ds
	movw	%ax,%dx
	movw	$INITSEG,%ax
	movw	%ax, %es
	movw	$0x200,%cx
	xorw	%si,%si
	xorw	%di,%di
	rep     movsw

/ Go to the start code
	ljmp	$INITSEG,$go

go:
	movw	%cs,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%ss
	movw	$0x400,%sp
	
/ Show startup message
	movw	$msg,%bp
	movw	$(msg_end - msg),%cx
	call	print_msg	

/ Read head.img
	movw	$SYSSEG,%ax
	movw	%ax,%es
	call	read_sect
	call	kill_motor

/ Try to get memory size	
	call	get_all_mem
	
/ - - - - - - - - - - - - - - - - - - -
/ Now try move to protected mode ...
start_pm:
	
   / ebx <- base physical address
	movl	$INITSEG<<4,%eax 
	
   / loading pointer to GDT  
	addl	$gdtr,%eax
    movl	%eax,(gdt_ptr+2)

   / disable interrupts
	cli	

   / Load gdt
	lgdt	gdt_ptr
	
   / Open A20 gate
enable_a20:
	call	wait8042
	movb	$0xd1, %al
	outb	%al, $0x64
	call	wait8042
	movb	$0xdf, %al
	outb	%al, $0x60
	call	wait8042
	
   / Wait
	movw	$0x1ff, %cx
wait1:
	nop
	loop	wait1
	
/ Try switch to unreal mode ( for copy kernel up to 1 MB )
	movl	%cr0, %eax
	orl		$1, %eax
	movl	%eax, %cr0
	
/ Store ds here
	pushw	%ds
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	
/ Back to (un)real mode 	
	movl	%cr0, %eax
	andb	$0xfe, %al
	movl	%eax, %cr0
	nop

/ Copying kernel at 1 MB
	cld				
	xorl	%eax,%eax
	movw	%ax,%es
	movw	%ax,%ds
	movl	$SYSSEG<<4,%esi
	movl	$KERNEL_START, %edi
	movl	$JIMP_SIZE,%ecx
	.byte  0x67
	rep		movsb	
	
/ Go to protected mode ... 
/ Remember ds here (for accessing to mem_size)
	popw	%ds
	movl	%cr0, %eax
	orl		$1, %eax
	movl	%eax, %cr0

/ Interface with head.S:
/ ecx  <- memory size from bios (in Kb)
	movl	(mem_size), %ecx
   .byte 0x66, 0xea				# prefix + jmpi-opcode
code32: .long   KERNEL_START	# will be set to 0x100000 for kernel
	    .word   0x18
		
/ - - - - - - - - - - - - - - - - - - - 
/ Getting memory above 1MB (bios int 15)
/  
get_all_mem:
	stc					
	xorl	%ecx,%ecx		
	xorl	%edx,%edx		
	movl	%edx,(mem_size)
						
	movw    $0xe801,%ax 
	int		$0x15
	jc		mem88

	cmpw	$0x0,%cx
	jne		e801cxdx	
	cmpw	$0x0,%dx
	jne		e801cxdx
	movw	%ax,%cx
	movw	%bx,%dx

e801cxdx:
	andw	$0xffff,%dx
	shlw	$0x6,%dx
	movw	%dx,(mem_size)
	andw	$0xffff, %cx
	addw	%cx,(mem_size)
	
mem88:
	movb	$0x88, %ah
	int		$0x15
	addw	%ax, (mem_size)
	ret


/ - - - - - - - - - - - - - - -
/ Reading sectors (for FDD)
/ es <- dest. segment
read_sect:
	xorw	%bx,%bx

read_sect2:
  / get cylinder
	movw	(cyl),%cx
	movb	%cl,%ah
	shrw	$2,%cx
	andw	$0xc0,%cx
	movb	%ah,%ch
	
  / get sector
	orw		(sect),%cx

  / get head
	movb	(head),%dh

/ - - - - - - - - - - - - - - -
	movb	$0,%dl
	movb	$1,%al
	movb	$2,%ah
	int		$0x13
	jc		_exit		
/ - - - - - - - - - - - - - - - 
	movw	(sect),%ax
	cmpw	$SECTORS,%ax
	jnz		_not_18

  / inc head
	incb	(head)
	andb	$0x1,(head)
	jnz		_not_inc_cyl

	incw	(cyl)
	
_not_inc_cyl:
	xorw	%ax,%ax

_not_18:
 	incw	%ax
 	movw	%ax,(sect)
		
  / print dot
	pushw	%ax
	movb	$'.',%al
	movb	$0x0E,%ah
	int		$0x10
	popw	%ax

	addw	$0x200,%bx
	cmpw	$JIMP_SIZE,%bx
	jae		_exit

	jmp		read_sect2

_exit:
	ret
	
/ Stop floppy motor
kill_motor:
	pushw	%dx
	movw	$0x3f2,%dx
	movb	$0,%al
	outb	%al,%dx
	popw	%dx
	ret
	
/ Printing message
print_msg:	
	pushw	%cx
	movb	$0x3,%ah
	xorb	%bh,%bh
	int		$0x10
	popw	%cx

	movw	$0x0002,%bx
	movw	$0x1301,%ax
	int		$0x10
	ret
	
wait8042:
	inb		$0x64,%al
	testb	$2,%al
	jnz		wait8042
	ret

gdtr:
	gdt_0:  .short 0, 0, 0, 0
				  
	gdt_gdt: 
			.short gdt_end - gdtr - 1
			.short 0x0000
			.short 0x9200 
			.short 0x00CF
				  
	gdt_ds:	 
			.short 0xFFFF 
			.short 0x0000
			.short 0x9200
			.short 0x00CF
				  
	gdt_cs:	
			.short 0xFFFF
			.short 0x0000
			.short 0x9A00
			.short 0x00CF
gdt_end:

gdt_ptr:	.short  gdt_end - gdtr - 1			
			.long  gdtr                

msg:		.ascii	"Jimp loader v0.3.1"
			.byte	13,10
msg_end:				

mem_size:
cyl:		.short	0
head:		.byte	0
sect:		.short	2

