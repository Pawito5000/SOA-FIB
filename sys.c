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

char buff[512];


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
  	if (list_empry(&freequeue)) return -ENOMEM;
	//First we look for the first element on the freequeue
        struct list_head *head = list_first(&freequeue);

        //we eliminate this element from the freequeue
        list_del(head);

        //with the head of the list we convert it into the task_union
        union task_union *child = (union task_union*) list_head_to_task_struct(head);

	copy_data(current(), child, sizeof(union task_union));

	allocate_DIR(child);

	if (alloc_frames(child)) return -ENOMEM;

	//TODO: apartats e i f
	//page_table_entry *child_PT = get_PT(child->task);

	set_cr3(get_DIR(current()->task));

	child->task()->PID = ;

  	return PID;
}

void sys_exit()
{  
}


int sys_write(int fd, char *buffer, int size){
	int check = check_fd(fd, ESCRIPTURA);
	if(check < 0) return check;
	if((buffer == NULL) || !access_ok(VERIFY_READ, buffer, size)) return -EFAULT;
	if(size <= 0) return -EINVAL;

	for(int i = 0; i < size; i+=512){
		if(i+512 > size) {
			copy_from_user(&buffer[i], buff, size-i);
			sys_write_console(buff, size-i);
		} else {
			copy_from_user(&buffer[i], buff, 512);
                        sys_write_console(buff, 512);
		}
					
	}
	return size;
	

}

int sys_gettime(){
	return zeos_tick;
	
}
