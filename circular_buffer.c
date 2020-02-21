#include <circular_buffer.h>

void write_buffer (struct circular_buffer* buffer,char mander){
		if(buffer_full(buffer))return;
		buffer->array[buffer->write_point] = mander;
		if(++buffer->write_point >= KEYBOARD_BUFFER_SIZE)buffer->write_point =0; 
		if(buffer->write_point == buffer->read_point)buffer->fullgym = 1;
		return;
}

char read_buffer(struct circular_buffer* buffer){
	char mander = buffer->array[buffer->read_point];
	if(++buffer->read_point >= KEYBOARD_BUFFER_SIZE)buffer->read_point =0; 
	if(buffer->write_point == buffer->read_point)buffer->fullgym = 0;
	return mander;
}


int buffer_full(struct circular_buffer* buffer){
	return buffer->fullgym;//TT
}

int buffer_length(struct circular_buffer* buffer){
	if(buffer->write_point > buffer->read_point) return buffer->write_point - buffer->read_point ;
	else if(buffer->write_point == buffer->read_point){
		if(buffer->fullgym)return KEYBOARD_BUFFER_SIZE;
		return 0;
	}
	return 	KEYBOARD_BUFFER_SIZE -  buffer->read_point + buffer->write_point; 
}
