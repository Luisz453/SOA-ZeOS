/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>

#include <errno.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %edx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %ebx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; \
      pushl %ecx; \
      pushl %edx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define EOI \
      movb $0x20, %al ; \
      outb %al, $0x20 ;

#define RESTORE_ALL \
      popl %edx; \
      popl %ecx; \
      popl %ebx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs; 

ENTRY (pagefault_handler)
      call pagefault_routine

ENTRY (keyboard_handler)
      SAVE_ALL
      call keyboard_routine
      EOI
      RESTORE_ALL
      iret

ENTRY (clock_handler)
      SAVE_ALL
      EOI
      call clock_routine
      RESTORE_ALL
      iret

ENTRY (syscall_handler_sysenter)
      push $__USER_DS
      push %ebp
      pushfl
      push $__USER_CS
      push 4(%EBP)
      SAVE_ALL
      cmpl $0, %eax
      jl sysenter_err
      cmpl $MAX_SYSCALL, %eax
      jg sysenter_err
      call *sys_call_table(, %eax, 0x04)
      jmp sysenter_fin
sysenter_err:
      movl $-ENOSYS, %eax
sysenter_fin:
      movl %EAX, 0x18(%esp)
      RESTORE_ALL
      movl (%esp), %edx
      movl 12(%esp), %ecx
      sti
      sysexit

ENTRY (write_msr)
      push %ebp
      movl %esp, %ebp
      movl 8(%esp), %ecx 
      movl 12(%esp), %eax
      movl $0, %edx
      wrmsr
      pop %ebp
      ret

ENTRY (task_switch)
      push %ebp
      movl %esp, %ebp
      push %esi
      push %edi
      push %ebx
      push 8(%ebp)
      call inner_task_switch
      addl $4, %esp
      popl %ebx
      popl %edi
      popl %esi
      mov %ebp,%esp
      popl %ebp
      ret


ENTRY (inner_task_switch_asm)
      push %ebp
      movl %esp, %ebp
      mov 8(%esp), %eax
      mov %ebp, (%eax)
      mov 12(%esp), %esp
      popl %ebp
      ret

