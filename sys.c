/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int zeos_tick;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}

int sys_write(int fd, char *buffer, int size){
	int check = check_fd(fd, ESCRIPTURA);
	if(check != 0) return check;
	if(buffer == NULL) return -EFAULT;
	if(size < 0) return -EINVAL;
	char buff[512];

	if(check == 0 && buffer != NULL && size > 0){
		copy_from_user(0, buffer, size);
		sys_write_console(buff, size);
		return size;			
	} else if(check == 0){
		return -22;
	} else return check;
	

}

int sys_gettime(){
	return zeos_tick;
}
