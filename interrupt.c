/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

	

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


char *long_to_char(unsigned long num, char* buff)
{
  int i = 0;
  if (num == 0) {
	buff[i++] = '0';
	buff[i] = '\0';
	return buff;
  }

  while (num != 0) {
	int res = num % 16;
	if (res < 10)
 		buff[i++] = res + '0';
	else
 		buff[i++] = res - 10 + 'A';
	num = num/16;
  }

  buff[i] = '\0';
 
  // reverse the char*
  int start = 0;
  int end = i-1;
  while (start < end) {
	char tmp = buff[start];
	buff[start] = buff[end];
	buff[end] = tmp;
	start++; end--;
  }
  return buff;
}

void clk_handler(void);

int zeos_tick = 0;

void clk_routine(void){
	zeos_tick += 1;
	//zeos_tick = 12;
	//char *tick = "a";
	//itoa(zeos_tick, tick);
	//printc(zeos_tick);
	zeos_show_clock();
}

void kbd_handler(void);

void kbd_routine(void){
	//print lletra 
	char c = inb(0x60);
	char mob = (c & 0x80) >> 7;
	if(mob == 0) {
		c = c & 0x7F;  
		printc(char_map[c]);
	}
}

void pf_handler(void);

void pf_routine(unsigned int error, unsigned int EIP){
	char buff[21];
	long_to_char(EIP,buff);
	printk("\nProcess generates a PAGE FAULT exception at EIP: 0x");
	printk(buff);
	
	while(1);
}

void syscall_handler(void);

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler (14, pf_handler, 0);	
  setInterruptHandler (32, clk_handler, 0);
  setInterruptHandler (33, kbd_handler, 0);

  writeMSR(__KERNEL_CS, 0x174);
  writeMSR(INITIAL_ESP, 0x175);
  writeMSR(syscall_handler,0x176);


  set_idt_reg(&idtR);
}

