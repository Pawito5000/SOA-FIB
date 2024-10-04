#include <libc.h>

char buff[24];

int pid;

int zeos_tick;

int addASM(int, int);

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  	

	//WRITE
	if(write(1, "\n", 1) == -1) perror();
	char *mesg = "Write: funciona\n";
	if(write(1, mesg, strlen(mesg)) == -1) perror();

	//GETTIME
	if(write(1, "\n", 1) == -1) perror();
	int time = gettime();
	if(time == -1) perror();
	mesg = "Gettime: ";
	if(write(1, mesg, strlen(mesg)) == -1) perror();
	itoa(time, mesg);
	if(write(1, mesg, strlen(mesg)) == -1) perror();

	
	while(1) { }
}

