#include <asm.h>
#include <segment.h>

#include <errno.h>

ENTRY (write)
    push %ebp
    movl %esp, %ebp
    push %edx
    push %ecx
    push %ebx
    movl 8(%ebp), %edx
    movl 12(%ebp), %ecx
    movl 16(%ebp), %ebx
    mov $0x04, %eax
    push %ecx
    push %edx
    movl $post, %esi
    push %esi
    push %ebp
    movl %esp, %ebp
    SYSENTER
post:
    popl %ebp
    popl %esi
    popl %edx
    popl %ecx
    cmpl $0, %eax
    jge fin_if

    neg %eax
    movl %eax, %ebx
negate:
    neg %eax
    movl %ebx, errno
    movl $-1, %eax
fin_if:
    pop %edx
    pop %ecx
    pop %ebx
    popl %ebp
    ret


ENTRY (gettime)
    push %ebp
    movl %esp, %ebp
    movl $10, %eax //ID syscall es 10
    push %ecx
    push %edx
    movl $post2, %esi
    push %esi
    push %ebp
    movl %esp, %ebp
    SYSENTER
post2:
    popl %ebp
    popl %esi
    popl %edx
    popl %ecx
    cmpl $0, %eax
    jge fin_if2

    neg %eax
    movl %eax, %ebx
negate2:
    neg %eax
    movl %ebx, errno
    movl -1, %eax
fin_if2:
    popl %ebp
    ret

ENTRY (getpid)
    push %ebp
    movl %esp, %ebp
    movl $20, %eax //ID syscall es 20
    push %ecx
    push %edx
    movl $post3, %esi
    push %esi
    push %ebp
    movl %esp, %ebp
    SYSENTER
post3:
    popl %ebp
    popl %esi
    popl %edx
    popl %ecx
    cmpl $0, %eax
    jge fin_if3

    neg %eax
    movl %eax, %ebx
negate3:
    neg %eax
    movl %ebx, errno
    movl -1, %eax
fin_if3:
    popl %ebp
    ret

ENTRY (fork)
    push %ebp
    movl %esp, %ebp
    movl $2, %eax //ID syscall es 10
    push %ecx
    push %edx
    movl $post4, %esi
    push %esi
    push %ebp
    movl %esp, %ebp
    SYSENTER
post4:
    popl %ebp
    popl %esi
    popl %edx
    popl %ecx
    cmpl $0, %eax
    jge fin_if4

    neg %eax
    movl %eax, %ebx
negate4:
    neg %eax
    movl %ebx, errno
    movl -1, %eax
fin_if4:
    popl %ebp
    ret

ENTRY (exit)
    push %ebp
    movl %esp, %ebp
    movl $1, %eax
    push %ecx
    push %edx
    movl $ret_exit, %esi
    push %esi
    push %ebp
    movl %esp, %ebp
    SYSENTER
ret_exit:
    popl %ebp
    popl %esi
    popl %edx
    popl %ecx
    popl %ebp
    ret

ENTRY (block)
    push %ebp
    movl %esp, %ebp
    movl $5, %eax
    push %ecx
    push %edx
    movl $ret_block, %esi
    push %esi
    push %ebp
    movl %esp, %ebp
    SYSENTER
ret_block:
    popl %ebp
    popl %esi
    popl %edx
    popl %ecx
    popl %ebp
    ret

ENTRY (unblock)
    push %ebp
    movl %esp, %ebp
    movl $6, %eax
    push %edx
    movl 8(%ebp), %edx
    push %edx
    movl $post5, %esi
    push %esi
    push %ebp
    movl %esp, %ebp
    SYSENTER
post5:
    popl %ebp
    popl %esi
    popl %edx
    cmpl $0, %eax
    jge fin_if5

    neg %eax
    movl %eax, %ebx
negate5:
    neg %eax
    movl %ebx, errno
    movl -1, %eax
fin_if5:
    pop %edx
    popl %ebp
    ret