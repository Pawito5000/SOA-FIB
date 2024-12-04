#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  
	//Set the inicial view
	struct Sprite ini;
	int i, j;
	for (i = 0; i < 25; i++){ //columns
		for (j = 0; j < 80; j ++) { //rows
			spritePut(i,j, ); //falta el sprite
		}			
	}
  	char a;
  	while(1) {
		//Polling for pressed keys
		if(getKey(&a) > 0){
	       		write(1, &a, sizeof(char));
			//Make the cursor changes [gotoXY]
			
				//If there is spritePut in the pos;

  	}
}
