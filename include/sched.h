/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>

#include <circular_buffer.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

 
enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct keyboard_info{
  int bytes_to_read;
  int bytes_read;
  int buffer_pointer;
};

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
    
  page_table_entry * dir_pages_baseAddr;
  int dir_pos;
  struct list_head list;
  unsigned int stack_position;
  struct stats statistics;
  struct keyboard_info keybinfo;  
  int quantum;
  enum state_t state;


  int break_pos;
  
};

struct data_page {
	int shared;
  int valid;  
};



union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};
 
extern union task_union protected_tasks[NR_TASKS+2];
extern union task_union *task; /* Vector de tasques */

extern struct task_struct *idle_task;
extern struct task_struct *task_1;

extern int PID_actual;
extern int run_quantum;

extern struct list_head freequeue;
extern struct list_head readyqueue;

extern struct list_head keyboardqueue;

extern struct circular_buffer keyboard_buffer;

extern struct data_page pages_data[1024];

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

void init_freequeue(void);

void init_readyqueue(void);

struct task_struct * current();

void task_switch(union task_union*t);
void inner_task_switch(union task_union*t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

void init_DIR();

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

void schedwork();


int get_quantum(struct task_struct *t);

void set_quantum(struct task_struct *t, int new_quantum);

#endif  /* __SCHED_H__ */
