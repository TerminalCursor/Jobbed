#ifndef UTIL_MUTEX_H
#define UTIL_MUTEX_H

#define NULL_PID 0
#define SYS_PID 1
#define SCHED_PID 2

struct Mutex {
	void* addr;
	unsigned long pid;
} __attribute__((packed, aligned(4)));;

unsigned char lock_mutex(struct Mutex*, unsigned long);
unsigned char release_mutex(struct Mutex*, unsigned long);
struct Mutex* create_mutex(void* addr);

#endif