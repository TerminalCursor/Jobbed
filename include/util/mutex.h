#ifndef UTIL_MUTEX_H
#define UTIL_MUTEX_H
#include <lib/queue.h>

#define NULL_PID 0
#define CORE0_PID 1
#define CORE1_PID 2
#define CORE2_PID 3
#define CORE3_PID 4
#define FIRST_AVAIL_PID CORE3_PID+1

#define MAX_MUTEXS 0x100

// PID field is first so that it can be treated
//  as a lock
struct Mutex {
	unsigned long pid;
	void* addr;
} __attribute__((packed, aligned(4)));

struct MutexManager {
	struct Queue free;
	unsigned long used_mutexes;
};

void mutex_init(void);
void uart_mutexes(void);
struct Mutex* create_mutex(void* addr);
unsigned char delete_mutex(struct Mutex* m);
unsigned char lock_mutex(struct Mutex* m);
void unlock_mutex(struct Mutex* m);

#endif
