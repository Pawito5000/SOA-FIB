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
int globalPID = 100;

extern struct list_head freequeue;
extern struct list_head readyqueue;

struct task_struct * child_task;

char buff[512];

int ret_from_fork(){
	printk("j");
	return 0;
}

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
	return (current()->PID);
}

int sys_fork()
{
  	if (list_empty(&freequeue)) return -ENOMEM;
	//First we look for the first element on the freequeue
        struct list_head *head = list_first(&freequeue);

        //we eliminate this element from the freequeue
        list_del(head);

        //with the head of the list we convert it into the task_union
        union task_union *child = (union task_union*) list_head_to_task_struct(head);

	copy_data(current(), child, sizeof(union task_union));

	//Initializes child's directory structrue
	allocate_DIR((struct task_struct *)child);
	
	//Get the child PT address
	page_table_entry *child_PT = get_PT(&child->task);
	printk("p");
	//allocate physical pages
	//if there is not enough space -> rollback
	int npages[NUM_PAG_DATA];
	for (int i = 0; i < NUM_PAG_DATA; i++){
		npages[i] = alloc_frame();
		
		if (npages[i] < 1){
			for(int j = 0; j < i; j++) free_frame(npages[j]);

			list_add_tail(&child->task.list, &freequeue);
			return -ENOMEM;
		}
	}

	page_table_entry *parent_PT = get_PT(current()); 
	printk("a");
	//Setting Kernel+Code segments into the child TP(same segments as the parent)
	for(int i = 0; i < NUM_PAG_KERNEL; i++){
		set_ss_pag(child_PT, i, get_frame(parent_PT,i));
	}

	for(int i = 0; i < NUM_PAG_CODE; i++){
                set_ss_pag(child_PT, PAG_LOG_INIT_CODE+i, get_frame(parent_PT, PAG_LOG_INIT_CODE+i));
	}

	//Setting Data+Stack segments into child TP(new physical pages for the child)
	for(int i = 0; i < NUM_PAG_DATA; i++){
		set_ss_pag(child_PT, PAG_LOG_INIT_DATA+i, npages[i]);
	}

	//Free space of the parent PT to write child's translation
	int FREE_SPACE = PAG_LOG_INIT_CODE+NUM_PAG_CODE;

	for (int i = 0; i < NUM_PAG_DATA; i++){
		set_ss_pag(parent_PT, FREE_SPACE+i, get_frame(child_PT,PAG_LOG_INIT_DATA+i));
		copy_data((void *)(PAG_LOG_INIT_DATA+i << 12), (void *)((FREE_SPACE+i)<<12), PAGE_SIZE);
		del_ss_pag(parent_PT, FREE_SPACE+i);
	}



	//Forcing TLB FLUSH to delete father-to-son translation
	set_cr3(get_DIR(current()));

	//Set the PID
	child->task.PID = ++globalPID;

	//Setting the child stack
	child->stack[KERNEL_STACK_SIZE-18] = (unsigned long) &ret_from_fork;
	child->stack[KERNEL_STACK_SIZE-19] = 0;
	child->task.kernel_esp = child->stack[KERNEL_STACK_SIZE-19];

	//Insert the new process into the ready list
	list_add_tail(&child->task.list,&readyqueue);
	
	child_task = &child->task;
  	return child->task.PID;
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
