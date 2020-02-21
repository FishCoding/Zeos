#ifndef _SEMA
#define _SEMA

#include <list.h>

struct semaphore{

	int valid ;
	struct task_struct* father;
	int value;
	struct list_head queue;
};
#endif
