/*
 * System.h - Capçalera del mòdul principal del sistema operatiu
 */

#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <types.h>

#define KEYBOARD_BUFFER_SIZE 15

struct circular_buffer{
	char array[KEYBOARD_BUFFER_SIZE];
	int write_point;
	int read_point;
	int fullgym;
};


void write_buffer(struct circular_buffer* buffer,char mander);
char read_buffer(struct circular_buffer* buffer);
int buffer_full(struct circular_buffer* buffer);
int buffer_length(struct circular_buffer* buffer);

#endif  /* __CIRCULAR_BUFFER_H__ */

