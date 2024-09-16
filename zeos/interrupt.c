/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <sched.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;


void keyboard_handler(void);
void clock_handler(void);
void pagefault_handler(long error, long eip);
void write_msr(long msr_id, long value);
void syscall_handler_sysenter();
void task_switch(union task_union *new);

extern int zeos_ticks;


char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */

  setInterruptHandler(33, keyboard_handler, 0);
  setInterruptHandler(32, clock_handler, 0);

  setInterruptHandler(14, pagefault_handler, 0);

  write_msr(0x174, __KERNEL_CS);
  write_msr(0x175, INITIAL_ESP);
  write_msr(0x176, syscall_handler_sysenter);

  set_idt_reg(&idtR);
}

extern union task_union *u_child;

void clock_routine(){
  zeos_ticks += 1;
  zeos_show_clock();
  if(zeos_ticks % 1000 == 0){
    //printc('a');
    //task_switch(change_task());
  }
  schedule();
}

void pagefault_routine(unsigned long error, unsigned long eip){ // eip 0x114000
  /*eip = 0x00114114;
  for(int i = 0; i < 8; ++i){
    unsigned long current = (eip & start);
    printc(current + '0');
    start = start << 4;
  }*/

  char hexadecimal[8];
  int i = 0;
  while(eip != 0){
    int restante = eip % 16;
    if(restante < 10){
      hexadecimal[i++] = restante + '0'; 
    } else {
      hexadecimal[i++] = restante + 'A';
    }
    eip >>= 4;
  }
  printk("\nProcess generates a PAGE FAULT exception at EIP: 0x");
  for(int j = i-1; j >= 0; j--){
    printc(hexadecimal[j]);
  }
  printk("\n");
  while(1);
}

/*void general_protection_routine(){
  printk("\nGeneral protection fault\n");
  while(1);
}*/


void keyboard_routine(){
  unsigned char b = inb(0x60);
  if (!(b&0x80)){
    int indice = b & 0x7F;
    char lletra = char_map[indice];
  if(lletra != "\0"){
      printc_xy(0, 0, lletra);
    }
  //printk(b);
  }
}



