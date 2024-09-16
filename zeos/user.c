#include <libc.h>

char buff[256];

int pid;

void pagefault(void){
  char* p = 0;
  *p = 'x';
}

void tests(void){
  write(1, "\n", 1);
  //gettime
  int current_time = gettime();
  itoa(current_time, &buff);
  write(1, &buff[0], strlen(buff)); // 1 because it's the first tick
  perror(); // no error
  write(1, "\n", 1);
  //write tests
  write(10, "hola\n", 5); // bad file descriptor
  perror();
  write(1, "hola\n", 2); // print ho

  write(1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB\n", 67); // more than buffer size

}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  //tests();
  //int a = getpid();
  int a = fork();

  if(a == 0){
    int test = getpid();
    itoa(test, buff);
    write(1, buff, strlen(buff));
    block();
  } else {
    char buff2[256];
    itoa(a, buff2);
    write(1, buff2, strlen(buff2));
    char buff3[256];
    int testing = fork();
    if(testing == 0){
      int pid = getpid();
      itoa(pid, buff3);
      write(1, buff3, strlen(buff3));
    } else {
      unblock(1001);
      exit();
    }
  }
  while(1) {  }
}
