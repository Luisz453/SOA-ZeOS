#include <libc.h>
#include <game.h>


char buff[24];
char buff_read[1024];

void test_fork(){
  int a = 1;
  int pid = fork();
  if(pid == 0){
    a = 3;
  } else {
    a = 5;
  }

  char buff[32];
  itoa(a, buff);
  write(1, buff, strlen(buff));
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
  int temps = gettime();
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  unsigned int* shared_mem_addr;
  shared_mem_addr = (unsigned int*) shmat(0, 0x00284000);
  int pid = fork();
  if(pid == 0){ // hijo
    struct t_game *game = 0;
    init_game(game, temps);
    start_game(game, shared_mem_addr);
  } else {
    char buff[16];
    itoa(pid, buff);
    write(1, buff, strlen(buff));
  }
  read_keyboard(shared_mem_addr);
  //test_fork();
  
  

  while(1) { 
  }
}
