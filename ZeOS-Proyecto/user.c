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
	SetColor(1,14);
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
	clear_screen();	
	//init_elems(); 
	while(1) {
		
	}
}
