#include <libc.h>
#include <io.h>

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


int __attribute__ ((__section__(".text.main")))
  main(void)
{
	//Set the inicial view
	clear_screen();	
  	while(1) {
		
	}
}
