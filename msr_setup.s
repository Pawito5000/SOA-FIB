# 0 "msr_setup.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "msr_setup.S"
# 1 "include/asm.h" 1
# 2 "msr_setup.S" 2
# 1 "include/segment.h" 1
# 3 "msr_setup.S" 2

.globl writeMSR; .type writeMSR, @function; .align 0; writeMSR:
 push %ebp
 movl %esp, %ebp
 movl 0, %edx
 movl 12(%ebp), %ecx
 movl 8(%ebp), %eax
 wrmsr
 movl %ebp, %esp
 pop %ebp
 ret
