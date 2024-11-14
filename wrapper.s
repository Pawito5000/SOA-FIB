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

 push %ebx
 push %ecx
 push %edx

 movl 8(%ebp), %edx
 movl 12(%ebp), %ecx
 movl 16(%ebp), %ebx

 movl $4, %eax

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
 pop %ebx

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

.globl getpid; .type getpid, @function; .align 0; getpid:
 push %ebp
 movl %esp, %ebp

 movl $20, %eax
 push %ecx
 push %edx

 push $retorn_gp

 push %ebp
 movl %esp, %ebp

 SYSENTER
retorn_gp:
 pop %ebp
        addl $4, %esp

        pop %edx
        pop %ecx

        cmpl $0, %eax
        jge fin_gp
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_gp:
        movl %ebp, %esp
        pop %ebp
        ret

.globl fork; .type fork, @function; .align 0; fork:
        push %ebp
        movl %esp, %ebp

        movl $2, %eax
        push %ecx
        push %edx

        push $retorn_gp

        push %ebp
        movl %esp, %ebp

        SYSENTER
retorn_fk:
        pop %ebp
        addl $4, %esp

        pop %edx
        pop %ecx

        cmpl $0, %eax
        jge fin_fk
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_fk:
        movl %ebp, %esp
        pop %ebp
        ret

.globl exit; .type exit, @function; .align 0; exit:
        push %ebp
        movl %esp, %ebp

        movl $1, %eax
        push %ecx
        push %edx

        push $retorn_ex

        push %ebp
        movl %esp, %ebp

        SYSENTER
retorn_ex:
        pop %ebp
        addl $4, %esp

        pop %edx
        pop %ecx

        cmpl $0, %eax
        jge fin_ex
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_ex:
        movl %ebp, %esp
        pop %ebp
        ret

.globl block; .type block, @function; .align 0; block:
        push %ebp
        movl %esp, %ebp

        movl $14, %eax
        push %ecx
        push %edx

        push $retorn_bl

        #dynamic link??
        push %ebp
        movl %esp, %ebp

        SYSENTER
retorn_bl:
        pop %ebp
        addl $4, %esp

        pop %edx
        pop %ecx

        cmpl $0, %eax
        jge fin_bl
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_bl:
        movl %ebp, %esp
        pop %ebp
        ret


.globl unblock; .type unblock, @function; .align 0; unblock:
        push %ebp
        movl %esp, %ebp

        movl $15, %eax
        push %ecx
        push %edx

 movl 8(%ebp), %edx

        push $retorn_ub

        #dynamic link??
        push %ebp
        movl %esp, %ebp

        SYSENTER
retorn_ub:
        pop %ebp
        addl $4, %esp

        pop %edx
        pop %ecx

        cmpl $0, %eax
        jge fin_ub
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_ub:
        movl %ebp, %esp
        pop %ebp
        ret

.globl schedule; .type schedule, @function; .align 0; schedule:
        push %ebp
        movl %esp, %ebp

        movl $11, %eax
        push %ecx
        push %edx

        movl 8(%ebp), %edx

        push $retorn_sh

        #dynamic link??
        push %ebp
        movl %esp, %ebp

        SYSENTER
retorn_sh:
        pop %ebp
        addl $4, %esp

        pop %edx
        pop %ecx

        cmpl $0, %eax
        jge fin_sh
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_sh:
        movl %ebp, %esp
        pop %ebp
        ret

.globl sleep; .type sleep, @function; .align 0; sleep:
        push %ebp
        movl %esp, %ebp

        push %ecx
        push %edx

        movl 8(%ebp), %edx

        push $retorn_sl

        #dynamic link??
        push %ebp
        movl %esp, %ebp

        int $0x60
retorn_sl:
        pop %ebp
        addl $4, %esp

        pop %edx
        pop %ecx

        cmpl $0, %eax
        jge fin_sl
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_sl:
        movl %ebp, %esp
        pop %ebp
        ret

.globl wakeup; .type wakeup, @function; .align 0; wakeup:
        push %ebp
        movl %esp, %ebp

        push %ecx
        push %edx

        movl 8(%ebp), %edx
 movl 12(%ebp), %ecx

        push $retorn_wk

        #dynamic link??
        push %ebp
        movl %esp, %ebp

        int $0x61
retorn_wk:
        pop %ebp
        addl $4, %esp

        pop %edx
        pop %ecx

        cmpl $0, %eax
        jge fin_wk
        negl %eax
        movl %eax, errno
        movl $-1, %eax
fin_wk:
        movl %ebp, %esp
        pop %ebp
        ret
