/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <libc.h>

#include <errno.h>

#include <entry.h>


#include <system.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define tamany 1024


int check_fd(int fd, int permissions)
{
  if (fd!=1 && fd != 0) return -9; /*EBADF*/

  if (permissions!=ESCRIPTURA && fd == 1) return -13; /*EACCES*/
  if (permissions!=LECTURA && fd == 0 ) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_write(int fd, char* buffer, int size){
	int err = check_fd(fd, ESCRIPTURA);

	if (err < 0){
		
		return err;
	}
	if(buffer == NULL){
		
		return -EFAULT;
	}
	if (size < 0){
		
		return -EINVAL;
	}

	if(!access_ok(VERIFY_READ,buffer,size)){
		return -EACCES;
	}
	char aux2[tamany];
	int iterations = 0;
	int aux = size;
    while(size > 0){
		int ret;
		if(size >= tamany) ret = copy_from_user(buffer+tamany*(iterations),aux2,tamany);
		else  ret = copy_from_user(buffer+tamany*(iterations),aux2,size);
		if(ret >= 0){
		  if(size < tamany) err = sys_write_console(aux2,size);
		  else err = sys_write_console(aux2,tamany);
		  if (err < 0){
			  errno = -err;
			  return err;
		  }
		  size -= err;
		  iterations++;
	    }
	    else return ret;
	}

	return aux-size;
}


int sys_read(int fd, char* buffer, int size){
	

	if(buffer == NULL){
		
		return -EFAULT;
	}
	
	if (check_fd(fd, LECTURA) < 0){
		
		return -EBADF;
	}
	if (size < 0){
		return -EINVAL;
	}
	if(!access_ok(VERIFY_WRITE,buffer,size)){
		return -EFAULT;
	}

	return sys_read_keyboard(buffer,size);
}

int sys_gettime(){
   return zeos_ticks;
}


int sys_getpid(){
	return current()->PID;
}


void free_structures(union task_union* t){

	struct task_struct* aux = (struct task_struct*)t;
	list_add_tail(&aux->list,&freequeue);
}


int sys_fork(){
	
	if(list_empty(&freequeue)){
		return -ENOMEM;
	}
	//a)
	struct task_struct* current_task_struct = current();
	union task_union* current_task_union =  (union task_union*) current_task_struct;

	struct list_head* child_head = list_first(&freequeue);

	list_del(list_first(&freequeue));
	struct task_struct* child_task_struct = list_head_to_task_struct(child_head);
	union task_union* child_task_union = (union task_union*)child_task_struct;
	
	//b)
	copy_data(current_task_union,child_task_union,sizeof(union task_union));
	
	//c)
	allocate_DIR(child_task_struct);
	
	
	//d)
	int i;
	int temp;
	int physical[NUM_PAG_DATA];
	for(i = 0; i < NUM_PAG_DATA; i++){
		temp = alloc_frame();
		if(temp < 0){
			free_structures(child_task_struct);
			return -ENOMEM;
		}
		else physical[i] = temp;
		
	
	}

	//e) i
	page_table_entry* child_pt = get_PT(child_task_struct);
	page_table_entry* father_pt = get_PT(current_task_struct);
	page_table_entry* father_dir = get_DIR(current_task_struct);

	for(i = 0; i < NUM_PAG_KERNEL;i++){
		child_pt[i].entry = father_pt[i].entry;
	}
	
	for(i = 0; i < NUM_PAG_CODE; i++){
		child_pt[PAG_LOG_INIT_CODE+i].entry = father_pt[PAG_LOG_INIT_CODE+i].entry;
	}
	for(i = 0; i < NUM_PAG_DATA; i++){
		set_ss_pag(child_pt,PAG_LOG_INIT_DATA+i,physical[i]);
	}
	//e) ii
	for(i = PAG_LOG_INIT_DATA; i < TOTAL_PAGES; i++){
		if (!(father_pt[i].bits.present)) break;
	}
	int j;
	set_cr3(father_dir);
	for(j = 0; j < NUM_PAG_DATA; j++){
		set_ss_pag(father_pt, i, physical[j]);
		copy_data( (void*)((PAG_LOG_INIT_DATA+j)*PAGE_SIZE), (void *)(i*PAGE_SIZE), PAGE_SIZE);
		del_ss_pag(father_pt,i);
		set_cr3(father_dir);
	}

	

	if(current_task_struct->break_pos != HEAP_START){
		int pages = (current_task_struct->break_pos - HEAP_START)/PAGE_SIZE +1;
		int heap[pages];
		for(i = 0; i < pages; i++){
			temp = alloc_frame();
			if(temp < 0){
				
				return -ENOMEM;
			}
			else heap[i] = temp;
		}

		for(i = 0; i < pages; i++){
			set_ss_pag(child_pt,(PAG_LOG_INIT_HEAP )+i,heap[i]);
		}

		for(i = PAG_LOG_INIT_HEAP; i < TOTAL_PAGES; i++){
			if (!(father_pt[i].bits.present)) break;
		}
		int j;
		
		for(j = 0; j < pages; j++){
			set_ss_pag(father_pt, i, heap[j]);
			copy_data( (void*)((PAG_LOG_INIT_HEAP+j)*PAGE_SIZE), (void *)(i*PAGE_SIZE), PAGE_SIZE);
			del_ss_pag(father_pt,i);
			set_cr3(father_dir);
		}
	}
	
	

	//f)
	child_task_struct->PID = ++PID_actual;
	//g)
	child_task_struct->state = ST_READY;
	child_task_struct->statistics.user_ticks = PAG_LOG_INIT_HEAP >> 12;
	child_task_struct->statistics.system_ticks = 0;
	child_task_struct->statistics.blocked_ticks = 0;
	child_task_struct->statistics.ready_ticks = 0;
	child_task_struct->statistics.elapsed_total_ticks = 0;
	child_task_struct->statistics.total_trans = 0;
	child_task_struct->statistics.remaining_ticks = 0;
	
	
	//h)
	unsigned int ebp;
	
	__asm__ __volatile__ (  
   "movl %%ebp, %0;"
   : "=r" (ebp)
	);

	unsigned int position = (ebp - (unsigned int)current_task_struct) / 4;



	child_task_union->stack[position+1] = (unsigned int)&ret_from_fork;

	child_task_struct->stack_position = (unsigned long)&child_task_union->stack[position];
    
	
	//i)
	list_add_tail(&child_task_struct->list,&readyqueue);
	//j)
//	task_switch(child_task_union);
//	idle_task = current_task_union;
//  task_1 = child_task_union;
	//task_switch(current_task_union);

	return child_task_struct->PID;
}


int sys_clone(void(*function), void* stack){

	if(list_empty(&freequeue)){
		return -ENOMEM;
	}
	if(!access_ok(VERIFY_WRITE,stack,4) || !access_ok(VERIFY_READ,function ,4)){
		return -EFAULT;
	}
	//a)
	struct task_struct* current_task_struct = current();
	union task_union* current_task_union =  (union task_union*) current_task_struct;

	struct list_head* child_head = list_first(&freequeue);

	list_del(list_first(&freequeue));
	struct task_struct* child_task_struct = list_head_to_task_struct(child_head);
	union task_union* child_task_union = (union task_union*)child_task_struct;
	
	//b)
	copy_data(current_task_union,child_task_union,sizeof(union task_union));
	
	//c)
	pages_data[current_task_struct->dir_pos].shared++;
	
	
	//f)
	child_task_struct->PID = ++PID_actual;
	//g)

	child_task_struct->break_pos = (PAG_LOG_INIT_HEAP) << 12;;

	child_task_struct->state = ST_READY;
	child_task_struct->statistics.user_ticks = 0;
	child_task_struct->statistics.system_ticks = 0;
	child_task_struct->statistics.blocked_ticks = 0;
	child_task_struct->statistics.ready_ticks = 0;
	child_task_struct->statistics.elapsed_total_ticks = 0;
	child_task_struct->statistics.total_trans = 0;
	child_task_struct->statistics.remaining_ticks = 0;
	
	
	//h)
	unsigned int ebp;
	
	__asm__ __volatile__ (  
   "movl %%ebp, %0;"
   : "=r" (ebp)
	);

	unsigned int position = (ebp - (unsigned int)current_task_struct) / 4;

	child_task_union->stack[position+1] = &ret_from_fork;
	
	child_task_union->stack[position+7] = stack;
	
	child_task_union->stack[position+13] = function;

	child_task_union->stack[position+16] = stack;

	child_task_struct->stack_position = (unsigned long)&child_task_union->stack[position];
    
	
	//i)
	list_add_tail(&child_task_struct->list,&readyqueue);

	return child_task_struct->PID;

}


void* sys_sbrk(int increment){

	struct task_struct* curr = current();
	int old_prog_break = curr->break_pos;
	page_table_entry* curr_pt = get_PT(curr);
	
	if(old_prog_break + increment < HEAP_START){
		increment = HEAP_START - old_prog_break;
	}

	int init_page = old_prog_break >> 12;
	int end_page = (old_prog_break + increment) >> 12;



	int end_page_false = ((old_prog_break + increment)%PAGE_SIZE == 0) ? 1 : 0;

	if(end_page > TOTAL_PAGES)return -ENOMEM;
	

	if(increment < 0){
		int i; 
		for(i = init_page; i > end_page || (i == end_page && end_page_false) ; i-- ){
			if(i < PAG_LOG_INIT_HEAP){
					curr->break_pos = HEAP_START;
					set_cr3(get_DIR(curr));
					return -EFAULT;
			}
			free_frame(get_frame(curr_pt,i));
			del_ss_pag(curr_pt,i);
		
		}
		set_cr3(get_DIR(curr));
		
	}
	else if(increment > 0){
		/*
		if(old_prog_break == HEAP_START){
		
			int phys = alloc_frame();
			if(phys < 0)return -ENOMEM;
			set_ss_pag(curr_pt,PAG_LOG_INIT_HEAP,phys);
		}*/
			
			int i,temp;
			
			for(i = init_page; i < end_page || (i == end_page && !end_page_false) ; i++){
				if(!curr_pt[i].bits.present){
					temp = alloc_frame();
					if(temp >= 0){
						set_ss_pag(curr_pt,i,temp);
					}
					else{	
						int j;
						for(j = i; j > init_page || (j == init_page && old_prog_break%PAGE_SIZE == 0);j--){
							free_frame(get_frame(curr_pt,j));
							del_ss_pag(curr_pt,j);
						}
						return -ENOMEM;
					}
				}
			}


		}
	curr->break_pos = old_prog_break  + increment;	
	return (void*)old_prog_break;


}

void sys_exit(){
	// a)  Free  the  data  structures  and  resources  of  this  process  (physical  memory,  task_struct, and so). It uses the free_frame function to free physical pages.
	struct task_struct* curr = current();
	page_table_entry* curr_pt = get_PT(curr);
	int i;
	
	for(i = 0; i < 20;i++){
		if(semaphores[i].father == current())sys_sem_destroy(i);
	}
	//b) Use the scheduler interface to select a new process to be executed and make a context switch. HACERLA DESPUES DEL SCHEDULER
	curr->PID = -1;
	if(--pages_data[curr->dir_pos].shared == 0){
		pages_data[curr->dir_pos].valid = 0;	
		for (i = PAG_LOG_INIT_DATA; i < PAG_LOG_INIT_DATA + NUM_PAG_DATA; i++){
		 	free_frame(curr_pt[i].bits.pbase_addr); 
		 	del_ss_pag(curr_pt, i);
	 }	
	}
	list_add_tail(&curr->list,&freequeue);
	sched_next_rr();
}



int sys_sem_init(int n_sem, unsigned int value){
	
	if(n_sem < 0 || n_sem >= 20)return -EINVAL;
	
	if(semaphores[n_sem].valid)return -EBUSY;
	
	semaphores[n_sem].valid = 1;
	semaphores[n_sem].value = value;
	semaphores[n_sem].father = current();
	INIT_LIST_HEAD(&semaphores[n_sem].queue);
 	return 0;	
}

int sys_sem_wait(int n_sem){
	if(n_sem < 0 || n_sem >= 20)return -EINVAL;
	if(!semaphores[n_sem].valid){
		return -EINVAL;
	}

	if(semaphores[n_sem].value <= 0){
		update_process_state_rr(current(),&semaphores[n_sem].queue);
		sched_next_rr();
		if(!semaphores[n_sem].valid){
			return -EINVAL;
		}
		return 0;
	}
	semaphores[n_sem].value--;

	

	return 0;

}

int sys_sem_signal(int n_sem){
	if(n_sem < 0 || n_sem >= 20)return -EINVAL;
	if(!semaphores[n_sem].valid){
		return -EINVAL;
	}
	
	if(list_empty(&semaphores[n_sem].queue)){
		semaphores[n_sem].value++;
		return 0;
	}
	
	struct task_struct* blocked  = list_head_to_task_struct(list_first(&semaphores[n_sem].queue));
	update_process_state_rr(blocked,&readyqueue);
	
	return 0;
	
}

int sys_sem_destroy(int n_sem){
	
	if(n_sem < 0 || n_sem >= 20)return -EINVAL;

	if(!semaphores[n_sem].valid){
		return -EINVAL;
	}


	if(semaphores[n_sem].father == current()){
		while(!list_empty(&semaphores[n_sem].queue)){
			struct task_struct* blocked = list_head_to_task_struct(list_first(&semaphores[n_sem].queue));
			update_process_state_rr(blocked,&readyqueue);
		}
		semaphores[n_sem].valid = 0;
		return 0;	
	}
	return -EPERM;

}


int sys_get_stats(int pid, struct stats *st){
	
	
	if(pid < 0){
		return -EINVAL;
	}
	if(!access_ok(VERIFY_WRITE,st,sizeof(struct stats))){
		
		return -EFAULT;
	}
	int i;
	for(i=0; i < NR_TASKS;i++){
		if(task[i].task.PID == pid){
			task[i].task.statistics.remaining_ticks=run_quantum;
			copy_to_user(&task[i].task.statistics,st,sizeof(struct stats));
			return 0;
		}
	}
	return -ESRCH;
}


int change_to_user(){
	struct task_struct* curr = current();
	curr->statistics.system_ticks += get_ticks() - curr->statistics.elapsed_total_ticks;
	curr->statistics.elapsed_total_ticks = get_ticks();
}


int change_to_system(){

	struct task_struct* curr = current();
	curr->statistics.user_ticks += get_ticks() - curr->statistics.elapsed_total_ticks;
	curr->statistics.elapsed_total_ticks = get_ticks();
}

int change_to_ready(){
	
	struct task_struct* curr = current();
	curr->statistics.ready_ticks += get_ticks() - curr->statistics.elapsed_total_ticks;
	curr->statistics.elapsed_total_ticks = get_ticks();
	}
