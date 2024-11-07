# 1 "kernel.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "kernel.S"
# 1 "include/asm.h" 1
# 2 "kernel.S" 2
# 1 "include/segment.h" 1
# 3 "kernel.S" 2

.globl task_switch; .type task_switch, @function; .align 0; task_switch:
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

.globl inner_switch; .type inner_switch, @function; .align 0; inner_switch:
 mov 4(%esp), %eax
 mov %ebp, (%eax)
 mov 8(%esp), %esp
 pop %ebp
 ret
