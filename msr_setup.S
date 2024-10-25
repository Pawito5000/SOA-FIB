#include <asm.h>
#include <segment.h>

ENTRY(writeMSR)
	push %ebp
	movl %esp, %ebp
	movl $0, %edx 
	movl 12(%ebp), %ecx
	movl 8(%ebp), %eax
	wrmsr
	movl %ebp, %esp
	pop %ebp
	ret	
