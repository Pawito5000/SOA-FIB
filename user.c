#include <libc.h>

char buff[24];

int pid;

//int zeos_tick;

int addASM(int, int);


int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  	

	//WRITE
	if(write(1, "\n", 1) == -1) perror();	
/*	
        char *mesg = "Write: prueba funcionamiento perror\n";
	if(write(1, mesg, strlen(mesg)) == -1) perror();
	//PERROR
	if(write(12, mesg, strlen(mesg)) == -1) perror();
*/
        //GETTIME      
/*        mesg = "Gettime: ";
        if(write(1, mesg, strlen(mesg)) == -1) perror();
        itoa(gettime(), buff);
        if(write(1, buff, strlen(buff)) == -1) perror();
	if(write(1, "\n", 1) == -1) perror();
*/
	//GETPID      
       	char *mesg = "Getpid: ";
        if(write(1, mesg, strlen(mesg)) == -1) perror();
        itoa(getpid(), buff);
        if(write(1, buff, strlen(buff)) == -1) perror();
        if(write(1, "\n", 1) == -1) perror();

	//Let's try to do a fork on the first process
	mesg = "Fork!";
        if(write(1, mesg, strlen(mesg)) == -1) perror();
	int pid = fork();

	if(pid == 0){
		mesg = "\nChild process: ";
        	if(write(1, mesg, strlen(mesg)) == -1) perror();       	
        	itoa(getpid(), buff);
        	if(write(1, buff, strlen(buff)) == -1) perror();
		//Let's try to block this process
		mesg = "\nChild blocked: ";
		if(write(1, mesg, strlen(mesg)) == -1) perror();
                itoa(getpid(), buff);
                if(write(1, buff, strlen(buff)) == -1) perror();
		block();
		mesg = "\nChild unblocked: ";
                if(write(1, mesg, strlen(mesg)) == -1) perror();
		itoa(getpid(), buff);
                if(write(1, buff, strlen(buff)) == -1) perror();
		//Let's try to do another fork now in the child process
		mesg = "\nFork!";
        	if(write(1, mesg, strlen(mesg)) == -1) perror();
		int pid2 = fork();
		if(pid2 == 0){
			//Let's try the exit process
			while(gettime() < 15000);
			mesg = "\nChild exits: ";
                	if(write(1, mesg, strlen(mesg)) == -1) perror();
                	itoa(getpid(), buff);
                	if(write(1, buff, strlen(buff)) == -1) perror();
			exit();
		}
		else {
			//Let's try to exit a process with alive childs
			mesg = "\nChild exits: ";
                	if(write(1, mesg, strlen(mesg)) == -1) perror();
			itoa(getpid(), buff);
                	if(write(1, buff, strlen(buff)) == -1) perror();
                	exit();
		}
	} else {
		mesg = "\nParent process: ";
        	if(write(1, mesg, strlen(mesg)) == -1) perror();
	       	itoa(getpid(), buff);
                if(write(1, buff, strlen(buff)) == -1) perror();
		while(gettime() < 5000);
		//Let's try to unblock the child
		mesg = "\nParent unblocks child: ";
                if(write(1, mesg, strlen(mesg)) == -1) perror();
		itoa(pid, buff);
                if(write(1, buff, strlen(buff)) == -1) perror();	
		unblock(pid);
		while(gettime() < 10000);
	}

	//In this point of the execution only the first process should print the following
	while(gettime() < 15020);
	mesg = "\nHello I am process: ";
        if(write(1, mesg, strlen(mesg)) == -1) perror();
        itoa(getpid(), buff);
        if(write(1, buff, strlen(buff)) == -1) perror();
        if(write(1, "\n", 1) == -1) perror();

	//To try if idle process is changed when no other process are in the ready queue
	while(gettime() < 17000);
	mesg = "\nParent blocked: ";
        if(write(1, mesg, strlen(mesg)) == -1) perror();
        itoa(getpid(), buff);
        if(write(1, buff, strlen(buff)) == -1) perror();
	block();

	while(1) {
		//It was for testing if the gettime() function worked
		/*
		if(write(1, "a", 1) == -1) perror();
        	mesg = "Gettime: ";
        	if(write(1, mesg, strlen(mesg)) == -1) perror();
        	itoa(gettime(), buff);
	        if(write(1, buff, strlen(buff)) == -1) perror();
		*/
	}
}

