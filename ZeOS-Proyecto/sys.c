/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

//Avoid implicit declaration
char *sys_sbrk(int size);

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  
  char *heap_start = current()->heap_srt_ptr;
  char *heap_end = current()->heap_pointer;

  // Calcular quants bytes s'han utilitzat en el heap
  unsigned int heap_used_bytes = heap_end - heap_start;
  // Calcular el nombre de pàgines utilitzades
  unsigned int heap_used_pages = (heap_used_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
  //fem alloc pages tant de data com heap (ja que son consecutius)
  for (pag=0; pag<NUM_PAG_DATA+heap_used_pages; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use the code pages as a temp logical pages to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA+heap_used_pages; pag += 8)
  {
  	int pages_to_copy = 8;
        if (pag + 8 > (NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA+heap_used_pages)) {
            pages_to_copy = (NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA+heap_used_pages) - pag; // Si és l'últim tros, copiem el que queda
        }
	for (int i = 0; i < pages_to_copy; i++) 
	{
    		/* Map one child page to parent's address space. */
    		set_ss_pag(parent_PT, PAG_LOG_INIT_CODE+i, get_frame(process_PT, pag+i));
    		copy_data((void*)((pag+i)<<12), (void*)((PAG_LOG_INIT_CODE+i)<<12), PAGE_SIZE);
    		del_ss_pag(parent_PT, PAG_LOG_INIT_CODE+i);
  	}
  	/* Deny access to the child's memory space */
  	set_cr3(get_DIR(current()));
  }
  /* Remap the code pages from the child to the parent */
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(parent_PT, PAG_LOG_INIT_CODE+pag, get_frame(process_PT, PAG_LOG_INIT_CODE+pag));
  }

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  uchild->task.heap_pointer = current()->heap_pointer;
  uchild->task.heap_srt_ptr = current()->heap_srt_ptr;
  uchild->task.heap_end_ptr = current()->heap_end_ptr;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Reinitialize the sem_t vector*/
  uchild->task.v_sem = (struct sem_t *)sys_sbrk(10*sizeof(struct sem_t));
  for(int i = 0; i < SEM_T_VECTOR_SIZE; i++) uchild->task.v_sem[i].id = -1;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}

int sys_getKey(char* b)
{
	if(!access_ok(VERIFY_WRITE, b, sizeof(char))) return -EFAULT;
	int a = read_circular_buff(b);
	return a;
}

char *sys_sbrk(int size)
{
	char *old_pointer = current()->heap_pointer;
	if(size > 0) {
		
		char *new_heap_end = current()->heap_pointer + size;

        	// Comprovar si el nou final del heap excedeix el límit
        	if (new_heap_end > current()->heap_end_ptr) {
            		// No podem assignar més memòria del límit
            		return NULL;
        	}	
		
		int pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
		
		// Assignar les pàgines
        	for (int i = 0; i < pages_needed; ++i) {
            		
			int new_ph_pag = alloc_frame();
            		if (new_ph_pag != -1) {
			
                	// Mapejar la pàgina al heap
                		set_ss_pag(get_PT(current()), (unsigned int)(current()->heap_pointer)/PAGE_SIZE, new_ph_pag);
                		current()->heap_pointer += PAGE_SIZE;
            		} else {
                		// Si no hi ha prou frames, alliberar les pàgines assignades fins ara
                		for (int j = 0; j < i; j++) {
					free_frame(get_frame(get_PT(current()), (unsigned int)(current()->heap_pointer - (j * PAGE_SIZE))));
                    			del_ss_pag(get_PT(current()), (unsigned int)(current()->heap_pointer - (j * PAGE_SIZE)));
                		}
				current()->heap_pointer = old_pointer;
                		return NULL;  // Error, no es poden assignar més pàgines
            	
			}
        	}
		
		return old_pointer;
	} else if (size == 0) {
		return current()->heap_pointer;
	} else {
		int pages_to_free = (-size + PAGE_SIZE - 1) / PAGE_SIZE;
                // Comprovar si el nou final del heap excedeix el límit
                if (current()->heap_pointer - pages_to_free * PAGE_SIZE < current()->heap_srt_ptr) {
                        // No podem assignar més memòria del límit
                        return NULL;
                }
		
		for(int i = 0; i < pages_to_free; ++i)
		{
			current()->heap_pointer -= PAGE_SIZE;
            		free_frame(get_frame(get_PT(current()), (unsigned int)(current()->heap_pointer)/PAGE_SIZE));
			del_ss_pag(get_PT(current()), (unsigned int)(current()->heap_pointer)/PAGE_SIZE);
		}

		return old_pointer;
	}
}

int sys_spritePut(int posX, int posY, Sprite *sp)
{
	return draw_sprite(posX, posY, sp);
}

int sys_gotoXY(int posX, int posY)
{
	return move_cursor(posX, posY);
}

int sys_SetColor(int color, int background)
{
	return change_color(color, background);
}

int global_TID=10000;

