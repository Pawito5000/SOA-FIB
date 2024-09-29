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
	
	push $retorn	

	#dynamic link??
	push %ebp
	movl %esp, %ebp

	SYSENTER

retorn:
	pop %ebp

	addl $4, %esp
	pop %edx
	pop %ecx

	cmpl $0, %eax
	jge fin
	negl %eax
	movl %eax, errno
	movl $-1, %eax
fin:
	movl %ebp, %esp
        pop %ebp

	ret
