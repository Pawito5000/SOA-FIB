#include <asm.h>
#include <segment.h>

ENTRY(write)
	push %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %ebx

	movl $4, %eax
	push %ecx
	push %edx
	
	push retorn	

	SYSENTER

retorn:
	pop %edx
	pop %ecx
	pop %eax

	movl %ebp, %esp
	pop %ebp
	ret
