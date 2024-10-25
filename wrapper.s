# 0 "wrapper.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "wrapper.S"
# 1 "include/asm.h" 1
# 2 "wrapper.S" 2
# 1 "include/segment.h" 1
# 3 "wrapper.S" 2


.globl write; .type write, @function; .align 0; write:
 push %ebp
 movl %esp, %ebp

 movl 8(%ebp), %edx
 movl 12(%ebp), %ecx
 movl 16(%ebp), %ebx

 movl $4, %eax
 push %ecx
 push %edx

 push $retorn_w

 #dynamic link??
 push %ebp
 movl %esp, %ebp

 SYSENTER

retorn_w:
 pop %ebp

 addl $4, %esp
 pop %edx
 pop %ecx

 cmpl $0, %eax
 jge fin_w
 negl %eax
 movl %eax, errno
 movl $-1, %eax
fin_w:
 movl %ebp, %esp
        pop %ebp

 ret

.globl gettime; .type gettime, @function; .align 0; gettime:
 push %ebp
        movl %esp, %ebp

 movl $10, %eax
 push %ecx
 push %edx

 push $retorn_gt

        #dynamic link??
        push %ebp
        movl %esp, %ebp

 SYSENTER
retorn_gt:
 pop %ebp
 addl $4, %esp

 pop %edx
 pop %ecx

 cmpl $0, %eax
        jge fin_gt
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_gt:
 movl %ebp, %esp
        pop %ebp
 ret
