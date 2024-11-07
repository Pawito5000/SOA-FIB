/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
 	return ((unsigned int)l&0xFFFFF000);
}


struct list_head readyqueue;
struct list_head freequeue;
extern struct list_head blocked;
struct task_struct * idle_task;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	
	printk("IDLE");
	while(1)
	{
	;
	}
}

void init_idle (void)
{
	//First we look for the first element on the freequeue
	struct list_head *head = list_first(&freequeue);

	//we eliminate this element from the freequeue
	list_del(head);

	//with the head of the list we convert it into the pcb task_struct
	struct task_struct *PCB = list_head_to_task_struct(head);

	//We assign the PID of the process, in this case 0
	PCB->PID = 0;
	
	//Set the quantum for the idle process
        set_quantum(PCB, 100);

	//And we use allocate_DIR to initialize the address of the pages
	allocate_DIR(PCB);

	//Now we take the PCB task_union to initialize the contect of the process
	union task_union *t_u = (union task_union *)PCB;
	//we store the address of the code we want to execute
	t_u->stack[KERNEL_STACK_SIZE - 1] = (unsigned long) &cpu_idle;
	//we store the initial value we want on the %ebp
	t_u->stack[KERNEL_STACK_SIZE - 2] = 0;
	//we store the position of the stack where we have stored the initial value for the %ebp
	//we will need a new field on the task_struct
	t_u->task.kernel_esp = &(t_u->stack[KERNEL_STACK_SIZE - 2]);
	
	//We initialize idle_task with the task_struct we have created
	idle_task = PCB;
}

void init_task1(void)
{
	//First we look for the first element on the freequeue
        struct list_head *head = list_first(&freequeue);

        //we eliminate this element from the freequeue
        list_del(head);

        //with the head of the list we convert it into the pcb task_struct
        struct task_struct *PCB = list_head_to_task_struct(head);

        //We assign the PID of the process, in this case 1
        PCB->PID = 1;

	//Set the quantum for the inicial process
	set_quantum(PCB, 200);

        //And we use allocate_DIR to initialize the address of the pages
        allocate_DIR(PCB);

	//We initialize the user address space
	set_user_pages(PCB);

	//Initialize child-parent structures
	INIT_LIST_HEAD(&(PCB->child_list));

        //Now we have to update the tss to point to the new_task system stack
        union task_union *t_u = (union task_union *)PCB;
	tss.esp0 = KERNEL_ESP(t_u);
	//And modify the MSR register
	writeMSR((int) tss.esp0, 0x175);

	//Set the cr3 register
	set_cr3(PCB->dir_pages_baseAddr);
}

void inner_task_switch(union task_union *t){
	//update the tss to point to the user code
	tss.esp0 = KERNEL_ESP(t);
        //And modify the MSR register
        writeMSR((int) tss.esp0, 0x175);

        //Set the cr3 register
        set_cr3(get_DIR(&(t->task)));

	//Now we have to save into the current task the current value of ebp
	//And the esp to point to the stored value of the new task
	inner_switch(&current()->kernel_esp, t->task.kernel_esp);
}

void init_sched()
{
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);
	int i;
	for(i = 0; i < NR_TASKS; i++){
	   list_add(&task[i].task.list, &freequeue);
	}
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}


int remaining_sys_quantum = 200;

int get_quantum (struct task_struct *t)
{
	return t->process_quantum;
}

void set_quantum (struct task_struct *t, int new_quantum)
{
	t->process_quantum = new_quantum; 
}

void update_sched_data_rr (void)
{
  	--remaining_sys_quantum;
}

int needs_sched_rr (void)
{
	// 1 is necessary to change the process, 0 otherwise
	if((remaining_sys_quantum == 0) && (!list_empty(&readyqueue))) return 1;
	if(remaining_sys_quantum == 0) remaining_sys_quantum = get_quantum(current());
	return 0;
}

void update_process_state_rr (struct task_struct *t, struct list_head *dst_queue)
{
	//update the ready queue
	if(t->state == ST_RUN){
		t->state = ST_READY;
		list_add_tail(&(t->list),dst_queue);
	}
	else if(t->state == ST_READY){
		t->state = ST_RUN;
		list_del(&(t->list));
	}
}

void sched_next_rr (void)
{
	struct list_head *new_lh;
	struct task_struct *new_ts;

	//there is no procces to switch, switch to idle
	if(list_empty(&readyqueue)) new_ts = idle_task;	
	else {
		new_lh = list_first(&readyqueue);
		new_ts = list_head_to_task_struct(new_lh);
		update_process_state_rr(new_ts,NULL);
	}

	remaining_sys_quantum = get_quantum(new_ts);
	task_switch((union task_union *)new_ts);
}

void schedule() 
{
	update_sched_data_rr();
	if(needs_sched_rr()){
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}
}
