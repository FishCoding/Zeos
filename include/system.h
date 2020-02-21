/*
 * system.h - Capçalera del mòdul principal del sistema operatiu
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <types.h>
#include <semaphore.h>



extern TSS         tss;
extern Descriptor* gdt;

extern struct semaphore semaphores[20];

#endif  /* __SYSTEM_H__ */
