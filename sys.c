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

/*int sys_fork()
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
	int FREE_SPACE = NUM_PAG_KERNEL + NUM_PAG_DATA + NUM_PAG_CODE;
	int COPY_SPACE = NUM_PAG_KERNEL; 

	for (int i = 0; i < NUM_PAG_DATA; i++){
		set_ss_pag(parent_PT, FREE_SPACE+i, get_frame(child_PT,PAG_LOG_INIT_DATA+i));
		copy_data((void *)(COPY_SPACE+i << 12), (void *)((FREE_SPACE+i)<<12), PAGE_SIZE);
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
}*/

int sys_fork() {

  // a)
  if(list_empty(&freequeue)) 
  	return -ENOMEM;

  struct list_head *first = list_first(&freequeue);
  list_del(first);
  struct task_struct *child_ts = list_head_to_task_struct(first);

  union task_union *child_tu = (union task_union *)child_ts;
  union task_union *parent_tu = (union task_union *)current();

  // b)
  copy_data(parent_tu, child_tu, KERNEL_STACK_SIZE*4);

  // c)
  allocate_DIR(child_ts);

  // d)
  int frames[NUM_PAG_DATA];
  int new_frame, i, j;
  for(i = 0; i < NUM_PAG_DATA; ++i) {
  	new_frame = alloc_frame();
  	if(new_frame != -1) 
  		frames[i] = new_frame;
  	else {
  		for(j = 0; j < i; ++j)
  			free_frame(frames[j]);
  		list_add_tail(first, &freequeue);
  		return -EAGAIN;			//añadir al perror en lib.c
  	}
  }

  // e)
  page_table_entry *child_pt = get_PT(child_ts);
  for(i = 0; i < NUM_PAG_DATA; ++i) 
  	set_ss_pag(child_pt, PAG_LOG_INIT_DATA+i, frames[i]);
 
  // f)
  // Compartido (system code, system data)
  page_table_entry *parent_pt = get_PT(current());
  for(i = 0; i < NUM_PAG_KERNEL; ++i)
  	set_ss_pag(child_pt, i , get_frame(parent_pt, i));

  // Compartido (user code)
  for(i = 0; i < NUM_PAG_CODE; ++i) {
  	set_ss_pag(child_pt, PAG_LOG_INIT_CODE+i , get_frame(parent_pt, PAG_LOG_INIT_CODE+i));
  }

  // Copiamos user data y user stack
  int ESPACIO_LIBRE = NUM_PAG_KERNEL + NUM_PAG_DATA + NUM_PAG_CODE;
  int ESPACIO_COPIA = NUM_PAG_KERNEL;

  for(i = 0; i < NUM_PAG_DATA ; ++i) {
  	set_ss_pag(parent_pt, ESPACIO_LIBRE+i, frames[i]);
  	copy_data((void*)((ESPACIO_COPIA+i) << 12), (void*)((ESPACIO_LIBRE+i) << 12), PAGE_SIZE); 
  	del_ss_pag(parent_pt, ESPACIO_LIBRE+i);
  }
  set_cr3(get_DIR(current()));

  /*// g)
  child_tu->task.PID = ++globalPID;
  //block and unblock modifications
  child_tu->task.parent = current();      //añadimos puntero al padre
  child_tu->task.pending_unblocks = 0;
  list_add_tail(&(child_tu->task.brother_list), &current()->child_list);     //añadimos el hijo al la lista de hijos del padre
  INIT_LIST_HEAD(&(child_tu->task.child_list)); 
  */

  // h) i)
  child_tu->stack[KERNEL_STACK_SIZE-18] = (unsigned long)ret_from_fork;
  child_tu->stack[KERNEL_STACK_SIZE-19] = 0;
  child_ts->kernel_esp =(unsigned long)&child_tu->stack[KERNEL_STACK_SIZE-19];
   
  // j)
  //child_tu->task.state = ST_READY;
  list_add_tail(&child_ts->list, &readyqueue);

  // k)
  return child_tu->task.PID;
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
