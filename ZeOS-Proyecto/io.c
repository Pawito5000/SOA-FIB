/*
 * io.c - 
 */

#include <io.h>
#include <errno.h>
#include <types.h>
#include <utils.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;
/*Color distribution:
 *  0 black
 *  1 blue
 *  2 green 
 *  14 orange
 *  15 grey*/
char c_col =  2;
char bg_col = 0;

#define CIRCULAR_BUFFER_SIZE 256

struct c_buff {
	char buff[CIRCULAR_BUFFER_SIZE];
	unsigned int read_ptr;
	unsigned int write_ptr;
	unsigned int char_written;
};

struct c_buff circular_buff;

void init_circular_buff(){
	circular_buff.read_ptr = 0;
	circular_buff.write_ptr = 0;
	circular_buff.char_written = 0;
}

int write_circular_buff(char c) {
	if(circular_buff.char_written > 0 && circular_buff.read_ptr == circular_buff.write_ptr) return -ENOMEM;
	circular_buff.buff[circular_buff.write_ptr] = c;
	circular_buff.write_ptr = (circular_buff.write_ptr+1) % CIRCULAR_BUFFER_SIZE;
	++circular_buff.char_written;
	return 1;
}

int read_circular_buff(char *b){
	if(circular_buff.read_ptr == circular_buff.write_ptr) return 0;
	copy_to_user(&circular_buff.buff[circular_buff.read_ptr], &b[0], sizeof(char));
	circular_buff.read_ptr = (circular_buff.read_ptr+1) % CIRCULAR_BUFFER_SIZE;
	--circular_buff.char_written;
	return 1;
}

int draw_sprite(int posX, int posY, Sprite *sp){
	if ((posX < 0) && (posX > NUM_COLUMNS) || (posY < 0) && (posY > NUM_ROWS)) {
		printk("a");
		return -EINVAL;
	}
	if ((posX + sp->x) > NUM_COLUMNS || (posY + sp->y) > NUM_ROWS) {
		printk("b");
		return -EINVAL;
	}
	/*if (!access_ok(VERIFY_READ, sp, sizeof(Sprite))) {
		printk("c");
		return -EINVAL;
	}*/

	if (!access_ok(VERIFY_READ, sp->content, sizeof(sp->y * sp->x))) {
		printk("d");
		return -EINVAL;
	}
	for(int i = 0; i < sp->x; i++){
		for(int j = 0; j < sp->y; j++){
			printc_xy(posX+i, posY+j,sp->content[i*sp->y+j]);
		}
	}
	return 0;
}


int move_cursor(int posX, int posY)
{
	if ((posX < 0) || (posX > NUM_COLUMNS) || (posY < 0) || (posY > NUM_ROWS)) return -EINVAL;
	//Set the actual cursor possition
	x = posX;
	y = posY;
	return 0;
}	

int change_color(int color, int background)
{
	if((color < 0) || (color > 15) || (background < 0) || (background > 15)) return -EINVAL;
	//Set the actual color settings
	c_col = color;
	bg_col = background;
	return 0;
}

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | (c_col << 8) | (bg_col << 12);
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}
