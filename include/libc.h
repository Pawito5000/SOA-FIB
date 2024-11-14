/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

extern int errno;

int write(int fd, char *buffer, int size);

int gettime();

void perror();

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

void block(void);

int unblock(int pid);

int schedule(int pid);

int sleep(int seconds);

int wakeup(int pid, int NOW);

void exit();

#endif  /* __LIBC_H__ */
