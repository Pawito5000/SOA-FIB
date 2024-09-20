#include <asm.h>

ENTRY(addASM)
	push %ebp
	movl %esp, %ebp
	movl %eax, 8(%ebp)
	addl 12(%ebp), %eax  
	movl %ebp, %esp
	pop %ebp
	ret
