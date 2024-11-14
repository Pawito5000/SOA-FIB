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
extern struct list_head blocked;
extern struct list_head sleeping;
extern int remaining_sys_quantum;

struct task_struct * child_task;

char buff[512];

int ret_from_fork(){
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

int find_consecutive_free_pages(page_table_entry *PT){
	for(int i = 0; i < TOTAL_PAGES -1; i++){
		if((PT[i].bits.present == 0) && (PT[i+1].bits.present == 0)) return i;
	}
	return -1;
}

int copy_memory_from(struct task_struct* SRC, char* from, int size, char* to){
	page_table_entry *SRC_PT = get_PT(SRC);
	page_table_entry *current_PT = get_PT(current());
	
	int copy_space = find_consecutive_free_pages(current_PT);
	if(copy_space == -1) return -ENOMEM;

	unsigned int from_alligned = (unsigned int) from/PAGE_SIZE;
	unsigned int to_alligned = (unsigned int) to/PAGE_SIZE;
	unsigned int pages = (unsigned int) size/PAGE_SIZE;

	for(int i = 0; i < pages; i++){
		if(i%2 != 0) copy_space += 1;
		set_ss_pag(current_PT, copy_space, get_frame(SRC_PT,from_alligned+i));
                copy_data((void *)(copy_space << 12), (void *)((to_alligned+i)<<12), PAGE_SIZE);
                del_ss_pag(current_PT, copy_space);
		if(i%2 != 0) {
			set_cr3(get_DIR(current()));
			copy_space -= 1;
		}
	}
	return 0;
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
	int PID = ++globalPID;
	child->task.PID = PID;


	child->task.parent = current();
	INIT_LIST_HEAD(&(child->task.anchor));
	list_add_tail(&(child->task.anchor), &current()->child_list);
	INIT_LIST_HEAD(&(child->task.child_list));

	//Setting the child stack
	child->stack[KERNEL_STACK_SIZE-18] = (unsigned long) &ret_from_fork;
	child->stack[KERNEL_STACK_SIZE-19] = 0;
	child->task.kernel_esp = &child->stack[KERNEL_STACK_SIZE-19];

	//Insert the new process into the ready list
	list_add_tail(&child->task.list,&readyqueue);
	child->task.state = ST_READY;
	set_quantum(&child->task,200); 
	child_task = &child->task;

	child->task.pending_unblocks = 0;

	return child->task.PID;
}

extern struct task_struct * idle_task;

void sys_exit()
{	
	struct task_struct *parent_ts = current()->parent; 
	struct list_head *head;
	struct task_struct *child_ts;
	struct list_head *n;	

        list_for_each_safe(head, n,&(parent_ts->child_list)){
                //for each element of the child list we need to obtain the PID
                child_ts = list_head_to_task_struct(head);
                if(child_ts->PID == current()->PID) list_del(head);
	}
//	struct list_head *first = list_first(&(current()->child_list));
//	list_add_tail(first, &(idle_task->child_list));

	list_for_each_safe(head, n,&(current()->child_list)){
		list_add_tail(head, &(idle_task->child_list));
		child_ts = list_head_to_task_struct(head);
		child_ts->parent = idle_task;
	}

	page_table_entry *current_PT = get_PT(current());
	for (int i = 0; i < NUM_PAG_DATA; i++){
                free_frame(get_frame(current_PT, PAG_LOG_INIT_DATA+i));
		del_ss_pag(current_PT, PAG_LOG_INIT_DATA+i);
	}
	
	current()->PID = -1;
	current()->state = NULL;
	list_add_tail(&(current()->list),&freequeue);
	sched_next_rr();	
}


void sys_block(void)
{	
	//block the process
	if(current()->pending_unblocks == 0){
		current()->state = ST_BLOCKED;
		list_add_tail(&(current()->list),&blocked);
		sched_next_rr();
	} else current()->pending_unblocks -= 1;
}

int sys_unblock(int pid)
{
	struct list_head *new_lh; //cursor of the loop
        struct task_struct *new_ts; // element to check the pid

	//first check if the pid's process is a child of current
	list_for_each(new_lh,&(current()->child_list)){
		//for each element of the child list we need to obtain the PID
		new_ts = list_head_to_task_struct(new_lh);
		if(new_ts->PID == pid) {
			//process need to be unblocked
			if(new_ts->state == ST_BLOCKED){
				new_ts->state = ST_READY;
				list_add_tail(&(new_ts->list),&readyqueue);
			}
			new_ts->pending_unblocks += 1;			
			return 0;
		}
	}
	return -1;
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

int sys_schedule(int pid){
	if(current()->PID == pid) return 0;	
        struct list_head *new_lh;
        struct task_struct *new_ts;
        struct list_head *n;

        list_for_each_safe(new_lh, n,&readyqueue){
                //for each element of the child list we need to obtain the PID
                new_ts = list_head_to_task_struct(new_lh);
                if(new_ts->PID == pid) {
			current()->state = ST_READY;
			list_add_tail(&(current()->list),&readyqueue);
			new_ts->state = ST_RUN;
                	list_del(&(new_ts->list));
        		remaining_sys_quantum = get_quantum(new_ts);
        		task_switch((union task_union *)new_ts);
			printk("fet");
			return 0;
		}
        }
	return -ESRCH;

}

int sys_sleep(int seconds){
	if(seconds < 0) return -EINVAL;
	current()->seconds = seconds;
	current()->state = ST_BLOCKED;
	list_add_tail(&(current()->list),&sleeping);
	sched_next_rr();
	if(current()->seconds != 0) return -EINTR;
	else return 0;
}

int sys_wake(int pid, int NOW){
	struct list_head *new_lh;
        struct task_struct *new_ts;
        struct list_head *n;
	if(NOW < 0 || NOW > 1) return -EINVAL;
        list_for_each_safe(new_lh, n,&sleeping){
                //for each element of the child list we need to obtain the PID
                new_ts = list_head_to_task_struct(new_lh);
                if(new_ts->PID == pid) {
			if(NOW == 1){
                        	current()->state = ST_READY;
                        	list_add_tail(&(current()->list),&readyqueue);
                        	new_ts->state = ST_RUN;
                        	list_del(&(new_ts->list));
                        	remaining_sys_quantum = get_quantum(new_ts);
                        	task_switch((union task_union *)new_ts);
                        	printk("fet");
                        	return 0;
			} else {
				new_ts->state = ST_READY;
				list_del(&(new_ts->list));
				list_add_tail(&(new_ts->list),&readyqueue);
				return 0;
			}

                }
        }
        return -EEXIST;
}


int sys_gettime(){
	return zeos_tick;
	
}
