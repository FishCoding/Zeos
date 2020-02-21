#include <io.h>
#include <utils.h>
#include <list.h>
#include <sched.h>
#include <circular_buffer.h>
// Queue for blocked processes in I/O 
struct list_head blocked;

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}


int sys_read_keyboard(char *buffer,int size)
{

	struct task_struct* curr = current();
	curr->keybinfo.bytes_to_read = size;
	curr->keybinfo.bytes_read = 0;

	if(!list_empty(&keyboardqueue)){
		//curr->keybinfo.pointer_buffer = 0;
		update_process_state_rr(current(),&keyboardqueue);
		sched_next_rr();
	}
	while(1){

		int remaining = curr->keybinfo.bytes_to_read - curr->keybinfo.bytes_read;
		if(buffer_length(&keyboard_buffer) >= remaining){
		
			int i;
			for(i =  curr->keybinfo.bytes_read ; i <  curr->keybinfo.bytes_read + remaining; i++ ){
				buffer[i] = read_buffer(&keyboard_buffer);
			}
			curr->keybinfo.bytes_read += remaining;
			return curr->keybinfo.bytes_read;
		}
		else if(buffer_full(&keyboard_buffer)){
			int i;
			for(i = curr->keybinfo.bytes_read; i < curr->keybinfo.bytes_read + KEYBOARD_BUFFER_SIZE; i++ ){
				buffer[i] = read_buffer(&keyboard_buffer);
			}		
			curr->keybinfo.bytes_read +=KEYBOARD_BUFFER_SIZE;
		}
		list_add(&curr->list,&keyboardqueue);	
		curr->state = ST_BLOCKED;	
		sched_next_rr();
	}

}
