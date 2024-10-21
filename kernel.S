#include <asm.h>
#include <segment.h>

ENTRY(task_switch)
	push %ebp
	mov %esp, %ebp
	
	push %esi
	push %edi
	push %ebx

	push 8(%ebp)

	call inner_task_switch

	addl $4, %esp
	pop %ebx
	pop %edi
	pop %esi

	movl %ebp, %esp
	pop %ebp
	ret

ENTRY(switch_stack)
	mov 4(%esp), %eax
	mov %ebp, (%eax)
	mov 8(%esp), %esp
	pop %ebp
	ret
