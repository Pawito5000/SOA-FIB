/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

#include <io.h>
extern int errno;

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

int strlen(char *a);

void perror();

int getKey(char* b);

int getpid();

int fork();

void exit();

int yield();

char *sbrk(int size);

int spritePut(int posX, int posY, Sprite *sp);

int gotoXY(int posX, int posY);

int SetColor(int color, int background);

int threadCreate(void (*function)(void*), void* parameter);

void threadExit(void);

int get_stats(int pid, struct stats *st);

int semCreate(int initial_value);

int semWait(int semid);

int semSignal(int semid);

int semDestroy(int semid);

void SAVE_REGS(void);
void RESTORE_REGS(void);

#endif  /* __LIBC_H__ */
