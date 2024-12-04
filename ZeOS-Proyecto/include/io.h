/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

typedef struct {
        int x; //number of rows
        int y; //number of columns
        char* content; //pointer to sprite content
} Sprite;


/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);
void init_circular_buff();
int write_circular_buff(char c);
int read_circular_buff(char *b);
int move_cursor(int posX, int posY);

#endif  /* __IO_H__ */
