#include <libc.h>

char buff[100];

int pid;

void savage(){
	int a = 2378;
	int b = 54;
	int sum = a+b;
	itoa(sum, buff);
	write(1, buff, strlen(buff));
	exit();
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  //  int holy[52];
//	int dep = clone(&savage, &holy[52]);	
//runjp();
     //char dep[24];
     //int x  =read(0,&dep,15);
     
       //write(1,&dep[0],x);
  	
  while(1) { 
      
  }
}
