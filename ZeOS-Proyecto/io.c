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

#define CIRCULAR_BUFFER_SIZE 256

int ac_posX, ac_posY; 

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


int move_cursor(int posX, int posY)
{
	if ((posX < 0) || (posX >= 80) || (posY < 0) || (posY >= 25)) return -EINVAL;
	//Set the actual cursor possition
	ac_posX = posX;
	ac_posY = posY;
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
    Word ch = (Word) (c & 0x00FF) | 0x0200;
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