int sys_threadCreate(void (*function_wrap), void (*function)(void* arg), void* parameter)
{
	//Control de la rutina wrap
	if (function_wrap == NULL) return -EFAULT;

	//Control de la funcion a ejecutar
	if (function == NULL) return -EFAULT;

	//Control memoria
	if (list_empty(&freequeue)) return -ENOMEM;

	//Asignar task al thread
	struct list_head *free_task = list_first(&freequeue);
	struct task_struct *new_task = list_head_to_task_struct(free_task);
	list_del(free_task);
	
	union task_union* new_task_union = (union task_union*) new_task;
	
	/*Encolar al thread en la lista de threads*/
	list_add_tail(&new_task->threads_list, new_task->thread_process);

	/*Copia contenido proceso*/
	copy_data(current(), new_task_union, sizeof(union task_union));

	/*Asignacion de memoria*/
	int new_ph_pag=alloc_frame();
    	if (new_ph_pag!=-1) {
		new_task->PID = -1;
		list_add_tail(&(new_task->list), &freequeue);
	}
	
	page_table_entry * sh_PT = get_PT(current());
	//set_ss_pag(sh_PT, PAGINA LOGICA, new_php_pag);
	//unsigned long *user_stack = @pagina logica

	/*Asignar TID*/
	new_task->TID = ++global_TID;

	/*Inicializar las estructuras del th_task_struct*/
	new_task->state = ST_READY;
	new_task->errno = 0;
		/* Set stats to 0 */
  	init_stats(&(new_task->p_stats));
	
	/*Preparar User Stack
	 *Estado de la pila usr:
			@ret = 0
			----
			@func
			----
			param
	USER_STACK_SIZE->	
	 * */
	/*user_stack[USER_STACK_SIZE-1] = (unsigned long) &parameter;
	user_stack[USER_STACK_SIZE-2] = (unsigned long) &function;
	user_stack[USER_STACK_SIZE-3] = 0;
	*/

	/*Preparar en el System Stack, el contexto de ejecucion
	 *Estado de la pila sys:
	 		%ebp
			----
			@ret
			----
			CTX SW (11 regs)
			----
			CTX HW (6 regs) -> [EIP CS PSW ESP SS]
KERNEL STACK SIZE ->
	 *Añadir:
		 ESP(user) [CTX HW] = pila de usuario - data de TCB 
		 EIP [CTX HW] = func a ejecutar - func wrap
		 ESP(sys) = ebp pila actual
	 */

	//ESP(user)
	//new_task_union->stack[KERNEL_STACK_SIZE-2] = (unsigned long)&user_stack[USER_STACK_SIZE -3];
	
	//EIP
	new_task_union->stack[KERNEL_STACK_SIZE-5] = (unsigned long)function_wrap;

	//ESP(sys)
	new_task->register_esp = (int) &new_task_union->stack[KERNEL_STACK_SIZE-18];

	
	/*Encolar el thread en la readyqueue*/
	list_add_tail(&(new_task->list), &readyqueue);
	return 0;
}

//Avoid implicit declaration
void sys_exit();

void sys_threadExit(void)
{
	if((current()->PID == 1) && (current()->TID == 1)) sys_exit();
	else {
		struct list_head *head;
		struct task_struct *thread_ts;
		struct list_head *n;
		list_for_each_safe(head, n, &(current()->threads_list)){
			thread_ts = list_head_to_task_struct(head);
			if(thread_ts->TID == current()->TID) list_del(head);
		}
	
		current()->state = NULL;

  		/* Free task_struct */
  		list_add_tail(&(current()->list), &freequeue);

  		current()->TID=-1;

  		/* Restarts execution of the next process */
  		sched_next_rr();
	}
}

extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i;

  page_table_entry *process_PT = get_PT(current());

  char *heap_start = current()->heap_srt_ptr;
  char *heap_end = current()->heap_pointer;

  // Calcular quants bytes s'han utilitzat en el heap
  unsigned int heap_used_bytes = heap_end - heap_start;
  // Calcular el nombre de pàgines utilitzades
  unsigned int heap_used_pages = (heap_used_bytes + PAGE_SIZE - 1) / PAGE_SIZE;

  // Deallocate all the propietary physical pages
  for (i=0; i<NUM_PAG_DATA+heap_used_pages; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }

  
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

int sys_semCreate(int initial_value)
{
	int i;
	int max_id = 1;
	
	if (current()->v_sem == NULL) {
		return -ENOMEM;
	}

	for (i = 0; i < SEM_T_VECTOR_SIZE; i++){
		struct sem_t *c_sem = &(current()->v_sem[i]);
		if(c_sem->id == -1){
			c_sem->count = initial_value;
			INIT_LIST_HEAD(&(c_sem->blocked_queue));
			c_sem->id = max_id;
			return c_sem->id;
		} else {
			max_id = (max_id < c_sem->id) ? c_sem->id : max_id;
		}

	}
	return -ENOSPC;
}

int sys_semWait(int semid)
{
	struct sem_t *c_sem = &(current()->v_sem[semid]);
	
	if (c_sem->id == -1) return -EINVAL;
	
	c_sem->count--;
	if (c_sem->count < 0) {
		current()->state = ST_BLOCKED;
		list_add_tail(&(current()->list),&(c_sem->blocked_queue));
		sched_next_rr();	
	}
	return 0;
}

int sys_semSignal(int semid)
{
	struct sem_t *c_sem = &(current()->v_sem[semid]);

        if (c_sem->id == -1) return -EINVAL;

        c_sem->count++;
        if (c_sem->count <= 0) {
		struct list_head *aux = list_first(&(c_sem->blocked_queue));
		struct task_struct *ready_th = list_head_to_task_struct(aux);
		list_del(&(ready_th->list));

		current()->state = ST_READY;
		list_add_tail(&(current()->list),&readyqueue);
	}
	return 0;
}

int sys_semDestroy(int semid)
{

}

