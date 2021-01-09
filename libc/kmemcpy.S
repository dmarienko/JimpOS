		.file "kmemcpy.S"
		.align 4
		.globl kmemcpy
kmemcpy:
		pushl	%ebp
		movl	%esp,%ebp
		pushl	%esi
		pushl	%edi
		movl	8(%ebp),%edi
		movl	12(%ebp),%esi
		movl	16(%ebp),%ecx
		cld
		rep movsb
		popl	%edi
		popl	%esi
		movl	8(%ebp),%eax
		leave
		ret
