/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <libc.h>
#include <errno.h>

#define DEFAULT_QUANTUM 50

int run_quantum;

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */


struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}


extern struct list_head blocked;

struct circular_buffer keyboard_buffer;

struct list_head keyboardqueue;
struct list_head freequeue;
struct list_head readyqueue;
struct task_struct *idle_task;
struct task_struct *task_1;

struct data_page pages_data[1024];

int PID_actual = 1 ;


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

void init_DIR(){
	int i;
	for(i = 0; i < 1024;i++){
		pages_data[i].valid = 0;
	}
}

int allocate_DIR(struct task_struct *t) 
{
	int i; 
	for(i = 0; i < 1024; i++){
		
		if(!pages_data[i].valid){
			break;
		}
	}
	if(i >= 1024){
		return -ENOMEM;
	}
	
	pages_data[i].valid = 1;
	pages_data[i].shared = 1;
	t->dir_pos = i;

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[i]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	while(1){
	
	}
}

void init_idle (void){

	/*
	1)  Get an available task_union from the freequeue to contain the characteristics of this process
	2)  Assign PID 0 to the process.
	3)  Initialize  field dir_pages_baseAaddr with  a  new  directory  to  store  the  process  address  space using the allocate_DIR routine.
	4)  Initialize  an  execution  context  for  the  procees  to  restore  it  when  it  gets  assigned  the  cpu (see section 4.5) and executes cpu_idle.
	5)  Define a global variable idle_task

	*/
	// 1)
	struct list_head* first_free = list_first(&freequeue);
	list_del(list_first(&freequeue));
	struct task_struct* idle_task_struct = list_head_to_task_struct(first_free);
	union task_union* idle_task_union = (union task_union*)idle_task_struct;
	// 2)
	idle_task_struct->PID = 0;
	// 3)
	allocate_DIR(idle_task_struct);
	// 4)
	idle_task_union->stack[KERNEL_STACK_SIZE-2] = 0;
	idle_task_union->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&cpu_idle;
	idle_task_struct->stack_position = (unsigned int)&(idle_task_union->stack[KERNEL_STACK_SIZE-2]);

	// 5)
	idle_task = idle_task_struct;	
	// ^_^ Scheduling
	set_quantum(idle_task,DEFAULT_QUANTUM);

	idle_task->break_pos = HEAP_START;
	idle_task->state = ST_READY;
	idle_task->statistics.user_ticks = 0;
	idle_task->statistics.system_ticks = 0;
	idle_task->statistics.blocked_ticks = 0;
	idle_task->statistics.ready_ticks = 0;
	idle_task->statistics.elapsed_total_ticks = get_ticks();
	idle_task->statistics.total_trans = 0;
	idle_task->statistics.remaining_ticks = 0;
//	list_add_tail(&idle_task_struct->list,&readyqueue);
}

void init_task1(void)
{
/*
	0) Getting the first freequeue element and doing some stuff.
	1) Assign PID 1 to the process
	2) Initialize ﬁeld dir_pages_baseAaddr with a new directory to store the process address space using the allocate_DIR routine.
	3) Complete the initialization of its address space, by using the function set_user_pages (see ﬁle 
	   mm.c). This function allocates physical pages to hold the user address space (both code and
	   data pages) and adds to the page table the logical-to-physical translation for these pages.
	   Remember that the region that supports the kernel address space is already conﬁgure for all
	   the possible processes by the function init_mm.
	4) Update the TSS to make it point to the new_task system stack.
	5) Set its page directory as the current page directory in the system, by using the set_cr3 function (see ﬁle mm.c).
	*/
 
	
	struct list_head* first_free = list_first(&freequeue);
	
	list_del(list_first(&freequeue));
	struct task_struct* init_task = list_head_to_task_struct(first_free);
	union task_union* task1_union = (union task_union*)init_task;
	task_1 = init_task;
	
	// 1) 
	task_1->PID = 1;
	// 2) 
	allocate_DIR(task_1);
	page_table_entry* dir = get_DIR(task_1);
	// 3) 
	set_user_pages(task_1);
	// 4)
	tss.esp0 = (unsigned long) &task1_union->stack[KERNEL_STACK_SIZE-1];
	// 5)
	set_cr3(dir);
	// *_* Scheduling
	set_quantum(task_1,DEFAULT_QUANTUM);

	task_1->break_pos =  HEAP_START;
	task_1->state = ST_RUN;
	task_1->statistics.user_ticks = 0;
	task_1->statistics.system_ticks = 0;
	task_1->statistics.blocked_ticks = 0;
	task_1->statistics.ready_ticks = 0;
	task_1->statistics.elapsed_total_ticks = get_ticks();
	task_1->statistics.total_trans = 0;
	task_1->statistics.remaining_ticks = 0;
	run_quantum = get_quantum(task_1);

}

