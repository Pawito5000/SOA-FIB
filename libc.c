/*
 * libc.c 
 */

#include <libc.h>
#include <errno.h>
#include <types.h>

int errno;

void perror(){
	char buff[25];
	switch(errno){
	case ENOSYS:
		buff = "Function not implemented";
		write(2, buff, strlen(buff));
		break;
	case EBADF:
		buff = "Bad file number";
		write(2, buff, strlen(buff));
		break;
	case EACCES:
                buff = "Permission denied";
                write(2, buff, strlen(buff));
		break;
	default:
		itoa(errno, buff);
		write(2, "Error given", 12);
		break;
	}
}

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

