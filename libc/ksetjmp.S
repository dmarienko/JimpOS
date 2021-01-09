#define OFS_EBP   0
#define OFS_EBX   4
#define OFS_EDI   8
#define OFS_ESI   12
#define OFS_ESP   16
#define OFS_EIP   20

.text
		
.align  4		
.globl	ksetjmp
		
ksetjmp:
		pushl %ebp
		movl  %esp,%ebp
		movl 0x8(%ebp,1), %edx
		movl 0x4(%ebp,1), %eax
		movl %eax, OFS_EIP(%edx,1)

		movl %ebx, OFS_EBX(%edx,1)
		movl %edi, OFS_EDI(%edx,1)
		movl %esi, OFS_ESI(%edx,1)
		movl %ebp, OFS_ESP(%edx,1)
		
		movl (%esp,1),%eax
		movl %eax,OFS_EBP(%edx,1)
		
		xorl %eax,%eax
		popl %ebp
		ret		

		
.align  4		
.globl	klongjmp
		
klongjmp:
		pushl %ebp
		movl  %esp,%ebp
		movl 0x8(%ebp,1), %edx   /* jmp_buf env */
		movl 0xc(%ebp,1), %eax   /* ret value */
		
		
		movl OFS_EBX(%edx,1),%ebx
		movl OFS_EDI(%edx,1),%edi
		movl OFS_ESI(%edx,1),%esi

		
		movl OFS_ESP(%edx,1),%esp
		movl OFS_EBP(%edx,1),%ebp
		jmp *OFS_EIP(%edx)