void task_switch(union task_union* t){
	__asm__ __volatile__ (
		"pushl %esi;"
		"pushl %edi;"
		"pushl %ebx;"	 
	   );
	   inner_task_switch(t);
	   __asm__ __volatile__ (
		"popl %ebx;"
		"popl %ebx;"
		"popl %edi;"
		"popl %esi;"	 
	   );
}


void inner_task_switch(union task_union*t){
/*
1) Update the TSS to make it point to the new_task system stack.
2) Change the user address space by updating the current page directory: use the set_cr3
   funtion to set the cr3 register to point to the page directory of the new_task.
3) Store the current value of the EBP register in the current PCB. EBP has the address of the current
   system stack where the inner_task_switch routine begins (the dynamic link).
4) Change the current system stack by setting ESP register to point to the stored value in the
   new PCB.
5) Restore the EBP register from the stack.
6) Return to the routine that called this one using the instruction RET (usually task_switch,
   but. . . ).
*/
	struct task_struct* curr = current();
	struct task_struct* t_struct = (struct task_struct*)t;
	// 1)
	tss.esp0 = (unsigned long)&t->stack[KERNEL_STACK_SIZE];
	// 2)
	page_table_entry* dir = get_DIR(t_struct);
	if(dir != get_DIR(curr))  set_cr3(dir);
	// 3)

	__asm__ __volatile__(
		"movl %%ebp,%0;"
	: "=r" (curr->stack_position)
	);
	// 4), 5) y 6)
	__asm__ __volatile__(
		"movl %%eax, %%esp;"
		"popl %%ebp;"
		"ret;"
	: 
	: "a" (t->task.stack_position)
	
	);
}



void init_freequeue(void){
	INIT_LIST_HEAD(&freequeue);
	int i;
	for(i = 0; i < NR_TASKS; i++){
		list_add_tail(&task[i].task.list,&freequeue);
	}
}

void init_readyqueue(void){
	INIT_LIST_HEAD(&readyqueue);
}

void init_sched(){

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


void update_sched_data_rr (){
	run_quantum--;
}

int needs_sched_rr (){
	if(run_quantum == 0){
		if(!list_empty(&readyqueue))return 1;
		current()->statistics.total_trans++;
	
		
		run_quantum = get_quantum(current());
	}
	return 0;
}

void update_process_state_rr ( struct task_struct * t, struct list_head * dst_queue){
//	if(t->state == ST_READY && dst_queue==NULL) list_del(&t->list);
//	printk("dep1");
	
	if(dst_queue == NULL){
		change_to_ready();
		t->statistics.total_trans++;
		t->state = ST_RUN;
	}
	else if(dst_queue == &readyqueue){
		if(t->state != ST_RUN){
			list_del(&t->list);	
		}
		t->state = ST_READY;
		
		list_add_tail(&t->list,dst_queue);
	
	//	printk("dep2");
	}
	else{
		t->state = ST_BLOCKED;
		list_add_tail(&t->list,dst_queue);
	}
	//Blocked queue implementation incumming

	t->statistics.elapsed_total_ticks = get_ticks();


}

void sched_next_rr (){
	struct list_head* list_it;
	struct task_struct* task_struct_it;

	if(!list_empty(&readyqueue)){
		list_it = list_first(&readyqueue);
		list_del(list_first(&readyqueue));
		task_struct_it = list_head_to_task_struct(list_it);
	}
	else task_struct_it = idle_task;

	//task_struct_it->state=ST_RUN;
	run_quantum = get_quantum(task_struct_it);
    update_process_state_rr(task_struct_it,NULL);
	task_switch((union task_union*)task_struct_it);
	//printk("Holy change");
}




int get_quantum(struct task_struct *t){
	return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum){
	t->quantum = new_quantum;
}
	

void schedwork(){

	update_sched_data_rr(); 
	if(needs_sched_rr()){
	  update_process_state_rr(current(),&readyqueue);
	  sched_next_rr();
	}
}
