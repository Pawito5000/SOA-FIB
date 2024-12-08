#include <libc.h>
#include <io.h>

char coin[3][3] = {
	{'X','X','X'},
	{'X',' ','X'},
	{'X','X','X'}
};

void print(char *msg)
{
	if(msg != (void *) 0) write(1,msg,strlen(msg));
}

void clear_screen()
{
	gotoXY(0,0);
	SetColor(7,15);
	int i, j;
	for (i = 0; i < 25; i++){	
		for (j = 0; j < 80; j ++) { 
			print("a");
		}		
		print("\n");
	}
  	
}

void init_elems()
{
	Sprite *p1; //Aqui habria que asignarle memoria dinamica Sprite *p1 = (Sprite *)sbrk(sizeof(Sprite));
	//Tratamiento de error con el sbrk
	
	p1->x = 3;
	p1->y = 3;
	p1->content = (char *) coin;
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
	//Set the inicial view

//	clear_screen();	
//	init_elems(); 
	
	//PROVA SBRK
	char buffer[20];
	char *initial_brk = sbrk(0);
	write(1, "Heap inicial: ", 14);
	itoa((int)initial_brk, buffer);
	write(1, buffer, strlen(buffer)); 
	write(1, "\n", 1);

	char *new_brk = sbrk(2*4096);
	if(new_brk == (void *)-1) write(1, "Error", 5); 
	else {
		write(1, "Heap +2 pag: ", 13);
        	itoa((int)sbrk(0), buffer);
        	write(1, buffer, strlen(buffer));
		write(1, "\n", 1);
	}

	char *data = new_brk;
	const char *message = "PROVA AL HEAP";
	for(int i = 0; message[i] != '\0'; i++) data[i] = message[i];
	write(1, "missatge escrit\n", 16);
	
	if(sbrk(-(1*4096)) == (void *)-1) write(1, "error", 5);
	else {
		write(1, "Heap -1 pag: ", 13);
                itoa((int)sbrk(0), buffer);
                write(1, buffer, strlen(buffer));
                write(1, "\n", 1);
	}

	if(sbrk(-(3*4096)) == (void *)NULL) write(1, "funciona\n", 9);
        else write(1, "error", 5);

	//farem fills i més proves sbrk
	int pid = fork();
	if(pid == 0){
		char buffer2[20];
	        char *initial_brk = sbrk(0);
        	write(1, "Heap inicial: ", 14);
        	itoa((int)initial_brk, buffer2);
        	write(1, buffer2, strlen(buffer2));
        	write(1, "\n", 1);

        	char *new_brk = sbrk(2*4096);
        	if(new_brk == (void *)-1) write(1, "Error", 5);
        	else {
                	write(1, "Heap +2 pag: ", 13);
                	itoa((int)sbrk(0), buffer2);
                	write(1, buffer2, strlen(buffer2));
                	write(1, "\n", 1);
        	}
		write(1, "Exit fill\n", 10);
		exit();

	} else {

	}

	
	while(1) {
		
	}
}
